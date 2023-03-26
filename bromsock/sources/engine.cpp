#include <bromsock/engine.h>
#include <bromsock/lua/luaSocket.h>
#include <bromsock/lua/luaPacket.h>

#include <GarrysMod/Lua/Interface.h>

using namespace GarrysMod;
using namespace GarrysMod::Lua;

namespace bromsock {
    std::map<lua_State*, std::unique_ptr<Engine>> Engine::states;

    Engine& Engine::createInstance(lua_State* state) {
        states[state] = std::make_unique<Engine>(state);
        return *states[state];
    }

    Engine& Engine::getInstance(lua_State* state) {
        return *states[state];
    }

    std::unique_ptr<Engine> Engine::removeInstance(lua_State* state) {
        auto e = std::move(states[state]);
        states.erase(states.find(state));

        return e;
    }

    Engine::Engine(lua_State* state) : state(state) {

    }

	bool Engine::hasSourceEngineTicketYet() const {
		return sourceEngineInitialized;
	}

    void Engine::init() {
        LUA->PushSpecial(GarrysMod::Lua::SPECIAL_GLOB);
			LUA->GetField(-1, "hook");
			LUA->GetField(-1, "Add");
			LUA->PushString("Tick");
			LUA->PushString("CPP_BROMSOCK::Think");
			LUA->PushCFunction(Engine::luaThink);
			LUA->Call(3, 0);
		LUA->Pop();

		LUA->PushSpecial(GarrysMod::Lua::SPECIAL_GLOB);
			LUA->GetField(-1, "hook");
			LUA->GetField(-1, "Add");
			LUA->PushString("Shutdown");
			LUA->PushString("CPP_BROMSOCK::Shutdown");
			LUA->PushCFunction(Engine::luaShutdown);
			LUA->Call(3, 0);
		LUA->Pop();

		registerTypes(state);
    }

    void Engine::shutdown() {
		// delete all sockets
		sockets.clear();

		// free references
		LUA->ReferenceFree(socketRef);
		LUA->ReferenceFree(packetRef);
    }

    void Engine::registerTypes(lua_State* state) {
		lua::SOCK_RegisterTypes(state);
		lua::PACK_RegisterTypes(state);
    }

    void Engine::addSocket(std::unique_ptr<Socket> socket) {
        sockets.push_back(std::move(socket));
    }

    void Engine::removeSocket(Socket* socket) {
        for (size_t i = 0; i < sockets.size(); i++) {
            if (sockets[i].get() == socket) {
                sockets.erase(sockets.begin() + i);
                break;
            }
        }
    }

    LUA_FUNCTION(Engine::luaThink) {
		auto& engine = Engine::getInstance(state);

		engine.sourceEngineInitialized = true;

		// copy all sockets into a temp buffer
		// incase we remove / add ones inside of the callbacks
		std::vector<Socket*> ptrCopy;
		for (auto& sock : engine.sockets) ptrCopy.push_back(sock.get());

		// execute pending events
		for (auto sock : ptrCopy) {
			while(sock->hasEvent()) {
				auto&& event = sock->getNextEvent();
				if (!sock->hasCallback(event->getName())) continue;
				auto callback = sock->getCallback(event->getName());

				LUA->ReferencePush(callback);
				sock->pushToStack(state);

				auto argumentCount = event->pushArguments(state);
				CALLLUAFUNC(argumentCount + 1);
			}
		}

        return 0;
    }

    LUA_FUNCTION(Engine::luaShutdown) {
		auto& engine = Engine::getInstance(state);

		// shutdown all sockets
        for (size_t i = 0; i < engine.sockets.size(); i++) engine.sockets[i]->beginShutdown();

		// call shutdown events if registered
		luaThink(state);

        return 0;
    }
}
