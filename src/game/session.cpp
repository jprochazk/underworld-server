
#include "game/session.hpp"
#include "game/handler.hpp"
#include "net/packet.hpp"
#include "net/socket.hpp"
#include <atomic>
#include <bits/stdint-uintn.h>
#include <functional>
#include <moodycamel/concurrentqueue.h>
#include <unordered_map>

template<typename T>
using Queue = moodycamel::ConcurrentQueue<T>;

namespace game {

class SessionImpl final : public Session
{
public:
    SessionImpl(uint32_t id, std::weak_ptr<net::Socket> socket)
      : id_{ id }
      , socket_{ socket }
    {}

    bool
    send(const std::vector<uint8_t>& data) override
    {
        if (auto socket = socket_.lock()) {
            socket->send(data);
            return true;
        }
        return false;
    }

    bool
    isOpen() const override
    {
        return socket_.expired();
    }

    uint32_t
    getId() const override
    {
        return id_;
    }

private:
    uint32_t id_;
    std::weak_ptr<net::Socket> socket_;
}; // class SessionImpl

class SessionManagerImpl final : public SessionManager
{
public:
    SessionManagerImpl()
      : queues_()
      , sessions_()
    {}

    void
    onOpen(uint32_t /* id */, std::weak_ptr<net::Socket> socket) override
    {
        ++queues_.openCount;
        queues_.open.enqueue(socket);
    }

    void
    onClose(uint32_t id) override
    {
        ++queues_.closeCount;
        queues_.close.enqueue(id);
    }

    void
    onMessage(uint32_t id, uint8_t* data, size_t size) override
    {
        Context context{ sessions_.find(id)->second };

        if (size < sizeof(uint16_t))
            return;

        net::Packet packet{ data, size };

        uint16_t opcode;
        packet.read(opcode);

        game::Handle(context, (Opcode)opcode, packet);
    }

    void
    onError(uint32_t id, std::string_view what, beast::error_code error) override
    {
        if (error == asio::error::operation_aborted || error == asio::error::connection_aborted ||
            error == beast::websocket::error::closed)
            return;
        util::log::Info("SessionManager", "[Socket#{}] Error: {}, {}", id, what, error.message());
    }

    void
    update() override
    {
        size_t openCount = queues_.openCount;
        queues_.openCount -= openCount;
        if (openCount > 0) {
            std::vector<std::weak_ptr<net::Socket>> opened;
            opened.resize(openCount, std::weak_ptr<net::Socket>{});
            queues_.open.try_dequeue_bulk(opened.begin(), openCount);

            sessions_.reserve(sessions_.size() + openCount);
            for (auto& socket : opened) {
                if (auto acquired = socket.lock()) {
                    sessions_.emplace(acquired->getId(), SessionImpl{ acquired->getId(), socket });
                }
            }
        }

        size_t closeCount = queues_.closeCount;
        queues_.closeCount -= closeCount;
        if (closeCount > 0) {
            std::vector<uint32_t> closed;
            closed.resize(closeCount, 0);
            queues_.close.try_dequeue_bulk(closed.begin(), closeCount);

            for (auto& id : closed) {
                sessions_.erase(id);
            }
        }
    }

    bool
    has(uint32_t id) const override
    {
        auto found = sessions_.find(id);
        return found != sessions_.end();
    }

    bool
    isOpen(uint32_t id) const override
    {
        return has(id) && find(id).isOpen();
    }

    Session&
    find(uint32_t id) override
    {
        return sessions_.find(id)->second;
    }
    const Session&
    find(uint32_t id) const override
    {
        return sessions_.find(id)->second;
    }

private:
    struct
    {
        std::atomic_size_t openCount = 0;
        Queue<std::weak_ptr<net::Socket>> open;
        std::atomic_size_t closeCount = 0;
        Queue<uint32_t> close;
    } queues_;
    std::unordered_map<uint32_t, SessionImpl> sessions_;
}; // class SessionManagerImpl

std::shared_ptr<SessionManager>
CreateSessionManager()
{
    return std::make_shared<SessionManagerImpl>();
}

} // namespace game