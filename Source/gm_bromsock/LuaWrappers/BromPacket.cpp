#include "BromPacket.h"

#include "../Objects/BSEzSock.h"
#include "../Objects/BSPacket.h"
#include "../Objects/LockObject.h"
#include "../Objects/SockWrapper.h"
#include "../Objects/Engine.h"

namespace GMBSOCK {
	GMOD_FUNCTION(CreatePacket) {
		DEBUGPRINTFUNC;

		Packet* p = nullptr;

		if (LUA->IsType(1, UD_TYPE_SOCKET)) p = new Packet((GETSOCK(1))->Sock);
		else p = new Packet();

		GarrysMod::Lua::UserData* ud = (GarrysMod::Lua::UserData*)LUA->NewUserdata(sizeof(GarrysMod::Lua::UserData));
		ud->data = p;
		ud->type = UD_TYPE_PACKET;

		Engine* engine = Engine::GetEngineByState(state);
		LUA->ReferencePush(engine->PacketRef);
		LUA->SetMetaTable(-2);

		p->RefCount++;

		return 1;
	}

	GMOD_FUNCTION(PACK__TOSTRING) {
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

	GMOD_FUNCTION(PACK__EQ) {
		DEBUGPRINTFUNC;

		LUA->CheckType(1, UD_TYPE_PACKET);
		LUA->CheckType(2, UD_TYPE_PACKET);

		LUA->PushBool(GETPACK(1) == GETPACK(2));

		return 1;
	}

	GMOD_FUNCTION(PACK__GC) {
		DEBUGPRINTFUNC;

		LUA->CheckType(1, UD_TYPE_PACKET);
		Packet* p = GETPACK(1);

		p->RefCount--;
		if (p->RefCount == 0) {
			delete p;
		}

		return 0;
	}

	GMOD_FUNCTION(PACK_SetEndian) { DEBUGPRINTFUNC; LUA->CheckType(1, UD_TYPE_PACKET); LUA->CheckType(2, GarrysMod::Lua::Type::NUMBER); (GETPACK(1))->EndianType = (unsigned char)LUA->GetNumber(2); return 0; }
	GMOD_FUNCTION(PACK_WRITEByte) { DEBUGPRINTFUNC; LUA->CheckType(1, UD_TYPE_PACKET); LUA->CheckType(2, GarrysMod::Lua::Type::NUMBER); (GETPACK(1))->WriteByte((unsigned char)LUA->GetNumber(2)); return 0; }
	GMOD_FUNCTION(PACK_WRITESByte) { DEBUGPRINTFUNC; LUA->CheckType(1, UD_TYPE_PACKET); LUA->CheckType(2, GarrysMod::Lua::Type::NUMBER); (GETPACK(1))->WriteByte((unsigned char)(LUA->GetNumber(2) + 128)); return 0; }
	GMOD_FUNCTION(PACK_WRITEShort) { DEBUGPRINTFUNC; LUA->CheckType(1, UD_TYPE_PACKET); LUA->CheckType(2, GarrysMod::Lua::Type::NUMBER); (GETPACK(1))->WriteShort((short)LUA->GetNumber(2)); return 0; }
	GMOD_FUNCTION(PACK_WRITEUShort) { DEBUGPRINTFUNC; LUA->CheckType(1, UD_TYPE_PACKET); LUA->CheckType(2, GarrysMod::Lua::Type::NUMBER); (GETPACK(1))->WriteUShort((unsigned short)LUA->GetNumber(2)); return 0; }
	GMOD_FUNCTION(PACK_WRITEFloat) { DEBUGPRINTFUNC; LUA->CheckType(1, UD_TYPE_PACKET); LUA->CheckType(2, GarrysMod::Lua::Type::NUMBER); (GETPACK(1))->WriteFloat((float)LUA->GetNumber(2)); return 0; }
	GMOD_FUNCTION(PACK_WRITEInt) { DEBUGPRINTFUNC; LUA->CheckType(1, UD_TYPE_PACKET); LUA->CheckType(2, GarrysMod::Lua::Type::NUMBER); (GETPACK(1))->WriteInt((int)LUA->GetNumber(2)); return 0; }
	GMOD_FUNCTION(PACK_WRITEUInt) { DEBUGPRINTFUNC; LUA->CheckType(1, UD_TYPE_PACKET); LUA->CheckType(2, GarrysMod::Lua::Type::NUMBER); (GETPACK(1))->WriteUInt((unsigned int)LUA->GetNumber(2)); return 0; }
	GMOD_FUNCTION(PACK_WRITEDouble) { DEBUGPRINTFUNC; LUA->CheckType(1, UD_TYPE_PACKET); LUA->CheckType(2, GarrysMod::Lua::Type::NUMBER); (GETPACK(1))->WriteDouble(LUA->GetNumber(2)); return 0; }
	GMOD_FUNCTION(PACK_WRITELong) { DEBUGPRINTFUNC; LUA->CheckType(1, UD_TYPE_PACKET); LUA->CheckType(2, GarrysMod::Lua::Type::NUMBER); (GETPACK(1))->WriteLong((long long)LUA->GetNumber(2)); return 0; }
	GMOD_FUNCTION(PACK_WRITEULong) { DEBUGPRINTFUNC; LUA->CheckType(1, UD_TYPE_PACKET); LUA->CheckType(2, GarrysMod::Lua::Type::NUMBER); (GETPACK(1))->WriteULong((unsigned long long)LUA->GetNumber(2)); return 0; }
	GMOD_FUNCTION(PACK_WRITEString) { DEBUGPRINTFUNC; LUA->CheckType(1, UD_TYPE_PACKET); LUA->CheckType(2, GarrysMod::Lua::Type::STRING); (GETPACK(1))->WriteString(LUA->GetString(2)); return 0; }
	GMOD_FUNCTION(PACK_WRITEStringNT) { DEBUGPRINTFUNC; LUA->CheckType(1, UD_TYPE_PACKET); LUA->CheckType(2, GarrysMod::Lua::Type::STRING); (GETPACK(1))->WriteStringNT(LUA->GetString(2)); return 0; }
	GMOD_FUNCTION(PACK_WRITELine) { DEBUGPRINTFUNC; LUA->CheckType(1, UD_TYPE_PACKET); LUA->CheckType(2, GarrysMod::Lua::Type::STRING); (GETPACK(1))->WriteLine(LUA->GetString(2)); return 0; }
	GMOD_FUNCTION(PACK_WRITEPacket) { DEBUGPRINTFUNC; LUA->CheckType(1, UD_TYPE_PACKET); LUA->CheckType(2, UD_TYPE_PACKET); if ((GETPACK(2))->OutBuffer == nullptr) return 0; (GETPACK(1))->WriteBytes((GETPACK(2))->OutBuffer, (GETPACK(2))->OutPos, false); return 0; }

	GMOD_FUNCTION(PACK_WRITEStringRaw) {
		DEBUGPRINTFUNC;
		LUA->CheckType(1, UD_TYPE_PACKET);
		LUA->CheckType(2, GarrysMod::Lua::Type::STRING);

		unsigned int strlen = 0;
		const char* str = LUA->GetString(2, &strlen);
		(GETPACK(1))->WriteStringRaw(str, strlen);

		return 0;
	}

	GMOD_FUNCTION(PACK_READByte) { DEBUGPRINTFUNC; LUA->CheckType(1, UD_TYPE_PACKET); LUA->PushNumber((double)(GETPACK(1))->ReadByte()); return 1; }
	GMOD_FUNCTION(PACK_READSByte) { DEBUGPRINTFUNC; LUA->CheckType(1, UD_TYPE_PACKET); LUA->PushNumber((double)((GETPACK(1))->ReadByte() - 128)); return 1; }
	GMOD_FUNCTION(PACK_READShort) { DEBUGPRINTFUNC; LUA->CheckType(1, UD_TYPE_PACKET); LUA->PushNumber((double)(GETPACK(1))->ReadShort()); return 1; }
	GMOD_FUNCTION(PACK_READUShort) { DEBUGPRINTFUNC; LUA->CheckType(1, UD_TYPE_PACKET); LUA->PushNumber((double)(GETPACK(1))->ReadUShort()); return 1; }
	GMOD_FUNCTION(PACK_READFloat) { DEBUGPRINTFUNC; LUA->CheckType(1, UD_TYPE_PACKET); LUA->PushNumber((double)(GETPACK(1))->ReadFloat()); return 1; }
	GMOD_FUNCTION(PACK_READInt) { DEBUGPRINTFUNC; LUA->CheckType(1, UD_TYPE_PACKET); LUA->PushNumber((double)(GETPACK(1))->ReadInt()); return 1; }
	GMOD_FUNCTION(PACK_READUInt) { DEBUGPRINTFUNC; LUA->CheckType(1, UD_TYPE_PACKET); LUA->PushNumber((double)(GETPACK(1))->ReadUInt()); return 1; }
	GMOD_FUNCTION(PACK_READDouble) { DEBUGPRINTFUNC;  LUA->CheckType(1, UD_TYPE_PACKET); LUA->PushNumber((double)(GETPACK(1))->ReadDouble()); return 1; }
	GMOD_FUNCTION(PACK_READLong) { DEBUGPRINTFUNC; LUA->CheckType(1, UD_TYPE_PACKET); LUA->PushNumber((double)(GETPACK(1))->ReadLong()); return 1; }
	GMOD_FUNCTION(PACK_READULong) { DEBUGPRINTFUNC; LUA->CheckType(1, UD_TYPE_PACKET); LUA->PushNumber((double)(GETPACK(1))->ReadULong()); return 1; }

	GMOD_FUNCTION(PACK_READStringNT) {
		DEBUGPRINTFUNC;
		LUA->CheckType(1, UD_TYPE_PACKET);

		unsigned int outlen = 0;
		char* str = (GETPACK(1))->ReadStringNT(&outlen);

		LUA->PushString(str, outlen);

		delete[] str;
		return 1;
	}

	GMOD_FUNCTION(PACK_READStringAll) {
		DEBUGPRINTFUNC;
		LUA->CheckType(1, UD_TYPE_PACKET);

		unsigned int outlen = 0;
		char* str = (GETPACK(1))->ReadStringAll(&outlen);
		LUA->PushString(str, outlen);

		delete[] str;
		return 1;
	}

	GMOD_FUNCTION(PACK_READLine) {
		DEBUGPRINTFUNC;
		LUA->CheckType(1, UD_TYPE_PACKET);

		unsigned int outlen = 0;
		char* str = (GETPACK(1))->ReadUntil("\r\n", 2, &outlen);
		LUA->PushString(str, outlen);

		delete[] str;
		return 1;
	}

	GMOD_FUNCTION(PACK_READUntil) {
		DEBUGPRINTFUNC;
		LUA->CheckType(1, UD_TYPE_PACKET);
		LUA->CheckType(2, GarrysMod::Lua::Type::STRING);


		unsigned int lualen = 0;
		char* luastr = (char*)LUA->GetString(2, &lualen);

		unsigned int outlen = 0;
		char* str = (GETPACK(1))->ReadUntil(luastr, lualen, &outlen);
		LUA->PushString(str, outlen);

		delete[] str;
		return 1;
	}

	GMOD_FUNCTION(PACK_READString) {
		DEBUGPRINTFUNC;
		LUA->CheckType(1, UD_TYPE_PACKET);

		int len = LUA->IsType(2, GarrysMod::Lua::Type::NUMBER) ? (int)LUA->GetNumber(2) : -1;
		char* str = (GETPACK(1))->ReadString(len);
		LUA->PushString(str, len == -1 ? 0U : len);

		delete[] str;
		return 1;
	}

	GMOD_FUNCTION(PACK_InSize) { DEBUGPRINTFUNC; LUA->CheckType(1, UD_TYPE_PACKET); LUA->PushNumber((double)(GETPACK(1))->InSize); return 1; }
	GMOD_FUNCTION(PACK_InPos) { DEBUGPRINTFUNC; LUA->CheckType(1, UD_TYPE_PACKET); LUA->PushNumber((double)(GETPACK(1))->InPos); return 1; }
	GMOD_FUNCTION(PACK_OutSize) { DEBUGPRINTFUNC; LUA->CheckType(1, UD_TYPE_PACKET); LUA->PushNumber((double)(GETPACK(1))->OutSize); return 1; }
	GMOD_FUNCTION(PACK_OutPos) { DEBUGPRINTFUNC; LUA->CheckType(1, UD_TYPE_PACKET); LUA->PushNumber((double)(GETPACK(1))->OutPos); return 1; }
	GMOD_FUNCTION(PACK_Clear) { DEBUGPRINTFUNC; LUA->CheckType(1, UD_TYPE_PACKET); (GETPACK(1))->Clear(); return 0; }
	GMOD_FUNCTION(PACK_IsValid) { DEBUGPRINTFUNC; LUA->CheckType(1, UD_TYPE_PACKET); LUA->PushBool((GETPACK(1))->Valid); return 1; }

	GMOD_FUNCTION(PACK_Copy) {
		DEBUGPRINTFUNC;
		LUA->CheckType(1, UD_TYPE_PACKET);

		Packet* source = GETPACK(1);

		Packet* p = new Packet(source->Sock);

		GarrysMod::Lua::UserData* ud = (GarrysMod::Lua::UserData*)LUA->NewUserdata(sizeof(GarrysMod::Lua::UserData));
		ud->data = p;
		ud->type = UD_TYPE_PACKET;

		Engine* engine = Engine::GetEngineByState(state);
		LUA->ReferencePush(engine->PacketRef);
		LUA->SetMetaTable(-2);

		p->RefCount++;

		if (source->OutBuffer != nullptr) {
			// Not sure if we should copy the entire out buffer (including additional allocated space)
			// or just the actual data, I think the actual data is more efficient for normal use.

			// if you'd like to have the space too, use OutSize, rather than OutPos
			p->OutBuffer = new unsigned char[source->OutPos];
			memcpy(p->OutBuffer, source->OutBuffer, source->OutPos);

			p->OutPos = source->OutPos;
			p->OutSize = source->OutPos; // this one too
		}

		if (source->InBuffer != nullptr) {
			p->InBuffer = new unsigned char[source->InSize];
			memcpy(p->InBuffer, source->InBuffer, source->InSize);

			p->InPos = source->InPos;
			p->InSize = source->InSize;
		}

		return 1;
	}
}