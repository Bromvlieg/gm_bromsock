#pragma once

#include <bromsock/lua/event/eventBase.h>

#include <GarrysMod/Lua/interface.h>

#include <vector>

namespace bromsock {
    namespace event {
        class EventWriteTo : public EventBase {
            public:

            std::string ip;
            int port;
            std::vector<uint8_t> buffer;

            static const std::string& getNameStatic();

            virtual const std::string& getName() const override;
            virtual int pushArguments(lua_State* state) const override;
            virtual void process(Socket* sock) override;
        };
    }
}
