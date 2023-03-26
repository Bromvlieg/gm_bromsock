#pragma once

#include <bromsock/lua/event/eventDisconnect.h>
#include <GarrysMod/Lua/interface.h>

namespace bromsock {
    namespace event {
        const std::string& EventDisconnect::getNameStatic() {
            static std::string name = "onDisconnect";
            return name;
        }

        const std::string& EventDisconnect::getName() const {
            return getNameStatic();
        }

        int EventDisconnect::pushArguments(lua_State* state) const {
            return 0;
        }

        void EventDisconnect::process(Socket* sock) {
            setShouldDisconnect(true);
        }
    }
}
