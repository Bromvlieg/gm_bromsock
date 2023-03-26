#include <bromsock/lua/event/eventBase.h>
#include <bromsock/socket.h>

#include <GarrysMod/Lua/Interface.h>

namespace bromsock {
    namespace event {
        void EventBase::setShouldDisconnect(bool should) {
            shouldDisconnect = should;
        }

        bool EventBase::getShouldDisconnect() {
            return shouldDisconnect;
        }
    }
}