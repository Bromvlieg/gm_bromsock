#pragma once

#include <bromsock/lua/event/eventReadFrom.h>
#include <GarrysMod/Lua/interface.h>

namespace bromsock {
    namespace event {
        const std::string& EventReadFrom::getNameStatic() {
            static std::string name = "onReceiveFrom";
            return name;
        }

        const std::string& EventReadFrom::getName() const {
            return getNameStatic();
        }

        int EventReadFrom::pushArguments(lua_State* state) const {
            return 0;
        }

        void EventReadFrom::process(Socket* sock) {
            
        }
    }
}
