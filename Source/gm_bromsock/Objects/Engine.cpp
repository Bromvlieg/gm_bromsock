#include "Engine.h"

#include "../Gmod_Headers/Lua/Interface.h"
#include "BSEzSock.h"
#include "BSPacket.h"
#include "LockObject.h"
#include "SockWrapper.h"

#include "../LuaWrappers/BromSock.h"
#include "../LuaWrappers/BromPacket.h"

#ifdef _MSC_VER
#include <Windows.h>
#else
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#endif

namespace GMBSOCK {
	std::vector<lua_State*> Engine::S_States;
	std::vector<Engine*> Engine::S_Engines;
	LockObject* Engine::S_Lock = nullptr;

	Engine::Engine() {
		this->IntialTickHappend = false;

		this->PacketRef = -1;
		this->SocketRef = -1;
	}

	Engine::~Engine() {
	}

	void Engine::RegisterTypes(lua_State* state) {
		// I really dislike having to register things like this, but I'm too lazy to write a objective wrapper for lua tables
		LUA->CreateTable();
			ADDFUNC("SetBlocking", SOCK_SetBlocking);
			ADDFUNC("Connect", SOCK_Connect);
			ADDFUNC("StartSSLClient", SOCK_StartSSLClient);
			ADDFUNC("Close", SOCK_Disconnect);
			ADDFUNC("Create", SOCK_Create);
			ADDFUNC("Disconnect", SOCK_Disconnect);
			ADDFUNC("Bind", SOCK_Bind);
			ADDFUNC("Listen", SOCK_Listen);
			ADDFUNC("Send", SOCK_Send);
			ADDFUNC("SendTo", SOCK_SendTo);
			ADDFUNC("Accept", SOCK_Accept);
			ADDFUNC("Receive", SOCK_Receive);
			ADDFUNC("ReceiveFrom", SOCK_ReceiveFrom);
			ADDFUNC("ReceiveUntil", SOCK_ReceiveUntil);
			ADDFUNC("GetIP", SOCK_GetIP);
			ADDFUNC("GetPort", SOCK_GetPort);
			ADDFUNC("GetState", SOCK_GetState);
			ADDFUNC("AddWorker", SOCK_AddWorker);
			ADDFUNC("SetTimeout", SOCK_SetTimeout);
			ADDFUNC("SetOption", SOCK_SetOption);
			ADDFUNC("SetMaxReceiveSize", SOCK_SetMaxReceiveSize);
			ADDFUNC("SetCallbackReceive", SOCK_CALLBACKReceive);
			ADDFUNC("SetCallbackReceiveFrom", SOCK_CALLBACKReceiveFrom);
			ADDFUNC("SetCallbackSend", SOCK_CALLBACKSend);
			ADDFUNC("SetCallbackSendTo", SOCK_CALLBACKSendTo);
			ADDFUNC("SetCallbackConnect", SOCK_CALLBACKConnect);
			ADDFUNC("SetCallbackAccept", SOCK_CALLBACKAccept);
			ADDFUNC("SetCallbackDisconnect", SOCK_CALLBACKDisconnect);
		int socktableref = LUA->ReferenceCreate();

		LUA->CreateTable();
			ADDFUNC("SetEndian", PACK_SetEndian);
			ADDFUNC("WriteByte", PACK_WRITEByte);
			ADDFUNC("WriteSByte", PACK_WRITESByte);
			ADDFUNC("WriteShort", PACK_WRITEShort);
			ADDFUNC("WriteFloat", PACK_WRITEFloat);
			ADDFUNC("WriteInt", PACK_WRITEInt);
			ADDFUNC("WriteDouble", PACK_WRITEDouble);
			ADDFUNC("WriteLong", PACK_WRITELong);
			ADDFUNC("WriteUShort", PACK_WRITEUShort);
			ADDFUNC("WriteUInt", PACK_WRITEUInt);
			ADDFUNC("WriteULong", PACK_WRITEULong);
			ADDFUNC("WriteString", PACK_WRITEString);
			ADDFUNC("WriteStringNT", PACK_WRITEStringNT);
			ADDFUNC("WriteStringRaw", PACK_WRITEStringRaw);
			ADDFUNC("WriteLine", PACK_WRITELine);
			ADDFUNC("WritePacket", PACK_WRITEPacket);
			ADDFUNC("ReadByte", PACK_READByte);
			ADDFUNC("ReadSByte", PACK_READSByte);
			ADDFUNC("ReadShort", PACK_READShort);
			ADDFUNC("ReadFloat", PACK_READFloat);
			ADDFUNC("ReadInt", PACK_READInt);
			ADDFUNC("ReadDouble", PACK_READDouble);
			ADDFUNC("ReadLong", PACK_READLong);
			ADDFUNC("ReadUShort", PACK_READUShort);
			ADDFUNC("ReadUInt", PACK_READUInt);
			ADDFUNC("ReadULong", PACK_READULong);
			ADDFUNC("ReadString", PACK_READString);
			ADDFUNC("ReadStringNT", PACK_READStringNT);
			ADDFUNC("ReadStringAll", PACK_READStringAll);
			ADDFUNC("ReadLine", PACK_READLine);
			ADDFUNC("ReadUntil", PACK_READUntil);
			ADDFUNC("InSize", PACK_InSize);
			ADDFUNC("InPos", PACK_InPos);
			ADDFUNC("OutSize", PACK_OutSize);
			ADDFUNC("OutPos", PACK_OutPos);
			ADDFUNC("Copy", PACK_Copy);
			ADDFUNC("Clear", PACK_Clear);
		int packtableref = LUA->ReferenceCreate();

		LUA->PushSpecial(GarrysMod::Lua::SPECIAL_GLOB);
			LUA->PushString("BromSock");
			LUA->PushCFunction(CreateSocket);
		LUA->SetTable(-3);

		LUA->PushSpecial(GarrysMod::Lua::SPECIAL_GLOB);
			LUA->PushString("BromPacket");
			LUA->PushCFunction(CreatePacket);
		LUA->SetTable(-3);

		LUA->PushSpecial(GarrysMod::Lua::SPECIAL_GLOB);
			LUA->PushString("BROMSOCK_TCP");
			LUA->PushNumber(IPPROTO_TCP);
		LUA->SetTable(-3);

		LUA->PushSpecial(GarrysMod::Lua::SPECIAL_GLOB);
			LUA->PushString("BROMSOCK_UDP");
			LUA->PushNumber(IPPROTO_UDP);
		LUA->SetTable(-3);

		LUA->PushSpecial(GarrysMod::Lua::SPECIAL_GLOB);
			LUA->PushString("BROMSOCK_ENDIAN_SYSTEM");
			LUA->PushNumber(0);
		LUA->SetTable(-3);

		LUA->PushSpecial(GarrysMod::Lua::SPECIAL_GLOB);
			LUA->PushString("BROMSOCK_ENDIAN_BIG");
			LUA->PushNumber(1);
		LUA->SetTable(-3);

		LUA->PushSpecial(GarrysMod::Lua::SPECIAL_GLOB);
			LUA->PushString("BROMSOCK_ENDIAN_LITTLE");
			LUA->PushNumber(2);
		LUA->SetTable(-3);

		LUA->CreateTable();
			LUA->ReferencePush(packtableref);
			LUA->SetField(-2, "__index");
			LUA->ReferencePush(packtableref);
			LUA->SetField(-2, "__newindex");
			ADDFUNC("__tostring", PACK__TOSTRING);
			ADDFUNC("__eq", PACK__EQ);
			ADDFUNC("__gc", PACK__GC);
		this->PacketRef = LUA->ReferenceCreate();

		LUA->CreateTable();
			LUA->ReferencePush(socktableref);
			LUA->SetField(-2, "__index");
			LUA->ReferencePush(socktableref);
			LUA->SetField(-2, "__newindex");
			ADDFUNC("__tostring", SOCK__TOSTRING);
			ADDFUNC("__eq", SOCK__EQ);
			ADDFUNC("__gc", SOCK__GC);
		this->SocketRef = LUA->ReferenceCreate();

		// clean up references
		LUA->ReferenceFree(socktableref);
		LUA->ReferenceFree(packtableref);
	}

	GMOD_FUNCTION(ShutdownHook) {
		DEBUGPRINTFUNC;

		// reset all sockets for the GC, think hook won't get called anymore anyway so no chance of networking stuff

		Engine* e = Engine::GetEngineByState(state);
		for (unsigned i = 0; i < e->Sockets.size(); i++) {
			e->Sockets[i]->Reset();
		}

		e->Think(state);

		return 0;
	}

	void Engine::Init(lua_State* state) {
		Engine::S_Lock->Lock();
		this->S_Engines.push_back(this);
		this->S_States.push_back(state);
		Engine::S_Lock->Unlock();

		this->IntialTickHappend = false;

		this->RegisterTypes(state);

		// add our hooks to the GMod engine
		LUA->PushSpecial(GarrysMod::Lua::SPECIAL_GLOB);
			LUA->GetField(-1, "hook");
			LUA->GetField(-1, "Add");
			LUA->PushString("Tick");
			LUA->PushString("CPP_BROMSOCK::Think");
			LUA->PushCFunction(Engine::Think);
			LUA->Call(3, 0);
		LUA->Pop();

		LUA->PushSpecial(GarrysMod::Lua::SPECIAL_GLOB);
			LUA->GetField(-1, "hook");
			LUA->GetField(-1, "Add");
			LUA->PushString("ShutDown");
			LUA->PushString("CPP_BROMSOCK::ShutDown");
			LUA->PushCFunction(ShutdownHook);
			LUA->Call(3, 0);
		LUA->Pop();
	}

	void Engine::Shutdown(lua_State* state) {
		LUA->ReferenceFree(this->SocketRef);
		LUA->ReferenceFree(this->PacketRef);

		Engine::S_Lock->Lock();
		for (unsigned i = 0; i < Engine::S_Engines.size(); i++) {
			if (Engine::S_Engines[i] == this) {
				Engine::S_Engines.erase(Engine::S_Engines.begin() + i);
				break;
			}
		}
		Engine::S_Lock->Unlock();
	}

	Engine* Engine::GetEngineByState(lua_State* luaState) {
		Engine::S_Lock->Lock();
		for (size_t i = 0; i < Engine::S_States.size(); i++) {
			if (Engine::S_States[i] == luaState) {
				Engine* e = Engine::S_Engines[i];
				
				Engine::S_Lock->Unlock();
				return e;
			}
		}

		Engine::S_Lock->Unlock();
		return nullptr;
	}


	GMOD_FUNCTION(Engine::Think) {
		// too much spam :v
		// DEBUGPRINTFUNC;

		// When a player joins for the first time, the server gets "activated", only after that, the tick hook get's called. so we should wait with all worker operations or the callbacks won't get called
		// and it'll mess up. This is a workaround. You can also do this in lua, but putting it here would be easier for people to use.


		Engine* engine = Engine::GetEngineByState(state);
		engine->IntialTickHappend = true;

		for (size_t i = 0; i < engine->Sockets.size(); i++) {
			SockWrapper* sw = engine->Sockets[i];

			while (true) {
				SockEvent* se = nullptr;
				sw->Mutex.Lock();
				if (sw->Callbacks.size() > 0) {
					se = sw->Callbacks[0];
					sw->Callbacks.erase(sw->Callbacks.begin());
				}
				sw->Mutex.Unlock();

				if (se == nullptr)
					break;

				switch (se->Type) {
					case EventType::Connect:{
						bool* ret = (bool*)se->data1;
						char* ip = (char*)se->data2;
						unsigned short* port = (unsigned short*)se->data3;

						if (*ret) sw->DidDisconnectCallback = false;

						LUA->ReferencePush(sw->Callback_Connect);
						sw->PushToStack(state);
						LUA->PushBool(*ret);
						LUA->PushString(ip);
						LUA->PushNumber((double)*port);
						CALLLUAFUNC(4);

						delete ret;
						delete[] ip;
						delete port;
					}break;

					case EventType::Accept:{
						SockWrapper* nsw = (SockWrapper*)se->data1;

						if (nsw != nullptr) {
							nsw->CreateWorkers();

							engine->Sockets.push_back(nsw);

							LUA->ReferencePush(sw->Callback_Accept);
							sw->PushToStack(state);
							nsw->PushToStack(state);
							CALLLUAFUNC(2);
						} else {
							sw->CallDisconnect();
						}
					}break;

					case EventType::Send:{
						bool* valid = (bool*)se->data1;
						int* size = (int*)se->data2;

						if (*valid) {
							LUA->ReferencePush(sw->Callback_Send);
							sw->PushToStack(state);
							LUA->PushNumber((double)*size);
							CALLLUAFUNC(2);
						} else {
							sw->CallDisconnect();
						}

						delete valid;
						delete size;
					}break;

					case EventType::SendTo:{
						bool* valid = (bool*)se->data1;
						int* size = (int*)se->data2;
						char* ip = (char*)se->data3;
						int* port = (int*)se->data4;

						if (sw->Callback_SendTo != -1) {
							LUA->ReferencePush(sw->Callback_SendTo);
							sw->PushToStack(state);
							LUA->PushNumber((double)*size);
							LUA->PushString(ip);
							LUA->PushNumber((double)*port);
							CALLLUAFUNC(4);
						}

						delete valid;
						delete size;
						delete port;
						delete[] ip;
					}break;

					case EventType::Receive:{
						Packet* p = (Packet*)se->data1;

						if (p != nullptr) {
							LUA->ReferencePush(sw->Callback_Receive);
							sw->PushToStack(state);

							GarrysMod::Lua::UserData* ud = (GarrysMod::Lua::UserData*)LUA->NewUserdata(sizeof(GarrysMod::Lua::UserData));
							ud->data = p;
							ud->type = UD_TYPE_PACKET;
							LUA->ReferencePush(engine->PacketRef);
							LUA->SetMetaTable(-2);
							p->RefCount++;

							CALLLUAFUNC(2);
						} else {
							sw->CallDisconnect();

							if (se->data2 != nullptr) {
								char buff[256];
								sprintf(buff, "error in bromsock_C++ allocating buffer to receive %d bytes from %s:%d", *(int*)se->data2, inet_ntoa(sw->Sock->addr.sin_addr), ntohs(sw->Sock->addr.sin_port));

								delete (int*)se->data2;
								LUA->ThrowError(buff);
							}
						}
					}break;

					case EventType::ReceiveFrom:{
						Packet* p = (Packet*)se->data1;
						sockaddr_in* caddr = (sockaddr_in*)se->data2;

						if (p != nullptr) {
							LUA->ReferencePush(sw->Callback_ReceiveFrom);
							sw->PushToStack(state);

							GarrysMod::Lua::UserData* ud = (GarrysMod::Lua::UserData*)LUA->NewUserdata(sizeof(GarrysMod::Lua::UserData));
							ud->data = p;
							ud->type = UD_TYPE_PACKET;
							LUA->ReferencePush(engine->PacketRef);
							LUA->SetMetaTable(-2);
							p->RefCount++;

							LUA->PushString(inet_ntoa(caddr->sin_addr));
							LUA->PushNumber(ntohs(caddr->sin_port));
							CALLLUAFUNC(4);
						} else {
							LUA->ReferencePush(sw->Callback_ReceiveFrom);
							sw->PushToStack(state);

							LUA->PushNil();
							LUA->PushString(inet_ntoa(caddr->sin_addr));
							LUA->PushNumber(ntohs(caddr->sin_port));
							CALLLUAFUNC(4);
						}

						delete caddr;
					}break;
				}

				delete se;
			}
		}

		return 0;
	}
}