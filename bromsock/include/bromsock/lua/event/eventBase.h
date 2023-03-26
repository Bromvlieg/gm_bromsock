#pragma once

#include <GarrysMod/Lua/interface.h>
#include <bromsock/defines.h>

#include <string>

namespace bromsock {
    class Socket;

    namespace event {
        class EventBase {
            bool shouldDisconnect = false;

            public:

            void setShouldDisconnect(bool should);
            bool getShouldDisconnect();

            virtual const std::string& getName() const = 0;
            virtual int pushArguments(lua_State* state) const = 0;
            virtual void process(Socket* sock) = 0;
        };
    }
}
