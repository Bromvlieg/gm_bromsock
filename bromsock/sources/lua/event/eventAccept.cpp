#include <bromsock/lua/event/eventAccept.h>
#include <bromsock/socket.h>
#include <bromsock/engine.h>

#include <GarrysMod/Lua/Interface.h>

namespace bromsock {
    namespace event {
        const std::string& EventAccept::getNameStatic() {
            static std::string name = "onAccept";
            return name;
        }

        const std::string& EventAccept::getName() const {
            return getNameStatic();
        }

        int EventAccept::pushArguments(lua_State* state) const {
            acceptedClient->pushToStack(state);
            return 1;
        }

        void EventAccept::process(Socket* sock) {
            auto client = std::make_unique<Socket>(sock->state, IPPROTO_TCP, sock->ipv6);
            if (!sock->sock.accept(&client->sock)) {
                setShouldDisconnect(true);
                return;
            }

            acceptedClient = client.get();
            Engine::getInstance(sock->state).addSocket(std::move(client));
        }
    }
}
