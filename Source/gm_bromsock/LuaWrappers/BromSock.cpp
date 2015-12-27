#include "BromSock.h"

#include "../Objects/Engine.h"
#include "../Objects/BSEzSock.h"
#include "../Objects/BSPacket.h"
#include "../Objects/LockObject.h"
#include "../Objects/SockWrapper.h"

#include <openssl/ssl.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/conf.h>
#include <openssl/x509.h>
#include <openssl/buffer.h>
#include <openssl/x509v3.h>
#include <openssl/opensslconf.h>

namespace GMBSOCK {
	GMOD_FUNCTION(CreateSocket) {
		DEBUGPRINTFUNC;

		int type = IPPROTO_TCP;
		if (LUA->IsType(1, GarrysMod::Lua::Type::NUMBER)) {
			type = (int)LUA->CheckNumber(1);
		}

		SockWrapper* nsw = new SockWrapper(state, type);

		Engine* engine = Engine::GetEngineByState(state);
		engine->Sockets.push_back(nsw);

		nsw->PushToStack(state);
		return 1;
	}

	GMOD_FUNCTION(SOCK_Connect) {
		DEBUGPRINTFUNC;

		LUA->CheckType(1, UD_TYPE_SOCKET);
		LUA->CheckType(2, GarrysMod::Lua::Type::STRING);
		LUA->CheckType(3, GarrysMod::Lua::Type::NUMBER);

		SockWrapper* s = GETSOCK(1);
		s->Sock->create(s->SocketType);
		s->CreateWorkers();

		if (s->Callback_Connect > -1) {
			const char* luaip = LUA->GetString(2);
			int iplen = strlen(luaip) + 1;
			char* ip = new char[iplen];
			memcpy(ip, luaip, iplen);

			SockEvent* se = new SockEvent();
			se->Type = EventType::Connect;
			se->data1 = ip;
			se->data2 = new unsigned short((unsigned short)LUA->GetNumber(3));

			s->Mutex.Lock();
			s->Todo.push_back(se);
			s->Mutex.Unlock();
			return 0;
		} else {
			LUA->PushBool(s->Sock->connect(LUA->GetString(2), (unsigned short)LUA->GetNumber(3)) == 0);
			return 1;
		}
	}

	GMOD_FUNCTION(SOCK_Listen) {
		DEBUGPRINTFUNC;

		LUA->CheckType(1, UD_TYPE_SOCKET);

		const char* ip = "0.0.0.0";
		unsigned short port;

		if (LUA->IsType(3, GarrysMod::Lua::Type::NUMBER) && LUA->IsType(2, GarrysMod::Lua::Type::STRING)) {
			ip = LUA->GetString(2);
			port = (unsigned short)LUA->GetNumber(3);
		} else if (LUA->IsType(2, GarrysMod::Lua::Type::NUMBER)) {
			port = (unsigned short)LUA->GetNumber(2);
		} else {
			LUA->PushBool((GETSOCK(1))->Sock->listen());
			return 1;
		}

		SockWrapper* s = GETSOCK(1);
		s->Sock->create(s->SocketType);

		bool ret = s->Sock->bind(ip, port) && s->Sock->listen();
		if (ret) s->CreateWorkers();

		return 1;
	}

	GMOD_FUNCTION(SOCK_Bind) {
		DEBUGPRINTFUNC;

		LUA->CheckType(1, UD_TYPE_SOCKET);

		const char* ip = "0.0.0.0";
		unsigned short port;

		if (LUA->IsType(3, GarrysMod::Lua::Type::NUMBER) && LUA->IsType(2, GarrysMod::Lua::Type::STRING)) {
			ip = LUA->GetString(2);
			port = (unsigned short)LUA->GetNumber(3);
		} else if (LUA->IsType(2, GarrysMod::Lua::Type::NUMBER)) {
			port = (unsigned short)LUA->GetNumber(2);
		} else {
			LUA->ThrowError("Expected (string ip, number port), or just a number port");
		}

		SockWrapper* s = GETSOCK(1);
		s->Sock->create(s->SocketType);

		LUA->PushBool(s->Sock->bind(ip, port));
		return 1;
	}

	GMOD_FUNCTION(SOCK_SetBlocking) {
		DEBUGPRINTFUNC;

		LUA->CheckType(1, UD_TYPE_SOCKET);
		LUA->CheckType(2, GarrysMod::Lua::Type::BOOL);

		(GETSOCK(1))->Sock->blocking = LUA->GetBool(2);

		return 0;
	}

	SSL_CTX* InitCTX(void) {
		const SSL_METHOD *method;
		SSL_CTX *ctx;

		OpenSSL_add_all_algorithms();
		SSL_load_error_strings();
		method = TLSv1_2_client_method();
		ctx = SSL_CTX_new(method);

		if (ctx == NULL) {
			ERR_print_errors_fp(stderr);
			abort();
		}

		return ctx;
	}

	GMOD_FUNCTION(SOCK_StartSSLClient) {
		DEBUGPRINTFUNC;

		LUA->CheckType(1, UD_TYPE_SOCKET);
		SockWrapper* sw = GETSOCK(1);

		sw->sslCtx = InitCTX();
		sw->ssl = SSL_new(sw->sslCtx);
		SSL_set_fd(sw->ssl, sw->Sock->sock);

		if (SSL_connect(sw->ssl) == -1) {
			ERR_print_errors_fp(stderr);

			SSL_CTX_free(sw->sslCtx);
			SSL_free(sw->ssl);

			sw->sslCtx = nullptr;
			sw->ssl = nullptr;

			LUA->PushBool(false);
			return 1;
		}

		LUA->PushBool(true);
		return 1;
	}

	GMOD_FUNCTION(SOCK_Disconnect) {
		DEBUGPRINTFUNC;

		LUA->CheckType(1, UD_TYPE_SOCKET);

		(GETSOCK(1))->Reset();

		return 0;
	}

	GMOD_FUNCTION(SOCK_SendTo) {
		DEBUGPRINTFUNC;

		LUA->CheckType(1, UD_TYPE_SOCKET);
		LUA->CheckType(2, UD_TYPE_PACKET);
		LUA->CheckType(3, GarrysMod::Lua::Type::STRING);
		LUA->CheckType(4, GarrysMod::Lua::Type::NUMBER);

		SockWrapper* s = (GETSOCK(1));
		Packet* p = GETPACK(2);

		s->Sock->create(s->SocketType);

		const char* luaip = LUA->GetString(3);
		int iplen = strlen(luaip) + 1;
		char* ip = new char[iplen];
		memcpy(ip, luaip, iplen);

		SockEvent* se = new SockEvent();
		se->Type = EventType::SendTo;
		se->data1 = p->OutBuffer;
		se->data2 = new int(p->OutPos);
		se->data3 = ip;
		se->data4 = new int((int)LUA->GetNumber(4));

		// reset the packet
		p->OutBuffer = nullptr;
		p->OutPos = 0;
		p->OutSize = 0;

		s->Mutex.Lock();
		s->Todo.push_back(se);
		s->Mutex.Unlock();

		if (s->CurrentWorkers == 0) {
			s->CreateWorkers();
		}

		return 0;
	}

	GMOD_FUNCTION(SOCK_Send) {
		DEBUGPRINTFUNC;

		LUA->CheckType(1, UD_TYPE_SOCKET);
		LUA->CheckType(2, UD_TYPE_PACKET);

		SockWrapper* s = (GETSOCK(1));
		Packet* p = GETPACK(2);
		bool sendsize = !LUA->IsType(3, GarrysMod::Lua::Type::BOOL) || !LUA->GetBool(3);

		if (s->Callback_Send > -1) {
			SockEvent* se = new SockEvent();
			se->Type = EventType::Send;
			se->data1 = p->OutBuffer;
			se->data2 = new bool(sendsize);
			se->data3 = new int(p->OutPos);

			// reset the packet
			p->OutBuffer = nullptr;
			p->OutPos = 0;
			p->OutSize = 0;

			s->Mutex.Lock();
			s->Todo.push_back(se);
			s->Mutex.Unlock();
			return 0;
		} else {
			if (sendsize) {
				p->Sock = s->Sock;
				p->Send(s->ssl);

				LUA->PushBool(s->Sock->state == EzSock::SockState::skCONNECTED);
				return 1;
			} else {
				int curpos = 0;
				while (curpos != p->OutPos) {
					int ret = s->ssl == nullptr ? s->Sock->SendRaw(p->OutBuffer + curpos, p->OutPos - curpos) : SSL_write(s->ssl, p->OutBuffer + curpos, p->OutPos - curpos);
					if (ret <= 0) {
						s->Sock->close();
						break;
					}

					curpos += ret;
				}

				p->Clear();

				LUA->PushBool(s->Sock->state == EzSock::SockState::skCONNECTED);
				return 1;
			}
		}
	}

	GMOD_FUNCTION(SOCK_SetMaxReceiveSize) {
		DEBUGPRINTFUNC;
		LUA->CheckType(1, UD_TYPE_SOCKET);
		LUA->CheckType(2, GarrysMod::Lua::Type::NUMBER);

		SockWrapper* s = GETSOCK(1);
		s->MaxReceiveSize = (int)LUA->GetNumber(2);

		return 0;
	}

	GMOD_FUNCTION(SOCK_Receive) {
		DEBUGPRINTFUNC;

		LUA->CheckType(1, UD_TYPE_SOCKET);

		int toread = LUA->IsType(2, GarrysMod::Lua::Type::NUMBER) ? (int)LUA->GetNumber(2) : -1;

		SockWrapper* s = GETSOCK(1);
		if (s->Callback_Receive > -1) {
			SockEvent* se = new SockEvent();
			se->Type = EventType::Receive;
			se->data1 = new int(toread);

			s->Mutex.Lock();
			s->Todo.push_back(se);
			s->Mutex.Unlock();
			return 0;
		} else {
			Packet* p = new Packet(s->Sock);
			if (toread == -1) {
				toread = p->ReadInt();
			}

			if (toread > s->MaxReceiveSize || toread < 0) {
				char buff[256];
				sprintf(buff, "error in bromsock_C++ allocating buffer to receive %d bytes from %s:%d", toread, inet_ntoa(s->Sock->addr.sin_addr), ntohs(s->Sock->addr.sin_port));

				LUA->ThrowError(buff);
				return 0;
			}

			if (!p->CanRead(toread, s->ssl)) {
				delete p;
				LUA->PushBool(false);
				return 1;
			}

			GarrysMod::Lua::UserData* ud = (GarrysMod::Lua::UserData*)LUA->NewUserdata(sizeof(GarrysMod::Lua::UserData));
			ud->data = p;
			ud->type = UD_TYPE_PACKET;

			Engine* engine = Engine::GetEngineByState(state);
			LUA->ReferencePush(engine->PacketRef);
			LUA->SetMetaTable(-2);
			p->RefCount++;
			return 1;
		}
	}

	GMOD_FUNCTION(SOCK_ReceiveFrom) {
		DEBUGPRINTFUNC;

		LUA->CheckType(1, UD_TYPE_SOCKET);

		SockWrapper* s = GETSOCK(1);
		if (s->Callback_ReceiveFrom == -1) LUA->ThrowError("ReceiveFrom only supports callbacks. Please use this. It's the way to go for networking.");

		int toread = LUA->IsType(2, GarrysMod::Lua::Type::NUMBER) ? (int)LUA->GetNumber(2) : 65535; // max "theoretical" diagram size

		if (toread > s->MaxReceiveSize || toread < 0) {
			char buff[256];
			sprintf(buff, "error in bromsock_C++ allocating buffer to receive %d bytes from %s:%d", toread, inet_ntoa(s->Sock->addr.sin_addr), ntohs(s->Sock->addr.sin_port));

			LUA->ThrowError(buff);
			return 0;
		}

		SockEvent* se = new SockEvent();
		se->Type = EventType::ReceiveFrom;
		se->data1 = new int(toread);

		if (LUA->IsType(3, GarrysMod::Lua::Type::STRING) && LUA->IsType(4, GarrysMod::Lua::Type::NUMBER)) {
			const char* luaip = LUA->GetString(3);
			int iplen = strlen(luaip) + 1;
			char* ip = new char[iplen];
			memcpy(ip, luaip, iplen);

			se->data2 = ip;
			se->data3 = new int((int)LUA->GetNumber(4));
		}

		s->Mutex.Lock();
		s->Todo.push_back(se);
		s->Mutex.Unlock();

		if (s->CurrentWorkers == 0) {
			s->CreateWorkers();
		}

		return 0;
	}

	GMOD_FUNCTION(SOCK_ReceiveUntil) {
		DEBUGPRINTFUNC;

		LUA->CheckType(1, UD_TYPE_SOCKET);
		LUA->CheckType(2, GarrysMod::Lua::Type::STRING);

		SockWrapper* s = GETSOCK(1);
		const char* luaseq = LUA->GetString(2);

		if (s->Callback_Receive > -1) {
			int seqlen = strlen(luaseq) + 1;
			char* seq = new char[seqlen];
			memcpy(seq, luaseq, seqlen);

			SockEvent* se = new SockEvent();
			se->Type = EventType::Receive;
			se->data2 = seq;

			s->Mutex.Lock();
			s->Todo.push_back(se);
			s->Mutex.Unlock();
			return 0;
		} else {
			Packet* p = new Packet(s->Sock);
			if (!p->CanRead((char*)luaseq, s->ssl)) {
				delete p;
				LUA->PushBool(false);
				return 1;
			}

			GarrysMod::Lua::UserData* ud = (GarrysMod::Lua::UserData*)LUA->NewUserdata(sizeof(GarrysMod::Lua::UserData));
			ud->data = p;
			ud->type = UD_TYPE_PACKET;

			Engine* engine = Engine::GetEngineByState(state);
			LUA->ReferencePush(engine->PacketRef);
			LUA->SetMetaTable(-2);
			p->RefCount++;
			return 1;
		}
	}

	GMOD_FUNCTION(SOCK_Accept) {
		DEBUGPRINTFUNC;

		LUA->CheckType(1, UD_TYPE_SOCKET);
		SockWrapper* s = GETSOCK(1);

		if (s->Callback_Accept > -1) {
			SockEvent* se = new SockEvent();
			se->Type = EventType::Accept;

			s->Mutex.Lock();
			s->Todo.push_back(se);
			s->Mutex.Unlock();
			return 0;
		} else {
			SockWrapper* csw = new SockWrapper(state);
			if (!s->Sock->accept(csw->Sock)) {
				delete csw;

				LUA->PushBool(false);
				return 1;
			}

			csw->PushToStack(state);

			Engine* engine = Engine::GetEngineByState(state);
			engine->Sockets.push_back(csw);

			return 1;
		}
	}

	GMOD_FUNCTION(SOCK__GC) {
		DEBUGPRINTFUNC;

		LUA->CheckType(1, UD_TYPE_SOCKET);
		SockWrapper* s = GETSOCK(1);

		s->RefCount--;
		if (s->RefCount == 0) {
			s->Mutex.Lock();
			bool isdone = s->CurrentWorkers == 0;
			s->Mutex.Unlock();

			if (!isdone) {
				return 0;
			}

			Engine* engine = Engine::GetEngineByState(state);
			for (unsigned i = 0; i < engine->Sockets.size(); i++) {
				if (engine->Sockets[i] == s) {
					engine->Sockets.erase(engine->Sockets.begin() + i);
					break;
				}
			}

			DEBUGPRINT("KILLING SOCKET");
			delete s;
		}

		return 0;
	}

	GMOD_FUNCTION(SOCK_AddWorker) {
		DEBUGPRINTFUNC;

		LUA->CheckType(1, UD_TYPE_SOCKET);
		SockWrapper* s = GETSOCK(1);

#ifdef _MSC_VER
		s->Threadhandles.push_back(CreateThread(nullptr, 0, SockWorkerFunction, s, 0, nullptr));
#else
		pthread_t a;
		pthread_create(&a, NULL, &SockWorkerFunction, s);
		pthread_detach(a);

		s->Threadhandles.push_back(a);
#endif

		return 0;
	}

	GMOD_FUNCTION(SOCK_SetOption) {
		DEBUGPRINTFUNC;

		LUA->CheckType(1, UD_TYPE_SOCKET);
		LUA->CheckType(2, GarrysMod::Lua::Type::NUMBER);
		LUA->CheckType(3, GarrysMod::Lua::Type::NUMBER);
		LUA->CheckType(4, GarrysMod::Lua::Type::NUMBER);

		int level = (int)LUA->GetNumber(2);
		int option = (int)LUA->GetNumber(3);
		int value = (int)LUA->GetNumber(4);

		SockWrapper* s = GETSOCK(1);

#ifdef _MSC_VER
		DWORD value_win = (DWORD)value;
		int ret = setsockopt(s->Sock->sock, level, option, (const char*)&value_win, sizeof(value_win));
#else
		int ret = setsockopt(s->Sock->sock, level, option, &value, sizeof(value));
#endif

		// failed == -1
		LUA->PushNumber(ret);
		return 1;
	}

	GMOD_FUNCTION(SOCK_SetTimeout) {
		DEBUGPRINTFUNC;

		LUA->CheckType(1, UD_TYPE_SOCKET);
		LUA->CheckType(2, GarrysMod::Lua::Type::NUMBER);
		SockWrapper* s = GETSOCK(1);

#ifdef _MSC_VER
		DWORD dwTime = (DWORD)LUA->GetNumber(2);
		int reta = setsockopt(s->Sock->sock, SOL_SOCKET, SO_SNDTIMEO, (const char*)&dwTime, sizeof(dwTime));
		int retb = setsockopt(s->Sock->sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&dwTime, sizeof(dwTime));
#else
		int dwTime = (int)LUA->GetNumber(2);
		int reta = setsockopt(s->Sock->sock, SOL_SOCKET, SO_SNDTIMEO, &dwTime, sizeof(dwTime));
		int retb = setsockopt(s->Sock->sock, SOL_SOCKET, SO_RCVTIMEO, &dwTime, sizeof(dwTime));
#endif

		// Should be SOCKET_ERROR, but linux does not have SOCKET_ERROR defined.
		LUA->PushBool(reta != -1 && retb != -1);
		return 1;
	}

	GMOD_FUNCTION(SOCK_Create) {
		DEBUGPRINTFUNC;

		LUA->CheckType(1, UD_TYPE_SOCKET);

		SockWrapper* s = GETSOCK(1);
		s->Sock->create(s->SocketType);

		return 0;
	}

	GMOD_FUNCTION(SOCK__TOSTRING) {
		DEBUGPRINTFUNC;

		LUA->CheckType(1, UD_TYPE_SOCKET);
		SockWrapper* s = GETSOCK(1);

		char* ipstr = inet_ntoa(s->Sock->addr.sin_addr);
		char buff[256];
		sprintf(buff, "bromsock{%s:%d}", ipstr, ntohs(s->Sock->addr.sin_port));

		LUA->PushString(buff);

		return 1;
	}

	GMOD_FUNCTION(SOCK__EQ) {
		DEBUGPRINTFUNC;

		LUA->CheckType(1, UD_TYPE_SOCKET);
		LUA->CheckType(2, UD_TYPE_SOCKET);

		LUA->PushBool(GETSOCK(1) == GETSOCK(2));

		return 1;
	}

	GMOD_FUNCTION(SOCK_CALLBACKSend) { DEBUGPRINTFUNC; LUA->CheckType(1, UD_TYPE_SOCKET); LUA->CheckType(2, GarrysMod::Lua::Type::FUNCTION); LUA->Push(2); (GETSOCK(1))->Callback_Send = LUA->ReferenceCreate(); return 0; }
	GMOD_FUNCTION(SOCK_CALLBACKSendTo) { DEBUGPRINTFUNC; LUA->CheckType(1, UD_TYPE_SOCKET); LUA->CheckType(2, GarrysMod::Lua::Type::FUNCTION); LUA->Push(2); (GETSOCK(1))->Callback_SendTo = LUA->ReferenceCreate(); return 0; }
	GMOD_FUNCTION(SOCK_CALLBACKReceive) { DEBUGPRINTFUNC; LUA->CheckType(1, UD_TYPE_SOCKET); LUA->CheckType(2, GarrysMod::Lua::Type::FUNCTION); LUA->Push(2); (GETSOCK(1))->Callback_Receive = LUA->ReferenceCreate(); return 0; }
	GMOD_FUNCTION(SOCK_CALLBACKReceiveFrom) { DEBUGPRINTFUNC; LUA->CheckType(1, UD_TYPE_SOCKET); LUA->CheckType(2, GarrysMod::Lua::Type::FUNCTION); LUA->Push(2); (GETSOCK(1))->Callback_ReceiveFrom = LUA->ReferenceCreate(); return 0; }
	GMOD_FUNCTION(SOCK_CALLBACKConnect) { DEBUGPRINTFUNC; LUA->CheckType(1, UD_TYPE_SOCKET); LUA->CheckType(2, GarrysMod::Lua::Type::FUNCTION); LUA->Push(2); (GETSOCK(1))->Callback_Connect = LUA->ReferenceCreate(); return 0; }
	GMOD_FUNCTION(SOCK_CALLBACKAccept) { DEBUGPRINTFUNC; LUA->CheckType(1, UD_TYPE_SOCKET); LUA->CheckType(2, GarrysMod::Lua::Type::FUNCTION); LUA->Push(2); (GETSOCK(1))->Callback_Accept = LUA->ReferenceCreate(); return 0; }
	GMOD_FUNCTION(SOCK_CALLBACKDisconnect) { DEBUGPRINTFUNC; LUA->CheckType(1, UD_TYPE_SOCKET); LUA->CheckType(2, GarrysMod::Lua::Type::FUNCTION); LUA->Push(2); (GETSOCK(1))->Callback_Disconnect = LUA->ReferenceCreate(); return 0; }

	GMOD_FUNCTION(SOCK_IsValid) { DEBUGPRINTFUNC; LUA->CheckType(1, UD_TYPE_SOCKET); LUA->PushBool((GETSOCK(1))->Sock->state == EzSock::SockState::skCONNECTED || (GETSOCK(1))->Sock->state == EzSock::SockState::skLISTENING); return 1; }
	GMOD_FUNCTION(SOCK_GetIP) { DEBUGPRINTFUNC; LUA->CheckType(1, UD_TYPE_SOCKET); LUA->PushString(inet_ntoa((GETSOCK(1))->Sock->addr.sin_addr)); return 1; }
	GMOD_FUNCTION(SOCK_GetPort) { DEBUGPRINTFUNC; LUA->CheckType(1, UD_TYPE_SOCKET); LUA->PushNumber(ntohs((GETSOCK(1))->Sock->addr.sin_port)); return 1; }
	GMOD_FUNCTION(SOCK_GetState) { DEBUGPRINTFUNC; LUA->CheckType(1, UD_TYPE_SOCKET); LUA->PushNumber((GETSOCK(1))->Sock->state); return 1; }
	}