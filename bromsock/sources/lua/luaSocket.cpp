#include <bromsock/lua/luaSocket.h>
#include <bromsock/lua/luaPacket.h>
#include <bromsock/packet.h>
#include <bromsock/engine.h>

#include <bromsock/lua/event/eventConnect.h>
#include <bromsock/lua/event/eventDisconnect.h>
#include <bromsock/lua/event/eventRead.h>
#include <bromsock/lua/event/eventReadFrom.h>
#include <bromsock/lua/event/eventWrite.h>
#include <bromsock/lua/event/eventWriteTo.h>
#include <bromsock/lua/event/eventAccept.h>

#include <fmt/format.h>

#ifndef BROMSOCK_NOSSL
#include <openssl/ssl.h>
#include <openssl/err.h>
#endif

namespace bromsock
{
    namespace lua
    {
        LUA_FUNCTION(CreateSocket)
        {
            DEBUGPRINTFUNC;

            int type = IPPROTO_TCP;
            bool ipv6 = false;
            if (LUA->IsType(1, GarrysMod::Lua::Type::Number))
            {
                type = (int)LUA->CheckNumber(1);
            }

            if (LUA->GetType(2) == GarrysMod::Lua::Type::Bool)
            {
                ipv6 = LUA->GetBool(2);
            }

            auto sock = std::make_unique<Socket>(state, type, ipv6);
            sock->pushToStack(state);

            Engine::getInstance(state).addSocket(std::move(sock));
            return 1;
        }

        LUA_FUNCTION(SOCK_Connect)
        {
            DEBUGPRINTFUNC;

            LUA->CheckType(1, UD_TYPE_SOCKET);
            LUA->CheckType(2, GarrysMod::Lua::Type::String);
            LUA->CheckType(3, GarrysMod::Lua::Type::Number);

            Socket* s = GETSOCK(1);
            s->sock.create(s->type);

            auto& eventName = event::EventConnect::getNameStatic();
            if (s->hasCallback(eventName))
            {
                auto event = std::make_unique<event::EventConnect>();
                event->setIp(LUA->GetString(2));
                event->setPort(static_cast<int>(LUA->GetNumber(3)));

                s->addEvent(std::move(event));
                return 0;
            }
            else
            {
                LUA->PushBool(s->sock.connect(LUA->GetString(2), (unsigned short)LUA->GetNumber(3)) == mainframe::networking::SocketError::success);
                return 1;
            }
        }

        LUA_FUNCTION(SOCK_Listen)
        {
            DEBUGPRINTFUNC;

            LUA->CheckType(1, UD_TYPE_SOCKET);

            //const char *ip = "0.0.0.0";
            unsigned short port;

            if (LUA->IsType(3, GarrysMod::Lua::Type::Number) && LUA->IsType(2, GarrysMod::Lua::Type::String))
            {
                //ip = LUA->GetString(2);
                port = (unsigned short)LUA->GetNumber(3);
            }
            else if (LUA->IsType(2, GarrysMod::Lua::Type::Number))
            {
                port = (unsigned short)LUA->GetNumber(2);
            }
            else
            {
                LUA->PushBool((GETSOCK(1))->sock.listen());
                return 1;
            }

            Socket* s = GETSOCK(1);
            s->sock.create(s->type);

            LUA->PushBool(s->sock.bind(port) && s->sock.listen());
            return 1;
        }

        LUA_FUNCTION(SOCK_Bind)
        {
            DEBUGPRINTFUNC;

            LUA->CheckType(1, UD_TYPE_SOCKET);

            //const char *ip = "0.0.0.0";
            unsigned short port = 0;

            if (LUA->IsType(3, GarrysMod::Lua::Type::Number) && LUA->IsType(2, GarrysMod::Lua::Type::String))
            {
                //ip = LUA->GetString(2);
                port = (unsigned short)LUA->GetNumber(3);
            }
            else if (LUA->IsType(2, GarrysMod::Lua::Type::Number))
            {
                port = (unsigned short)LUA->GetNumber(2);
            }
            else
            {
                LUA->ThrowError("Expected (string ip, number port), or just a number port");
            }

            Socket* s = GETSOCK(1);
            s->sock.create(s->type);

            LUA->PushBool(s->sock.bind(port));
            return 1;
        }

        LUA_FUNCTION(SOCK_SetBlocking)
        {
            DEBUGPRINTFUNC;

            LUA->CheckType(1, UD_TYPE_SOCKET);
            LUA->CheckType(2, GarrysMod::Lua::Type::Bool);

            (GETSOCK(1))->sock.blocking = LUA->GetBool(2);

            return 0;
        }

#ifndef BROMSOCK_NOSSL
        SSL_CTX *InitCTX(void)
        {
            const SSL_METHOD *method;
            SSL_CTX *ctx;

            OpenSSL_add_all_algorithms();
            SSL_load_error_strings();
            method = TLS_client_method();
            ctx = SSL_CTX_new(method);

            if (ctx == NULL)
            {
                ERR_print_errors_fp(stderr);
                abort();
            }

            return ctx;
        }
#endif

        LUA_FUNCTION(SOCK_StartSSLClient)
        {
            DEBUGPRINTFUNC;

#ifndef BROMSOCK_NOSSL
            LUA->CheckType(1, UD_TYPE_SOCKET);
            Socket* sw = GETSOCK(1);

            sw->sslCtx = InitCTX();
            sw->ssl = SSL_new(sw->sslCtx);
            SSL_set_fd(sw->ssl, sw->sock.sock);

            if (SSL_connect(sw->ssl) == -1)
            {
                ERR_print_errors_fp(stderr);

                SSL_CTX_free(sw->sslCtx);
                SSL_free(sw->ssl);

                sw->sslCtx = nullptr;
                sw->ssl = nullptr;

                LUA->PushBool(false);
                return 1;
            }

            LUA->PushBool(true);
            return 1;
#else
            LUA->ThrowError("bromsock has been build without SSL support, download/build the version with SSL!");
#endif
            return 0;
        }

        LUA_FUNCTION(SOCK_Disconnect)
        {
            DEBUGPRINTFUNC;

            LUA->CheckType(1, UD_TYPE_SOCKET);

            (GETSOCK(1))->sock.close();

            return 0;
        }

        LUA_FUNCTION(SOCK_SendTo)
        {
            DEBUGPRINTFUNC;

            LUA->CheckType(1, UD_TYPE_SOCKET);
            LUA->CheckType(2, UD_TYPE_PACKET);
            LUA->CheckType(3, GarrysMod::Lua::Type::String);
            LUA->CheckType(4, GarrysMod::Lua::Type::Number);

            Socket* s = GETSOCK(1);
            Packet* p = GETPACK(2);

            s->sock.create(s->type);

            auto event = std::make_unique<event::EventWriteTo>();
            event->ip = LUA->GetString(3);
            event->port = static_cast<int>(LUA->GetNumber(4));
            event->buffer = std::vector(p->OutBuffer, p->OutBuffer + p->OutPos);

            s->addEvent(std::move(event));

            p->Clear();
            return 0;
        }

        LUA_FUNCTION(SOCK_Send)
        {
            DEBUGPRINTFUNC;

            LUA->CheckType(1, UD_TYPE_SOCKET);
            LUA->CheckType(2, UD_TYPE_PACKET);

            Socket* s = GETSOCK(1);
            Packet* p = GETPACK(2);

            auto event = std::make_unique<event::EventWrite>();
            event->buffer = std::vector(p->OutBuffer, p->OutBuffer + p->OutPos);
            event->writeSize = !LUA->IsType(3, GarrysMod::Lua::Type::Bool) || !LUA->GetBool(3);

            s->addEvent(std::move(event));
            p->Clear();

            return 0;
        }

        LUA_FUNCTION(SOCK_SetMaxReceiveSize)
        {
            DEBUGPRINTFUNC;
            LUA->CheckType(1, UD_TYPE_SOCKET);
            LUA->CheckType(2, GarrysMod::Lua::Type::Number);

            Socket* s = GETSOCK(1);
            s->maxReceiveSize = static_cast<int>(LUA->GetNumber(2));

            return 0;
        }

        LUA_FUNCTION(SOCK_Receive)
        {
            DEBUGPRINTFUNC;

            LUA->CheckType(1, UD_TYPE_SOCKET);

            Socket* s = GETSOCK(1);
            auto event = std::make_unique<event::EventRead>();
            event->amount = LUA->IsType(2, GarrysMod::Lua::Type::Number) ? (int)LUA->GetNumber(2) : 0;

            s->addEvent(std::move(event));

            return 0;
        }

        LUA_FUNCTION(SOCK_ReceiveFrom)
        {
            DEBUGPRINTFUNC;

            LUA->CheckType(1, UD_TYPE_SOCKET);

            auto amount = LUA->IsType(2, GarrysMod::Lua::Type::Number) ? static_cast<int>(LUA->GetNumber(2)) : 65535; // max "theoretical" diagram size;
            if (amount <= 0) LUA->ThrowError("amount must be more than 0");

            Socket* s = GETSOCK(1);

            if (LUA->IsType(3, GarrysMod::Lua::Type::String) && LUA->IsType(4, GarrysMod::Lua::Type::Number))
            {
                auto event = std::make_unique<event::EventReadFrom>();
                event->amount = static_cast<size_t>(amount);
                event->ip = LUA->GetString(3);
                event->port = static_cast<int>(LUA->GetNumber(4));

                if (event->amount > s->maxReceiveSize)
                {
                    auto err = fmt::format("error in bromsock_C++ allocating buffer to receive {} bytes from {}", event->amount, s->sock.getIpAddress());
                    LUA->ThrowError(err.c_str());
                    return 0;
                }
                s->addEvent(std::move(event));
            } else {
                auto event = std::make_unique<event::EventRead>();
                event->amount = static_cast<size_t>(amount);

                if (event->amount > s->maxReceiveSize)
                {
                    auto err = fmt::format("error in bromsock_C++ allocating buffer to receive {} bytes from {}", event->amount, s->sock.getIpAddress());
                    LUA->ThrowError(err.c_str());
                    return 0;
                }
                s->addEvent(std::move(event));
            }

            return 0;
        }

        LUA_FUNCTION(SOCK_ReceiveUntil)
        {
            DEBUGPRINTFUNC;

            LUA->CheckType(1, UD_TYPE_SOCKET);
            LUA->CheckType(2, GarrysMod::Lua::Type::String);

            Socket* s = GETSOCK(1);

            auto event = std::make_unique<event::EventRead>();
            event->sequence = LUA->GetString(2);

            s->addEvent(std::move(event));
            return 0;
        }

        LUA_FUNCTION(SOCK_Accept)
        {
            DEBUGPRINTFUNC;

            LUA->CheckType(1, UD_TYPE_SOCKET);
            Socket* s = GETSOCK(1);

            auto event = std::make_unique<event::EventAccept>();
            s->addEvent(std::move(event));

            return 0;
        }

        LUA_FUNCTION(SOCK__GC)
        {
            DEBUGPRINTFUNC;

            LUA->CheckType(1, UD_TYPE_SOCKET);
            Socket* s = GETSOCK(1);
            auto& e = Engine::getInstance(state);
            e.removeSocket(s);

            return 0;
        }

        LUA_FUNCTION(SOCK_AddWorker)
        {
            DEBUGPRINTFUNC;

            LUA->ThrowError("deprecated, there should be no reason to have more workers, make an issue on github please.");
            return 0;
        }

        LUA_FUNCTION(SOCK_SetOption)
        {
            DEBUGPRINTFUNC;

            LUA->CheckType(1, UD_TYPE_SOCKET);
            LUA->CheckType(2, GarrysMod::Lua::Type::Number);
            LUA->CheckType(3, GarrysMod::Lua::Type::Number);
            LUA->CheckType(4, GarrysMod::Lua::Type::Number);

            int level = (int)LUA->GetNumber(2);
            int option = (int)LUA->GetNumber(3);
            int value = (int)LUA->GetNumber(4);

            Socket* s = GETSOCK(1);

#ifdef _MSC_VER
            DWORD value_win = (DWORD)value;
            int ret = setsockopt(s->sock.sock, level, option, (const char *)&value_win, sizeof(value_win));
#else
            int ret = setsockopt(s->sock.sock, level, option, &value, sizeof(value));
#endif

            // failed == -1
            LUA->PushNumber(ret);
            return 1;
        }

        LUA_FUNCTION(SOCK_SetTimeout)
        {
            DEBUGPRINTFUNC;

            LUA->CheckType(1, UD_TYPE_SOCKET);
            LUA->CheckType(2, GarrysMod::Lua::Type::Number);
            Socket* s = GETSOCK(1);

#ifdef _MSC_VER
            DWORD dwTime = (DWORD)LUA->GetNumber(2);
            int reta = setsockopt(s->sock.sock, SOL_SOCKET, SO_SNDTIMEO, (const char *)&dwTime, sizeof(dwTime));
            int retb = setsockopt(s->sock.sock, SOL_SOCKET, SO_RCVTIMEO, (const char *)&dwTime, sizeof(dwTime));
#else
            int dwTime = (int)LUA->GetNumber(2);
            int reta = setsockopt(s->sock.sock, SOL_SOCKET, SO_SNDTIMEO, &dwTime, sizeof(dwTime));
            int retb = setsockopt(s->sock.sock, SOL_SOCKET, SO_RCVTIMEO, &dwTime, sizeof(dwTime));
#endif

            // Should be SOCKET_ERROR, but linux does not have SOCKET_ERROR defined.
            LUA->PushBool(reta != -1 && retb != -1);
            return 1;
        }

        LUA_FUNCTION(SOCK_Create)
        {
            DEBUGPRINTFUNC;

            LUA->CheckType(1, UD_TYPE_SOCKET);

            Socket* s = GETSOCK(1);
            s->sock.create(s->type);

            return 0;
        }

        LUA_FUNCTION(SOCK_GetLastError)
        {
            DEBUGPRINTFUNC;

            char lastError[256];
            memset(lastError, 0, sizeof(lastError));
            #ifdef _MSC_VER
                int errCode = WSAGetLastError();
                if(!FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                    NULL, errCode,
                    MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
                    lastError, sizeof(lastError), NULL)){
                    // FormatMessage failed, set lastError to just the code instead.
                    snprintf(lastError, sizeof(lastError), "Code: %d", errCode);
                }
            #else
                char* errptr = strerror_r(errno, lastError, sizeof(lastError)); // GNU version of strerror_r
                snprintf(lastError, sizeof(lastError), "%s", errptr);
            #endif

            LUA->CheckType(1, UD_TYPE_SOCKET);
            LUA->PushString(lastError);

            return 1;
        }

        LUA_FUNCTION(SOCK__TOSTRING)
        {
            DEBUGPRINTFUNC;

            LUA->CheckType(1, UD_TYPE_SOCKET);
            Socket* s = GETSOCK(1);

            // TODO: add port
            auto tostr = fmt::format("bromsock: {}", s->sock.getIpAddress());
            LUA->PushString(tostr.c_str());

            return 1;
        }

        LUA_FUNCTION(SOCK__EQ)
        {
            DEBUGPRINTFUNC;

            LUA->CheckType(1, UD_TYPE_SOCKET);
            LUA->CheckType(2, UD_TYPE_SOCKET);

            LUA->PushBool(GETSOCK(1) == GETSOCK(2));

            return 1;
        }

        LUA_FUNCTION(SOCK_CALLBACKSend)
        {
            DEBUGPRINTFUNC;
            LUA->CheckType(1, UD_TYPE_SOCKET);
            LUA->CheckType(2, GarrysMod::Lua::Type::Function);
            LUA->Push(2);

            GETSOCK(1)->setCallback(event::EventWrite::getNameStatic(), LUA->ReferenceCreate());
            return 0;
        }

        LUA_FUNCTION(SOCK_CALLBACKSendTo)
        {
            DEBUGPRINTFUNC;
            LUA->CheckType(1, UD_TYPE_SOCKET);
            LUA->CheckType(2, GarrysMod::Lua::Type::Function);
            LUA->Push(2);

            GETSOCK(1)->setCallback(event::EventWriteTo::getNameStatic(), LUA->ReferenceCreate());
            return 0;
        }

        LUA_FUNCTION(SOCK_CALLBACKReceive)
        {
            DEBUGPRINTFUNC;
            LUA->CheckType(1, UD_TYPE_SOCKET);
            LUA->CheckType(2, GarrysMod::Lua::Type::Function);
            LUA->Push(2);

            GETSOCK(1)->setCallback(event::EventRead::getNameStatic(), LUA->ReferenceCreate());
            return 0;
        }

        LUA_FUNCTION(SOCK_CALLBACKReceiveFrom)
        {
            DEBUGPRINTFUNC;
            LUA->CheckType(1, UD_TYPE_SOCKET);
            LUA->CheckType(2, GarrysMod::Lua::Type::Function);
            LUA->Push(2);

            GETSOCK(1)->setCallback(event::EventReadFrom::getNameStatic(), LUA->ReferenceCreate());
            return 0;
        }

        LUA_FUNCTION(SOCK_CALLBACKConnect)
        {
            DEBUGPRINTFUNC;
            LUA->CheckType(1, UD_TYPE_SOCKET);
            LUA->CheckType(2, GarrysMod::Lua::Type::Function);
            LUA->Push(2);

            GETSOCK(1)->setCallback(event::EventConnect::getNameStatic(), LUA->ReferenceCreate());
            return 0;
        }

        LUA_FUNCTION(SOCK_CALLBACKAccept)
        {
            DEBUGPRINTFUNC;
            LUA->CheckType(1, UD_TYPE_SOCKET);
            LUA->CheckType(2, GarrysMod::Lua::Type::Function);
            LUA->Push(2);

            GETSOCK(1)->setCallback(event::EventAccept::getNameStatic(), LUA->ReferenceCreate());
            return 0;
        }

        LUA_FUNCTION(SOCK_CALLBACKDisconnect)
        {
            DEBUGPRINTFUNC;
            LUA->CheckType(1, UD_TYPE_SOCKET);
            LUA->CheckType(2, GarrysMod::Lua::Type::Function);
            LUA->Push(2);

            GETSOCK(1)->setCallback(event::EventDisconnect::getNameStatic(), LUA->ReferenceCreate());
            return 0;
        }

        LUA_FUNCTION(SOCK_IsValid)
        {
            DEBUGPRINTFUNC;
            LUA->CheckType(1, UD_TYPE_SOCKET);
            LUA->PushBool((GETSOCK(1))->sock.state == mainframe::networking::Socket::SockState::skCONNECTED || GETSOCK(1)->sock.state == mainframe::networking::Socket::SockState::skLISTENING);
            return 1;
        }

        LUA_FUNCTION(SOCK_GetIP)
        {
            DEBUGPRINTFUNC;
            LUA->CheckType(1, UD_TYPE_SOCKET);
            LUA->PushString(GETSOCK(1)->sock.getIpAddress().c_str());
            return 1;
        }

        LUA_FUNCTION(SOCK_GetPort)
        {
            DEBUGPRINTFUNC;
            LUA->CheckType(1, UD_TYPE_SOCKET);
            LUA->PushNumber(ntohs(GETSOCK(1)->sock.addr.sin_port));
            return 1;
        }

        LUA_FUNCTION(SOCK_GetState)
        {
            DEBUGPRINTFUNC;
            LUA->CheckType(1, UD_TYPE_SOCKET);
            LUA->PushNumber(static_cast<int>(GETSOCK(1)->sock.state));
            return 1;
        }

        LUA_FUNCTION(SOCK_RegisterTypes)
        {
            DEBUGPRINTFUNC;
            auto& engine = Engine::getInstance(state);

            // register our class
            LUA->CreateTable();
                ADDFUNC("SetBlocking", SOCK_SetBlocking);
                ADDFUNC("Connect", SOCK_Connect);
                ADDFUNC("StartSSLClient", SOCK_StartSSLClient);
                ADDFUNC("Close", SOCK_Disconnect);
                ADDFUNC("Create", SOCK_Create);
                ADDFUNC("Disconnect", SOCK_Disconnect);
                ADDFUNC("Bind", SOCK_Bind);
                ADDFUNC("Listen", SOCK_Listen);
                ADDFUNC("Send", SOCK_Send);
                ADDFUNC("SendTo", SOCK_SendTo);
                ADDFUNC("Accept", SOCK_Accept);
                ADDFUNC("Receive", SOCK_Receive);
                ADDFUNC("ReceiveFrom", SOCK_ReceiveFrom);
                ADDFUNC("ReceiveUntil", SOCK_ReceiveUntil);
                ADDFUNC("GetIP", SOCK_GetIP);
                ADDFUNC("GetPort", SOCK_GetPort);
                ADDFUNC("GetState", SOCK_GetState);
                ADDFUNC("GetLastError", SOCK_GetLastError);
                ADDFUNC("AddWorker", SOCK_AddWorker);
                ADDFUNC("SetTimeout", SOCK_SetTimeout);
                ADDFUNC("SetOption", SOCK_SetOption);
                ADDFUNC("SetMaxReceiveSize", SOCK_SetMaxReceiveSize);
                ADDFUNC("SetCallbackReceive", SOCK_CALLBACKReceive);
                ADDFUNC("SetCallbackReceiveFrom", SOCK_CALLBACKReceiveFrom);
                ADDFUNC("SetCallbackSend", SOCK_CALLBACKSend);
                ADDFUNC("SetCallbackSendTo", SOCK_CALLBACKSendTo);
                ADDFUNC("SetCallbackConnect", SOCK_CALLBACKConnect);
                ADDFUNC("SetCallbackAccept", SOCK_CALLBACKAccept);
                ADDFUNC("SetCallbackDisconnect", SOCK_CALLBACKDisconnect);

            auto tempTableRef = LUA->ReferenceCreate();

            LUA->CreateTable();
                LUA->ReferencePush(tempTableRef);
                LUA->SetField(-2, "__index");
                LUA->ReferencePush(tempTableRef);
                LUA->SetField(-2, "__newindex");
                ADDFUNC("__tostring", SOCK__TOSTRING);
                ADDFUNC("__eq", SOCK__EQ);
                ADDFUNC("__gc", SOCK__GC);
            engine.socketRef = LUA->ReferenceCreate();

            // clear temp
            LUA->ReferenceFree(tempTableRef);

            // register our globals
            LUA->PushSpecial(GarrysMod::Lua::SPECIAL_GLOB);
                LUA->PushString("BromSock");
                LUA->PushCFunction(CreateSocket);
            LUA->SetTable(-3);

            LUA->PushSpecial(GarrysMod::Lua::SPECIAL_GLOB);
                LUA->PushString("BROMSOCK_TCP");
                LUA->PushNumber(IPPROTO_TCP);
            LUA->SetTable(-3);

            LUA->PushSpecial(GarrysMod::Lua::SPECIAL_GLOB);
                LUA->PushString("BROMSOCK_UDP");
                LUA->PushNumber(IPPROTO_UDP);
            LUA->SetTable(-3);

            return 0;
        }
    }
}