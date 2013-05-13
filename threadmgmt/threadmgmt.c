#include "threadmgmt.h"
#include <signal.h>
#include <string.h>
#include <pthread.h>

#define CPU_RELAX() asm("pause")
#define SMP_WB()

typedef struct thread_mgmt_op
{
    int op;
    volatile int done;
    volatile int res;
}thread_mgmt_op_t;

struct sigaction old_sigusr1;
static __thread volatile int thread_state = 1;

static void thread_mgmt_handler(int, siginfo_t*, void*);
static int thread_mgmt_send_op(pthread_t, int);


int thread_mgmt_init(void)
{
  struct sigaction sa;
  memset(&sa, 0, sizeof(sa));
  sa.sa_sigaction = (void*)thread_mgmt_handler;
  sa.sa_flags = SA_SIGINFO;      
  sigfillset(&sa.sa_mask);
  //register signal handler which will full signal mask
  return sigaction(USED_SIG, &sa, NULL);
}

int thread_mgmt_release()
{
    //restore previous signal handler
    return sigaction(USED_SIG, &old_sigusr1, NULL);
}

int thread_mgmt_suspend(pthread_t t)
{
    return thread_mgmt_send_op(t, 0);
}

int thread_mgmt_resume(pthread_t t)
{    
    return thread_mgmt_send_op(t, 1);
}

static int thread_mgmt_send_op(pthread_t t, int opnum)
{
    thread_mgmt_op_t op = {.op = opnum, .done = 0, .res = 0};
    sigval_t val = {.sival_ptr = &op};
    if(pthread_sigqueue(t, USED_SIG, val))
        return -1;
    
    //spin wait till signal is delivered
    while(!op.done) 
        CPU_RELAX();
    
    return op.res;
}

static void thread_mgmt_handler(int signum, siginfo_t* info, void* ctx)
{
    thread_mgmt_op_t *op = (thread_mgmt_op_t*)(info->si_value.sival_ptr);
    if(op->op == 0 && thread_state == 1)
    {
        //suspend
        thread_state = 0;
        op->res = 0;
        SMP_WB();
        op->done = 1;
        
        sigset_t mask;
        sigfillset(&mask);
        sigdelset(&mask, USED_SIG);

        //wait till SIGUSR1
        sigsuspend(&mask);
    }
    else if(op->op == 1 && thread_state == 0)
    {
        //resume
        thread_state = 1;
        op->res = 0;
        SMP_WB();
        op->done = 1;
    }
    else
    {
        //resume resumed thread or
        //suspend suspended thread
        op->res = -1;
        SMP_WB();
        op->done = 1;
    }
}
