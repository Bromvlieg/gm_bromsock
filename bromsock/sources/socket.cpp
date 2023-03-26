#include <bromsock/socket.h>
#include <bromsock/defines.h>
#include <bromsock/engine.h>
#include <bromsock/lua/event/eventShutdown.h>

#include <GarrysMod/Lua/Interface.h>

namespace bromsock {
    Socket::Socket(lua_State* state, int type, bool ipv6) : state(state), type(type), ipv6(ipv6), sock(ipv6), eventsQueue(32), eventsFinished(32) {
        workers.push_back(new std::thread([this](){ workerLoop(); }));
        workers.push_back(new std::thread([this](){ workerLoop(); }));
    }

    Socket::~Socket() {
        beginShutdown();

        // clean callback references
        for (auto pair : callbacks) {
            LUA->ReferenceFree(pair.second);
        }
    }

    bool Socket::hasCallback(const std::string& name) const {
        return callbacks.find(name) != callbacks.end();
    }

    int Socket::getCallback(const std::string& name) const {
        return callbacks.at(name);
    }

    void Socket::setCallback(const std::string& name, int callback) {
        callbacks[name] = callback;
    }

    void Socket::beginShutdown() {
        shutdown = true;
        sock.close();

        for (auto& w : workers) addEvent(std::make_unique<event::EventShutdown>());
        for (auto& w : workers) if (w->joinable()) w->join();
        workers.clear();
    }

    void Socket::workerLoop() {
        auto& engine = Engine::getInstance(state);

        while(!shutdown) {
            // don't process things if SourceEngine didn't tick yet
            // lua "think" won't be called, so everything will time out if we accept anything
            if (!engine.hasSourceEngineTicketYet()) std::this_thread::sleep_for(std::chrono::milliseconds(100));

            // get next even from the queue
            auto&& event = eventsQueue.pop();
            if (shutdown) break;

            // handle the internal logic of a connect, read, write, etc.
            event->process(this);

            // push the result back into the queue that the engine processes
            // at lua "think"
            eventsFinished.push(std::move(event));
        }
    }

    void Socket::pushToStack(lua_State* state) {
        if (luaReference != 0) {
            LUA->PushUserdata(luaReference);
            return;
        }

        GarrysMod::Lua::UserData* ud = (GarrysMod::Lua::UserData*)LUA->NewUserdata(sizeof(GarrysMod::Lua::UserData));
		ud->data = this;
		ud->type = UD_TYPE_SOCKET;

		LUA->ReferencePush(Engine::getInstance(state).socketRef);
		LUA->SetMetaTable(-2);
        luaReference = LUA->GetUserdata();
    }

    void Socket::addEvent(std::unique_ptr<event::EventBase> event) {
        eventsQueue.push(std::move(event));
    }

    bool Socket::hasEvent() {
        return eventsFinished.available();
    }

    std::unique_ptr<event::EventBase> Socket::getNextEvent() {
        return std::move(eventsFinished.pop());
    }
}
