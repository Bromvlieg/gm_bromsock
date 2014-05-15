#include "LockObject.h"

LockObject::LockObject(void) {
	this->CurrentLocks = 0;
#ifdef _MSC_VER
	InitializeCriticalSectionAndSpinCount(&this->LockHandle, 0x00000400);
#else
	pthread_mutex_init(&this->LockHandle, NULL);
#endif
}

LockObject::~LockObject(void){
#ifdef _MSC_VER
	DeleteCriticalSection(&this->LockHandle);
#else
	pthread_mutex_destroy(&this->LockHandle);
#endif
}

void LockObject::Lock(){
#ifdef _MSC_VER
	EnterCriticalSection(&this->LockHandle);
#else
	pthread_mutex_lock(&this->LockHandle);
#endif

	this->CurrentLocks++;
}

void LockObject::Unlock(){
#ifdef _MSC_VER
	LeaveCriticalSection(&this->LockHandle);
#else
	pthread_mutex_unlock(&this->LockHandle);
#endif

	this->CurrentLocks--;
}