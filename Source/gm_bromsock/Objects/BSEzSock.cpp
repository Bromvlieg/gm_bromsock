#include <iostream>
#include "BSPacket.h"
#include "BSEzSock.h"

#ifdef _MSC_VER
#pragma comment(lib,"wsock32.lib")
typedef int socklen_t;
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#endif

#if !defined(SOCKET_ERROR)
#define SOCKET_ERROR -1
#endif

#if !defined(SOCKET_NONE)
#define SOCKET_NONE 0
#endif

#if !defined(INVALID_SOCKET)
#define INVALID_SOCKET -1
#endif

namespace GMBSOCK {
	EzSock::EzSock(){
		MAXCON = 64;
		memset (&addr, 0, sizeof(addr));
		
#ifdef _MSC_VER
		WSAStartup(MAKEWORD(1, 1), &wsda );
#endif

		this->sock = INVALID_SOCKET;
		this->blocking = true;
		this->scks = new fd_set;
		this->times = new timeval;
		this->times->tv_sec = 0;
		this->times->tv_usec = 0;
		this->state = skDISCONNECTED;
		this->totaldata = 0;
		
		memset(this->lastError, 0, sizeof(this->lastError));
	}

	EzSock::~EzSock(){
		if (this->check())
			close();

		delete scks;
		delete times;
	}

	void EzSock::StoreSocketError() {
		// Used to store socket function errors (SOCKET_ERROR) into the lastError string.
#ifdef _MSC_VER
		int errCode = WSAGetLastError();
		if(!FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, errCode,
			MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
			lastError, sizeof(lastError), NULL)){
			// FormatMessage failed, set lastError to just the code instead.
			snprintf(lastError, sizeof(lastError), "Code: %d", errCode);
		}
#else
		char* errptr = strerror_r(errno, lastError, sizeof(lastError)); // GNU version of strerror_r
		snprintf(lastError, sizeof(lastError), "%s", errptr);
#endif
	}

	bool EzSock::check(){
		return sock > SOCKET_NONE;
	}

	bool EzSock::create(){
		return this->create(IPPROTO_TCP, SOCK_STREAM);
	}

	bool EzSock::create(int Protocol){
		switch(Protocol){
		case IPPROTO_TCP: return this->create(IPPROTO_TCP, SOCK_STREAM);
		case IPPROTO_UDP: return this->create(IPPROTO_UDP, SOCK_DGRAM);
		default:          return this->create(Protocol, SOCK_RAW);
		}
	}

	bool EzSock::create(int Protocol, int Type){
		if (this->check())
			return false;
			
		state = skDISCONNECTED;
		sock = ::socket(AF_INET, Type, Protocol);
		lastCode = sock;

		return sock > SOCKET_NONE;
	}

	bool EzSock::bind(const char* ip, unsigned short port){
		if(!check()) return false;

		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = inet_addr(ip);
		addr.sin_port = htons(port);
		lastCode = ::bind(sock,(struct sockaddr*)&addr, sizeof(addr));
		if (lastCode == SOCKET_ERROR) {
			StoreSocketError();
			return false;
		}
		return true;
	}

	bool EzSock::listen(){
		lastCode = ::listen(sock, MAXCON);
		if (lastCode == SOCKET_ERROR) {
			StoreSocketError();
			return false;
		}

		state = skLISTENING;
		return true;
	}

	bool EzSock::accept(EzSock* socket){
		if (!blocking && !CanRead()) return false;

		int length = sizeof(socket->addr);
		socket->sock = ::accept(sock,(struct sockaddr*) &socket->addr, (socklen_t*) &length);

		lastCode = socket->sock;
		if (lastCode == SOCKET_ERROR) {
			StoreSocketError();
			return false;
		}

		socket->state = skCONNECTED;
		return true;
	}

	void EzSock::close(){
		state = skDISCONNECTED;
		
#ifdef _MSC_VER
		::closesocket(sock);
#else
		::shutdown(sock, SHUT_RDWR);
		::close(sock);
#endif

		sock = INVALID_SOCKET;
	}

	long EzSock::uAddr(){
		return addr.sin_addr.s_addr;
	}

	int EzSock::connect(const char* host, unsigned short port){
		if(!check())
			return 1;

		struct hostent* phe;
		phe = gethostbyname(host);
		if (phe == NULL)
			return 2;

		memcpy(&addr.sin_addr, phe->h_addr, sizeof(struct in_addr));

		addr.sin_family = AF_INET;
		addr.sin_port   = htons(port);

		if (::connect(sock, (struct sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
			StoreSocketError();
			return 3;
		}

		state = skCONNECTED;
		return 0;
	}

	bool EzSock::CanRead(){
		FD_ZERO(scks);
		FD_SET((unsigned)sock, scks);

		return select(sock + 1, scks, NULL, NULL, times) > 0;
	}

	bool EzSock::IsError(){
		if (state == skERROR)
			return true;

		FD_ZERO(scks);
		FD_SET((unsigned)sock, scks);

		if (select(sock + 1, NULL, NULL, scks, times) >= 0)
			return false;

		state = skERROR;
		return true;
	}

	int EzSock::ReceiveUDP(unsigned char* buffer, int size, sockaddr_in* from){
#ifdef _MSC_VER
		int client_length = (int)sizeof(struct sockaddr_in);
		return recvfrom(this->sock, (char*)buffer, size, 0, (struct sockaddr*)from, &client_length);
#else
		unsigned int client_length = (unsigned int)sizeof(struct sockaddr_in);
		return recvfrom(this->sock, (char*)buffer, size, 0, (struct sockaddr*)from, &client_length);
#endif
	}

	int EzSock::Receive(unsigned char* buffer, int size, int spos){
		return recv(this->sock, (char*)buffer + spos, size, 0);
	}

	int EzSock::SendUDP(unsigned char* buffer, int size, sockaddr_in* to){
		return sendto(this->sock, (char*)buffer, size, 0, (struct sockaddr *)&to, sizeof(struct sockaddr_in));
	}

	int EzSock::SendRaw(unsigned char* data, int dataSize){
		return send(this->sock, (char*)data, dataSize, 0);
	}
}
