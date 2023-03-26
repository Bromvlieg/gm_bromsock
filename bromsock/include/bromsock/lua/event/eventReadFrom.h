#pragma once

#include <bromsock/lua/event/eventBase.h>

#include <GarrysMod/Lua/interface.h>

#include <vector>

namespace bromsock {
    namespace event {
        class EventReadFrom : public EventBase {
            public:

            size_t amount = 0;
            std::string ip;
            int port = 0;

            static const std::string& getNameStatic();

            virtual const std::string& getName() const override;
            virtual int pushArguments(lua_State* state) const override;
            virtual void process(Socket* sock) override;
        };
    }
}