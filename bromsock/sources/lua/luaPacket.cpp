#include <bromsock/lua/luaPacket.h>

#include <bromsock/engine.h>
#include <bromsock/packet.h>

#include <stdio.h>

namespace bromsock
{
    namespace lua
    {
        LUA_FUNCTION(CreatePacket)
        {
            DEBUGPRINTFUNC;

            Packet *p = new Packet();

            GarrysMod::Lua::UserData *ud = (GarrysMod::Lua::UserData *)LUA->NewUserdata(sizeof(GarrysMod::Lua::UserData));
            ud->data = p;
            ud->type = UD_TYPE_PACKET;

            auto& e = Engine::getInstance(state);
            LUA->ReferencePush(e.packetRef);
            LUA->SetMetaTable(-2);

            return 1;
        }

        LUA_FUNCTION(PACK__TOSTRING)
        {
            DEBUGPRINTFUNC;

            LUA->CheckType(1, UD_TYPE_PACKET);
            Packet *p = GETPACK(1);

            char buff[256];
#ifdef _MSC_VER
            sprintf_s(buff, "brompacket{i:%d,o:%d}", p->InSize, p->OutSize);
#else
            sprintf(buff, "brompacket{i:%d,o:%d}", p->InSize, p->OutSize);
#endif

            LUA->PushString(buff);

            return 1;
        }

        LUA_FUNCTION(PACK__EQ)
        {
            DEBUGPRINTFUNC;

            LUA->CheckType(1, UD_TYPE_PACKET);
            LUA->CheckType(2, UD_TYPE_PACKET);

            LUA->PushBool(GETPACK(1) == GETPACK(2));

            return 1;
        }

        LUA_FUNCTION(PACK__GC)
        {
            DEBUGPRINTFUNC;

            LUA->CheckType(1, UD_TYPE_PACKET);
            Packet *p = GETPACK(1);

            delete p;
            return 0;
        }

        LUA_FUNCTION(PACK_SetEndian)
        {
            DEBUGPRINTFUNC;
            LUA->CheckType(1, UD_TYPE_PACKET);
            LUA->CheckType(2, GarrysMod::Lua::Type::Number);
            (GETPACK(1))->EndianType = (unsigned char)LUA->GetNumber(2);
            return 0;
        }
        LUA_FUNCTION(PACK_WRITEByte)
        {
            DEBUGPRINTFUNC;
            LUA->CheckType(1, UD_TYPE_PACKET);
            LUA->CheckType(2, GarrysMod::Lua::Type::Number);
            (GETPACK(1))->WriteByte((unsigned char)LUA->GetNumber(2));
            return 0;
        }
        LUA_FUNCTION(PACK_WRITESByte)
        {
            DEBUGPRINTFUNC;
            LUA->CheckType(1, UD_TYPE_PACKET);
            LUA->CheckType(2, GarrysMod::Lua::Type::Number);
            (GETPACK(1))->WriteByte((unsigned char)(LUA->GetNumber(2) + 128));
            return 0;
        }
        LUA_FUNCTION(PACK_WRITEShort)
        {
            DEBUGPRINTFUNC;
            LUA->CheckType(1, UD_TYPE_PACKET);
            LUA->CheckType(2, GarrysMod::Lua::Type::Number);
            (GETPACK(1))->WriteShort((short)LUA->GetNumber(2));
            return 0;
        }
        LUA_FUNCTION(PACK_WRITEUShort)
        {
            DEBUGPRINTFUNC;
            LUA->CheckType(1, UD_TYPE_PACKET);
            LUA->CheckType(2, GarrysMod::Lua::Type::Number);
            (GETPACK(1))->WriteUShort((unsigned short)LUA->GetNumber(2));
            return 0;
        }
        LUA_FUNCTION(PACK_WRITEFloat)
        {
            DEBUGPRINTFUNC;
            LUA->CheckType(1, UD_TYPE_PACKET);
            LUA->CheckType(2, GarrysMod::Lua::Type::Number);
            (GETPACK(1))->WriteFloat((float)LUA->GetNumber(2));
            return 0;
        }
        LUA_FUNCTION(PACK_WRITEInt)
        {
            DEBUGPRINTFUNC;
            LUA->CheckType(1, UD_TYPE_PACKET);
            LUA->CheckType(2, GarrysMod::Lua::Type::Number);
            (GETPACK(1))->WriteInt((int)LUA->GetNumber(2));
            return 0;
        }
        LUA_FUNCTION(PACK_WRITEUInt)
        {
            DEBUGPRINTFUNC;
            LUA->CheckType(1, UD_TYPE_PACKET);
            LUA->CheckType(2, GarrysMod::Lua::Type::Number);
            (GETPACK(1))->WriteUInt((unsigned int)LUA->GetNumber(2));
            return 0;
        }
        LUA_FUNCTION(PACK_WRITEDouble)
        {
            DEBUGPRINTFUNC;
            LUA->CheckType(1, UD_TYPE_PACKET);
            LUA->CheckType(2, GarrysMod::Lua::Type::Number);
            (GETPACK(1))->WriteDouble(LUA->GetNumber(2));
            return 0;
        }
        LUA_FUNCTION(PACK_WRITELong)
        {
            DEBUGPRINTFUNC;
            LUA->CheckType(1, UD_TYPE_PACKET);
            LUA->CheckType(2, GarrysMod::Lua::Type::Number);
            (GETPACK(1))->WriteLong((long long)LUA->GetNumber(2));
            return 0;
        }
        LUA_FUNCTION(PACK_WRITEULong)
        {
            DEBUGPRINTFUNC;
            LUA->CheckType(1, UD_TYPE_PACKET);
            LUA->CheckType(2, GarrysMod::Lua::Type::Number);
            (GETPACK(1))->WriteULong((unsigned long long)LUA->GetNumber(2));
            return 0;
        }
        LUA_FUNCTION(PACK_WRITEString)
        {
            DEBUGPRINTFUNC;
            LUA->CheckType(1, UD_TYPE_PACKET);
            LUA->CheckType(2, GarrysMod::Lua::Type::String);
            (GETPACK(1))->WriteString(LUA->GetString(2));
            return 0;
        }
        LUA_FUNCTION(PACK_WRITEStringNT)
        {
            DEBUGPRINTFUNC;
            LUA->CheckType(1, UD_TYPE_PACKET);
            LUA->CheckType(2, GarrysMod::Lua::Type::String);
            (GETPACK(1))->WriteStringNT(LUA->GetString(2));
            return 0;
        }
        LUA_FUNCTION(PACK_WRITELine)
        {
            DEBUGPRINTFUNC;
            LUA->CheckType(1, UD_TYPE_PACKET);
            LUA->CheckType(2, GarrysMod::Lua::Type::String);
            (GETPACK(1))->WriteLine(LUA->GetString(2));
            return 0;
        }
        LUA_FUNCTION(PACK_WRITEPacket)
        {
            DEBUGPRINTFUNC;
            LUA->CheckType(1, UD_TYPE_PACKET);
            LUA->CheckType(2, UD_TYPE_PACKET);
            if ((GETPACK(2))->OutBuffer == nullptr)
                return 0;
            (GETPACK(1))->WriteBytes((GETPACK(2))->OutBuffer, (GETPACK(2))->OutPos, false);
            return 0;
        }

        LUA_FUNCTION(PACK_WRITEStringRaw)
        {
            DEBUGPRINTFUNC;
            LUA->CheckType(1, UD_TYPE_PACKET);
            LUA->CheckType(2, GarrysMod::Lua::Type::String);

            unsigned int strlen = 0;
            const char *str = LUA->GetString(2, &strlen);
            (GETPACK(1))->WriteStringRaw(str, strlen);

            return 0;
        }

        LUA_FUNCTION(PACK_READByte)
        {
            DEBUGPRINTFUNC;
            LUA->CheckType(1, UD_TYPE_PACKET);
            LUA->PushNumber((double)(GETPACK(1))->ReadByte());
            return 1;
        }
        LUA_FUNCTION(PACK_READSByte)
        {
            DEBUGPRINTFUNC;
            LUA->CheckType(1, UD_TYPE_PACKET);
            LUA->PushNumber((double)((GETPACK(1))->ReadByte() - 128));
            return 1;
        }
        LUA_FUNCTION(PACK_READShort)
        {
            DEBUGPRINTFUNC;
            LUA->CheckType(1, UD_TYPE_PACKET);
            LUA->PushNumber((double)(GETPACK(1))->ReadShort());
            return 1;
        }
        LUA_FUNCTION(PACK_READUShort)
        {
            DEBUGPRINTFUNC;
            LUA->CheckType(1, UD_TYPE_PACKET);
            LUA->PushNumber((double)(GETPACK(1))->ReadUShort());
            return 1;
        }
        LUA_FUNCTION(PACK_READFloat)
        {
            DEBUGPRINTFUNC;
            LUA->CheckType(1, UD_TYPE_PACKET);
            LUA->PushNumber((double)(GETPACK(1))->ReadFloat());
            return 1;
        }
        LUA_FUNCTION(PACK_READInt)
        {
            DEBUGPRINTFUNC;
            LUA->CheckType(1, UD_TYPE_PACKET);
            LUA->PushNumber((double)(GETPACK(1))->ReadInt());
            return 1;
        }
        LUA_FUNCTION(PACK_READUInt)
        {
            DEBUGPRINTFUNC;
            LUA->CheckType(1, UD_TYPE_PACKET);
            LUA->PushNumber((double)(GETPACK(1))->ReadUInt());
            return 1;
        }
        LUA_FUNCTION(PACK_READDouble)
        {
            DEBUGPRINTFUNC;
            LUA->CheckType(1, UD_TYPE_PACKET);
            LUA->PushNumber((double)(GETPACK(1))->ReadDouble());
            return 1;
        }
        LUA_FUNCTION(PACK_READLong)
        {
            DEBUGPRINTFUNC;
            LUA->CheckType(1, UD_TYPE_PACKET);
            LUA->PushNumber((double)(GETPACK(1))->ReadLong());
            return 1;
        }
        LUA_FUNCTION(PACK_READULong)
        {
            DEBUGPRINTFUNC;
            LUA->CheckType(1, UD_TYPE_PACKET);
            LUA->PushNumber((double)(GETPACK(1))->ReadULong());
            return 1;
        }

        LUA_FUNCTION(PACK_READStringNT)
        {
            DEBUGPRINTFUNC;
            LUA->CheckType(1, UD_TYPE_PACKET);

            unsigned int outlen = 0;
            char *str = (GETPACK(1))->ReadStringNT(&outlen);

            LUA->PushString(str, outlen);

            delete[] str;
            return 1;
        }

        LUA_FUNCTION(PACK_READStringAll)
        {
            DEBUGPRINTFUNC;
            LUA->CheckType(1, UD_TYPE_PACKET);

            unsigned int outlen = 0;
            char *str = (GETPACK(1))->ReadStringAll(&outlen);
            LUA->PushString(str, outlen);

            delete[] str;
            return 1;
        }

        LUA_FUNCTION(PACK_READLine)
        {
            DEBUGPRINTFUNC;
            LUA->CheckType(1, UD_TYPE_PACKET);

            unsigned int outlen = 0;
            char *str = (GETPACK(1))->ReadUntil("\r\n", 2, &outlen);
            LUA->PushString(str, outlen);

            delete[] str;
            return 1;
        }

        LUA_FUNCTION(PACK_READUntil)
        {
            DEBUGPRINTFUNC;
            LUA->CheckType(1, UD_TYPE_PACKET);
            LUA->CheckType(2, GarrysMod::Lua::Type::String);

            unsigned int lualen = 0;
            char *luastr = (char *)LUA->GetString(2, &lualen);

            unsigned int outlen = 0;
            char *str = (GETPACK(1))->ReadUntil(luastr, lualen, &outlen);
            LUA->PushString(str, outlen);

            delete[] str;
            return 1;
        }

        LUA_FUNCTION(PACK_READString)
        {
            DEBUGPRINTFUNC;
            LUA->CheckType(1, UD_TYPE_PACKET);

            int len = LUA->IsType(2, GarrysMod::Lua::Type::Number) ? (int)LUA->GetNumber(2) : -1;
            char *str = (GETPACK(1))->ReadString(len);
            LUA->PushString(str, len == -1 ? 0U : len);

            delete[] str;
            return 1;
        }

        LUA_FUNCTION(PACK_InSize)
        {
            DEBUGPRINTFUNC;
            LUA->CheckType(1, UD_TYPE_PACKET);
            LUA->PushNumber((double)(GETPACK(1))->InSize);
            return 1;
        }
        LUA_FUNCTION(PACK_InPos)
        {
            DEBUGPRINTFUNC;
            LUA->CheckType(1, UD_TYPE_PACKET);
            LUA->PushNumber((double)(GETPACK(1))->InPos);
            return 1;
        }
        LUA_FUNCTION(PACK_OutSize)
        {
            DEBUGPRINTFUNC;
            LUA->CheckType(1, UD_TYPE_PACKET);
            LUA->PushNumber((double)(GETPACK(1))->OutSize);
            return 1;
        }
        LUA_FUNCTION(PACK_OutPos)
        {
            DEBUGPRINTFUNC;
            LUA->CheckType(1, UD_TYPE_PACKET);
            LUA->PushNumber((double)(GETPACK(1))->OutPos);
            return 1;
        }
        LUA_FUNCTION(PACK_Clear)
        {
            DEBUGPRINTFUNC;
            LUA->CheckType(1, UD_TYPE_PACKET);
            (GETPACK(1))->Clear();
            return 0;
        }
        LUA_FUNCTION(PACK_IsValid)
        {
            DEBUGPRINTFUNC;
            LUA->CheckType(1, UD_TYPE_PACKET);
            LUA->PushBool((GETPACK(1))->Valid);
            return 1;
        }

        LUA_FUNCTION(PACK_Copy)
        {
            DEBUGPRINTFUNC;
            LUA->CheckType(1, UD_TYPE_PACKET);

            Packet *source = GETPACK(1);

            Packet *p = new Packet();

            GarrysMod::Lua::UserData *ud = (GarrysMod::Lua::UserData *)LUA->NewUserdata(sizeof(GarrysMod::Lua::UserData));
            ud->data = p;
            ud->type = UD_TYPE_PACKET;

            auto& engine = Engine::getInstance(state);
            LUA->ReferencePush(engine.packetRef);
            LUA->SetMetaTable(-2);

            if (source->OutBuffer != nullptr)
            {
                // Not sure if we should copy the entire out buffer (including additional allocated space)
                // or just the actual data, I think the actual data is more efficient for normal use.

                // if you'd like to have the space too, use OutSize, rather than OutPos
                p->OutBuffer = new unsigned char[source->OutPos];
                memcpy(p->OutBuffer, source->OutBuffer, source->OutPos);

                p->OutPos = source->OutPos;
                p->OutSize = source->OutPos; // this one too
            }

            if (source->InBuffer != nullptr)
            {
                p->InBuffer = new unsigned char[source->InSize];
                memcpy(p->InBuffer, source->InBuffer, source->InSize);

                p->InPos = source->InPos;
                p->InSize = source->InSize;
            }

            return 1;
        }

        LUA_FUNCTION(PACK_RegisterTypes)
        {
            DEBUGPRINTFUNC;
            auto& engine = Engine::getInstance(state);

            // register our class
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
            auto tempTableRef = LUA->ReferenceCreate();

            LUA->CreateTable();
                LUA->ReferencePush(tempTableRef);
                LUA->SetField(-2, "__index");
                LUA->ReferencePush(tempTableRef);
                LUA->SetField(-2, "__newindex");
                ADDFUNC("__tostring", lua::PACK__TOSTRING);
                ADDFUNC("__eq", lua::PACK__EQ);
                ADDFUNC("__gc", lua::PACK__GC);
            engine.packetRef = LUA->ReferenceCreate();

            // clear temp
            LUA->ReferenceFree(tempTableRef);

            // register our globals
            LUA->PushSpecial(GarrysMod::Lua::SPECIAL_GLOB);
                LUA->PushString("BromPacket");
                LUA->PushCFunction(CreatePacket);
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

            return 0;
        }
    }
}