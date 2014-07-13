#define GMMODULE

#include "Gmod_Headers/Lua/Interface.h"
#include "Objects/BSEzSock.h"
#include "Objects/BSPacket.h"
#include "Objects/LockObject.h"

#ifdef _MSC_VER
#include <Windows.h>
#else
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#endif

using namespace BromScript;
#define GMOD_FUNCTION(fn) int fn(lua_State* state)
#define GETSOCK(num) (SockWrapper*)((GarrysMod::Lua::UserData*)LUA->GetUserdata(num))->data
#define GETPACK(num) (Packet*)((GarrysMod::Lua::UserData*)LUA->GetUserdata(num))->data
#define ADDFUNC(fn, f) LUA->PushCFunction(f); LUA->SetField(-2, fn);
#define CALLLUAFUNC(args) LUA->Call(args, 0);
#define UD_TYPE_SOCKET 122
#define UD_TYPE_PACKET 123

#ifdef _DEBUG
#define DEBUGPRINTFUNC LUA->PushSpecial(GarrysMod::Lua::SPECIAL_GLOB); LUA->GetField( -1, "print" ); char dbuff[256]; sprintf_s(dbuff, "BS: CURF: %s", __FUNCTION__); LUA->PushString(dbuff); LUA->Call( 1, 0 );
#else
#define DEBUGPRINTFUNC
#endif

static int PacketRef = 0;
static int SocketRef = 0;

class SockWrapper;
static std::vector<SockWrapper*> AllocatedSockets;

#ifdef _MSC_VER
DWORD WINAPI SockWorker(void* obj);
#else
void* SockWorker(void *obj);
#endif

enum class EventType {
	NONE, Connect, Send, Receive, Accept
};

class SockEvent{
public:
	EventType Type;
	void* data1;
	void* data2;
	void* data3;

	SockEvent():data1(null), data2(null), data3(null), Type(EventType::NONE) {}
};

class SockWrapper{
public:
	EzSock* Sock;
	lua_State* state;

	int Callback_Receive;
	int Callback_Send;
	int Callback_Connect;
	int Callback_Disconnect;
	int Callback_Accept;
	int CurrentWorkers;
	int RefCount;
	bool DestoryWorkers;
	bool DidDisconnectCallback;
	
	std::vector<SockEvent*> Todo;
	std::vector<SockEvent*> Callbacks;
	LockObject Mutex;
	
	SockWrapper(lua_State* ls):Callback_Accept(-1),Callback_Receive(-1),Callback_Connect(-1),Callback_Send(-1),Callback_Disconnect(-1),CurrentWorkers(0),RefCount(0),DestoryWorkers(false),DidDisconnectCallback(false){
		this->Sock = new EzSock();
		this->state = ls;
	}

	void PushToStack(lua_State* state){
		DEBUGPRINTFUNC;

		GarrysMod::Lua::UserData* ud = (GarrysMod::Lua::UserData*)LUA->NewUserdata(sizeof(GarrysMod::Lua::UserData));
		ud->data = this;
		ud->type = UD_TYPE_SOCKET;

		LUA->ReferencePush(SocketRef);
		LUA->SetMetaTable( -2 );

		this->RefCount++;
	}

	void CreateWorkers(){
		#ifdef _MSC_VER
				CreateThread(null, null, SockWorker, this, null, null);
				CreateThread(null, null, SockWorker, this, null, null);
		#else
				pthread_t uselessshit;
				pthread_create(&uselessshit, NULL, &SockWorker, this);
				pthread_create(&uselessshit, NULL, &SockWorker, this);
		#endif
	}

	void Reset(){
		DEBUGPRINTFUNC;

		this->CallDisconnect();
		this->Sock->close();
		
		this->KillWorkers();

		delete this->Sock;
		this->Sock = new EzSock();
	}

	void CallDisconnect(){
		DEBUGPRINTFUNC;

		if (this->DidDisconnectCallback || this->Callback_Disconnect == -1) return;
		this->DidDisconnectCallback = true;

		LUA->ReferencePush(this->Callback_Disconnect);
		this->PushToStack(state);
		LUA->Call(1, 0);
	}

	void KillWorkers(){
		DEBUGPRINTFUNC;

		// beter safe than sorry.
		this->Sock->close();

		this->DestoryWorkers = true;
		bool isdone = false;
		while(!isdone){
#ifdef _MSC_VER
			Sleep(1);
#else
			sleep(1);
#endif
			
			this->Mutex.Lock();
			isdone = this->CurrentWorkers == 0;
			this->Mutex.Unlock();
		}

		this->DestoryWorkers = false;
	}

	~SockWrapper(){
		DEBUGPRINTFUNC;

		if (this->Callback_Accept != -1) LUA->ReferenceFree(this->Callback_Accept);
		if (this->Callback_Connect != -1) LUA->ReferenceFree(this->Callback_Connect);
		if (this->Callback_Receive != -1) LUA->ReferenceFree(this->Callback_Receive);
		if (this->Callback_Send != -1) LUA->ReferenceFree(this->Callback_Send);
		if (this->Callback_Disconnect != -1) LUA->ReferenceFree(this->Callback_Disconnect);
		
		this->Sock->close();
		
		this->KillWorkers();

		delete this->Sock;
	}
};

#ifdef _MSC_VER
DWORD WINAPI SockWorker(void* obj){
#else
void* SockWorker(void *obj){
#endif
	SockWrapper* sock = (SockWrapper*)obj;
	sock->Mutex.Lock();
	sock->CurrentWorkers++;
	sock->Mutex.Unlock();

	while(true){
		if (sock->DestoryWorkers){
			sock->Mutex.Lock();
			sock->CurrentWorkers--;
			sock->Mutex.Unlock();
			return 0;
		}

		SockEvent* cur = null;
			
		sock->Mutex.Lock();
		if (sock->Todo.size() > 0){
			cur = sock->Todo[0];
			sock->Todo.erase(sock->Todo.begin());
		}
		sock->Mutex.Unlock();

		if (cur == null){
#ifdef _MSC_VER
			Sleep(1);
#else
			sleep(1);
#endif
			continue;
		}

		SockEvent* ne = new SockEvent();

		switch(cur->Type){
		case EventType::Accept:{
			SockWrapper* nsock = new SockWrapper(sock->state);
			if(!sock->Sock->accept(nsock->Sock)){
				delete nsock;

				ne->data1 = null;
			}else{
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

			if (cur->data1 == null){
				if (p->CanRead(seq)){
					ne->data1 = p;
				}else{
					delete p;
				}
			}else{
				if ((*len == -1 && (*len = p->ReadInt()) == 0) || !p->CanRead(*len)){
					delete p;
				}else{
					ne->data1 = p;
				}
			}

			ne->Type = EventType::Receive;
			
			if (cur->data1 != null) delete (int*)cur->data1;
			if (cur->data2 != null) delete[] (char*)cur->data1;
		}break;

		case EventType::Send:{
			unsigned char* outbuffer = (unsigned char*)cur->data1;
			bool* sendsize = (bool*)cur->data2;
			int outpos = *(int*)cur->data3;
			int sent = 0;

			if (*sendsize){
				Packet p;
				p.OutBuffer = outbuffer;
				p.OutPos = outpos;
				p.Sock = sock->Sock;
				p.Send();

				sent = outpos + 4;
			}else{
				int curpos = 0;
				while(curpos != outpos){
					int ret = sock->Sock->SendRaw(outbuffer + curpos, outpos - curpos);
					if (ret <= 0){
						sock->Sock->Valid = false;
						break;
					}

					curpos += ret;
				}
				
				sent = curpos;
				
				delete[] outbuffer;
			}
			
			delete sendsize;
			delete (int*)cur->data3;
			
			ne->data1 = new bool(sock->Sock->Valid);
			ne->data2 = new int(sent);
			ne->Type = EventType::Send;
		}break;

		}
		
		sock->Mutex.Lock();
		sock->Callbacks.push_back(ne);
		sock->Mutex.Unlock();
		delete cur;
	}

	return 0;
}

GMOD_FUNCTION(CreatePacket){
	DEBUGPRINTFUNC;

	Packet* p = null;

	if (LUA->IsType(1, UD_TYPE_SOCKET)) p = new Packet((GETSOCK(1))->Sock);
	else p = new Packet();

	GarrysMod::Lua::UserData* ud = (GarrysMod::Lua::UserData*)LUA->NewUserdata(sizeof(GarrysMod::Lua::UserData));
	ud->data = p;
	ud->type = UD_TYPE_PACKET;

	LUA->ReferencePush(PacketRef);
	LUA->SetMetaTable( -2 );
	
	p->RefCount++;

	return 1;
}

GMOD_FUNCTION(CreateSocket){
	DEBUGPRINTFUNC;

	SockWrapper* nsw = new SockWrapper(state);
	AllocatedSockets.push_back(nsw);

	nsw->PushToStack(state);
	return 1;
}

GMOD_FUNCTION(ThinkHook){
	// too much spam :v
	// DEBUGPRINTFUNC;

	for (size_t i = 0; i < AllocatedSockets.size(); i++){
		SockWrapper* sw = AllocatedSockets[i];

		while(true){
			SockEvent* se = null;
			sw->Mutex.Lock();
			if (sw->Callbacks.size() > 0){
				se = sw->Callbacks[0];
				sw->Callbacks.erase(sw->Callbacks.begin());
			}
			sw->Mutex.Unlock();

			if (se == null)
				break;

			switch(se->Type){
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

				if (nsw != null){
					nsw->CreateWorkers();

					AllocatedSockets.push_back(nsw);

					LUA->ReferencePush(sw->Callback_Accept);
					sw->PushToStack(state);
					nsw->PushToStack(state);
					CALLLUAFUNC(2);
				}else{
					sw->CallDisconnect();
				}
			}break;

			case EventType::Send:{
				bool* valid = (bool*)se->data1;
				int* size = (int*)se->data2;
				
				if (*valid){
					LUA->ReferencePush(sw->Callback_Send);
					sw->PushToStack(state);
					LUA->PushNumber((double)*size);
					CALLLUAFUNC(2);
				}else{
					sw->CallDisconnect();
				}

				delete valid;
				delete size;
			}break;

			case EventType::Receive:{
				Packet* p = (Packet*)se->data1;

				if (p != null){
					LUA->ReferencePush(sw->Callback_Receive);
					sw->PushToStack(state);

					GarrysMod::Lua::UserData* ud = (GarrysMod::Lua::UserData*)LUA->NewUserdata(sizeof(GarrysMod::Lua::UserData));
					ud->data = p;
					ud->type = UD_TYPE_PACKET;
					LUA->ReferencePush(PacketRef);
					LUA->SetMetaTable( -2 );
					p->RefCount++;

					CALLLUAFUNC(2);
				}else{
					sw->CallDisconnect();
				}
			}break;
			}

			delete se;
		}
	}

	return 0;
}

GMOD_FUNCTION(SOCK_Connect){
	DEBUGPRINTFUNC;

	LUA->CheckType(1, UD_TYPE_SOCKET);
	LUA->CheckType(2, GarrysMod::Lua::Type::STRING);
	LUA->CheckType(3, GarrysMod::Lua::Type::NUMBER);

	SockWrapper* s = GETSOCK(1);
	s->Sock->create();
	s->CreateWorkers();

	if (s->Callback_Connect > -1){
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
	}else{
		LUA->PushBool(s->Sock->connect(LUA->GetString(2), (unsigned short)LUA->GetNumber(3)) == 0);
		return 1;
	}
}

GMOD_FUNCTION(SOCK_Listen){
	DEBUGPRINTFUNC;

	LUA->CheckType(1, UD_TYPE_SOCKET);
	LUA->CheckType(2, GarrysMod::Lua::Type::NUMBER);
	
	SockWrapper* s = GETSOCK(1);
	s->Sock->create();

	bool ret = s->Sock->bind((unsigned short)LUA->GetNumber(2)) && s->Sock->listen();
	if (ret) s->CreateWorkers();

	LUA->PushBool(ret);
	return 1;
}


GMOD_FUNCTION(SOCK_SetBlocking){
	DEBUGPRINTFUNC;

	LUA->CheckType(1, UD_TYPE_SOCKET);
	LUA->CheckType(2, GarrysMod::Lua::Type::BOOL);

	(GETSOCK(1))->Sock->blocking = LUA->GetBool(2);

	return 0;
}

GMOD_FUNCTION(SOCK_Disconnect){
	DEBUGPRINTFUNC;

	LUA->CheckType(1, UD_TYPE_SOCKET);

	(GETSOCK(1))->Reset();

	return 0;
}

GMOD_FUNCTION(SOCK_Send){
	DEBUGPRINTFUNC;

	LUA->CheckType(1, UD_TYPE_SOCKET);
	LUA->CheckType(2, UD_TYPE_PACKET);

	SockWrapper* s = (GETSOCK(1));
	Packet* p = GETPACK(2);
	bool sendsize = !LUA->IsType(3, GarrysMod::Lua::Type::BOOL) || !LUA->GetBool(3);
	
	if (s->Callback_Send > -1){
		SockEvent* se = new SockEvent();
		se->Type = EventType::Send;
		se->data1 = p->OutBuffer;
		se->data2 = new bool(sendsize);
		se->data3 = new int(p->OutPos);

		// reset the packet
		p->OutBuffer = null; 
		p->OutPos = 0;
		p->OutSize = 0;
		
		s->Mutex.Lock();
		s->Todo.push_back(se);
		s->Mutex.Unlock();
		return 0;
	}else{
		if (sendsize){
			p->Sock = s->Sock;
			p->Send();

			LUA->PushBool(s->Sock->Valid);
			return 1;
		}else{
			int curpos = 0;
			while(curpos != p->OutPos){
				int ret = s->Sock->SendRaw(p->OutBuffer + curpos, p->OutPos - curpos);
				if (ret <= 0){
					s->Sock->Valid = false;
					break;
				}
				
				curpos += ret;
			}
			
			p->Clear();
			
			LUA->PushBool(s->Sock->Valid);
			return 1;
		}
	}
}

GMOD_FUNCTION(SOCK_Receive){
	DEBUGPRINTFUNC;

	LUA->CheckType(1, UD_TYPE_SOCKET);
	if (!LUA->IsType(2, GarrysMod::Lua::Type::NIL)){
		LUA->CheckNumber(2);
	}

	int toread = LUA->IsType(2, GarrysMod::Lua::Type::NUMBER) ? (int)LUA->GetNumber(2) : -1;
	
	SockWrapper* s = GETSOCK(1);
	if (s->Callback_Receive > -1){
		SockEvent* se = new SockEvent();
		se->Type = EventType::Receive;
		se->data1 = new int(toread);

		s->Mutex.Lock();
		s->Todo.push_back(se);
		s->Mutex.Unlock();
		return 0;
	}else{
		Packet* p = new Packet(s->Sock);
		if (toread == -1){
			toread = p->ReadInt();
		}

		if (!p->CanRead(toread)){
			delete p;
			LUA->PushBool(false);
			return 1;
		}
		
		GarrysMod::Lua::UserData* ud = (GarrysMod::Lua::UserData*)LUA->NewUserdata(sizeof(GarrysMod::Lua::UserData));
		ud->data = p;
		ud->type = UD_TYPE_PACKET;

		LUA->ReferencePush(PacketRef);
		LUA->SetMetaTable( -2 );
		p->RefCount++;
		return 1;
	}
}

GMOD_FUNCTION(SOCK_ReceiveUntil){
	DEBUGPRINTFUNC;

	LUA->CheckType(1, UD_TYPE_SOCKET);
	LUA->CheckType(2, GarrysMod::Lua::Type::STRING);
	
	SockWrapper* s = GETSOCK(1);
	const char* luaseq = LUA->GetString(2);

	if (s->Callback_Receive > -1){
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
	}else{
		Packet* p = new Packet(s->Sock);
		if (!p->CanRead((char*)luaseq)){
			delete p;
			LUA->PushBool(false);
			return 1;
		}
		
		GarrysMod::Lua::UserData* ud = (GarrysMod::Lua::UserData*)LUA->NewUserdata(sizeof(GarrysMod::Lua::UserData));
		ud->data = p;
		ud->type = UD_TYPE_PACKET;

		LUA->ReferencePush(PacketRef);
		LUA->SetMetaTable( -2 );
		p->RefCount++;
		return 1;
	}
}

GMOD_FUNCTION(SOCK_CALLBACKSend){ DEBUGPRINTFUNC; LUA->CheckType(1, UD_TYPE_SOCKET); LUA->CheckType(2, GarrysMod::Lua::Type::FUNCTION); LUA->Push(2); (GETSOCK(1))->Callback_Send = LUA->ReferenceCreate(); return 0; }
GMOD_FUNCTION(SOCK_CALLBACKReceive){ DEBUGPRINTFUNC; LUA->CheckType(1, UD_TYPE_SOCKET); LUA->CheckType(2, GarrysMod::Lua::Type::FUNCTION); LUA->Push(2); (GETSOCK(1))->Callback_Receive = LUA->ReferenceCreate(); return 0; }
GMOD_FUNCTION(SOCK_CALLBACKConnect){ DEBUGPRINTFUNC; LUA->CheckType(1, UD_TYPE_SOCKET); LUA->CheckType(2, GarrysMod::Lua::Type::FUNCTION); LUA->Push(2); (GETSOCK(1))->Callback_Connect = LUA->ReferenceCreate(); return 0; }
GMOD_FUNCTION(SOCK_CALLBACKAccept){ DEBUGPRINTFUNC; LUA->CheckType(1, UD_TYPE_SOCKET); LUA->CheckType(2, GarrysMod::Lua::Type::FUNCTION); LUA->Push(2); (GETSOCK(1))->Callback_Accept = LUA->ReferenceCreate(); return 0; }
GMOD_FUNCTION(SOCK_CALLBACKDisconnect){ DEBUGPRINTFUNC; LUA->CheckType(1, UD_TYPE_SOCKET); LUA->CheckType(2, GarrysMod::Lua::Type::FUNCTION); LUA->Push(2); (GETSOCK(1))->Callback_Disconnect = LUA->ReferenceCreate(); return 0; }

GMOD_FUNCTION(SOCK_Accept){
	DEBUGPRINTFUNC;

	LUA->CheckType(1, UD_TYPE_SOCKET);
	SockWrapper* s = GETSOCK(1);

	if (s->Callback_Accept > -1){
		SockEvent* se = new SockEvent();
		se->Type = EventType::Accept;

		s->Mutex.Lock();
		s->Todo.push_back(se);
		s->Mutex.Unlock();
		return 0;
	}else{
		SockWrapper* csw = new SockWrapper(state);
		if (!s->Sock->accept(csw->Sock)){
			delete csw;

			LUA->PushBool(false);
			return 1;
		}

		csw->PushToStack(state);
		AllocatedSockets.push_back(csw);

		return 1;
	}
}

GMOD_FUNCTION(SOCK_AddWorker){
	DEBUGPRINTFUNC;

	LUA->CheckType(1, UD_TYPE_SOCKET);
	
#ifdef _MSC_VER
		CreateThread(null, null, SockWorker, GETSOCK(1), null, null);
#else
		pthread_t uselessshit;
		pthread_create(&uselessshit, NULL, &SockWorker, GETSOCK(1));
#endif

	return 0;
}

GMOD_FUNCTION(SOCK_SetTimeout){
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

GMOD_FUNCTION(SOCK__TOSTRING){
	DEBUGPRINTFUNC;

	LUA->CheckType(1, UD_TYPE_SOCKET);
	SockWrapper* s = GETSOCK(1);
	
	char* ipstr = inet_ntoa(s->Sock->addr.sin_addr);
	char buff[256];
#ifdef _MSC_VER
	sprintf_s(buff, "bromsock{%s:%d}", ipstr, ntohs(s->Sock->addr.sin_port));
#else
	sprintf(buff, "bromsock{%s:%d}", ipstr, ntohs(s->Sock->addr.sin_port));
#endif

	LUA->PushString(buff);

	return 1;
}

GMOD_FUNCTION(SOCK__EQ){
	DEBUGPRINTFUNC;

	LUA->CheckType(1, UD_TYPE_SOCKET);
	LUA->CheckType(2, UD_TYPE_SOCKET);

	LUA->PushBool(GETSOCK(1) == GETSOCK(2));

	return 1;
}

GMOD_FUNCTION(SOCK__GC){
	DEBUGPRINTFUNC;

	LUA->CheckType(1, UD_TYPE_SOCKET);
	SockWrapper* s = GETSOCK(1);

	s->RefCount--;
	if (s->RefCount == 0){
		for (unsigned i = 0; i < AllocatedSockets.size(); i++){
			if (AllocatedSockets[i] == s){
				AllocatedSockets.erase(AllocatedSockets.begin() + i);
				break;
			}
		}

		delete s;
	}

	return 0;
}

GMOD_FUNCTION(PACK__TOSTRING){
	DEBUGPRINTFUNC;

	LUA->CheckType(1, UD_TYPE_PACKET);
	Packet* p = GETPACK(1);
	
	char buff[256];
#ifdef _MSC_VER
	sprintf_s(buff, "brompacket{i:%d,o:%d}", p->InSize, p->OutSize);
#else
	sprintf(buff, "brompacket{i:%d,o:%d}", p->InSize, p->OutSize);
#endif

	LUA->PushString(buff);

	return 1;
}

GMOD_FUNCTION(PACK__EQ){
	DEBUGPRINTFUNC;

	LUA->CheckType(1, UD_TYPE_PACKET);
	LUA->CheckType(2, UD_TYPE_PACKET);

	LUA->PushBool(GETPACK(1) == GETPACK(2));

	return 1;
}

GMOD_FUNCTION(PACK__GC){
	DEBUGPRINTFUNC;

	LUA->CheckType(1, UD_TYPE_PACKET);
	Packet* p = GETPACK(1);

	p->RefCount--;
	if (p->RefCount == 0){
		delete p;
	}

	return 0;
}

GMOD_FUNCTION(PACK_WRITEByte){ DEBUGPRINTFUNC; LUA->CheckType(1, UD_TYPE_PACKET); (GETPACK(1))->WriteByte((unsigned char)LUA->GetNumber(2)); return 0; }
GMOD_FUNCTION(PACK_WRITEShort){ DEBUGPRINTFUNC; LUA->CheckType(1, UD_TYPE_PACKET); (GETPACK(1))->WriteShort((short)LUA->GetNumber(2)); return 0; }
GMOD_FUNCTION(PACK_WRITEUShort){ DEBUGPRINTFUNC; LUA->CheckType(1, UD_TYPE_PACKET); (GETPACK(1))->WriteUShort((unsigned short)LUA->GetNumber(2)); return 0; }
GMOD_FUNCTION(PACK_WRITEFloat){ DEBUGPRINTFUNC; LUA->CheckType(1, UD_TYPE_PACKET); (GETPACK(1))->WriteFloat((float)LUA->GetNumber(2)); return 0; }
GMOD_FUNCTION(PACK_WRITEInt){ DEBUGPRINTFUNC; LUA->CheckType(1, UD_TYPE_PACKET); (GETPACK(1))->WriteInt((int)LUA->GetNumber(2)); return 0; }
GMOD_FUNCTION(PACK_WRITEUInt){ DEBUGPRINTFUNC; LUA->CheckType(1, UD_TYPE_PACKET); (GETPACK(1))->WriteUInt((unsigned int)LUA->GetNumber(2)); return 0; }
GMOD_FUNCTION(PACK_WRITEDouble){ DEBUGPRINTFUNC; LUA->CheckType(1, UD_TYPE_PACKET); (GETPACK(1))->WriteDouble(LUA->GetNumber(2)); return 0; }
GMOD_FUNCTION(PACK_WRITELong){ DEBUGPRINTFUNC; LUA->CheckType(1, UD_TYPE_PACKET); (GETPACK(1))->WriteLong((long long)LUA->GetNumber(2)); return 0; }
GMOD_FUNCTION(PACK_WRITEULong){ DEBUGPRINTFUNC; LUA->CheckType(1, UD_TYPE_PACKET); (GETPACK(1))->WriteULong((unsigned long long)LUA->GetNumber(2)); return 0; }
GMOD_FUNCTION(PACK_WRITEString){ DEBUGPRINTFUNC; LUA->CheckType(1, UD_TYPE_PACKET); (GETPACK(1))->WriteString(LUA->GetString(2)); return 0; }
GMOD_FUNCTION(PACK_WRITEStringNT){ DEBUGPRINTFUNC; LUA->CheckType(1, UD_TYPE_PACKET); (GETPACK(1))->WriteStringNT(LUA->GetString(2)); return 0; }
GMOD_FUNCTION(PACK_WRITELine){ DEBUGPRINTFUNC; LUA->CheckType(1, UD_TYPE_PACKET); (GETPACK(1))->WriteLine(LUA->GetString(2)); return 0; }

GMOD_FUNCTION(PACK_READByte){ DEBUGPRINTFUNC; LUA->CheckType(1, UD_TYPE_PACKET); LUA->PushNumber((double)(GETPACK(1))->ReadByte()); return 1; }
GMOD_FUNCTION(PACK_READShort){ DEBUGPRINTFUNC; LUA->CheckType(1, UD_TYPE_PACKET); LUA->PushNumber((double)(GETPACK(1))->ReadShort()); return 1; }
GMOD_FUNCTION(PACK_READUShort){ DEBUGPRINTFUNC; LUA->CheckType(1, UD_TYPE_PACKET); LUA->PushNumber((double)(GETPACK(1))->ReadUShort()); return 1; }
GMOD_FUNCTION(PACK_READFloat){ DEBUGPRINTFUNC; LUA->CheckType(1, UD_TYPE_PACKET); LUA->PushNumber((double)(GETPACK(1))->ReadFloat()); return 1; }
GMOD_FUNCTION(PACK_READInt){ DEBUGPRINTFUNC; LUA->CheckType(1, UD_TYPE_PACKET); LUA->PushNumber((double)(GETPACK(1))->ReadInt()); return 1; }
GMOD_FUNCTION(PACK_READUInt){ DEBUGPRINTFUNC; LUA->CheckType(1, UD_TYPE_PACKET); LUA->PushNumber((double)(GETPACK(1))->ReadUInt()); return 1; }
GMOD_FUNCTION(PACK_READDouble){DEBUGPRINTFUNC;  LUA->CheckType(1, UD_TYPE_PACKET); LUA->PushNumber((double)(GETPACK(1))->ReadDouble()); return 1; }
GMOD_FUNCTION(PACK_READLong){ DEBUGPRINTFUNC; LUA->CheckType(1, UD_TYPE_PACKET); LUA->PushNumber((double)(GETPACK(1))->ReadLong()); return 1; }
GMOD_FUNCTION(PACK_READULong){ DEBUGPRINTFUNC; LUA->CheckType(1, UD_TYPE_PACKET); LUA->PushNumber((double)(GETPACK(1))->ReadULong()); return 1; }
GMOD_FUNCTION(PACK_READString){ DEBUGPRINTFUNC; LUA->CheckType(1, UD_TYPE_PACKET); LUA->PushString((GETPACK(1))->ReadString(LUA->IsType(2, GarrysMod::Lua::Type::NUMBER) ? (int)LUA->GetNumber(2) : -1)); return 1; }
GMOD_FUNCTION(PACK_READStringNT){ DEBUGPRINTFUNC; LUA->CheckType(1, UD_TYPE_PACKET); LUA->PushString((GETPACK(1))->ReadStringNT()); return 1; }
GMOD_FUNCTION(PACK_READStringAll){ DEBUGPRINTFUNC; LUA->CheckType(1, UD_TYPE_PACKET); LUA->PushString((GETPACK(1))->ReadStringAll()); return 1; }
GMOD_FUNCTION(PACK_READLine){ DEBUGPRINTFUNC; LUA->CheckType(1, UD_TYPE_PACKET); LUA->PushString((GETPACK(1))->ReadUntil("\r\n")); return 1; }
GMOD_FUNCTION(PACK_READUntil){ DEBUGPRINTFUNC; LUA->CheckType(1, UD_TYPE_PACKET); LUA->PushString((GETPACK(1))->ReadUntil((char*)LUA->GetString(2))); return 1; }

GMOD_FUNCTION(PACK_InSize){ DEBUGPRINTFUNC; LUA->CheckType(1, UD_TYPE_PACKET); LUA->PushNumber((double)(GETPACK(1))->InSize); return 1; }
GMOD_FUNCTION(PACK_InPos){ DEBUGPRINTFUNC; LUA->CheckType(1, UD_TYPE_PACKET); LUA->PushNumber((double)(GETPACK(1))->InPos); return 1; }
GMOD_FUNCTION(PACK_OutSize){ DEBUGPRINTFUNC; LUA->CheckType(1, UD_TYPE_PACKET); LUA->PushNumber((double)(GETPACK(1))->OutSize); return 1; }
GMOD_FUNCTION(PACK_OutPos){ DEBUGPRINTFUNC; LUA->CheckType(1, UD_TYPE_PACKET); LUA->PushNumber((double)(GETPACK(1))->OutPos); return 1; }
GMOD_FUNCTION(PACK_CLEAR){ DEBUGPRINTFUNC; LUA->CheckType(1, UD_TYPE_PACKET); (GETPACK(1))->Clear(); return 0; }

GMOD_FUNCTION(PACK_IsValid){ DEBUGPRINTFUNC; LUA->CheckType(1, UD_TYPE_PACKET); LUA->PushBool((GETPACK(1))->Valid); return 1; }
GMOD_FUNCTION(SOCK_IsValid){ DEBUGPRINTFUNC; LUA->CheckType(1, UD_TYPE_SOCKET); LUA->PushBool((GETSOCK(1))->Sock->Valid); return 1; }
GMOD_FUNCTION(SOCK_GetIP){ DEBUGPRINTFUNC; LUA->CheckType(1, UD_TYPE_SOCKET); LUA->PushString(inet_ntoa((GETSOCK(1))->Sock->addr.sin_addr)); return 1; }
GMOD_FUNCTION(SOCK_GetPort){ DEBUGPRINTFUNC; LUA->CheckType(1, UD_TYPE_SOCKET); LUA->PushNumber(ntohs((GETSOCK(1))->Sock->addr.sin_port)); return 1; }
GMOD_FUNCTION(SOCK_GetState){ DEBUGPRINTFUNC; LUA->CheckType(1, UD_TYPE_SOCKET); LUA->PushNumber((GETSOCK(1))->Sock->state); return 1; }

GMOD_MODULE_OPEN(){
	DEBUGPRINTFUNC;

	LUA->CreateTable();
		ADDFUNC("SetBlocking", SOCK_SetBlocking);
		ADDFUNC("Connect", SOCK_Connect);
		ADDFUNC("Close", SOCK_Disconnect);
		ADDFUNC("Disconnect", SOCK_Disconnect);
		ADDFUNC("Listen", SOCK_Listen);
		ADDFUNC("Send", SOCK_Send);
		ADDFUNC("Accept", SOCK_Accept);
		ADDFUNC("Receive", SOCK_Receive);
		ADDFUNC("ReceiveUntil", SOCK_ReceiveUntil);
		ADDFUNC("GetIP", SOCK_GetIP);
		ADDFUNC("GetPort", SOCK_GetPort);
		ADDFUNC("GetState", SOCK_GetState);
		ADDFUNC("AddWorker", SOCK_AddWorker);
		ADDFUNC("SetTimeout", SOCK_SetTimeout);
		ADDFUNC("SetCallbackReceive", SOCK_CALLBACKReceive);
		ADDFUNC("SetCallbackSend", SOCK_CALLBACKSend);
		ADDFUNC("SetCallbackConnect", SOCK_CALLBACKConnect);
		ADDFUNC("SetCallbackAccept", SOCK_CALLBACKAccept);
		ADDFUNC("SetCallbackDisconnect", SOCK_CALLBACKDisconnect);
	int socktableref = LUA->ReferenceCreate();

	LUA->CreateTable();
		ADDFUNC("WriteByte", PACK_WRITEByte);
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
		ADDFUNC("WriteLine", PACK_WRITELine);
		ADDFUNC("ReadByte", PACK_READByte);
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
		ADDFUNC("Clear", PACK_CLEAR);
	int packtableref = LUA->ReferenceCreate();

	LUA->PushSpecial(GarrysMod::Lua::SPECIAL_GLOB);
	LUA->PushString("BromSock");
	LUA->PushCFunction(CreateSocket);
	LUA->SetTable(-3);

	LUA->PushSpecial(GarrysMod::Lua::SPECIAL_GLOB);
	LUA->PushString("BromPacket");
	LUA->PushCFunction(CreatePacket);
	LUA->SetTable(-3);

	LUA->CreateTable();
		LUA->ReferencePush(packtableref); 
		LUA->SetField(-2, "__index");
		LUA->ReferencePush(packtableref);
		LUA->SetField(-2, "__newindex");
		ADDFUNC("__tostring", PACK__TOSTRING);
		ADDFUNC("__eq", PACK__EQ);
		ADDFUNC("__gc", PACK__GC);
	PacketRef = LUA->ReferenceCreate();

	LUA->CreateTable();
		LUA->ReferencePush(socktableref);
		LUA->SetField(-2, "__index");
		LUA->ReferencePush(socktableref);
		LUA->SetField(-2, "__newindex");
		ADDFUNC("__tostring", SOCK__TOSTRING);
		ADDFUNC("__eq", SOCK__EQ);
		ADDFUNC("__gc", SOCK__GC);

	SocketRef = LUA->ReferenceCreate();

	LUA->PushSpecial(GarrysMod::Lua::SPECIAL_GLOB);
	LUA->GetField(-1, "hook");
	LUA->GetField(-1, "Add");
	LUA->PushString("Tick");
	LUA->PushString("BS::TICK");
	LUA->PushCFunction(ThinkHook);
	LUA->Call(3, 0);
	LUA->Pop();
	
	// clean up references
	LUA->ReferenceFree(socktableref);
	LUA->ReferenceFree(packtableref);

	return 0;
}

GMOD_MODULE_CLOSE(){
	DEBUGPRINTFUNC;

	// reset to original state. Just to be sure.
	AllocatedSockets.clear();

	LUA->ReferenceFree(SocketRef);
	LUA->ReferenceFree(PacketRef);

	return 0;
}