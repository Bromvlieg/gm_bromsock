#include <bromsock/lua/event/eventReadFrom.h>
#include <bromsock/socket.h>
#include <bromsock/packet.h>

#include <GarrysMod/Lua/Interface.h>

namespace bromsock {
    namespace event {
        const std::string& EventReadFrom::getNameStatic() {
            static std::string name = "onReceiveFrom";
            return name;
        }

        const std::string& EventReadFrom::getName() const {
            return getNameStatic();
        }

        int EventReadFrom::pushArguments(lua_State* state) const {
            auto p = new Packet();
            p->AllocateMoreSpaceIn(buffer.size());
            memcpy(p->InBuffer, buffer.data(), buffer.size());

            p->pushToStack(state);

            LUA->PushString(ip.c_str());
            LUA->PushNumber(static_cast<double>(port));
            return 3;
        }

        bool EventReadFrom::read(Socket* sock, uint8_t* data, size_t size, sockaddr* endpoint, size_t endpointLen) {
            size_t recieved = 0;

			while (recieved != size) {
#ifdef _MSC_VER
                int client_length = static_cast<int>(endpointLen);
#else
                unsigned int client_length = static_cast<unsigned int>(endpointLen);
#endif

                auto ret = recvfrom(sock->sock.sock, reinterpret_cast<char*>(data) + recieved, size - recieved, 0, endpoint, &client_length);

				if (ret <= 0) return false;
				recieved += static_cast<size_t>(ret);
			}

            return true;
        }

        void EventReadFrom::process(Socket* sock) {
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

            if (!sequence.empty()) {
                while(true) {
                    buffer.reserve(buffer.size() + 1);
                    uint8_t buffChar;
                    if (!read(sock, &buffChar, sizeof(buffChar), endpoint, endpointLength)) {
                        setShouldDisconnect(true);
                        return;
                    }

                    buffer.push_back(buffChar);
                    if (buffer.size() < sequence.size()) continue;
                    if (std::equal(buffer.end() - sequence.size(), buffer.end(), sequence.begin())) {
                        break;
                    }
                }

                updateInfoFromRecv(sock, addr, addr6);
                response = true;
                return;
            }

            if (amount == 0) {
                uint32_t len;
                if (!read(sock, reinterpret_cast<uint8_t*>(&len), sizeof(len), endpoint, endpointLength)) {
                    setShouldDisconnect(true);
                    return;
                }

                amount = static_cast<size_t>(len);
            }

            buffer.resize(amount);
            if (!read(sock, buffer.data(), buffer.size(), endpoint, endpointLength)) {
                setShouldDisconnect(true);
                return;
            }

            updateInfoFromRecv(sock, addr, addr6);
            response = true;
        }

        void EventReadFrom::updateInfoFromRecv(Socket* sock, sockaddr_in& ipv4, sockaddr_in6& ipv6) {
            if (sock->ipv6) {
                char buff[INET6_ADDRSTRLEN];
                inet_ntop(AF_INET6, reinterpret_cast<void*>(&ipv6.sin6_addr), buff, sizeof(buff));
                ip = buff;
                port = static_cast<int>(ntohs(ipv6.sin6_port));
            } else {
                char buff[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, reinterpret_cast<void*>(&ipv4.sin_addr), buff, sizeof(buff));
                ip = buff;
                port = static_cast<int>(ntohs(ipv4.sin_port));
            }
        }

        void EventReadFrom::setAmount(size_t val) {
            amount = val;
        }

        size_t EventReadFrom::getAmount() {
            return amount;
        }

        void EventReadFrom::setSequence(const std::string& seq) {
            sequence = seq;
        }

        void EventReadFrom::setIp(const std::string& targetIp) {
            ip = targetIp;
        }

        void EventReadFrom::setPort(int targetPort) {
            port = targetPort;
        }
    }
}
