#pragma once

#include <bromsock/lua/event/eventBase.h>

#include <GarrysMod/Lua/Interface.h>

#include <vector>

namespace bromsock {
    namespace event {
        class EventAccept : public EventBase {
            public:

            static const std::string& getNameStatic();

            virtual const std::string& getName() const override;
            virtual int pushArguments(lua_State* state) const override;
            virtual void process(Socket* sock) override;
        };
    }
}
