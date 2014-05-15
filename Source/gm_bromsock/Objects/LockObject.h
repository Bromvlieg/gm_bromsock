#ifndef BROMMC_LOCKOBJECT
#define BROMMC_LOCKOBJECT

#ifdef _MSC_VER
#include "Windows.h"
#else
#include <pthread.h>
#endif

class LockObject
{
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

#endif