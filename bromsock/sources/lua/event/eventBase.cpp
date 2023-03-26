#pragma once

#include <bromsock/lua/event/eventBase.h>
#include <bromsock/socket.h>

#include <GarrysMod/Lua/interface.h>

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