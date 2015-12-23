#include "LockObject.h"

#ifdef _MSC_VER
#define _m_init(hndl) InitializeCriticalSectionAndSpinCount(&hndl, 0x00000400);
#define _m_destroy(hndl) DeleteCriticalSection(&hndl);
#define _m_lock(hndl) EnterCriticalSection(&hndl);
#define _m_unlock(hndl) LeaveCriticalSection(&hndl);
#else
#define _m_init(hndl) pthread_mutex_init(&hndl, NULL);
#define _m_destroy(hndl) pthread_mutex_destroy(&hndl);
#define _m_lock(hndl) pthread_mutex_lock(&hndl);
#define _m_unlock(hndl) pthread_mutex_unlock(&hndl);
#endif

namespace GMBSOCK {
	LockObject::LockObject(void) : CurrentLocks(0) {
		_m_init(this->LockHandle);
	}

	LockObject::~LockObject(void) {
		_m_destroy(this->LockHandle);
	}

	void LockObject::Lock() {
		_m_lock(this->LockHandle);
		this->CurrentLocks++;
	}

	void LockObject::Unlock() {
		_m_unlock(this->LockHandle);
		this->CurrentLocks--;
	}
}