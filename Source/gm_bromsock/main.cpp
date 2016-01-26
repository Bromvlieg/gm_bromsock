#include "Defines.h"

#include "Gmod_Headers/Lua/Interface.h"
#include "Objects/Engine.h"
#include "Objects/LockObject.h"

#ifndef BROMSOCK_NOSSL
#include <openssl/ssl.h>
#endif

using namespace GMBSOCK;

GMOD_MODULE_OPEN(){
	DEBUGPRINTFUNC;

#ifndef BROMSOCK_NOSSL
	SSL_library_init();
#endif

	static LockObject l;
	l.Lock();

	if (Engine::S_Lock == nullptr) {
		Engine::S_Lock = new LockObject();
	}

	l.Unlock();

	Engine* e = new Engine();
	e->Init(state);

	return 0;
}

GMOD_MODULE_CLOSE(){
	DEBUGPRINTFUNC;

	Engine* engine = Engine::GetEngineByState(state);
	engine->Shutdown(state);

	delete engine;

	return 0;
}
