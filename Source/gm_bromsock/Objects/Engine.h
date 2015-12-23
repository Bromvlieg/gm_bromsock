#ifndef __H_GMBSOCK_O_ENGINE
#define __H_GMBSOCK_O_ENGINE

#include "../Defines.h"
#include "../Gmod_Headers/Lua/Interface.h"
#include <vector>

namespace GMBSOCK {
	class SockWrapper;
	class LockObject;

	class Engine {
	public:
		static Engine* GetEngineByState(lua_State* luaState);
		static std::vector<lua_State*> S_States;
		static std::vector<Engine*> S_Engines;
		static LockObject* S_Lock;

		std::vector<SockWrapper*> Sockets;
		int PacketRef;
		int SocketRef;
		bool IntialTickHappend;

		Engine(void);
		~Engine(void);

		void RegisterTypes(lua_State* state);

		void Init(lua_State* luaState);
		void Shutdown(lua_State* luaState);

		static GMOD_FUNCTION(Think);
	};
}

#endif