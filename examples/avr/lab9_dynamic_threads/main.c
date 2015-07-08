/*===========================================================================
     _____        _____        _____        _____
 ___|    _|__  __|_    |__  __|__   |__  __| __  |__  ______
|    \  /  | ||    \      ||     |     ||  |/ /     ||___   |
|     \/   | ||     \     ||     \     ||     \     ||___   |
|__/\__/|__|_||__|\__\  __||__|\__\  __||__|\__\  __||______|
    |_____|      |_____|      |_____|      |_____|

--[Mark3 Realtime Platform]--------------------------------------------------

Copyright (c) 2012-2015 Funkenstein Software Consulting, all rights reserved.
See license.txt for more information
===========================================================================*/
#include "mark3.h"

/*===========================================================================

Lab Example 9:  Dynamic Threading

Lessons covered in this example include:
- Creating, pausing, and destorying dynamically-created threads at runtime

Takeaway:

In addition to being able to specify a static set of threads during system
initialization, Mark3 gives the user the ability to create and manipulate
threads at runtime.  These threads can act as "temporary workers" that can
be activated when needed, without impacting the responsiveness of the rest
of the application.

===========================================================================*/

//---------------------------------------------------------------------------
// This block declares the thread data for one main application thread.  It
// defines a thread object, stack (in word-array form), and the entry-point
// function used by the application thread.
#define APP1_STACK_SIZE      (320/sizeof(K_WORD))
static Thread_t  clApp1Thread;
static K_WORD  awApp1Stack[APP1_STACK_SIZE];
static void    App1Main(void *unused_);

//---------------------------------------------------------------------------
// This block declares the thread stack data for a thread that we'll create
// dynamically.
#define APP2_STACK_SIZE      (320/sizeof(K_WORD))
static K_WORD  awApp2Stack[APP2_STACK_SIZE];

//---------------------------------------------------------------------------
int main(void)
{
    // See the annotations in previous labs for details on init.
    Kernel_Init();

    Thread_Init( &clApp1Thread, awApp1Stack,  APP1_STACK_SIZE,  1, App1Main,  0);
    Thread_Start( &clApp1Thread );

    Kernel_Start();

    return 0;
}

//---------------------------------------------------------------------------
static void WorkerMain1(void *arg_)
{
    Semaphore_t *pclSem = (Semaphore_t*)arg_;
    K_ULONG ulCount = 0;

    // Do some work.  Post a semaphore to notify the other thread that the
    // work has been completed.
    while (ulCount < 10000)
    {
        ulCount++;
    }

    KernelAware_Print( "Worker1 -- Done Work\n");
    Semaphore_Post( pclSem );

    // Work is completed, just spin now.  Let another thread destory us.
    while(1) { }
}
//---------------------------------------------------------------------------
static void WorkerMain2(void *arg_)
{
    K_ULONG ulCount = 0;
    while (ulCount < 10000)
    {
        ulCount++;
    }

    KernelAware_Print( "Worker2 -- Done Work\n");

    // A dynamic thread can self-terminate as well:
    Thread_Exit( Scheduler_GetCurrentThread() );
}

//---------------------------------------------------------------------------
void App1Main(void *unused_)
{
    Thread_t clMyThread;
    Semaphore_t clMySem;

    Semaphore_Init( &clMySem, 0,1);
    while (1)
    {
        // Example 1 - create a worker thread at our current priority in order to
        // parallelize some work.
        Thread_Init( &clMyThread, awApp2Stack, APP2_STACK_SIZE, 1, WorkerMain1, (void*)&clMySem );
        Thread_Start( &clMyThread );

        // Do some work of our own in parallel, while the other thread works on its project.
        K_ULONG ulCount = 0;
        while (ulCount < 10000)
        {
            ulCount++;
        }

        KernelAware_Print( "Thread -- Done Work\n" );

        // Wait for the other thread to finish its job.
        Semaphore_Pend( &clMySem );

        // Once the thread has signalled us, we can safely call "Exit" on the thread to
        // remove it from scheduling and recycle it later.
        Thread_Exit( &clMyThread );

        // Spin the thread up again to do something else in parallel.  This time, the thread
        // will run completely asynchronously to this thread.
        Thread_Init( &clMyThread, awApp2Stack, APP2_STACK_SIZE, 1, WorkerMain2, 0 );
        Thread_Start( &clMyThread );

        ulCount = 0;
        while (ulCount < 10000)
        {
            ulCount++;
        }

        KernelAware_Print( "Thread -- Done Work\n" );

        // Check that we're sure the worker thread has terminated before we try running the
        // test loop again.
        while (Thread_GetState( &clMyThread ) != THREAD_STATE_EXIT) { }

        KernelAware_Print ("  Test Done\n");
        Thread_Sleep(100);
    }
}
