#pragma once

#define LUA_FUNCTION(fn) int fn(lua_State* state)
#define GETSOCK(num) reinterpret_cast<bromsock::Socket*>(((GarrysMod::Lua::UserData*)LUA->GetUserdata(num))->data)
#define GETPACK(num) reinterpret_cast<bromsock::Packet*>(((GarrysMod::Lua::UserData*)LUA->GetUserdata(num))->data)
#define ADDFUNC(fn, f) LUA->PushCFunction(f); LUA->SetField(-2, fn);
#define CALLLUAFUNC(args) LUA->Call(args, 0);
#define UD_TYPE_SOCKET 122
#define UD_TYPE_PACKET 123
//#define LUA (state->luabase)


#define LUA_FUNCTION_STATIC( FUNC )                          \
    static int FUNC##__Imp( GarrysMod::Lua::ILuaBase* LUA ); \
    static int FUNC( lua_State* L )                          \
    {                                                 \
        GarrysMod::Lua::ILuaBase* LUA = L->luabase;   \
        LUA->SetState(L);                             \
        return FUNC##__Imp( LUA );                    \
    }                                                 \
    static int FUNC##__Imp( GarrysMod::Lua::ILuaBase* LUA )


#ifdef _DEBUG
#define DEBUGPRINTFUNC LUA->PushSpecial(GarrysMod::Lua::SPECIAL_GLOB); LUA->GetField(-1, "print"); char dbuff[256]; sprintf_s(dbuff, "BS: CURF: %s", __FUNCTION__); LUA->PushString(dbuff); LUA->Call(1, 0); LUA->Pop();
#define DEBUGPRINT(msg) LUA->PushSpecial(GarrysMod::Lua::SPECIAL_GLOB); LUA->GetField(-1, "print"); LUA->PushString("BS: DEBUG:"); LUA->PushString(msg); LUA->Call(2, 0); LUA->Pop();
#else
#define DEBUGPRINTFUNC
#define DEBUGPRINT(msg)
#endif
