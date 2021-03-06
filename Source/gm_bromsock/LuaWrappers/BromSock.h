#ifndef __H_GMBSOCK_L_BROMSOCK
#define __H_GMBSOCK_L_BROMSOCK

#include "../Defines.h"
#include "../Gmod_Headers/Lua/Interface.h"

namespace GMBSOCK {
	GMOD_FUNCTION(CreateSocket);
	GMOD_FUNCTION(SOCK__EQ);
	GMOD_FUNCTION(SOCK__GC);
	GMOD_FUNCTION(SOCK__TOSTRING);

	GMOD_FUNCTION(SOCK_Connect);
	GMOD_FUNCTION(SOCK_Listen);
	GMOD_FUNCTION(SOCK_Bind);
	GMOD_FUNCTION(SOCK_SetBlocking);
	GMOD_FUNCTION(SOCK_Disconnect);
	GMOD_FUNCTION(SOCK_SendTo);
	GMOD_FUNCTION(SOCK_Send);
	GMOD_FUNCTION(SOCK_Receive);
	GMOD_FUNCTION(SOCK_ReceiveFrom);
	GMOD_FUNCTION(SOCK_ReceiveUntil);
	GMOD_FUNCTION(SOCK_Accept);
	GMOD_FUNCTION(SOCK_AddWorker);
	GMOD_FUNCTION(SOCK_SetOption);
	GMOD_FUNCTION(SOCK_SetTimeout);
	GMOD_FUNCTION(SOCK_Create);
	GMOD_FUNCTION(SOCK_StartSSLClient);
	GMOD_FUNCTION(SOCK_SetMaxReceiveSize);

	GMOD_FUNCTION(SOCK_CALLBACKSend);
	GMOD_FUNCTION(SOCK_CALLBACKSendTo);
	GMOD_FUNCTION(SOCK_CALLBACKReceive);
	GMOD_FUNCTION(SOCK_CALLBACKReceiveFrom);
	GMOD_FUNCTION(SOCK_CALLBACKConnect);
	GMOD_FUNCTION(SOCK_CALLBACKAccept);
	GMOD_FUNCTION(SOCK_CALLBACKDisconnect);

	GMOD_FUNCTION(SOCK_IsValid);
	GMOD_FUNCTION(SOCK_GetIP);
	GMOD_FUNCTION(SOCK_GetPort);
	GMOD_FUNCTION(SOCK_GetState);
	GMOD_FUNCTION(SOCK_GetLastError);
}

#endif