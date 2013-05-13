#pragma once
#include <pthread.h>

#define USED_SIG SIGUSR1

#ifdef	__cplusplus
extern "C" {
#endif

int thread_mgmt_init(void);

int thread_mgmt_release(void);

int thread_mgmt_suspend(pthread_t t);

int thread_mgmt_resume(pthread_t t);


#ifdef	__cplusplus
}
#endif


