#ifndef __H_GMBSOCK_DEFINES
#define __H_GMBSOCK_DEFINES

#define GMMODULE

#define GMOD_FUNCTION(fn) int fn(lua_State* state)
#define GETSOCK(num) (SockWrapper*)((GarrysMod::Lua::UserData*)LUA->GetUserdata(num))->data
#define GETPACK(num) (Packet*)((GarrysMod::Lua::UserData*)LUA->GetUserdata(num))->data
#define ADDFUNC(fn, f) LUA->PushCFunction(f); LUA->SetField(-2, fn);
#define CALLLUAFUNC(args) LUA->Call(args, 0);
#define UD_TYPE_SOCKET 122
#define UD_TYPE_PACKET 123

#ifdef _DEBUG
#define DEBUGPRINTFUNC LUA->PushSpecial(GarrysMod::Lua::SPECIAL_GLOB); LUA->GetField(-1, "print"); char dbuff[256]; sprintf_s(dbuff, "BS: CURF: %s", __FUNCTION__); LUA->PushString(dbuff); LUA->Call(1, 0); LUA->Pop();
#define DEBUGPRINT(msg) LUA->PushSpecial(GarrysMod::Lua::SPECIAL_GLOB); LUA->GetField(-1, "print"); LUA->PushString("BS: DEBUG:"); LUA->PushString(msg); LUA->Call(2, 0); LUA->Pop();
#else
#define DEBUGPRINTFUNC
#define DEBUGPRINT(msg)
#endif

#ifndef _MSC_VER
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#endif

#endif