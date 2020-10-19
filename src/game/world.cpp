#include "game/world.hpp"
#include "game/handler.hpp"
#include "game/player.hpp"
#include "game/script.hpp"
#include "net/net.hpp"
#include "util/util.hpp"
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

        uint16_t id_;

        struct
        {
            std::atomic_size_t copen;
            moodycamel::ConcurrentQueue<std::weak_ptr<net::Socket>> open;
            std::atomic_size_t cclose;
            moodycamel::ConcurrentQueue<uint32_t> close;
            std::atomic_size_t cmessage;
            moodycamel::ConcurrentQueue<std::pair<uint32_t, net::Packet>> message;
        } queues;

        SocketEventHandler(uint16_t id)
          : id_{ id }
          , queues()
        {}

        uint16_t
        id() const override
        {
            return id_;
        }

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

    WorldImpl(uint16_t id)
      : id_{ id }
      , msgHandler_{ std::make_shared<SocketEventHandler>(id) }
      , registry_{}
      , players_{}
      , updateInterval_{ 1000. / static_cast<double>(util::Config::get().updateRate) }
      , stop_{ false }
      , script_{}
    {}

    uint16_t
    id() const override
    {
        return id_;
    }

    void
    run() override
    {
        auto last = util::time::Now();
        while (!stop_) {
            // 1 tick = 1000 ms
            auto now = util::time::Now();
            auto diff = (now - last).count();
            if (diff < updateInterval_)
                continue;

            last = now;

            update();
        }
    }

    void
    stop() override
    {
        stop_ = true;
    }

    void
    update() override
    {
        // each connection is a new player
        for (auto& connection : msgHandler_->getConnections()) {
            if (auto socket = connection.lock()) {
                // TODO: query player info from DB at some point
                players_.emplace(socket->getId(), Player::create(registry_, connection));
            }
        }

        for (auto& [id, packet] : msgHandler_->getMessages()) {
            // check for minimum packet size
            if (packet.size() < sizeof(Opcode))
                continue;

            if (auto it = players_.find(id); it != players_.end()) {
                auto& [socketId, player] = *it;

                uint16_t opcode;
                packet.read(opcode);

                Context context{ player, script_ };
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
    uint16_t id_;
    std::shared_ptr<SocketEventHandler> msgHandler_;
    entt::registry registry_;
    std::unordered_map<uint32_t, Player> players_;
    double updateInterval_;
    std::atomic_bool stop_;
    script::Context script_;
}; // class WorldImpl

std::shared_ptr<World>
CreateWorld(uint16_t id)
{
    return std::make_shared<WorldImpl>(id);
}

class WorldManagerImpl : public WorldManager
{
public:
    WorldManagerImpl(size_t size)
      : worlds_{}
      , threads_{}
    {
        worlds_.reserve(size);
        for (size_t i = 0; i < size; ++i) {
            worlds_.emplace(i, std::static_pointer_cast<WorldImpl>(CreateWorld(i)));
        }
    }

    void
    start() override
    {
        // TODO: start/stop worlds based on some load-balancing algorithm
        for (auto& [id, world] : worlds_) {
            threads_.emplace(id, [this, id = id] { worlds_[id]->run(); });
        }
    }

    void
    stop() override
    {
        for (auto& [id, world] : worlds_) {
            world->stop();
        }
    }

    std::shared_ptr<World>
    get(uint16_t id) override
    {
        if (auto it = worlds_.find(id); it != worlds_.end()) {
            return it->second;
        } else {
            return {};
        }
    }

    std::shared_ptr<net::Handler>
    select() override
    {
        size_t lowest_population = static_cast<size_t>(-1);
        uint16_t lowest_population_id = static_cast<uint16_t>(-1);
        for (auto& [id, world] : worlds_) {
            auto population = world->size();
            if (population < lowest_population) {
                lowest_population = population;
                lowest_population_id = id;
            }
        }
        return select(lowest_population_id);
    }

    std::shared_ptr<net::Handler>
    select(uint16_t id) override
    {
        if (auto it = worlds_.find(id); it != worlds_.end()) {
            return it->second->getHandler();
        } else {
            return {};
        }
    }

private:
    // map (world id => world)
    std::unordered_map<uint16_t, std::shared_ptr<WorldImpl>> worlds_;
    // map (world id => thread)
    std::unordered_map<uint16_t, util::ScopedThread> threads_;
}; // class WorldManager

std::shared_ptr<WorldManager>
CreateWorldManager(size_t size)
{
    return std::make_shared<WorldManagerImpl>(size);
}

} // namespace game