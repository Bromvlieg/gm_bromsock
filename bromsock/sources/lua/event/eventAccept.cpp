#pragma once

#include <bromsock/lua/event/eventAccept.h>
#include <GarrysMod/Lua/Interface.h>

namespace bromsock {
    namespace event {
        const std::string& EventAccept::getNameStatic() {
            static std::string name = "onAccept";
            return name;
        }

        const std::string& EventAccept::getName() const {
            return getNameStatic();
        }

        int EventAccept::pushArguments(lua_State* state) const {
            return 0;
        }

        void EventAccept::process(Socket* sock) {
            
        }
    }
}
