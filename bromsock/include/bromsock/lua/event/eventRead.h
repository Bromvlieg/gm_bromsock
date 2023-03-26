#pragma once

#include <bromsock/lua/event/eventBase.h>

#include <GarrysMod/Lua/interface.h>

#include <vector>

namespace bromsock {
    namespace event {
        class EventRead : public EventBase {
            std::vector<uint8_t> buffer;
            bool response = false;
            bool read(Socket* sock, uint8_t* data, size_t size);

        public:
            size_t amount = 0;
            std::string sequence;

            static const std::string& getNameStatic();

            virtual const std::string& getName() const override;
            virtual int pushArguments(lua_State* state) const override;
            virtual void process(Socket* sock) override;
        };
    }
}
