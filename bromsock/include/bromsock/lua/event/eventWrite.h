#pragma once

#include <bromsock/lua/event/eventBase.h>

#include <GarrysMod/Lua/Interface.h>

#include <vector>

namespace bromsock {
    namespace event {
        class EventWrite : public EventBase {
            bool response = false;

            bool write(Socket* sock, const uint8_t* data, size_t size);
        public:

            std::vector<uint8_t> buffer;
            bool writeSize = false;

            static const std::string& getNameStatic();

            virtual const std::string& getName() const override;
            virtual int pushArguments(lua_State* state) const override;
            virtual void process(Socket* sock) override;
        };
    }
}
