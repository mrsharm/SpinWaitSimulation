
#include <windows.h>
#include <limits.h>
#include <stdio.h>
#include <stdint.h>
#include <intrin.h>
#include <conio.h>
#include <chrono>

const unsigned HS_CACHE_LINE_SIZE = 128;
char cache_line_separator0[HS_CACHE_LINE_SIZE];
volatile size_t g_global_location = 10;
char cache_line_separator1[HS_CACHE_LINE_SIZE];

// Credit: https://stackoverflow.com/a/1449859
void printfcomma2(long long n) {
    if (n < 1000) {
        printf("%d", n);
        return;
    }
    printfcomma2(n / 1000);
    printf("%03d", n % 1000);
    //printf(",%03d", n % 1000);
}

DWORD WINAPI MyThreadFunction(LPVOID lpParam)
{
    size_t original_value = g_global_location;
    unsigned int cyclesToWait = (unsigned int)lpParam;
    size_t waited_count = 0;
    size_t latency = UINT64_MAX;

    unsigned __int64 start = __rdtsc();
    auto beginTimer = std::chrono::steady_clock::now();
    while (g_global_location == original_value)
    {
        _mm_monitorx((const void*)&g_global_location, 0, 0);

        if (g_global_location == original_value) {
            waited_count++;
            //printf(" ");
            _mm_mwaitx(2, 0, cyclesToWait);            
            //_mm_pause();
            
            //start = __rdtsc();
        }
    }
    unsigned __int64 end = __rdtsc();
    latency = end > start ? (end - start) : (start - end);
    long long latency_iteration = latency / waited_count;
    long long diff = cyclesToWait - latency_iteration;
    auto elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - beginTimer).count();
    
    printfcomma2(cyclesToWait); printf(" | ");
    printfcomma2(waited_count); printf(" | ");
    printfcomma2(latency); printf(" | ");
    printfcomma2(latency_iteration); printf(" | ");
    printfcomma2(elapsed_time); printf(" | ");
    printfcomma2(diff); printf(" |\n");
    g_global_location = 10;
    return 0;
}

int main(int argc, char** argv)
{
    int secondsToSleep = atoi(argv[1]);
    //int cyclesToWait = atoi(argv[2]);

    printf("TIMEOUT | WAIT_COUNT | TOTAL_LATENCY | LATENCY_ITERATION | ELAPSED_TIME | DIFF |\n");
    for (unsigned int cyclesToWait = 0; cyclesToWait < 200000000; cyclesToWait += 500)
    {

        DWORD tid;
        HANDLE hThread = CreateThread(
            NULL,                   // default security attributes
            0,                      // use default stack size
            MyThreadFunction,     // thread function name
            (LPVOID)cyclesToWait, // argument to thread function
            CREATE_SUSPENDED,
            &tid);   // returns the thread identifier

        if (hThread == NULL)
        {
            printf("CreateThread failed: %d\n", GetLastError());
            return 1;
        }

        ResumeThread(hThread);

        Sleep((DWORD)secondsToSleep * 1000);
        g_global_location = 5;

        // Wait for worker thread to complete.
        while (g_global_location == 5)
        {
            _mm_pause();
        }
        fflush(stdin);
    }

    return 0;
}