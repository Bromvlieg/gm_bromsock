#include <bromsock/packet.h>
#include <bromsock/engine.h>
#include <bromsock/defines.h>

namespace bromsock {
	void swap_2(void* source) {
		char ret[2];
		for (int i = 0; i < 2; i++) {
			ret[i] = ((char*)source)[1 - i];
		}

		memcpy(source, ret, 2);
	}

	void swap_4(void* source) {
		char ret[4];
		for (int i = 0; i < 4; i++) {
			ret[i] = ((char*)source)[3 - i];
		}

		memcpy(source, ret, 4);
	}

	void swap_8(void* source) {
		char ret[8];
		for (int i = 0; i < 8; i++) {
			ret[i] = ((char*)source)[7 - i];
		}

		memcpy(source, ret, 8);
	}

	int get_local_endian(void) {
		// compiler magic, GCC makes the result of this a constant, moar speeds
		union {
			uint32_t i;
			char c[4];
		} bint = {0x01000002}; // outcome: 0x01 == big endian, 0x02 == little endian

		return bint.c[0];
	}

	Packet::Packet(){
		this->Valid = false;

		this->InPos = 0;
		this->InSize = 0;

		this->OutPos = 0;
		this->OutSize = 0;
		this->OutBuffer = nullptr;
		this->InBuffer = nullptr;

		this->EndianType = 0;
	}

	Packet::~Packet(){
		this->Clear();
	}

	void Packet::Clear(){
		if (this->InBuffer != nullptr) delete[] this->InBuffer;
		if (this->OutBuffer != nullptr) delete[] this->OutBuffer;

		this->InPos = 0;
		this->InSize = 0;
		this->InBuffer = nullptr;

		this->OutPos = 0;
		this->OutSize = 0;
		this->OutBuffer = nullptr;
	}

	void Packet::CheckSpaceOut(int needed){
		if (this->OutPos + needed >= this->OutSize){
			this->AllocateMoreSpaceOut(needed < 128 ? 128 : needed);
		}
	}

	void Packet::AllocateMoreSpaceOut(int addsize){
		this->OutSize += addsize;
		unsigned char* newbuff = new unsigned char[this->OutSize];

		if(this->OutBuffer != nullptr) {
			memcpy(newbuff, this->OutBuffer, this->OutSize - addsize);
			delete[] this->OutBuffer;
		}

		this->OutBuffer = newbuff;
	}

	void Packet::CheckSpaceIn(int needed){
		if (this->InPos + needed >= this->InSize){
			this->AllocateMoreSpaceIn(needed - (this->InSize - this->InPos));
		}
	}

	void Packet::AllocateMoreSpaceIn(int addsize){
		this->InSize += addsize;
		unsigned char* newbuff = new unsigned char[this->InSize];

		if(this->InBuffer != nullptr) {
			memcpy(newbuff, this->InBuffer, this->InSize - addsize);
			delete[] this->InBuffer;
		}

		this->InBuffer = newbuff;
	}

	unsigned char Packet::ReadByte(){
		if (!this->CanRead(1)) return 0;

		return this->InBuffer[this->InPos++];
	}

	bool Packet::ReadBool(){
		if (!this->CanRead(1)) return 0;

		return this->InBuffer[this->InPos++] == 1;
	}

	unsigned char* Packet::ReadBytes(int len){
		if (!this->CanRead(len)) return nullptr;

		this->InPos += len;
		return this->InBuffer + (this->InPos - len);
	}

	short Packet::ReadShort(){
		if (!this->CanRead(2)) return 0;

		short ret = 0;
		memcpy(&ret, this->InBuffer + this->InPos, 2);
		this->InPos += 2;

		if (this->EndianType != 0 && this->EndianType != get_local_endian()) swap_2(&ret);
		return ret;
	}

	unsigned short Packet::ReadUShort(){
		if (!this->CanRead(2)) return 0;

		unsigned short ret = 0;
		memcpy(&ret, this->InBuffer + this->InPos, 2);
		this->InPos += 2;

		if (this->EndianType != 0 && this->EndianType != get_local_endian()) swap_2(&ret);
		return ret;
	}

	float Packet::ReadFloat(){
		if (!this->CanRead(4)) return 0;

		float ret = 0;
		memcpy(&ret, this->InBuffer + this->InPos, 4);
		this->InPos += 4;

		if (this->EndianType != 0 && this->EndianType != get_local_endian()) swap_4(&ret);
		return ret;
	}

	double Packet::ReadDouble(){
		if (!this->CanRead(8)) return 0;

		double ret = 0;
		memcpy(&ret, this->InBuffer + this->InPos, 8);
		this->InPos += 8;

		if (this->EndianType != 0 && this->EndianType != get_local_endian()) swap_8(&ret);
		return ret;
	}

	int Packet::ReadInt(){
		if (!this->CanRead(4)) return 0;

		int ret = 0;
		memcpy(&ret, this->InBuffer + this->InPos, 4);
		this->InPos += 4;

		if (this->EndianType != 0 && this->EndianType != get_local_endian()) swap_4(&ret);
		return ret;
	}

	unsigned int Packet::ReadUInt(){
		if (!this->CanRead(4)) return 0;

		unsigned int ret = 0;
		memcpy(&ret, this->InBuffer + this->InPos, 4);
		this->InPos += 4;

		if (this->EndianType != 0 && this->EndianType != get_local_endian()) swap_4(&ret);
		return ret;
	}

	long long Packet::ReadLong(){
		if (!this->CanRead(8)) return 0;

		long long ret = 0;
		memcpy(&ret, this->InBuffer + this->InPos, 8);
		this->InPos += 8;

		if (this->EndianType != 0 && this->EndianType != get_local_endian()) swap_8(&ret);
		return ret;
	}

	unsigned long long Packet::ReadULong(){
		if (!this->CanRead(8)) return 0;

		unsigned long long ret = 0;
		memcpy(&ret, this->InBuffer + this->InPos, 8);
		this->InPos += 8;

		if (this->EndianType != 0 && this->EndianType != get_local_endian()) swap_8(&ret);
		return ret;
	}

	char* Packet::ReadString(int len){
		if (len == -1) len = this->ReadShort();
		if (!this->CanRead(len)) return new char[1] {0};

		if (len == 0) return new char[1] {0};

		char* buff = new char[len + 1];
		memcpy(buff, this->InBuffer + this->InPos, len);
		buff[len] = 0;

		this->InPos += len;
		return buff;
	}

	char* Packet::ReadStringAll(unsigned int* outlen){
		int len = this->InSize - this->InPos;
		if (len == 0) return new char[1] {0};

		char* buff = new char[len + 1];
		memcpy(buff, this->InBuffer + this->InPos, len);
		buff[len] = 0;

		this->InPos += len;
		*outlen = len;
		return buff;
	}

	char* Packet::ReadStringNT(unsigned int* outlen) {
		int startpos = this->InPos;

		while (this->CanRead(1) && this->InBuffer[this->InPos++] != 0) {
			// a buttload of nothing
		}

		if (startpos == this->InPos) return new char[1] {0};

		char* buff = new char[this->InPos - startpos + 1];
		memcpy(buff, this->InBuffer + startpos, this->InPos - startpos);
		buff[this->InPos - startpos] = 0;

		*outlen = this->InPos - startpos;
		return buff;
	}

	char* Packet::ReadUntil(const char* seq, unsigned int seqsize, unsigned int* outlen) {
		unsigned int startpos = this->InPos;

		while (this->CanRead(seqsize)) {
			bool done = true;
			
			for (unsigned int i = 0; i < seqsize; i++){
				if (this->InBuffer[this->InPos + i] != seq[i]){
					done = false;
					break;
				}
			}

			if (done){
				break;
			}

			this->InPos++;
		}

		if (startpos == this->InPos) return new char[1] {0};

		*outlen = this->InPos - startpos;

		char* buff = new char[this->InPos - startpos + 1];
		memcpy(buff, this->InBuffer + startpos, this->InPos - startpos);
		buff[this->InPos - startpos] = 0;

		return buff;
	}

	int Packet::DataLeft(){
		return this->InSize - this->InPos;
	}

	bool Packet::HasDataLeft(){
		return this->InSize - this->InPos > 0;
	}

	bool Packet::CanRead(int numofbytes){
		if (numofbytes == 0) return true;
		return this->InSize - this->InPos >= (unsigned int)(numofbytes);
	}

	void Packet::WriteByte(unsigned char num){
		this->CheckSpaceOut(1);
		this->OutBuffer[this->OutPos++] = num;
	}

	void Packet::WriteBool(bool num){
		this->CheckSpaceOut(1);
		this->OutBuffer[this->OutPos++] = num ? 1 : 0;
	}

	void Packet::WriteBytes(unsigned char* arr, int size, bool sendsize){
		this->CheckSpaceOut(size + sendsize);

		if (sendsize) this->WriteInt(size);
		memcpy(this->OutBuffer + this->OutPos, arr, size);
		this->OutPos += size;
	}

	void Packet::WriteShort(short num){
		if (this->EndianType != 0 && this->EndianType != get_local_endian()) swap_2(&num);

		this->CheckSpaceOut(2);
		memcpy(this->OutBuffer + this->OutPos, &num, 2);
		this->OutPos += 2;
	}

	void Packet::WriteUShort(unsigned short num) {
		if (this->EndianType != 0 && this->EndianType != get_local_endian()) swap_2(&num);

		this->CheckSpaceOut(2);
		memcpy(this->OutBuffer + this->OutPos, &num, 2);
		this->OutPos += 2;
	}

	void Packet::WriteInt(int num) {
		if (this->EndianType != 0 && this->EndianType != get_local_endian()) swap_4(&num);

		this->CheckSpaceOut(4);
		memcpy(this->OutBuffer + this->OutPos, &num, 4);
		this->OutPos += 4;
	}

	void Packet::WriteUInt(unsigned int num) {
		if (this->EndianType != 0 && this->EndianType != get_local_endian()) swap_4(&num);

		this->CheckSpaceOut(4);
		memcpy(this->OutBuffer + this->OutPos, &num, 4);
		this->OutPos += 4;
	}

	void Packet::WriteLong(long long num) {
		if (this->EndianType != 0 && this->EndianType != get_local_endian()) swap_8(&num);

		this->CheckSpaceOut(8);
		memcpy(this->OutBuffer + this->OutPos, &num, 8);
		this->OutPos += 8;
	}

	void Packet::WriteULong(unsigned long long num) {
		if (this->EndianType != 0 && this->EndianType != get_local_endian()) swap_8(&num);

		this->CheckSpaceOut(8);
		memcpy(this->OutBuffer + this->OutPos, &num, 8);
		this->OutPos += 8;
	}

	void Packet::WriteFloat(float num) {
		if (this->EndianType != 0 && this->EndianType != get_local_endian()) swap_4(&num);

		this->CheckSpaceOut(4);
		memcpy(this->OutBuffer + this->OutPos, &num, 4);
		this->OutPos += 4;
	}

	void Packet::WriteDouble(double num) {
		if (this->EndianType != 0 && this->EndianType != get_local_endian()) swap_8(&num);

		this->CheckSpaceOut(8);
		memcpy(this->OutBuffer + this->OutPos, &num, 8);
		this->OutPos += 8;
	}

	void Packet::WriteString(const char* str){
		int size = strlen(str);
		this->CheckSpaceOut(size + 2);

		this->WriteShort(size);

		for (int i = 0; i < size; i++)
			this->OutBuffer[this->OutPos++] = str[i];
	}

	void Packet::WriteStringNT(const char* str){
		int size = strlen(str);
		this->CheckSpaceOut(size + 1);

		for (int i = 0; i < size; i++)
			this->OutBuffer[this->OutPos++] = str[i];

		this->OutBuffer[this->OutPos++] = 0;
	}

	void Packet::WriteStringRaw(const char* str, unsigned int size){
		this->CheckSpaceOut(size);

		for (unsigned int i = 0; i < size; i++)
			this->OutBuffer[this->OutPos++] = str[i];
	}

	void Packet::WriteLine(const char* str){
		int size = strlen(str);
		this->CheckSpaceOut(size + 2);

		for (int i = 0; i < size; i++)
			this->OutBuffer[this->OutPos++] = str[i];

		this->OutBuffer[this->OutPos++] = '\r';
		this->OutBuffer[this->OutPos++] = '\n';
	}

    void Packet::pushToStack(lua_State* state) {
        if (luaReference != 0) {
            LUA->PushUserdata(luaReference);
            return;
        }

        GarrysMod::Lua::UserData* ud = (GarrysMod::Lua::UserData*)LUA->NewUserdata(sizeof(GarrysMod::Lua::UserData));
		ud->data = this;
		ud->type = UD_TYPE_PACKET;

		LUA->ReferencePush(Engine::getInstance(state).packetRef);
		LUA->SetMetaTable(-2);
        luaReference = LUA->GetUserdata();
    }
}