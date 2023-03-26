#pragma once

#include <bromsock/lua/event/eventBase.h>
#include <bromsock/socket.h>

#include <GarrysMod/Lua/interface.h>

namespace bromsock {
    namespace event {
        class EventConnect : public EventBase {
            mainframe::networking::SocketError sockResponse = mainframe::networking::SocketError::failed;
            std::string ip;
            int port = 0;

        public:
            static const std::string& getNameStatic();

            virtual const std::string& getName() const override;
            virtual int pushArguments(lua_State* state) const override;
            virtual void process(Socket* sock) override;

            void setIp(const std::string& targetIp);
            void setPort(int targetPort);
        };
    }
}
