#ifndef __H_GMBSOCK_O_LOCKOBJECT
#define __H_GMBSOCK_O_LOCKOBJECT

#ifdef _MSC_VER
#include "Windows.h"
#else
#include <pthread.h>
#endif

namespace GMBSOCK {
	class LockObject {
	public:
		LockObject(void);
		~LockObject(void);

		void Lock();
		void Unlock();

	private:
		int CurrentLocks;

#ifdef _MSC_VER
		CRITICAL_SECTION LockHandle;
#else
		pthread_mutex_t LockHandle;
#endif
	};
}

#endif