#include <GarrysMod/Lua/interface.h>
#include <bromsock/engine.h>

#include <openssl/ssl.h>

#include <map>
#include <memory>

using namespace bromsock;

GMOD_MODULE_OPEN(){
	SSL_library_init();

    auto& e = Engine::createInstance(state);
    e.init();

	return 0;
}

GMOD_MODULE_CLOSE(){
    auto ptr = Engine::removeInstance(state);
    ptr->shutdown();

	return 0;
}
