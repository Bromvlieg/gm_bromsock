#include <bromsock/lua/event/eventRead.h>
#include <bromsock/socket.h>
#include <bromsock/packet.h>

#include <GarrysMod/Lua/Interface.h>

namespace bromsock {
    namespace event {
        const std::string& EventRead::getNameStatic() {
            static std::string name = "onReceive";
            return name;
        }

        const std::string& EventRead::getName() const {
            return getNameStatic();
        }

        int EventRead::pushArguments(lua_State* state) const {
            auto p = new Packet();
            p->AllocateMoreSpaceIn(buffer.size());
            memcpy(p->InBuffer, buffer.data(), buffer.size());

            p->pushToStack(state);
            return 1;
        }

        bool EventRead::read(Socket* sock, uint8_t* data, size_t size) {
            size_t recieved = 0;

			while (recieved != size) {
#ifndef BROMSOCK_NOSSL
                int ret = sock->ssl != nullptr ? SSL_read(sock->ssl, data + recieved, size - recieved) : sock->sock.receive(data + recieved, size - recieved);
#else
                int ret = sock->sock.receive(data + recieved, size - recieved);
#endif

				if (ret <= 0) return false;
				recieved += static_cast<size_t>(ret);
			}

            return true;
        }

        void EventRead::process(Socket* sock) {
            if (!sequence.empty()) {
                while(true) {
                    buffer.reserve(buffer.size() + 1);
                    uint8_t buffChar;
                    if (!read(sock, &buffChar, sizeof(buffChar))) {
                        setShouldDisconnect(true);
                        return;
                    }

                    buffer.push_back(buffChar);
                    if (buffer.size() < sequence.size()) continue;
                    if (std::equal(buffer.end() - sequence.size(), buffer.end(), sequence.begin())) {
                        break;
                    }
                }

                response = true;
                return;
            }

            if (amount == 0) {
                uint32_t len;
                if (!read(sock, reinterpret_cast<uint8_t*>(&len), sizeof(len))) {
                    setShouldDisconnect(true);
                    return;
                }

                amount = static_cast<size_t>(len);
            }

            buffer.resize(amount);
            if (!read(sock, buffer.data(), buffer.size())) {
                setShouldDisconnect(true);
                return;
            }

            response = true;
        }

        void EventRead::setAmount(size_t val) {
            amount = val;
        }

        size_t EventRead::getAmount() {
            return amount;
        }

        void EventRead::setSequence(const std::string& seq) {
            sequence = seq;
        }
    }
}
