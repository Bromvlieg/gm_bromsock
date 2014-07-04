#ifndef CLASS_PACKET
#define CLASS_PACKET

#include "BSEzSock.h"

#ifndef null
#define null 0
#endif

namespace BromScript{
	class Packet{
	public:
		int RefCount;
		bool Valid;
		EzSock* Sock;
		unsigned int OutPos;
		unsigned int OutSize;
		unsigned int InPos;
		unsigned int InSize;
		unsigned char* InBuffer;
		unsigned char* OutBuffer;

		Packet();
		Packet(EzSock*);
		~Packet();

		void Clear();

		void WriteByte(unsigned char);
		void WriteBool(bool);
		void WriteBytes(unsigned char* arr, int size, bool sendsize);
		void WriteShort(short);
		void WriteUShorts(unsigned short* arr, int size, bool sendsize);
		void WriteInt(int);
		void WriteLong(long long);
		void WriteFloat(float);
		void WriteDouble(double);
		void WriteString(const char*);
		void WriteStringNT(const char*);
		void WriteLine(const char*);

		unsigned char ReadByte();
		bool ReadBool();
		unsigned char* ReadBytes(int len);
		short ReadShort();
		int ReadInt();
		long long ReadLong();
		float ReadFloat();
		double ReadDouble();
		char* ReadString();
		char* ReadLine();
		
		bool CanRead(int len);
		bool CanRead(char* seq);
		bool HasDataLeft();
		int DataLeft();

		void Send();

		void CheckSpaceOut(int);
		void AllocateMoreSpaceOut(int);
		void CheckSpaceIn(int);
		void AllocateMoreSpaceIn(int);
	};
}

#endif