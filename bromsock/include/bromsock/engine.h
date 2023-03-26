#pragma once
#include <GarrysMod/Lua/Interface.h>
#include <bromsock/defines.h>
#include <bromsock/socket.h>

#include <map>
#include <memory>
#include <vector>

namespace bromsock {
    class Engine {
        static std::map<lua_State*, std::unique_ptr<Engine>> states;
        lua_State* state = nullptr;
        std::vector<std::unique_ptr<Socket>> sockets;
        bool sourceEngineInitialized = false;

    public:
		int packetRef = -1;
		int socketRef = -1;

        static Engine& createInstance(lua_State* state);
        static Engine& getInstance(lua_State* state);
        static std::unique_ptr<Engine> removeInstance(lua_State* state);

        Engine(lua_State* state);

        void init();
        void shutdown();
        bool hasSourceEngineTicketYet() const;

        void addSocket(std::unique_ptr<Socket> socket);
        void removeSocket(Socket* socket);
        void registerTypes(lua_State* state);

        static LUA_FUNCTION(luaThink);
        static LUA_FUNCTION(luaShutdown);
    };
}
