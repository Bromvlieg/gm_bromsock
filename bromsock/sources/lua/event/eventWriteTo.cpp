#include <bromsock/lua/event/eventWriteTo.h>
#include <bromsock/socket.h>

#include <GarrysMod/Lua/Interface.h>

#ifdef _MSC_VER
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib,"wsock32.lib")
#else

#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <cstring>
#endif

namespace bromsock {
    namespace event {
        const std::string& EventWriteTo::getNameStatic() {
            static std::string name = "onSendTo";
            return name;
        }

        const std::string& EventWriteTo::getName() const {
            return getNameStatic();
        }

        int EventWriteTo::pushArguments(lua_State* state) const {
            if (response) {
                LUA->PushNumber(static_cast<double>(buffer.size()));
            } else {
                LUA->PushNumber(0.0);
            }

            LUA->PushString(ip.c_str());
            LUA->PushNumber(static_cast<double>(port));
            return 3;
        }

        bool EventWriteTo::write(Socket* sock, const uint8_t* data, size_t size, const sockaddr* endpoint, size_t endpointLen) {
            size_t sent = 0;

			while (sent != size) {
                int ret = ::sendto(sock->sock.sock, reinterpret_cast<const char*>(data) + sent, size - sent, 0, endpoint, endpointLen);

				if (ret <= 0) return false;
				sent += static_cast<size_t>(ret);
			}

            return true;
        }

        void EventWriteTo::process(Socket* sock) {
            struct addrinfo* result = nullptr;
			struct addrinfo hints;
			memset(&hints, 0, sizeof(hints));

			hints.ai_family = AF_UNSPEC;
			hints.ai_socktype = SOCK_STREAM;
			hints.ai_protocol = IPPROTO_TCP;
			auto resp = getaddrinfo(ip.c_str(), std::to_string(port).c_str(), &hints, &result);
			if (resp != 0) return; //SocketError::invalidHostname;
			if (result == nullptr) return; //SocketError::invalidHostname;

            // verify if we're targeting the same ipv4/ipv6 as the socket
			bool targetIsIpv6 = result->ai_family == AF_INET6;
			if (targetIsIpv6 != sock->sock.ipv6) return;

			sockaddr_in addr;
			sockaddr_in6 addr6;
			memset(&addr, 0, sizeof(addr));
			memset(&addr6, 0, sizeof(addr6));

            sockaddr* endpoint = nullptr;
            size_t endpointLength = 0;

            int connectResult = 0;
			if (sock->sock.ipv6) {
				memcpy(&addr6, result->ai_addr, result->ai_addrlen);

				addr6.sin6_family = AF_INET6;
				addr6.sin6_port = htons(port);
				endpoint = reinterpret_cast<struct sockaddr*>(&addr6);
                endpointLength = sizeof(addr6);
			} else {
				memcpy(&addr, result->ai_addr, result->ai_addrlen);

				addr.sin_family = AF_INET;
				addr.sin_port = htons(port);
				endpoint = reinterpret_cast<struct sockaddr*>(&addr);
                endpointLength = sizeof(addr);
			}

            if (writeSize) {
                uint32_t size = static_cast<uint32_t>(buffer.size());
                if (!write(sock, reinterpret_cast<const uint8_t*>(&size), sizeof(size), endpoint, endpointLength)) {
                    return;
                }
            }

            response = write(sock, buffer.data(), buffer.size(), endpoint, endpointLength);
        }

        void EventWriteTo::setIp(const std::string& targetIp) {
            ip = targetIp;
        }

        void EventWriteTo::setPort(int targetPort) {
            port = targetPort;
        }

        void EventWriteTo::setBuffer(const std::vector<uint8_t>& buff) {
            buffer = buff;
        }

        void EventWriteTo::setWriteSize(bool val) {
            writeSize = val;
        }
    }
}
