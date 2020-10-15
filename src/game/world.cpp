#include "game/world.hpp"
#include "entt/entity/fwd.hpp"
#include "game/handler.hpp"
#include "game/player.hpp"
#include "net/packet.hpp"
#include "net/socket.hpp"
#include "util/log.hpp"
#include <atomic>
#include <entt/entt.hpp>
#include <moodycamel/concurrentqueue.h>
#include <unordered_map>
#include <vector>

namespace game {

// Helper fn to dequeue from a queue into a vector
template<typename T>
std::vector<T>
dequeue_from(moodycamel::ConcurrentQueue<T>& queue, std::size_t count)
{
    std::vector<T> out;
    out.resize(count, T{});
    queue.try_dequeue_bulk(out.begin(), count);
    return out;
}

class WorldImpl final : public World
{
public:
    // Uses concurrent queues to store network events
    struct SocketEventHandler final : public net::Handler
    {
        friend class WorldImpl;

        struct
        {
            std::atomic_size_t copen;
            moodycamel::ConcurrentQueue<std::weak_ptr<net::Socket>> open;
            std::atomic_size_t cclose;
            moodycamel::ConcurrentQueue<uint32_t> close;
            std::atomic_size_t cmessage;
            moodycamel::ConcurrentQueue<std::pair<uint32_t, net::Packet>> message;
        } queues;

        void
        onOpen(uint32_t /* id */, std::weak_ptr<net::Socket> socket) override
        {
            queues.open.enqueue(socket);
            queues.copen++;
        }
        void
        onClose(uint32_t id) override
        {
            queues.close.enqueue(id);
            queues.cclose++;
        }
        void
        onMessage(uint32_t id, uint8_t* data, size_t size) override
        {
            queues.message.enqueue(std::make_pair(id, net::Packet{ data, size }));
            queues.cmessage++;
        }
        void
        onError(uint32_t id, std::string_view what, beast::error_code error) override
        {
            if (error == asio::error::operation_aborted || error == asio::error::connection_aborted ||
                error == beast::websocket::error::closed)
                return;
            util::log::Error("WorldImpl::MesageHandler", "Socket#{} error: {}, {}", id, what, error.message());
        }

        inline std::vector<std::weak_ptr<net::Socket>>
        getConnections()
        {
            return dequeue_from(queues.open, queues.copen);
        }

        inline std::vector<uint32_t>
        getDisconnections()
        {
            return dequeue_from(queues.close, queues.cclose);
        }

        inline std::vector<std::pair<uint32_t, net::Packet>>
        getMessages()
        {
            return dequeue_from(queues.message, queues.cmessage);
        }
    };

    WorldImpl()
      : msgHandler_{ std::make_shared<SocketEventHandler>() }
      , registry_{}
      , players_{}
    {}

    void
    update() override
    {
        for (auto& connection : msgHandler_->getConnections()) {
            if (auto socket = connection.lock()) {
                players_.emplace(socket->getId(), Player::create(registry_, connection));
            }
        }

        for (auto& [id, packet] : msgHandler_->getMessages()) {
            // minimum size is sizeof(Opcode), which is uint16_t
            if (packet.size() < sizeof(Opcode))
                continue;

            if (auto it = players_.find(id); it != players_.end()) {
                auto& [socketId, player] = *it;

                uint16_t opcode;
                packet.read(opcode);

                Context context{ player };
                game::Handle(context, (Opcode)opcode, packet);
            }
        }

        for (auto& id : msgHandler_->getDisconnections()) {
            if (auto it = players_.find(id); it != players_.end()) {
                auto& [socketId, player] = *it;
                player.destroy();
            }
            players_.erase(id);
        }
    }

    // Thread safe, call from anywhere
    std::shared_ptr<net::Handler>
    getHandler() override
    {
        return msgHandler_;
    }

    size_t
    size() const override
    {
        return players_.size();
    }

private:
    std::shared_ptr<SocketEventHandler> msgHandler_;
    entt::registry registry_;
    std::unordered_map<uint32_t, Player> players_;
};

std::shared_ptr<World>
CreateWorld()
{
    return std::make_shared<WorldImpl>();
}

} // namespace game