#ifndef __H_GMBSOCK_O_SOCKWRAPPER
#define __H_GMBSOCK_O_SOCKWRAPPER

#include "../Gmod_Headers/Lua/Interface.h"
#include "LockObject.h"
#include <vector>

#ifdef _MSC_VER
#include <Windows.h>
#endif

#include <openssl/ssl.h>

namespace GMBSOCK {
	class EzSock;

	enum class EventType {
		NONE, Connect, Send, Receive, Accept, SendTo, ReceiveFrom
	};

#ifdef _MSC_VER
	DWORD WINAPI SockWorkerFunction(void* obj);
#else
	void* SockWorkerFunction(void *obj);
#endif

	class SockEvent {
	public:
		EventType Type;
		void* data1;
		void* data2;
		void* data3;
		void* data4;

		SockEvent() :data1(nullptr), data2(nullptr), data3(nullptr), data4(nullptr), Type(EventType::NONE) {}
	};

	class SockWrapper {
	public:
		EzSock* Sock;
		lua_State* state;

		SSL_CTX* sslCtx;
		SSL* ssl;

		int Callback_Receive;
		int Callback_ReceiveFrom;
		int Callback_Send;
		int Callback_SendTo;
		int Callback_Connect;
		int Callback_Disconnect;
		int Callback_Accept;
		int CurrentWorkers;
		int RefCount;
		bool DestoryWorkers;
		bool DidDisconnectCallback;

		int SocketType;

		std::vector<SockEvent*> Todo;
		std::vector<SockEvent*> Callbacks;

#ifdef _MSC_VER
		std::vector<HANDLE> Threadhandles;
#else
		std::vector<pthread_t> Threadhandles;
#endif

		LockObject Mutex;

		SockWrapper(lua_State* luaState, int socketType = IPPROTO_TCP);
		void PushToStack(lua_State* luaState);
		void CreateWorkers();
		void Reset();
		void CallDisconnect();
		void KillWorkers();
		~SockWrapper();
	};
}

#endif