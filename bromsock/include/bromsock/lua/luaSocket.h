#pragma once

#include <bromsock/socket.h>
#include <bromsock/defines.h>

#include <GarrysMod/Lua/Interface.h>

namespace bromsock
{
    namespace lua
    {
        LUA_FUNCTION(CreateSocket);
        LUA_FUNCTION(SOCK__EQ);
        LUA_FUNCTION(SOCK__GC);
        LUA_FUNCTION(SOCK__TOSTRING);

        LUA_FUNCTION(SOCK_Connect);
        LUA_FUNCTION(SOCK_Listen);
        LUA_FUNCTION(SOCK_Bind);
        LUA_FUNCTION(SOCK_SetBlocking);
        LUA_FUNCTION(SOCK_Disconnect);
        LUA_FUNCTION(SOCK_SendTo);
        LUA_FUNCTION(SOCK_Send);
        LUA_FUNCTION(SOCK_Receive);
        LUA_FUNCTION(SOCK_ReceiveFrom);
        LUA_FUNCTION(SOCK_ReceiveUntil);
        LUA_FUNCTION(SOCK_Accept);
        LUA_FUNCTION(SOCK_AddWorker);
        LUA_FUNCTION(SOCK_SetOption);
        LUA_FUNCTION(SOCK_SetTimeout);
        LUA_FUNCTION(SOCK_Create);
        LUA_FUNCTION(SOCK_StartSSLClient);
        LUA_FUNCTION(SOCK_SetMaxReceiveSize);

        LUA_FUNCTION(SOCK_CALLBACKSend);
        LUA_FUNCTION(SOCK_CALLBACKSendTo);
        LUA_FUNCTION(SOCK_CALLBACKReceive);
        LUA_FUNCTION(SOCK_CALLBACKReceiveFrom);
        LUA_FUNCTION(SOCK_CALLBACKConnect);
        LUA_FUNCTION(SOCK_CALLBACKAccept);
        LUA_FUNCTION(SOCK_CALLBACKDisconnect);

        LUA_FUNCTION(SOCK_IsValid);
        LUA_FUNCTION(SOCK_GetIP);
        LUA_FUNCTION(SOCK_GetPort);
        LUA_FUNCTION(SOCK_GetState);
        LUA_FUNCTION(SOCK_GetLastError);

        LUA_FUNCTION(SOCK_RegisterTypes);
    }
}
