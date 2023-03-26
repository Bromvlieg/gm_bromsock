#pragma once

#include <mainframe/networking/socket.h>
#include <mainframe/utils/ringbuffer.hpp>

#include <bromsock/lua/event/eventBase.h>

#include <GarrysMod/Lua/interface.h>

#include <map>
#include <mutex>
#include <thread>
#include <memory>
#include <string>

#ifndef BROMSOCK_NOSSL
#include <openssl/ssl.h>
#endif

namespace bromsock {
    class Socket {
        std::vector<std::thread*> workers;

        mainframe::utils::ringbuffer<std::unique_ptr<event::EventBase>> eventsQueue;
        mainframe::utils::ringbuffer<std::unique_ptr<event::EventBase>> eventsFinished;
        std::map<std::string, int> callbacks;
        bool shutdown = false;

        void workerLoop();

        public:
            lua_State* state = nullptr;
            void* luaReference = 0;
            int type = 0;
            bool ipv6 = false;

            size_t maxReceiveSize = 1024 * 1024 * 10; // 10mb

#ifndef BROMSOCK_NOSSL
            SSL_CTX* sslCtx = nullptr;
    		SSL* ssl = nullptr;
#endif

            mainframe::networking::Socket sock;

            Socket(lua_State* state, int type, bool ipv6);
            ~Socket();

            void beginShutdown();

            void pushToStack(lua_State* state);

            bool hasCallback(const std::string& name) const;
            int getCallback(const std::string& name) const;
            void setCallback(const std::string& name, int callback);

            void addEvent(std::unique_ptr<event::EventBase> event);
            bool hasEvent();
            std::unique_ptr<event::EventBase> getNextEvent();
    };
}
