#pragma once

#include <GarrysMod/Lua/Interface.h>
#include <bromsock/defines.h>

namespace bromsock
{
    namespace lua
    {
        LUA_FUNCTION(CreatePacket);

        LUA_FUNCTION(PACK__TOSTRING);
        LUA_FUNCTION(PACK__EQ);
        LUA_FUNCTION(PACK__GC);

        LUA_FUNCTION(PACK_InSize);
        LUA_FUNCTION(PACK_InPos);
        LUA_FUNCTION(PACK_OutSize);
        LUA_FUNCTION(PACK_OutPos);
        LUA_FUNCTION(PACK_Clear);
        LUA_FUNCTION(PACK_Copy);
        LUA_FUNCTION(PACK_SetEndian);
        LUA_FUNCTION(PACK_IsValid);

        LUA_FUNCTION(PACK_WRITEByte);
        LUA_FUNCTION(PACK_WRITESByte);
        LUA_FUNCTION(PACK_WRITEShort);
        LUA_FUNCTION(PACK_WRITEUShort);
        LUA_FUNCTION(PACK_WRITEFloat);
        LUA_FUNCTION(PACK_WRITEInt);
        LUA_FUNCTION(PACK_WRITEUInt);
        LUA_FUNCTION(PACK_WRITEDouble);
        LUA_FUNCTION(PACK_WRITELong);
        LUA_FUNCTION(PACK_WRITEULong);
        LUA_FUNCTION(PACK_WRITEString);
        LUA_FUNCTION(PACK_WRITEStringNT);
        LUA_FUNCTION(PACK_WRITELine);
        LUA_FUNCTION(PACK_WRITEPacket);
        LUA_FUNCTION(PACK_WRITEStringRaw);

        LUA_FUNCTION(PACK_READByte);
        LUA_FUNCTION(PACK_READSByte);
        LUA_FUNCTION(PACK_READShort);
        LUA_FUNCTION(PACK_READUShort);
        LUA_FUNCTION(PACK_READFloat);
        LUA_FUNCTION(PACK_READInt);
        LUA_FUNCTION(PACK_READUInt);
        LUA_FUNCTION(PACK_READDouble);
        LUA_FUNCTION(PACK_READLong);
        LUA_FUNCTION(PACK_READULong);
        LUA_FUNCTION(PACK_READStringNT);
        LUA_FUNCTION(PACK_READStringAll);
        LUA_FUNCTION(PACK_READLine);
        LUA_FUNCTION(PACK_READUntil);
        LUA_FUNCTION(PACK_READString);

        LUA_FUNCTION(PACK_RegisterTypes);
    }
}