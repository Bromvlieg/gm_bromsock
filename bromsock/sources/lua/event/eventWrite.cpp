#pragma once

#include <bromsock/lua/event/eventWrite.h>
#include <bromsock/socket.h>

#include <GarrysMod/Lua/interface.h>

namespace bromsock {
    namespace event {
        const std::string& EventWrite::getNameStatic() {
            static std::string name = "onSend";
            return name;
        }

        const std::string& EventWrite::getName() const {
            return getNameStatic();
        }

        int EventWrite::pushArguments(lua_State* state) const {
            LUA->PushNumber(static_cast<double>(buffer.size()));
            return 1;
        }

        bool EventWrite::write(Socket* sock, const uint8_t* data, size_t size) {
            size_t sent = 0;

			while (sent != size) {
#ifndef BROMSOCK_NOSSL
                int ret = sock->ssl != nullptr ? SSL_write(sock->ssl, data + sent, size - sent) : sock->sock.send(data + sent, size - sent);
#else
                int ret = sock->sock.send(data + sent, size - sent);
#endif

				if (ret <= 0) return false;
				sent += static_cast<size_t>(ret);
			}

            return true;
        }

        void EventWrite::process(Socket* sock) {
            if (writeSize) {
                uint32_t size = static_cast<uint32_t>(buffer.size());
                if (!write(sock, reinterpret_cast<const uint8_t*>(&size), sizeof(size))) {
                    setShouldDisconnect(true);
                    return;
                }
            }

            response = write(sock, buffer.data(), buffer.size());
            if (!response) setShouldDisconnect(true);
        }
    }
}
