#include <bromsock/lua/event/eventShutdown.h>
#include <GarrysMod/Lua/Interface.h>

namespace bromsock {
    namespace event {
        const std::string& EventShutdown::getNameStatic() {
            static std::string name = "onShutdown";
            return name;
        }

        const std::string& EventShutdown::getName() const {
            return getNameStatic();
        }

        int EventShutdown::pushArguments(lua_State* state) const {
            return 0;
        }

        void EventShutdown::process(Socket* sock) {
            
        }
    }
}
