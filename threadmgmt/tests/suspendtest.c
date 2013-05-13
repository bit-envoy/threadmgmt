#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "threadmgmt.h"

void*  function(void*  arg)
{
    int i = 0;
    while(1) printf("thread(%d) %p\n", arg, i++);
    return 0;
}

int main( void )
{
    if(thread_mgmt_init())
        return -1;
    
   pthread_t t1, t2;
   pthread_create(&t1, NULL, function, (void*)1);
   pthread_create(&t2, NULL, function, (void*)2);

   
   sleep(1);
   if(thread_mgmt_suspend(t1))
       return -2;

   if(thread_mgmt_suspend(t2))
       return -2;
   
   sleep(2);
   if(thread_mgmt_resume(t1))
       return -3;

   if(thread_mgmt_resume(t2))
       return -3;
   
   sleep(4);
   thread_mgmt_release();
   
   return 0;
}