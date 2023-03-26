#pragma once

#include <bromsock/lua/event/eventBase.h>

#include <GarrysMod/Lua/Interface.h>

namespace bromsock {
    namespace event {
        class EventShutdown : public EventBase {
            public:

            std::string ip;
            int port;

            static const std::string& getNameStatic();

            virtual const std::string& getName() const override;
            virtual int pushArguments(lua_State* state) const override;
            virtual void process(Socket* sock) override;
        };
    }
}
