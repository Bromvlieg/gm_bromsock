#include <bromsock/lua/event/eventWriteTo.h>
#include <GarrysMod/Lua/Interface.h>

namespace bromsock {
    namespace event {
        const std::string& EventWriteTo::getNameStatic() {
            static std::string name = "onSendTo";
            return name;
        }

        const std::string& EventWriteTo::getName() const {
            return getNameStatic();
        }

        int EventWriteTo::pushArguments(lua_State* state) const {
            return 0;
        }

        void EventWriteTo::process(Socket* sock) {

        }
    }
}
