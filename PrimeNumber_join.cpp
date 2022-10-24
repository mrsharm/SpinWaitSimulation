
#include <windows.h>
#include <limits.h>
#include <stdio.h>
#include <stdint.h>
#include <intrin.h>
#include <conio.h>
#include <queue>
#include <thread>
#include <chrono>
#include "Volatile.h"
#include "EventImpl.h"
#include "ProcessorInfo.h"

typedef unsigned long long ulong;
//_mm_monitorx((const void*)&g_global_location, 0, 0);
//_mm_mwaitx(2, 0, waitTime);

const int SPIN_COUNT = 128 * 1000;
int PROCESSOR_COUNT = GetProcessorCount();

#define PRINT_STATS(msg, ...) printf(msg ".\n", __VA_ARGS__);

#ifdef _DEBUG
#define PRINT_PROGRESS(msg, ...) printf("[PROGRESS #%d] " msg ".\n", __VA_ARGS__);
#define PRINT_ANSWER(msg, ...) printf("[ANSWER #%d] " msg ".\n", __VA_ARGS__);
#define PRINT_SOFT_WAIT(msg, ...) printf("[SOFT WAIT #%d] " msg ".\n", __VA_ARGS__);
#define PRINT_HARD_WAIT(msg, ...) printf("[HARD WAIT #%d] " msg ".\n", __VA_ARGS__);
#define PRINT_RELEASE(msg, ...) printf("[RELEASE #%d] " msg ".\n", __VA_ARGS__);


#ifndef PRINT_PROGRESS
#define PRINT_PROGRESS(msg, ...)
#endif // !PRINT_PROGRESS

#ifndef  PRINT_ANSWER
#define PRINT_ANSWER(msg, ...)
#endif // ! PRINT_ANSWER

#ifndef PRINT_SOFT_WAIT
#define PRINT_SOFT_WAIT(msg, ...)
#endif // !PRINT_SOFT_WAIT

#ifndef PRINT_HARD_WAIT
#define PRINT_HARD_WAIT(msg, ...)
#endif // !PRINT_SOFT_WAIT

#ifndef PRINT_RELEASE
#define PRINT_RELEASE(msg, ...)
#endif // !PRINT_RELEASE

#ifndef PRINT_STATS
#define PRINT_STATS(msg, ...)
#endif // !PRINT_STATS


#else // !_DEBUG
#define PRINT_PROGRESS(msg, ...)
#define PRINT_ANSWER(msg, ...)
#define PRINT_SOFT_WAIT(msg, ...)
#define PRINT_HARD_WAIT(msg, ...)
#define PRINT_RELEASE(msg, ...)
#endif


struct join_structure
{
    // Shared non volatile keep on separate line to prevent eviction
    int n_threads;
    EventImpl joined_event[3];
    Volatile<int> lock_color;
    Volatile<bool> wait_done;
    Volatile<bool> joined_p;
    Volatile<int> join_lock;
    unsigned __int64 restartStartTime;
};

EventImpl waitToComplete;

class t_join
{
    join_structure join_struct;

public:
    bool init(int n_th)
    {
        join_struct.n_threads = n_th;
        join_struct.lock_color = 0;
        for (int i = 0; i < 3; i++)
        {
            if (!join_struct.joined_event[i].IsValid())
            {
                join_struct.joined_p = FALSE;

                if (!join_struct.joined_event[i].CreateManualEvent(false))
                {
                    return false;
                }
            }
        }
        join_struct.join_lock = join_struct.n_threads;
        join_struct.wait_done = false;
        return true;
    }

    ulong join(int inputIndex, int threadId, bool* wasHardWait)
    {
        ulong totalIterations = 0;
        *wasHardWait = false;
        int color = join_struct.lock_color.LoadWithoutBarrier();
        if (_InterlockedDecrement((long*)&join_struct.join_lock) != 0)
        {
            if (color == join_struct.lock_color.LoadWithoutBarrier())
            {
            respin:
                for (int j = 0; j < SPIN_COUNT; j++)
                {
                    totalIterations++;
                    if (color != join_struct.lock_color.LoadWithoutBarrier())
                    {
                        PRINT_SOFT_WAIT("%d. %llu iterations.", threadId, inputIndex, totalIterations);
                        break;
                    }
                    YieldProcessor();           // indicate to the processor that we are spinning
                }

                // we've spun, and if color still hasn't changed, fall into hard wait
                if (color == join_struct.lock_color.LoadWithoutBarrier())
                {
                    PRINT_HARD_WAIT("%d. %llu iterations.", threadId, inputIndex, totalIterations);
                    *wasHardWait = true;
                    uint32_t dwJoinWait = join_struct.joined_event[color].Wait(INFINITE, FALSE);

                    if (dwJoinWait != WAIT_OBJECT_0)
                    {
                        printf("Fatal error");
                        exit(1);
                    }
                }

                // avoid race due to the thread about to reset the event (occasionally) being preempted before ResetEvent()
                if (color == join_struct.lock_color.LoadWithoutBarrier())
                {
                    goto respin;
                }
            }
        }
        else
        {
            PRINT_RELEASE("%d", threadId, inputIndex);
            PRINT_RELEASE("---------------\n", threadId);
            join_struct.joined_p = true;
            join_struct.joined_event[!color].Reset();
        }
        return totalIterations;
    }

    __forceinline void restart(int threadId, int numberIndex, bool isLastIteration)
    {
        //printf("%d. Restart for Thread# %d\n", numberIndex, threadId);
        join_struct.joined_p = false;
        join_struct.join_lock = join_struct.n_threads;
        int color = join_struct.lock_color.LoadWithoutBarrier();

        // Record the start of "wake up time". Do this before changing
        // the color so we don't have a thread who reaches the place that
        // measures the "wakeupTimeTicks" before we even record the start
        // of "restart time".
        recordRestartStartTime();

        join_struct.lock_color = !color;
        join_struct.joined_event[color].Set();

        if (isLastIteration)
        {
            waitToComplete.Set();
        }
    }

    void recordRestartStartTime()
    {
        join_struct.restartStartTime = __rdtsc();
    }

    unsigned __int64 getTicksSinceRestart()
    {
        assert(join_struct.restartStartTime != 0);
        return __rdtsc() - join_struct.restartStartTime;
    }

    bool joined()
    {
        return join_struct.joined_p;
    }
};

class ThreadInput
{
public:
    // Just to keep track of answers so the compiler doesn't discard them
    // during optimization.
    int answer;
    int processed;

    // Input to the Threadworker
    int threadId;
    ulong* input;
    int count;

    // Output from the processing
    ulong totalIterations;
    int hardWaitCount;
    int softWaitCount;

    unsigned __int64 wakeupTimeTicks;

    ThreadInput(int threadId, int numPrimeNumbers) :
        threadId(threadId),
        count(numPrimeNumbers),
        answer(0),
        processed(0),
        input(nullptr),
        totalIterations(0),
        hardWaitCount(0),
        softWaitCount(0) {}
};

t_join joinData;
std::chrono::steady_clock::time_point beginTimer;
unsigned __int64 start;

/// <summary>
/// Given a number 'input', each thread finds smallest prime number greater than 'n'.
/// If next prime number is beyond INT_MAX, it will return 0.
/// </summary>
/// <param name="n"></param>
/// <returns></returns>
ulong FindNextPrimeNumber(ulong input)
{
    // Start checking each number from input upto input * 2.
    for (ulong i = input; i < input * 2; i++)
    {
        bool found = true;
        for (ulong j = 2; j < i / 2; j++)
        {
            if (i % j == 0)
            {
                found = false;
                break;
            }
        }

        if (found)
        {
            return i;
        }
    }
    return 0;
}

/// <summary>
/// Threads will fetch a number from the queue and start the work to find the prime number.
/// Once all threads are done with their respective input, it will proceed to fetch next number.
/// </summary>
/// <param name="lpParam"></param>
/// <returns>status</returns>
DWORD WINAPI ThreadWorker(LPVOID lpParam)
{
    ThreadInput* tInput = (ThreadInput*)lpParam;

    // Make sure things are initialized correctly.
    assert(tInput->hardWaitCount == 0);
    assert(tInput->softWaitCount == 0);
    assert(tInput->totalIterations == 0);
    int threadId = tInput->threadId;
    int processedCount = 0;

    for (int i = 0; i < tInput->count; i++)
    {
        PRINT_PROGRESS("*** Processing: %u out of %u..", threadId, tInput->processed, tInput->count);
        ulong input = tInput->input[i];
        ulong answer = FindNextPrimeNumber(input);
        tInput->processed++;

        // So the compiler doesn't throw away answer and processedCount;
        tInput->answer |= answer;

        PRINT_ANSWER(" %u %llu= %llu", threadId, processedCount, input, answer);

        bool wasHardWait = false;
        tInput->totalIterations += joinData.join(i, threadId, &wasHardWait);

        // The last thread to complete will return here and "restart()".
        if (joinData.joined())
        {
            joinData.restart(tInput->threadId, i, tInput->processed == tInput->count);
        }
        else
        {
            // Other threads that were waiting so long, will record the latency for
            // wakeup time as soon as things are restarted.
            tInput->wakeupTimeTicks += joinData.getTicksSinceRestart();
        }

        if (wasHardWait)
        {
            tInput->hardWaitCount++;
        }
        else
        {
            tInput->softWaitCount++;
        }
    }

    PRINT_PROGRESS("*** Total processed: %u out of %u..", threadId, processedCount, tInput->count);
    return 0;
}

class PrimeNumbers
{
public:
    /// <summary>
    /// Given a number 'n', each thread finds smallest prime number greater than 'n'.
    /// If next prime number is beyond INT_MAX, it will return 0.
    /// 
    /// The test will produce `numPrimeNumbers` random numbers per thread and add in thread's queue.
    /// </summary>
    /// <param name="numPrimeNumbers"></param>
    /// <param name="complexity"></param>
    /// <returns></returns>
    bool PrimeNumbersTest(int numPrimeNumbers, int complexity)
    {
        std::vector<HANDLE> threads(PROCESSOR_COUNT);
        std::vector<DWORD> threadIds(PROCESSOR_COUNT);
        std::vector<ThreadInput*> threadInputs(PROCESSOR_COUNT);
        joinData.init(PROCESSOR_COUNT);

        // Create all the threads
        for (int i = 0; i < PROCESSOR_COUNT; i++)
        {
            ThreadInput* tInput = new ThreadInput(i, numPrimeNumbers);
            if (tInput != NULL)
            {
                float n;
                tInput->input = (ulong*)malloc((sizeof(ulong) * numPrimeNumbers));
                for (int i = 0; i < numPrimeNumbers; i++)
                {
                    if (complexity == 0)
                    {
                        tInput->input[i] = i;
                    }
                    else
                    {
                        n = (float)rand() / RAND_MAX;
                        tInput->input[i] = (ulong)(n * (100 + pow(2, complexity)));
                    }
                }
            }
            else {
                assert(!"Failed to allocate tInput");
            }

            DWORD tid;
            threads[i] = CreateThread(
                NULL,                           // default security attributes
                0,                              // use default stack size
                ThreadWorker,                   // thread function name
                (LPVOID)tInput,                 // argument to thread function
                CREATE_SUSPENDED,
                &tid);                          // returns the thread identifier

            threadIds[i] = tid;
            threadInputs[i] = tInput;

            wchar_t buffer[15];
            wsprintf(buffer, L"Thread# %d", i);

            SetThreadDescription(threads[i], buffer);
        }

        // Create an event to wait for all threads to complete.
        waitToComplete.CreateManualEvent(false);

        // https://stackoverflow.com/a/27739925
        beginTimer = std::chrono::steady_clock::now();
        start = __rdtsc();

        // Start all the threads
        for (int i = 0; i < PROCESSOR_COUNT; i++)
        {
            ResumeThread(threads[i]);
        }

        // Wait till last thread would signal that it is done.
        uint32_t dwJoinWait = waitToComplete.Wait(INFINITE, FALSE);

        if (dwJoinWait != WAIT_OBJECT_0)
        {
            DWORD error = GetLastError();
            printf("WaitForMultipleObjects() failed with error code %d and return code %d\n", error, dwJoinWait);
            exit(1);
        }

        unsigned __int64 elapsed_ticks = __rdtsc() - start;
        auto elapsed_time = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - beginTimer).count();

        int totalHardWaits = 0, totalSoftWaits = 0;
        ulong totalIterations = 0;
        unsigned __int64 totalWakeupTimeTicks = 0;
        for (int i = 0; i < PROCESSOR_COUNT; i++)
        {
            ThreadInput* outputData = threadInputs[i];
            assert(outputData->hardWaitCount <= numPrimeNumbers);
            assert(outputData->softWaitCount <= numPrimeNumbers);
            totalHardWaits += outputData->hardWaitCount;
            totalSoftWaits += outputData->softWaitCount;
            totalIterations += outputData->totalIterations;
            totalWakeupTimeTicks += outputData->wakeupTimeTicks;

            PRINT_STATS("[Thread #%d] Iterations: %llu, HardWait: %d, SoftWait: %d, WakeupTime: %llu", i, outputData->totalIterations, outputData->hardWaitCount, outputData->softWaitCount, outputData->wakeupTimeTicks);
        }
        PRINT_STATS("-----------------------------------------------------------");

#define AVG_NUMBER(n) (n / numPrimeNumbers) + 1
#define AVG_THREAD(n) (n / PROCESSOR_COUNT) + 1
        
        PRINT_STATS("(Average/number) Iterations: %llu, HardWait: %d, SoftWait: %d, WakeupTime: %llu", AVG_NUMBER(totalIterations), AVG_NUMBER(totalHardWaits), AVG_NUMBER(totalSoftWaits), AVG_NUMBER(totalWakeupTimeTicks));
        PRINT_STATS("(Average/thread) Iterations: %llu, HardWait: %d, SoftWait: %d, WakeupTime: %llu", AVG_THREAD(totalIterations), AVG_THREAD(totalHardWaits), AVG_THREAD(totalSoftWaits), AVG_THREAD(totalWakeupTimeTicks));
        PRINT_STATS("Time taken: %llu ticks", elapsed_ticks);
        PRINT_STATS("Time difference = %lld msec", elapsed_time);

        return true;
    }
};

void PrintUsageAndExit()
{
    printf("\nUsage: PrimeNumbers.exe <numPrimeNumbers> <complexity> [*threadCount*]\n");
    printf("<numPrimeNumbers>: Number of prime numbers per thread.\n");
    printf("<complexity>: Number between 0~31.\n");
    printf("Higher the number, bigger the input numbers will beand thus more probability of threads going into hard - waits.\n");
    printf("If complexity == 0, it will generate uniform, identical inputs from 0~(<numPrimeNumbers> - 1) for all the threads.\n");
    printf("[*threadCount*]: Optional number of threads to use. By default it will use number of cores available in all groups.\n");
    exit(1);
}

int main(int argc, char** argv)
{
    if (argc <= 2)
    {
        PrintUsageAndExit();
    }
    int numPrimeNumbers = atoi(argv[1]);
    int complexity = atoi(argv[2]) % 32;
    if (argc == 4)
    {
        PROCESSOR_COUNT = atoi(argv[3]);
    }
    else if (argc > 4)
    {
        PrintUsageAndExit();
    }

    PrimeNumbers p;
    p.PrimeNumbersTest(numPrimeNumbers, complexity);

    return 0;
}