#pragma once

#include <bromsock/lua/event/eventConnect.h>
#include <bromsock/socket.h>

#include <GarrysMod/Lua/Interface.h>

namespace bromsock {
    namespace event {
        const std::string& EventConnect::getNameStatic() {
            static std::string name = "onConnect";
            return name;
        }

        const std::string& EventConnect::getName() const {
            return getNameStatic();
        }

        int EventConnect::pushArguments(lua_State* state) const {
            LUA->PushBool(sockResponse == mainframe::networking::SocketError::success);
            LUA->PushString(ip.c_str());
            LUA->PushNumber(port);
            return 3;
        }

        void EventConnect::process(Socket* sock) {
            sockResponse = sock->sock.connect(ip, static_cast<unsigned short>(port));
        }

        void EventConnect::setIp(const std::string& targetIp) {
            ip = targetIp;
        }

        void EventConnect::setPort(int targetPort) {
            port = targetPort;
        }
    }
}
