
#include "kerneltypes.h"
#include "mark3cfg.h"
#include "kernel.h"
#include "thread.h"
#include "driver.h"
#include "drvUART.h"
#include "profile.h"
#include "kernelprofile.h"
#include "ksemaphore.h"
#include "mutex.h"
#include "message.h"
#include "timerlist.h"

//---------------------------------------------------------------------------
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

//---------------------------------------------------------------------------
static volatile K_UCHAR ucTestVal;

//---------------------------------------------------------------------------
static K_UCHAR aucTxBuf[32];

#define PROFILE_TEST 1
#if PROFILE_TEST

//---------------------------------------------------------------------------
#define TEST_STACK1_SIZE            (384)
#define TEST_STACK2_SIZE            (32)
#define TEST_STACK3_SIZE            (32)
#define MAIN_STACK_SIZE            (384)
#define IDLE_STACK_SIZE            (384)

//---------------------------------------------------------------------------
static ProfileTimer_t stProfileOverhead;

static ProfileTimer_t stSemInitTimer;
static ProfileTimer_t stSemPostTimer;
static ProfileTimer_t stSemPendTimer;

static ProfileTimer_t stMutexInitTimer;
static ProfileTimer_t stMutexClaimTimer;
static ProfileTimer_t stMutexReleaseTimer;

static ProfileTimer_t stThreadInitTimer;
static ProfileTimer_t stThreadStartTimer;
static ProfileTimer_t stThreadExitTimer;
static ProfileTimer_t stContextSwitchTimer;

static ProfileTimer_t stSemaphoreFlyback;
static ProfileTimer_t stSchedulerTimer;
#endif

//---------------------------------------------------------------------------
static Thread_t stMainThread;
static Thread_t stIdleThread;

static Thread_t stTestThread1;

//---------------------------------------------------------------------------
static K_UCHAR aucMainStack[MAIN_STACK_SIZE];
static K_UCHAR aucIdleStack[IDLE_STACK_SIZE];
static K_UCHAR aucTestStack1[TEST_STACK1_SIZE];

//---------------------------------------------------------------------------
static void AppMain( void *unused );
static void IdleMain( void *unused );

//---------------------------------------------------------------------------
int main(void)
{
    Kernel_Init();

    Thread_Init( &stMainThread,
                 aucMainStack,
                 MAIN_STACK_SIZE,
                 1,
                 (ThreadEntry_t)AppMain,
                 NULL );

    Thread_Init( &stIdleThread,
                 aucIdleStack,
                 MAIN_STACK_SIZE,
                 0,
                 (ThreadEntry_t)IdleMain,
                 NULL );

    Thread_Start( &stMainThread );
    Thread_Start( &stIdleThread );

    Driver_SetName( (Driver_t*) &stUART, "/dev/tty");
    ATMegaUART_Init( &stUART );
    
    DriverList_Add( (Driver_t*)&stUART );
    
    Kernel_Start();
}

//---------------------------------------------------------------------------
static void IdleMain( void *unused )
{
    while(1)
    {
#if 1
        // LPM code;
        set_sleep_mode(SLEEP_MODE_IDLE);
        cli();
        sleep_enable();
        sei();
        sleep_cpu();
        sleep_disable();
        sei();
#endif        
    }
    return 0;
}

//---------------------------------------------------------------------------
// Basic string routines
K_USHORT KUtil_Strlen( const K_CHAR *szStr_ )
{
    K_CHAR *pcData = (K_CHAR*)szStr_;
    K_USHORT usLen = 0;

    while (*pcData++)
    {
        usLen++;
    }
    return usLen;
}

//---------------------------------------------------------------------------
void KUtil_Ultoa( K_ULONG ucData_, K_CHAR *szText_ )
{
    K_ULONG ucMul;
    K_ULONG ucMax;

    // Find max index to print...
    ucMul = 10;
    ucMax = 1;
    while (( ucMul < ucData_ ) && (ucMax < 15))
    {
        ucMax++;
        ucMul *= 10; 
    }
    
    szText_[ucMax] = 0;
    while (ucMax--)
    {
        szText_[ucMax] = '0' + (ucData_ % 10);
        ucData_/=10;
    }
}

#if PROFILE_TEST
//---------------------------------------------------------------------------
static void ProfileInit()
{
    
    ProfileTimer_Init( &stProfileOverhead );
    ProfileTimer_Init( &stSemInitTimer );
    ProfileTimer_Init( &stSemPendTimer );
    ProfileTimer_Init( &stSemPostTimer );
    ProfileTimer_Init( &stSemaphoreFlyback );
    
    ProfileTimer_Init( &stMutexInitTimer );
    ProfileTimer_Init( &stMutexClaimTimer );
    ProfileTimer_Init( &stMutexReleaseTimer );
    
    ProfileTimer_Init( &stThreadExitTimer );
    ProfileTimer_Init( &stThreadInitTimer );
    ProfileTimer_Init( &stThreadStartTimer );
    ProfileTimer_Init( &stContextSwitchTimer );
    
    ProfileTimer_Init( &stSchedulerTimer );
}

//---------------------------------------------------------------------------
static void ProfileOverhead()
{
    K_USHORT i;
    for (i = 0; i < 100; i++)
    {
        ProfileTimer_Start( &stProfileOverhead );
        ProfileTimer_Stop( &stProfileOverhead );
    }
}

//---------------------------------------------------------------------------
static void Semaphore_Flyback( Semaphore_t *pstSe )
{    

    ProfileTimer_Start( &stSemaphoreFlyback );
    Semaphore_Pend( pstSe );
    ProfileTimer_Stop( &stSemaphoreFlyback );
    
    Thread_Exit( Scheduler_GetCurrentThread() );
}

//---------------------------------------------------------------------------
static void Semaphore_Profiling()
{
    Semaphore_t stSem;

    K_USHORT i;
    
    for (i = 0; i < 100; i++)
    {
        ProfileTimer_Start( &stSemInitTimer );
        Semaphore_Init( &stSem, 0, 1000);
        ProfileTimer_Stop( &stSemInitTimer );
    }

    for (i = 0; i < 100; i++)
    {        
        ProfileTimer_Start( &stSemPostTimer );
        Semaphore_Post( &stSem );
        ProfileTimer_Stop( &stSemPostTimer );
    }
    
    for (i = 0; i < 100; i++)
    {    
        ProfileTimer_Start( &stSemPendTimer );
        Semaphore_Pend( &stSem );
        ProfileTimer_Stop( &stSemPendTimer );
    }
    
    Semaphore_Init( &stSem, 0, 1);
    for (i = 0; i < 100; i++)
    {
        Thread_Init( &stTestThread1, aucTestStack1, TEST_STACK1_SIZE, 2, (ThreadEntry_t)Semaphore_Flyback, (void*)&stSem);
        Thread_Start( &stTestThread1 );
        
        Semaphore_Post( &stSem );
    }
    
    return;
}

//---------------------------------------------------------------------------
static void Mutex_Profiling()
{
    K_USHORT i;
    Mutex_t stMutex;
    
    for (i = 0; i < 10; i++)
    {
        ProfileTimer_Start( &stMutexInitTimer );
        Mutex_Init( &stMutex );
        Mutex_Init( &stMutex );
        Mutex_Init( &stMutex );
        Mutex_Init( &stMutex );
        Mutex_Init( &stMutex );
        Mutex_Init( &stMutex );
        Mutex_Init( &stMutex );
        Mutex_Init( &stMutex );
        Mutex_Init( &stMutex );
        Mutex_Init( &stMutex );
        ProfileTimer_Stop( &stMutexInitTimer );
    }
    
    for (i = 0; i < 100; i++)
    {
        ProfileTimer_Start( &stMutexClaimTimer );
        Mutex_Claim( &stMutex );
        ProfileTimer_Stop( &stMutexClaimTimer );

        ProfileTimer_Start( &stMutexReleaseTimer );
        Mutex_Release( &stMutex );
        ProfileTimer_Stop( &stMutexReleaseTimer );
    }
}

//---------------------------------------------------------------------------
static void Thread_ProfilingThread()
{
    // Stop the "Thread_t start" profiling timer, which was started from the
    // main app Thread_t
    ProfileTimer_Stop( &stThreadStartTimer );
    
    // Start the "Thread_t exit" profiling timer, which will be stopped after
    // returning back to the main app Thread_t
    ProfileTimer_Start( &stThreadExitTimer );
    Thread_Exit( Scheduler_GetCurrentThread() );
}

//---------------------------------------------------------------------------
static void Thread_Profiling()
{
    K_USHORT i;
    
    for (i = 0; i < 100; i++)
    {
        // Profile the amount of time it takes to initialize a representative
        // test Thread_t, simulating an "average" system Thread_t.  Create the
        // Thread_t at a higher priority than the current Thread_t.
        ProfileTimer_Start( &stThreadInitTimer );
        Thread_Init( &stTestThread1, aucTestStack1, TEST_STACK1_SIZE, 2, (ThreadEntry_t)Thread_ProfilingThread, NULL);
        ProfileTimer_Stop( &stThreadInitTimer );
        
        // Profile the time it takes from calling "start" to the time when the
        // Thread_t becomes active
        ProfileTimer_Start( &stThreadStartTimer );
        
        Thread_Start( &stTestThread1 ); //-- Switch to the test Thread_t --
        
        // Stop the Thread_t-exit profiling timer, which was started from the
        // test Thread_t
        ProfileTimer_Stop( &stThreadExitTimer );
    }
    
    Scheduler_SetScheduler(0);
    for (i = 0; i < 100; i++)
    {
        // Context switch profiling - this is equivalent to what's actually
        // done within the AVR-implementation.
        ProfileTimer_Start( &stContextSwitchTimer );
        {
            Thread_SaveContext();
            g_pstNext = g_pstCurrent;
            Thread_RestoreContext();
        }
        ProfileTimer_Stop( &stContextSwitchTimer );
    }
    Scheduler_SetScheduler(1);
}

//---------------------------------------------------------------------------
void Scheduler_Profiling()
{
    K_USHORT i;
    
    for (i = 0; i < 100; i++)
    {
        // Profile the scheduler.  Running at priority 1, we'll get
        // the worst-case scheduling time (not necessarily true of all 
        // schedulers, but true of ours).
        ProfileTimer_Start( &stSchedulerTimer );
        Scheduler_Schedule();
        ProfileTimer_Stop( &stSchedulerTimer );
    }    
}

//---------------------------------------------------------------------------
static void PrintWait( Driver_t *pstDriver_, K_USHORT usSize_, const K_CHAR *data )
{
    K_USHORT usWritten = 0;
    
    while (usWritten < usSize_)
    {
        usWritten += Driver_Write( pstDriver_, (usSize_ - usWritten), (K_UCHAR*)(&data[usWritten]));
        if (usWritten != usSize_)
        {
            Thread_Sleep(5);
        }
    }
}

//---------------------------------------------------------------------------
void ProfilePrint( ProfileTimer_t *pstProfile, const K_CHAR *szName_ )
{
    Driver_t *pstUART = DriverList_FindByPath("/dev/tty");
    K_CHAR szBuf[16];
    K_ULONG ulVal = ProfileTimer_GetAverage( pstProfile ) - ProfileTimer_GetAverage( &stProfileOverhead );
    ulVal *= 8;
    int i;
    for( i = 0; i < 16; i++ )
    {
        szBuf[i] = 0;
    }
    szBuf[0] = '0';
    
    PrintWait( pstUART, KUtil_Strlen(szName_), szName_ );    
    PrintWait( pstUART, 2, ": " );
    KUtil_Ultoa(ulVal, szBuf);
    PrintWait( pstUART, KUtil_Strlen(szBuf), szBuf );
    PrintWait( pstUART, 1, "\n" );
}

//---------------------------------------------------------------------------
void ProfilePrintResults()
{
    ProfilePrint( &stMutexInitTimer, "MI");
    ProfilePrint( &stMutexClaimTimer, "MC");
    ProfilePrint( &stMutexReleaseTimer, "MR");
    ProfilePrint( &stSemInitTimer, "SI");
    ProfilePrint( &stSemPendTimer, "SPo");
    ProfilePrint( &stSemPostTimer, "SPe");
    ProfilePrint( &stSemaphoreFlyback, "SF");
    ProfilePrint( &stThreadExitTimer, "TE");
    ProfilePrint( &stThreadInitTimer, "TI");
    ProfilePrint( &stThreadStartTimer, "TS");
    ProfilePrint( &stContextSwitchTimer, "CS");
    ProfilePrint( &stSchedulerTimer, "SC");
}

#endif

//---------------------------------------------------------------------------
static void AppMain( void *unused )
{
    Driver_t *pstUART = DriverList_FindByPath("/dev/tty");
    
#if UNIT_TEST    
    UT_Init();
#endif
    
#if PROFILE_TEST
    ProfileInit();
#endif    

    Driver_Control( pstUART, CMD_SET_BUFFERS, 0, NULL, 32, aucTxBuf );
    {
        K_ULONG ulBaudRate = 57600;
        Driver_Control( pstUART, CMD_SET_BAUDRATE, 0, &ulBaudRate, 0, 0 );
        Driver_Control( pstUART, CMD_SET_RX_DISABLE, 0, 0, 0, 0);
    }
    
    Driver_Open( pstUART );
    Driver_Write( pstUART, 6,(K_UCHAR*)"START\n");

    while(1)
    {

#if PROFILE_TEST
        //---[ API Profiling ]-----------------------------
        Profiler_Start();
        ProfileOverhead();        
        Semaphore_Profiling();
        Mutex_Profiling();
        Thread_Profiling();
        Scheduler_Profiling();
        Profiler_Stop();
                
        ProfilePrintResults();
        Thread_Sleep(500);
#endif        
    }
}
