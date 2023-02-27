
#include <windows.h>
#include <limits.h>
#include <stdio.h>
#include <stdint.h>
#include <intrin.h>
#include <conio.h>
#include <queue>
#include <thread>
#include <chrono>
#include "ProcessorInfo.h"
#include "common.h"
#include "t_join.h"

const char* str_join_types[] =
{
    "invalid",
    "t_join_pause",
    "t_join_pause_soft_wait_only",
    "t_join_mwaitx_loop",
    "t_join_mwaitx_loop_soft_wait_only",
    "t_join_mwaitx_noloop",
    "t_join_mwaitx_noloop_soft_wait_only",
    "t_join_hard_wait_only",
    "t_join_no_pause",
    "t_join_pause2",
    "t_join_pause10"
};

int SPIN_COUNT;

bool HYPERTHREADING_ENABLED = false;

enum affinity_attribute
{
    // default, how Server GC threads are today
    hard_affinitized = 0,
    // in the HT case, affinitize to every other logical core which belongs to a different physical core.
    hard_affinitized_physical = 1,
    not_affinitized = 2
};

const char* str_affinity_attribute[] =
{
    "affinity",
    "affinity physical core",
    "no affinity"
};

// this is via the "--affi" arg, default is 0
int affinity_type = 0;

t_join* joinData = nullptr;
std::chrono::steady_clock::time_point beginTimer;
unsigned __int64 start;

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
    ulong totalIterationsSoftWaits;
    ulong totalIterationsHardWaits;
    int hardWaitCount;
    int softWaitCount;

    unsigned __int64 spinLoopTimeTicksHardWait;
    unsigned __int64 spinLoopTimeTicksSoftWait;
    unsigned __int64 softWaitWakeupTimeTicks;
    unsigned __int64 hardWaitWakeupTimeTicks;

    int64_t elapsed_time;
    unsigned __int64 elapsed_ticks;

    ThreadInput(int threadId, int numPrimeNumbers) :
        threadId(threadId),
        count(numPrimeNumbers),
        answer(0),
        processed(0),
        input(nullptr),
        totalIterationsSoftWaits(0),
        totalIterationsHardWaits(0),
        hardWaitCount(0),
        softWaitCount(0) {}
};

const double _1Q = pow(10, 15);
const double _1T = pow(10, 12);
const double _1B = pow(10, 9);
const double _1M = pow(10, 6);
const double _1K = pow(10, 3);
const char* formatNumber(uint64_t input)
{
    char* buffer = (char*)malloc(sizeof(char) * 100);
    //sprintf_s(buffer, 100, "%4.2f%c", scaledInput, scaledUnit);

    int base = 10;
    int char_index = 0;
    int num_chars = 0;
    while (char_index < 100)
    {
        if (input)
        {
            buffer[char_index] = (char)(input % base + '0');
            input /= 10;
            num_chars++;

            //printf("input is %I64d, num_chars %d, char_index %d->%c\n", input, num_chars, char_index, buffer[char_index]);

            if (num_chars == 3)
            {
                //printf("setting char at index %d to ,\n", (char_index + 1));
                buffer[++char_index] = ',';
                num_chars = 0;
            }
            char_index++;
        }
        else
        {
            break;
        }
    }

    if (char_index > 0)
    {
        if (buffer[char_index - 1] == ',')
        {
            char_index--;
        }
    }

    if (char_index == 0)
    {
        buffer[char_index++] = '0';
    }
    buffer[char_index] = '\0';

    //printf("setting char at index %d to null, now string is %s\n", char_index, buffer);

    for (int i = 0; i < (char_index / 2); i++)
    {
        char c = buffer[i];
        buffer[i] = buffer[char_index - 1 - i];
        buffer[char_index - 1 - i] = c;
    }

    //printf("input is %I64d -> %s\n", saved_input, buffer);

    return buffer;
}

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
    auto beginTimer = std::chrono::steady_clock::now();
    auto start = __rdtsc();

    ThreadInput* tInput = (ThreadInput*)lpParam;

    // Make sure things are initialized correctly.
    assert(tInput->hardWaitCount == 0);
    assert(tInput->softWaitCount == 0);
    assert(tInput->totalIterationsSoftWaits == 0);
    assert(tInput->totalIterationsHardWaits == 0);
    int threadId = tInput->threadId;
    int processedCount = 0;

    for (int i = 0; i < tInput->count; i++)
    {
        PRINT_PROGRESS("*** Processing: %d out of %d..", threadId, tInput->processed, tInput->count);
        ulong input = tInput->input[i];
        ulong answer = FindNextPrimeNumber(input);
        tInput->processed++;

        // So the compiler doesn't throw away answer and processedCount;
        tInput->answer |= answer;

        PRINT_ANSWER(" %u %llu= %llu", threadId, processedCount, input, answer);

        bool wasHardWait = false;
        unsigned __int64 spinLoopStopTime = 0;
        unsigned __int64 spinLoopStartTime = 0;

        uint64_t iterations = joinData->join(i, threadId, &wasHardWait, &spinLoopStartTime, &spinLoopStopTime);

        // The last thread to complete will return here and "restart()".
        if (joinData->joined())
        {
            joinData->restart(tInput->threadId, i, tInput->processed == tInput->count);
        }
        else
        {
            // Even though we hard-wait, we also did spin-loop. See how much time was spent in that.
            unsigned __int64 spinWaitCpuCycles = spinLoopStopTime - spinLoopStartTime;
            

            // Other threads that were waiting so long, will record the latency for
            // wakeup time as soon as things are restarted.
            if (wasHardWait)
            {
                unsigned __int64 hardWaitWakeupLatency = joinData->getTicksSinceRestart();
                tInput->hardWaitWakeupTimeTicks += hardWaitWakeupLatency;
                tInput->spinLoopTimeTicksHardWait += spinWaitCpuCycles;
                tInput->hardWaitCount++;
                tInput->totalIterationsHardWaits += iterations;

                //PRINT_HARD_WAIT_LATENCY("%d. wakeup latency %lld cycles, %llu total spin-loop cycles", threadId, i, hardWaitWakeupLatency, spinWaitCpuCycles);
                //printf("thread %d - hardwait %d, total wait %d [%d out of %d]\n", threadId, tInput->hardWaitCount, (tInput->hardWaitCount + tInput->softWaitCount), tInput->processed, tInput->count);
            }
            else
            {
                unsigned __int64 softWaitWakeupLatency = joinData->getTicksSinceRestart();
                tInput->softWaitWakeupTimeTicks += softWaitWakeupLatency;
                tInput->spinLoopTimeTicksSoftWait += spinWaitCpuCycles;
                tInput->softWaitCount++;
                tInput->totalIterationsSoftWaits += iterations;

                //PRINT_SOFT_WAIT_LATENCY("%d. wakeup latency %lld cycles, %llu total spin-loop cycles.", threadId, i, softWaitWakeupLatency, spinWaitCpuCycles);
                //printf("thread %d - softwait %d, total wait %d [%d out of %d]\n", threadId, tInput->softWaitCount, (tInput->hardWaitCount + tInput->softWaitCount), tInput->processed, tInput->count);
                //printf("thread %d - softwait %d, total waited %I64d, wake up latency %I64d cycles\n",
                //    threadId, tInput->softWaitCount, spinWaitCpuCycles, softWaitWakeupLatency);
            }
        }
    }

    tInput->elapsed_ticks = __rdtsc() - start;
    tInput->elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - beginTimer).count();

    PRINT_PROGRESS("*** Total processed: %u out of %u..", threadId, processedCount, tInput->count);
    return 0;
}

class PrimeNumbers
{
private:
    int PROCESSOR_COUNT = -1;
    int PROCESSOR_GROUP_COUNT = -1; 
    int MWAITX_CYCLES = 0;
    int INPUT_COUNT = -1;
    int COMPLEXITY = -1;
    int JOIN_TYPE = -1;
    bool setThreadHighPri = true;

    void DiffWakeTime(ulong hardWaitWakeTime, ulong softWaitWakeTime, ulong* diff, char* diffCh)
    {
        *diffCh = ' ';
        *diff = hardWaitWakeTime - softWaitWakeTime;
        if (hardWaitWakeTime < softWaitWakeTime)
        {
            *diffCh = '-';
            *diff = softWaitWakeTime - hardWaitWakeTime;
        }
    }

    void parseArgs(int argc, char** argv)
    {
#define ARGS(argumentName)                  \
    int argumentName = 0;                   \
    bool argumentName##_used = false;

#define VALIDATE_AND_SET(paramName)                                 \
        if (_strcmpi(parameterName,"--" # paramName) == 0)          \
        {                                                           \
            if (paramName##_used)                                   \
            {                                                       \
                printf("--" # paramName ## "already specified.\n"); \
                PrintUsageAndExit();                                \
            }                                                       \
            paramName = atoi(parameterValue);                       \
            paramName##_used = true;                                \
            continue;                                               \
        }

        ARGS(input_count);
        ARGS(complexity);
        ARGS(thread_count);
        ARGS(mwaitx_cycle_count);
        ARGS(join_type);
        ARGS(spin_count);
        ARGS(affi);
        ARGS(thread_priority);
        ARGS(ht);

        if (argc == 1)
        {
            PrintUsageAndExit();
        }

        for (int i = 1; i < argc; i++)
        {
            char* parameterName = argv[i++];
            if ((_strcmpi(parameterName, "-?") == 0) || (_strcmpi(parameterName, "-h") == 0) || (_strcmpi(parameterName, "-help") == 0))
            {
                PrintUsageAndExit();
            }

            if (i == argc)
            {
                printf("Missing value for parameter: '%s'\n", parameterName);
                PrintUsageAndExit();
            }
            char* parameterValue = argv[i];

            VALIDATE_AND_SET(input_count);
            VALIDATE_AND_SET(complexity);
            VALIDATE_AND_SET(thread_count);
            VALIDATE_AND_SET(join_type);
            VALIDATE_AND_SET(mwaitx_cycle_count);
            VALIDATE_AND_SET(spin_count);
            VALIDATE_AND_SET(thread_priority);
            VALIDATE_AND_SET(ht);
            VALIDATE_AND_SET(affi);

            printf("Unknown parameter: '%s'\n", parameterName);
            PrintUsageAndExit();
        }

        complexity %= 32;

        // Verifications
        if (!input_count_used || !complexity_used || !ht_used)
        {
            printf("Missing mandatory arguments.\n");
            PrintUsageAndExit();
        }
        else
        {
            INPUT_COUNT = input_count;
            COMPLEXITY = complexity;
        }

        if (thread_count_used)
        {
            if (thread_count <= 0)
            {
                printf("Invalid value '%d' for '--thread_count'. Should be > 1.\n", thread_count);
                PrintUsageAndExit();
            }
            else
            {
                PROCESSOR_COUNT = thread_count;
            }
        }

        if (join_type_used)
        {
            if ((join_type < 1) || (join_type > 10))
            {
                printf("Invalid value '%d' for '--join_type'. Should be between 1 and 10.\n", join_type);
                PrintUsageAndExit();
            }
            else
            {
                JOIN_TYPE = join_type;
            }
        }
        else
        {
            JOIN_TYPE = 1;
        }

        if (mwaitx_cycle_count_used)
        {
            if ((join_type < 3) || (join_type > 6))
            {
                printf("Warning: '--mwaitx_cycle_count' is specified, but value will not be used for 'pause' wait type.\n");
            }
            else
            {
                MWAITX_CYCLES = mwaitx_cycle_count;
            }
        }
        else
        {
            if ((join_type >= 3) && (join_type <= 6))
            {
                printf("Warning: '--mwaitx_cycle_count' is needed when join_type is related to mwaitx.\n");
                PrintUsageAndExit();
            }
        }

        if (spin_count_used)
        {
            SPIN_COUNT = spin_count;
        }
        else
        {
            SPIN_COUNT = 128 * 1000;
        }

        if (ht_used)
        {
            if (ht_used != 0 && ht_used != 1)
            {
                printf("Invalid value for '--ht' - this value can either be 0 or 1.\n");
                PrintUsageAndExit();
            }

            if (HYPERTHREADING_ENABLED != (ht == 1))
            {
                printf("Invalid value '%d' for '--ht' - this is an incorrect configuration. HT is currently %s, but user has passed in %d. Setting the correct --ht parameter is required.\n", ht, (HYPERTHREADING_ENABLED ? "enabled" : "disabled"), ht);
                PrintUsageAndExit();
            }
        }

        if (affi_used)
        {
            if (!HYPERTHREADING_ENABLED && (affinity_type == hard_affinitized_physical))
            {
                printf("Invalid value '%d' for '--affi'. Cannot set 'affinity physical core' on machine with hyper threading disabled.\n", affinity_type);
                PrintUsageAndExit();
            }
            affinity_type = affi;
        }

        if (thread_priority_used)
        {
            setThreadHighPri = thread_priority == 1;
        }
        else
        {
            setThreadHighPri = true;
        }
    }

    void PrintUsageAndExit()
    {
        printf("\nUsage: PrimeNumbers.exe --input_count <numPrimeNumbers> --complexity <complexity> --ht <ht> [options]\n\n");
        printf("--input_count <N>: Number of prime numbers per thread.\n\n");
        printf("--complexity <N>: Number between 0~31.\n");
        printf("--ht [0|1]: If 1 then Hyperthread is on else, it is not. If the input doesn't match the machine configuration, then the program will throw an error. \n");
        printf("\n");
        printf("Options:\n");
        printf("--thread_count <N>: Number of threads to use. By default it will use number of cores available in all groups.\n\n");
        printf("--thread_priority [0|1]: If 1 (default), create threads with high priority otherwise create them with normal priority.\n\n");
        printf("--spin_count <N>: If specified, the number of iterations to spin before going to hardwait. Default= 128000.\n\n");
        printf("--mwaitx_cycle_count <N>: TIMEOUT value to use in mwaitx(). Required if --join_type is between [3-6].\n\n");
        printf("--affi <AFF_TYPE>: Affinitization to conduct. <AFF_TYPE> can be:\n");
        printf("  0= affinity (default)\n");
        printf("  1= affinity physical core\n");
        printf("  2= no affinity\n\n");
        printf("--join_type <TYPE>: Join algorithm to use. <TYPE> can be:\n");
        printf("  1= The current GC implementation [t_join_pause]\n");
        printf("  2= Use 'pause', only use in spin-loop, no hard-wait [t_join_pause_soft_wait_only]\n");
        printf("  3= Use 'mwaitx', use inside spin-loop [t_join_mwaitx_loop]\n");
        printf("  4= Use 'mwaitx', only use in spin-loop, no hard-wait [t_join_mwaitx_loop_soft_wait_only]\n");
        printf("  5= Use 'mwaitx', no spin-loop involved [t_join_mwaitx_noloop]\n");
        printf("  6= Use 'mwaitx', no spin-loop involved, no hard-wait [t_join_mwaitx_noloop_soft_wait_only]\n");
        printf("  7= Only hard-wait. [t_join_hard_wait_only]\n");
        printf("  8= Same as the current impl but with no pause. [t_join_no_pause]\n");
        printf("  9= Same as the current impl but with 2 pause instructions per iteration. [t_join_pause2]\n");
        printf(" 10= Same as the current impl but with 10 pause instructions per iteration. [t_join_pause10]\n");
        exit(1);
    }

public:

    PrimeNumbers(int argc, char** argv)
    {
        HYPERTHREADING_ENABLED = IsHyperThreadingEnabled();
        parseArgs(argc, argv);

        int userInput_processor_count = PROCESSOR_COUNT;
        GetProcessorInfo(&PROCESSOR_COUNT, &PROCESSOR_GROUP_COUNT);
        if (userInput_processor_count != -1)
        {
            PROCESSOR_COUNT = userInput_processor_count;
        }
        
        PRINT_STATS("Running: SPIN_COUNT= %d, numbers= %d, complexity= %d, JOIN_TYPE= %s, threads= %d, %s %s, mwait cycles %s",
            SPIN_COUNT, INPUT_COUNT, COMPLEXITY, str_join_types[JOIN_TYPE], PROCESSOR_COUNT,
            (HYPERTHREADING_ENABLED ? "HT" : "no HT"), str_affinity_attribute[affinity_type], formatNumber(MWAITX_CYCLES));
    }

    /// <summary>
    /// Given a number 'n', each thread finds smallest prime number greater than 'n'.
    /// If next prime number is beyond INT_MAX, it will return 0.
    /// 
    /// The test will produce `numPrimeNumbers` random numbers per thread and add in thread's queue.
    /// </summary>
    /// <param name="args"></param>
    /// <returns></returns>
    bool PrimeNumbersTest()
    {
        std::vector<HANDLE> threadHandles(PROCESSOR_COUNT);
        std::vector<DWORD> threadIds(PROCESSOR_COUNT);
        std::vector<ThreadInput*> threadInputs(PROCESSOR_COUNT);

        switch (JOIN_TYPE)
        {
        case 1:
            joinData = new t_join_pause(PROCESSOR_COUNT);
            break;
        case 2:
            joinData = new t_join_pause_soft_wait_only(PROCESSOR_COUNT);
            break;
        case 3:
            joinData = new t_join_mwaitx_loop(PROCESSOR_COUNT, MWAITX_CYCLES);
            break;
        case 4:
            joinData = new t_join_mwaitx_loop_soft_wait_only(PROCESSOR_COUNT, MWAITX_CYCLES);
            break;
        case 5:
            joinData = new t_join_mwaitx_noloop(PROCESSOR_COUNT, MWAITX_CYCLES);
            break;
        case 6:
            joinData = new t_join_mwaitx_noloop_soft_wait_only(PROCESSOR_COUNT, MWAITX_CYCLES);
            break;
        case 7:
            joinData = new t_join_hard_wait_only(PROCESSOR_COUNT);
            break;
        case 8:
            joinData = new t_join_no_pause(PROCESSOR_COUNT);
            break;
        case 9:
            joinData = new t_join_pause2(PROCESSOR_COUNT);
            break;
        case 10:
            joinData = new t_join_pause10(PROCESSOR_COUNT);
            break;
        default:
            printf("");
            break;
        }

        // Create all the threads
        for (int i = 0; i < PROCESSOR_COUNT; i++)
        {
            ThreadInput* tInput = new ThreadInput(i, INPUT_COUNT);
            if (tInput != NULL)
            {
                tInput->input = (ulong*)malloc((sizeof(ulong) * INPUT_COUNT));
                for (int i = 0; i < INPUT_COUNT; i++)
                {
                    if (COMPLEXITY == 0)
                    {
                        tInput->input[i] = i;
                    }
                    else
                    {
                        //n = (float)rand() / RAND_MAX;
                        //tInput->input[i] = (ulong)(n * (100 + pow(2, COMPLEXITY)));
                        ulong num = (ulong)pow(2, COMPLEXITY);
                        tInput->input[i] = rand() % num + 1;
                        //printf("t %d: num %I64d\n", i, tInput->input[i]);
                    }
                }
            }
            else {
                assert(!"Failed to allocate tInput");
            }

            DWORD tid;
            threadHandles[i] = CreateThread(
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

            SetThreadDescription(threadHandles[i], buffer);

            if (setThreadHighPri)
            {
                SetThreadPriority(threadHandles[i], THREAD_PRIORITY_HIGHEST);
            }
        }

        PRINT_STATS("setting affinity for %d threads, %d cpu groups", PROCESSOR_COUNT, PROCESSOR_GROUP_COUNT);

        if (affinity_type != not_affinitized)
        {
            // Hard affinitize the threads to cores.
			SetThreadAffinity(PROCESSOR_COUNT, PROCESSOR_GROUP_COUNT, threadHandles, (affinity_type == hard_affinitized_physical));
        }

        // https://stackoverflow.com/a/27739925
        beginTimer = std::chrono::steady_clock::now();
        start = __rdtsc();

        // Start all the threads
        for (int i = 0; i < PROCESSOR_COUNT; i++)
        {
            ResumeThread(threadHandles[i]);
        }

        // Wait till last thread would signal that it is done
        joinData->waitForThreads();

        unsigned __int64 elapsed_ticks = __rdtsc() - start;
        int64_t elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - beginTimer).count();

        int maxThreadsToPrint = min(4, PROCESSOR_COUNT);
        for (int i = 0; i < maxThreadsToPrint; i++)
        {
            FILETIME create_t, exit_t, kernel_t, user_t;
            if (!GetThreadTimes(threadHandles[i], &create_t, &exit_t, &kernel_t, &user_t))
            {
                printf("GetThreadTimes failed, %d\n", GetLastError());
                break;
            }

            const ULONGLONG umode_cpu_time = *reinterpret_cast<const ULONGLONG*>(&user_t);
            const ULONGLONG kmode_cpu_time = *reinterpret_cast<const ULONGLONG*>(&kernel_t);

            uint64_t cycles = 0;
            QueryThreadCycleTime(threadHandles[i], &cycles);

            PRINT_THEAD_STATS("T#%d: umode cpu %5I64dms, kmode cpu %5I64dms, cycles on thread %s, elapsed_time (ms)  %s , elapsed_ticks  %s",
                i, (umode_cpu_time / 10000), (kmode_cpu_time / 10000), formatNumber(cycles), formatNumber(threadInputs[i]->elapsed_time), formatNumber(threadInputs[i]->elapsed_ticks));
        }

        int totalHardWaits = 0, totalSoftWaits = 0;
        ulong totalIterations = 0;
        ulong totalIterationsSoftWaits = 0;
        ulong totalIterationsHardWaits = 0;
        unsigned __int64 totalSoftWaitWakeupTimeTicks = 0;
        unsigned __int64 totalHardWaitWakeupTimeTicks = 0;
        unsigned __int64 totalSpinLoopTime = 0;
        unsigned __int64 totalSpinLoopTimeSoftWait = 0;
        unsigned __int64 totalSpinLoopTimeHardWait = 0;
        double totalElapsedTime = 0;
        double totalElapsedTicks = 0;
        for (int i = 0; i < PROCESSOR_COUNT; i++)
        {
            char diffCh;
            ulong diff;
            ThreadInput* outputData = threadInputs[i];
            assert(outputData->hardWaitCount <= INPUT_COUNT);
            assert(outputData->softWaitCount <= INPUT_COUNT);
            totalHardWaits += outputData->hardWaitCount;
            totalSoftWaits += outputData->softWaitCount;
            totalIterationsSoftWaits += outputData->totalIterationsSoftWaits;
            totalIterationsHardWaits += outputData->totalIterationsHardWaits;
            totalSoftWaitWakeupTimeTicks += outputData->softWaitWakeupTimeTicks;
            totalHardWaitWakeupTimeTicks += outputData->hardWaitWakeupTimeTicks;
            totalSpinLoopTimeSoftWait += outputData->spinLoopTimeTicksSoftWait;
            totalSpinLoopTimeHardWait += outputData->spinLoopTimeTicksHardWait;
            totalElapsedTime += outputData->elapsed_time;
            totalElapsedTicks += outputData->elapsed_ticks;
            DiffWakeTime(outputData->hardWaitWakeupTimeTicks, outputData->softWaitWakeupTimeTicks, &diff, &diffCh);
            PRINT_THEAD_STATS("[Thread #%d] HardWait: %d, SoftWait: %d, SpinLoop cycles: %llu, HardWaitWakeupTime: %llu, SoftWaitWakeupTime: %llu, Diff: %c%llu", 
                i,
                outputData->hardWaitCount, outputData->softWaitCount, outputData->spinLoopTimeTicksSoftWait, 
                outputData->hardWaitWakeupTimeTicks, outputData->softWaitWakeupTimeTicks, diffCh, diff);
        }
        totalSpinLoopTime = totalSpinLoopTimeSoftWait + totalSpinLoopTimeHardWait;
        totalIterations = totalIterationsSoftWaits + totalIterationsHardWaits;

#define AVG(n) ((n / (INPUT_COUNT * PROCESSOR_COUNT)) + 1)
#define AVG_NUMBER(n) ((n / INPUT_COUNT) + 1)
#define AVG_THREAD(n) ((n / PROCESSOR_COUNT) + 1)

#define AVG_WAKETIME(n, count) ((n / count) + 1)

        ulong avgDiff;
        char avgDiffChar;

        ulong avgHardWaitWakeupTime = totalHardWaits == 0 ? 0 : AVG_WAKETIME(totalHardWaitWakeupTimeTicks, totalHardWaits);
        ulong avgSoftWaitWakeupTime = totalSoftWaits == 0 ? 0 : AVG_WAKETIME(totalSoftWaitWakeupTimeTicks, totalSoftWaits);
        // We spin-loop for both, hard-wait and soft-wait. So take both into account.
        ulong avgSpinLoopTimePerWait = (totalHardWaits + totalSoftWaits) == 0 ? 0 : AVG_WAKETIME(totalSpinLoopTime, (totalHardWaits + totalSoftWaits));
        ulong avgSpinLoopTimePerSoftWait = (totalSoftWaits == 0) ? 0 : AVG_WAKETIME(totalSpinLoopTimeSoftWait, (totalSoftWaits));
        ulong avgSpinLoopTimePerHardWait = (totalHardWaits == 0) ? 0 : AVG_WAKETIME(totalSpinLoopTimeHardWait, (totalHardWaits));
        ulong avgIterationsPerSoftWait = (totalSoftWaits == 0) ? 0 : AVG_WAKETIME(totalIterationsSoftWaits, (totalSoftWaits));
        ulong avgIterationsPerHardWait = (totalHardWaits == 0) ? 0 : AVG_WAKETIME(totalIterationsHardWaits, (totalHardWaits));

        double totalHardWaitCost = totalHardWaits * (avgSpinLoopTimePerHardWait + avgHardWaitWakeupTime);
        double totalSoftWaitCost = totalSoftWaits * (avgSpinLoopTimePerSoftWait + avgSoftWaitWakeupTime);
        double grandCost = totalHardWaitCost + totalSoftWaitCost;

        DiffWakeTime(avgHardWaitWakeupTime, avgSoftWaitWakeupTime, &avgDiff, &avgDiffChar);

        ulong totalSoftWaitIterations = totalIterations - (totalHardWaits * SPIN_COUNT);

        PRINT_STATS("___________________________________________________________________________________________________________");
        PRINT_STATS("%10s | %10s | %10s | %10s | %20s | %30s |", "wait type", "total", "per number", "iterations", "spin (cycles/wait)", "wakeup latency (cycles/wait)");
        PRINT_STATS("___________________________________________________________________________________________________________");
        PRINT_STATS("%10s | %10s | %10.03f | %10s | % 20s | % 30s |", "soft", formatNumber(totalSoftWaits), ((double)totalSoftWaits / (INPUT_COUNT * (PROCESSOR_COUNT - 1))),
            formatNumber(avgIterationsPerSoftWait), formatNumber(avgSpinLoopTimePerSoftWait), formatNumber(avgSoftWaitWakeupTime));
        PRINT_STATS("%10s | %10s | %10.03f | %10s | % 20s | % 30s |", "hard", formatNumber(totalHardWaits), ((double)totalHardWaits / (INPUT_COUNT * (PROCESSOR_COUNT - 1))),
            formatNumber(avgIterationsPerHardWait), formatNumber(avgSpinLoopTimePerHardWait), formatNumber(avgHardWaitWakeupTime));
        PRINT_STATS("___________________________________________________________________________________________________________");
        PRINT_STATS("Elapsed cycles: %s; Elapsed time (ms): %s; Total cycles in spin: %s; Spin cycles / thread: %s; ", formatNumber(elapsed_ticks), formatNumber(elapsed_time), formatNumber(totalSpinLoopTime), formatNumber(totalSpinLoopTime / PROCESSOR_COUNT));
        PRINT_STATS("Elapsed cycles: %s; Elapsed time (ms): %s; // Average number per thread", formatNumber(totalElapsedTicks/ PROCESSOR_COUNT), formatNumber(totalElapsedTime / PROCESSOR_COUNT));

        //printf("HT | Affinity | Input_count | complexity | join_type | Spin_count | Thread_Count | Soft Wait: Total | Soft Wait: #/input | Soft Wait: Iterations/wait | Soft Wait: Spin time/wait | Soft wait: wakeup latency/wait Hard Wait: Total | Hard Wait: #/input | Hard Wait: Iterations/wait | Hard Wait: Spin time/wait | Hard wait: wakeup latency/wait Total cycles spinning | Cycles spin per thread Elapsed time | Elapsed cycles | MWaitXCycles\n");
        PRINT_ONELINE_STATS("%s|%s|%d|%d|%s|%d|%d|%d|%10.03f|%llu|%llu|%llu|%d|%10.03f|%llu|%llu|%llu|%llu|%llu|%llu|%llu|%s",
            //  HT | Affinity | Input_count | complexity | join_type | Spin_count | Thread_Count |
            (HYPERTHREADING_ENABLED ? "HT" : "no HT"), str_affinity_attribute[affinity_type], INPUT_COUNT, COMPLEXITY, str_join_types[JOIN_TYPE], SPIN_COUNT, PROCESSOR_COUNT,
            // Soft Wait: Total | Soft Wait: #/input | Soft Wait: Iterations/wait | Soft Wait: Spin time/wait | Soft wait: wakeup latency/wait
            totalSoftWaits, ((double)totalSoftWaits / (INPUT_COUNT * (PROCESSOR_COUNT - 1))), avgIterationsPerSoftWait, avgSpinLoopTimePerSoftWait, avgSoftWaitWakeupTime,
            // Hard Wait: Total | Hard Wait: #/input | Hard Wait: Iterations/wait | Hard Wait: Spin time/wait | Hard wait: wakeup latency/wait
            totalHardWaits, ((double)totalHardWaits / (INPUT_COUNT * (PROCESSOR_COUNT - 1))), avgIterationsPerHardWait, avgSpinLoopTimePerHardWait, avgHardWaitWakeupTime,
            // Total cycles spinning | Cycles spin per thread
            totalSpinLoopTime, (totalSpinLoopTime / PROCESSOR_COUNT),
            // Elapsed time | Elapsed cycles | MWaitXCycles
            elapsed_time, elapsed_ticks, formatNumber(MWAITX_CYCLES));

        FILE* res_file = NULL;
        errno_t err = fopen_s(&res_file, "res.csv", "a");
        if (err)
        {
            printf("file could not be opened\n");
            return 1;
        }
        err = fseek(res_file, 0, SEEK_END);
        if (err)
        {
            printf("file could not be seeked\n");
            return 1;
        }

        char res_buf[1024];
        long pos = ftell(res_file);
        if (pos == 0)
        {
            // Add header if we are creating new file.
            sprintf_s(res_buf, sizeof(res_buf), "sep=,\nHT, AFFINITY, thread_count, input_count, spin_count, mwait_count,complexity,join_type,exec time(ms), num of softwaits, iterations/softwait, cycles in spin/softwait, waitup latency/softwait, num of hardwaits, iterations/hardwait, cycles in spin/hardwait, waitup latency/hardwait");
            fputs(res_buf, res_file);
        }

        sprintf_s(res_buf, sizeof(res_buf), "\n%d,%s,%d,%d,%d,%d,%d,%s,%I64d,%d,%I64d,%I64d,%I64d,%d,%I64d,%I64d,%I64d",
            HYPERTHREADING_ENABLED, str_affinity_attribute[affinity_type], PROCESSOR_COUNT, INPUT_COUNT, SPIN_COUNT, MWAITX_CYCLES, COMPLEXITY, str_join_types[JOIN_TYPE], elapsed_time,
            totalSoftWaits, avgIterationsPerSoftWait, avgSpinLoopTimePerSoftWait, avgSoftWaitWakeupTime, 
            totalHardWaits, avgIterationsPerHardWait, avgSpinLoopTimePerHardWait, avgHardWaitWakeupTime);
        fputs(res_buf, res_file);

        return true;
    }
};

int main(int argc, char** argv)
{
    PrimeNumbers p(argc, argv);
    p.PrimeNumbersTest();

    fflush(stdout);
    return 0;
}