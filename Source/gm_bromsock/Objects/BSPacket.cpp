#include "BSPacket.h"
#include <stdio.h>

#ifndef _MSC_VER
#include <string.h>
#endif

namespace BromScript{
	Packet::Packet(){
		this->Valid = false;

		this->Sock = null;
		this->InPos = 0;
		this->InSize = 0;

		this->OutPos = 0;
		this->OutSize = 0;
		this->OutBuffer = null;
		this->InBuffer = null;
	}

	Packet::Packet(EzSock* sock){
		this->Valid = true;

		this->Sock = sock;
		this->InPos = 0;
		this->InSize = 0;

		this->OutPos = 0;
		this->OutSize = 0;
		this->OutBuffer = null;
		this->InBuffer = null;
	}

	Packet::~Packet(){
		this->Clear();
	}

	void Packet::Clear(){
		if (this->InBuffer != null) delete[] this->InBuffer;
		if (this->OutBuffer != null) delete[] this->OutBuffer;
	
		this->InPos = 0;
		this->InSize = 0;
		this->InBuffer = null;

		this->OutPos = 0;
		this->OutSize = 0;
		this->OutBuffer = null;
	}

	void Packet::CheckSpaceOut(int needed){
		if (this->OutPos + needed >= this->OutSize){
			this->AllocateMoreSpaceOut(needed < 128 ? 128 : needed);
		}
	}

	void Packet::AllocateMoreSpaceOut(int addsize){
		this->OutSize += addsize;
		unsigned char* newbuff = new unsigned char[this->OutSize];

		if(this->OutBuffer != null) {
			memcpy(newbuff, this->OutBuffer, this->OutSize - addsize);
			delete[] this->OutBuffer;
		}

		this->OutBuffer = newbuff;
	}

	void Packet::CheckSpaceIn(int needed){
		if (this->InPos + needed >= this->InSize){
			this->AllocateMoreSpaceIn(needed);
		}
	}

	void Packet::AllocateMoreSpaceIn(int addsize){
		this->InSize += addsize;
		unsigned char* newbuff = new unsigned char[this->InSize];

		if(this->InBuffer != null) {
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
		if (!this->CanRead(len)) return null;

		this->InPos += len;
		return this->InBuffer + (this->InPos - len);
	}

	short Packet::ReadShort(){
		if (!this->CanRead(2)) return 0;

		short ret = 0;
		memcpy(&ret, this->InBuffer + this->InPos, 2);
		this->InPos += 2;

		return ret;
	}

	float Packet::ReadFloat(){
		if (!this->CanRead(4)) return 0;

		float ret = 0;
		memcpy(&ret, this->InBuffer + this->InPos, 4);
		this->InPos += 4;

		return ret;
	}

	double Packet::ReadDouble(){
		if (!this->CanRead(8)) return 0;

		double ret = 0;
		memcpy(&ret, this->InBuffer + this->InPos, 8);
		this->InPos += 8;

		return ret;
	}

	int Packet::ReadInt(){
		if (!this->CanRead(4)) return 0;

		int ret = 0;
		memcpy(&ret, this->InBuffer + this->InPos, 4);
		this->InPos += 4;

		return ret;
	}

	long long Packet::ReadLong(){
		if (!this->CanRead(8)) return 0;

		long long ret = 0;
		memcpy(&ret, this->InBuffer + this->InPos, 8);
		this->InPos += 8;

		return ret;
	}

	char* Packet::ReadString(){
		int len = this->ReadShort();
		if (!this->CanRead(len)) return "";

		if (len == 0) return "";

		char* buff = new char[len + 1];
		memcpy(buff, this->InBuffer + this->InPos, len);
		buff[len] = 0;

		this->InPos += len;
		return buff;
	}

	int Packet::DataLeft(){
		return this->InSize - this->InPos;
	}

	bool Packet::HasDataLeft(){
		return this->InSize - this->InPos > 0;
	}

	bool Packet::CanRead(int numofbytes){
		bool res = this->InSize - this->InPos >= (unsigned int)(numofbytes);
		if (res == false && this->Sock != null){
			unsigned char* tmp = new unsigned char[numofbytes];
			int recamount = 0;
			while(recamount != numofbytes){
				recamount += recv(this->Sock->sock, (char*)tmp + recamount, numofbytes - recamount, 0);

				if (recamount == -1 || recamount == 0){
					this->Sock->state = EzSock::skERROR;
					this->Valid = false;
					delete[] tmp;
					return false;
				}
			}

			this->CheckSpaceIn(numofbytes);
			memcpy(this->InBuffer + this->InPos, tmp, numofbytes);
			delete[] tmp;

			return true;
		}

		return res;
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
		this->CheckSpaceOut(2);
		memcpy(this->OutBuffer + this->OutPos, &num, 2);
		this->OutPos += 2;
	}

	void Packet::WriteUShorts(unsigned short* arr, int size, bool sendsize){
		size *= 2;

		this->CheckSpaceOut(size + sendsize);

		if (sendsize) this->WriteInt(size);
		memcpy(this->OutBuffer + this->OutPos, arr, size);
		this->OutPos += size;
	}

	void Packet::WriteInt(int num){
		this->CheckSpaceOut(4);
		memcpy(this->OutBuffer + this->OutPos, &num, 4);
		this->OutPos += 4;
	}

	void Packet::WriteLong(long long num){
		this->CheckSpaceOut(8);
		memcpy(this->OutBuffer + this->OutPos, &num, 8);
		this->OutPos += 8;
	}

	void Packet::WriteFloat(float num){
		this->CheckSpaceOut(4);
		memcpy(this->OutBuffer + this->OutPos, &num, 4);
		this->OutPos += 4;
	}

	void Packet::WriteDouble(double num){
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

	void Packet::Send(){
		this->Sock->SendRaw((unsigned char*)&this->OutPos, 4);
		this->Sock->SendRaw(this->OutBuffer, this->OutPos);

		delete[] this->OutBuffer;
		this->OutBuffer = null;
		this->OutPos = 0;
		this->OutSize = 0;
	}
}