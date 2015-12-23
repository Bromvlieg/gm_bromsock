#include "BSEzSock.h"
#include "BSPacket.h"

#include "../Defines.h"
#include "SockWrapper.h"

#include "../Gmod_Headers/Lua/Interface.h"
#include "Engine.h"
#include "LockObject.h"

namespace GMBSOCK {
#ifdef _MSC_VER
	DWORD WINAPI SockWorkerFunction(void* obj) {
#else
	void* SockWorkerFunction(void *obj) {
#endif
		SockWrapper* sock = (SockWrapper*)obj;
		sock->Mutex.Lock();
		sock->CurrentWorkers++;
		sock->Mutex.Unlock();

		Engine* engine = Engine::GetEngineByState(sock->state);
		while (true) {
			if (sock->DestoryWorkers) {
				sock->Mutex.Lock();
				sock->CurrentWorkers--;
				sock->Mutex.Unlock();
				return 0;
			}

			if (!engine->IntialTickHappend) {
#ifdef _MSC_VER
				Sleep(1);
#else
				struct timespec ts;
				ts.tv_sec = 0;
				ts.tv_nsec = 1000000;
				nanosleep(&ts, NULL);
#endif

				continue;
			}

			SockEvent* cur = nullptr;

			sock->Mutex.Lock();
			if (sock->Todo.size() > 0) {
				cur = sock->Todo[0];
				sock->Todo.erase(sock->Todo.begin());
			}
			sock->Mutex.Unlock();

			if (cur == nullptr) {
#ifdef _MSC_VER
				Sleep(1);
#else
				struct timespec ts;
				ts.tv_sec = 0;
				ts.tv_nsec = 1000000;
				nanosleep(&ts, NULL);
#endif
				continue;
			}

			SockEvent* ne = new SockEvent();

			switch (cur->Type) {
				case EventType::Accept:{
					SockWrapper* nsock = new SockWrapper(sock->state);
					if (!sock->Sock->accept(nsock->Sock)) {
						delete nsock;

						ne->data1 = nullptr;
					} else {
						ne->data1 = nsock;
					}

					ne->Type = EventType::Accept;
				}break;

				case EventType::Connect:{
					char* ip = (char*)cur->data1;
					unsigned short* port = (unsigned short*)cur->data2;

					bool ret = sock->Sock->connect(ip, *port) == 0;

					ne->Type = EventType::Connect;
					ne->data1 = new bool(ret);
					ne->data2 = ip;
					ne->data3 = port;
				}break;

				case EventType::Receive:{
					int* len = (int*)cur->data1;
					char* seq = (char*)cur->data2;
					Packet* p = new Packet(sock->Sock);

					if (cur->data1 == nullptr) {
						if (p->CanRead(seq, sock->ssl)) {
							ne->data1 = p;
						} else {
							delete p;
						}
					} else {
						if ((*len == -1 && (*len = p->ReadInt()) == 0) || !p->CanRead(*len, sock->ssl)) {
							delete p;
						} else {
							ne->data1 = p;
						}
					}

					ne->Type = EventType::Receive;

					if (cur->data1 != nullptr) delete (int*)cur->data1;
					if (cur->data2 != nullptr) delete[](char*)cur->data1;
				}break;

				case EventType::ReceiveFrom:{
					int* len = (int*)cur->data1;
					Packet* p = new Packet(sock->Sock);

					unsigned char* buffer = new unsigned char[*len];
					sockaddr_in* clientaddr = new sockaddr_in;

					if (cur->data2 != nullptr) {
						struct hostent* lookup = gethostbyname((char*)cur->data2);
						if (lookup != nullptr) {
							memset((char *)&clientaddr, 0, sizeof(clientaddr));
							clientaddr->sin_family = AF_INET;
							memcpy(&clientaddr->sin_addr, lookup->h_addr_list[0], lookup->h_length);
							clientaddr->sin_port = htons(*(int*)cur->data3);
						}

						delete[](char*)cur->data2;
						delete (int*)cur->data3;
					}

					int recdata = sock->Sock->ReceiveUDP(buffer, *len, clientaddr);

					if (recdata == -1) {
						delete p;
					} else {
						p->InBuffer = new unsigned char[recdata];
						p->InSize = recdata;

						memcpy(p->InBuffer, buffer, recdata);
						delete[] buffer;

						ne->data1 = p;
					}

					ne->Type = EventType::ReceiveFrom;
					ne->data2 = clientaddr;

					if (cur->data1 != nullptr) delete (int*)cur->data1;
				}break;

				case EventType::Send:{
					unsigned char* outbuffer = (unsigned char*)cur->data1;
					bool* sendsize = (bool*)cur->data2;
					int outpos = *(int*)cur->data3;
					int sent = 0;

					if (*sendsize) {
						Packet p;
						p.OutBuffer = outbuffer;
						p.OutPos = outpos;
						p.Sock = sock->Sock;
						p.Send(sock->ssl);

						sent = outpos + 4;
					} else {
						int curpos = 0;
						while (curpos != outpos) {
							int ret = sock->ssl != nullptr ? SSL_write(sock->ssl, outbuffer + curpos, outpos - curpos) : sock->Sock->SendRaw(outbuffer + curpos, outpos - curpos);
							if (ret <= 0) {
								sock->Sock->close();
								break;
							}

							curpos += ret;
						}

						sent = curpos;

						delete[] outbuffer;
					}

					delete sendsize;
					delete (int*)cur->data3;

					ne->data1 = new bool(sock->Sock->state == EzSock::SockState::skCONNECTED);
					ne->data2 = new int(sent);
					ne->Type = EventType::Send;
				}break;

				case EventType::SendTo:{
					unsigned char* outbuffer = (unsigned char*)cur->data1;
					int outpos = *(int*)cur->data2;
					char* ip = (char*)cur->data3;
					int port = *(int*)cur->data4;

					struct sockaddr_in servaddr;
					struct hostent* lookup = gethostbyname(ip);
					if (lookup != nullptr) {
						memset((char *)&servaddr, 0, sizeof(servaddr));
						servaddr.sin_family = AF_INET;
						memcpy(&servaddr.sin_addr, lookup->h_addr_list[0], lookup->h_length);
						servaddr.sin_port = htons(port);

						sendto(sock->Sock->sock, (char*)outbuffer, outpos, 0, (struct sockaddr*)&servaddr, sizeof(servaddr));
					}

					delete[] outbuffer;

					ne->data1 = new bool(lookup != nullptr);
					ne->data2 = cur->data2;
					ne->data3 = cur->data3;
					ne->data4 = cur->data4;
					ne->Type = EventType::SendTo;
				}break;

			}

			sock->Mutex.Lock();
			sock->Callbacks.push_back(ne);
			sock->Mutex.Unlock();
			delete cur;
		}

		return 0;
	}

	SockWrapper::SockWrapper(lua_State* ls, int type) :
		Sock(new EzSock()),
		state(ls),
		SocketType(type),
		Callback_Accept(-1),
		Callback_Receive(-1),
		Callback_Connect(-1),
		Callback_Send(-1),
		Callback_Disconnect(-1),
		Callback_ReceiveFrom(-1),
		Callback_SendTo(-1),
		CurrentWorkers(0),
		RefCount(0),
		DestoryWorkers(false),
		DidDisconnectCallback(false),
		sslCtx(nullptr),
		ssl(nullptr)
	{
		
	}

	void SockWrapper::PushToStack(lua_State* state) {
		DEBUGPRINTFUNC;

		GarrysMod::Lua::UserData* ud = (GarrysMod::Lua::UserData*)LUA->NewUserdata(sizeof(GarrysMod::Lua::UserData));
		ud->data = this;
		ud->type = UD_TYPE_SOCKET;

		Engine* engine = Engine::GetEngineByState(state);
		LUA->ReferencePush(engine->SocketRef);
		LUA->SetMetaTable(-2);

		this->RefCount++;
	}

	void SockWrapper::CreateWorkers() {

		this->Mutex.Lock();
		bool old = this->DestoryWorkers;
		this->DestoryWorkers = false;

#ifdef _MSC_VER
		Threadhandles.push_back(CreateThread(nullptr, 0, SockWorkerFunction, this, 0, nullptr));
		Threadhandles.push_back(CreateThread(nullptr, 0, SockWorkerFunction, this, 0, nullptr));
#else
		pthread_t a;
		pthread_t b;

		pthread_create(&a, NULL, &SockWorkerFunction, this);
		pthread_create(&b, NULL, &SockWorkerFunction, this);

		pthread_detach(a);
		pthread_detach(b);

		Threadhandles.push_back(a);
		Threadhandles.push_back(b);
#endif

		this->DestoryWorkers = old;
		this->Mutex.Unlock();
	}

	void SockWrapper::Reset() {
		DEBUGPRINTFUNC;

		this->CallDisconnect();

		this->KillWorkers();
		this->Sock->close();

		delete this->Sock;
		this->Sock = new EzSock();

		// we have a new sock now, so reset this flag
		this->DidDisconnectCallback = false;
	}

	void SockWrapper::CallDisconnect() {
		DEBUGPRINTFUNC;

		if (this->DidDisconnectCallback || this->Callback_Disconnect == -1) return;
		this->DidDisconnectCallback = true;

		this->Sock->state = EzSock::SockState::skDISCONNECTED;

		LUA->ReferencePush(this->Callback_Disconnect);
		this->PushToStack(state);
		LUA->Call(1, 0);
	}

	void SockWrapper::KillWorkers() {
		DEBUGPRINTFUNC;

		if (this->sslCtx != nullptr) {
			SSL_CTX_free(this->sslCtx);
			this->sslCtx = nullptr;
		}

		if (this->ssl != nullptr) {
			SSL_free(this->ssl);
			this->ssl = nullptr;
		}

		this->Sock->close();

		this->DestoryWorkers = true;
		bool isdone = false;
		while (!isdone) {
#ifdef _MSC_VER
			Sleep(1);
#else
			struct timespec ts;
			ts.tv_sec = 0;
			ts.tv_nsec = 1000000;
			nanosleep(&ts, NULL);
#endif

			this->Mutex.Lock();
			isdone = this->CurrentWorkers == 0;
			if (isdone) {
				for (unsigned int i = 0; i < this->Threadhandles.size(); i++) {
#ifdef _MSC_VER
					CloseHandle(this->Threadhandles[i]);
#endif
				}

				this->Threadhandles.clear();
			}
			this->Mutex.Unlock();
		}

		this->DestoryWorkers = false;
	}

	SockWrapper::~SockWrapper() {
		DEBUGPRINTFUNC;

		if (this->Callback_Accept != -1) LUA->ReferenceFree(this->Callback_Accept);
		if (this->Callback_Connect != -1) LUA->ReferenceFree(this->Callback_Connect);
		if (this->Callback_Receive != -1) LUA->ReferenceFree(this->Callback_Receive);
		if (this->Callback_ReceiveFrom != -1) LUA->ReferenceFree(this->Callback_ReceiveFrom);
		if (this->Callback_Send != -1) LUA->ReferenceFree(this->Callback_Send);
		if (this->Callback_SendTo != -1) LUA->ReferenceFree(this->Callback_SendTo);
		if (this->Callback_Disconnect != -1) LUA->ReferenceFree(this->Callback_Disconnect);

		this->KillWorkers();

		this->Mutex.Lock();
		while (this->Todo.size() > 0) {
			delete this->Todo[0];
			this->Todo.erase(this->Todo.begin());
		}

		Engine* engine = Engine::GetEngineByState(this->state);
		engine->Think(this->state);

		this->Mutex.Unlock();

		delete this->Sock;
	}
}