#pragma once

#include <bromsock/lua/event/eventBase.h>

#include <GarrysMod/Lua/Interface.h>

#include <vector>

struct sockaddr;

namespace bromsock {
    namespace event {
        class EventWriteTo : public EventBase {
            bool response = false;
            std::string ip;
            int port = 0;
            std::vector<uint8_t> buffer;
            bool writeSize = false;

            bool write(Socket* sock, const uint8_t* data, size_t size, const sockaddr* endpoint, size_t endpointLen);
        public:

            static const std::string& getNameStatic();

            virtual const std::string& getName() const override;
            virtual int pushArguments(lua_State* state) const override;
            virtual void process(Socket* sock) override;

            void setIp(const std::string& targetIp);
            void setPort(int targetPort);
            void setBuffer(const std::vector<uint8_t>& buff);
            void setWriteSize(bool val);
        };
    }
}
