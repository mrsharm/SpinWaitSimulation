# Multi-threaded application to simulate spin-wait

This utility simulates the behavior of `join` inside .NET's GC, except that it replaces the actual GC work with a work to calculate smallest prime number which is greater than input 'n'.

### Details

The program operates on as many threads as number of available processors `N`. It starts by creating `N` threads and generating `M` random `integer` input for each of those threads. When all threads are created and inputs have been generated, they resume the execution. Each thread will fetch the `ith` input and find the smallest prime number that is greater than the input. Once done, it will wait for all the other threads to finish before proceeding to the next input. The `t_join` class (and their dependency like `Volatile.h` and `EventImpl.h`) are  extracted from `dotnet/runtime` repo. The `join()` method is used by `N - 1` threads to spin-wait for `Nth` thread to complete its execution. It will spin-loop for `XXX` iterations (soft-wait) and if the last thread is not yet finished, it will go to the hard-wait. We will keep track of `totalIterations` a thread executed spin-loop, number of times it didn't had to go to hard-wait (`softWaitCount`) and number of times it had to hard-wait (`hardWaitCount`). In the end, it will print the stats for individual thread along with time took to complete the entire program execution.

### Usage

```
Usage: PrimeNumbers.exe <numPrimeNumbers> <complexity> [*threadCount*]
<numPrimeNumbers>: Number of prime numbers per thread.
<complexity>: Number between 0~31.
Higher the number, bigger the input numbers will beand thus more probability of threads going into hard - waits.
If complexity == 0, it will generate uniform, identical inputs from 0~(<numPrimeNumbers> - 1) for all the threads.
[*threadCount*]: Optional number of threads to use. By default it will use number of cores available in all groups.
```

A note about `complexity`: This number lets us adjust the approxiamate time each thread would need to find the next prime number. Higher the complexity, more time will be needed for all threads to finish and more chances for threads to go in hard-wait state.

### Sample Usage

1. `PrimeNumbers.exe 100 4`

Create `N` threads and create `100` random numbers between `0 ~ pow(2, 4)` that each thread will operate on. `N` depends on number of active processors present in active group.

2. `PrimeNumbers.exe 100 4 64`

Creates `64` threads and create `100` random numbers between `0 ~ pow(2, 4)` that each thread will operate on.
