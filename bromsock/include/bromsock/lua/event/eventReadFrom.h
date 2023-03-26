#pragma once

#include <bromsock/lua/event/eventBase.h>

#include <GarrysMod/Lua/Interface.h>

#include <vector>

struct sockaddr;
struct sockaddr_in;
struct sockaddr_in6;

namespace bromsock {
    namespace event {
        class EventReadFrom : public EventBase {
            std::vector<uint8_t> buffer;
            bool response = false;

            size_t amount = 0;
            std::string sequence;
            std::string ip;
            int port = 0;

            bool read(Socket* sock, uint8_t* data, size_t size, sockaddr* endpoint, size_t endpointLen);
            void updateInfoFromRecv(Socket* sock, sockaddr_in& ipv4, sockaddr_in6& ipv6);

        public:
            static const std::string& getNameStatic();

            virtual const std::string& getName() const override;
            virtual int pushArguments(lua_State* state) const override;
            virtual void process(Socket* sock) override;

            void setAmount(size_t val);
            size_t getAmount();
            void setSequence(const std::string& seq);
            void setIp(const std::string& targetIp);
            void setPort(int targetPort);
        };
    }
}
