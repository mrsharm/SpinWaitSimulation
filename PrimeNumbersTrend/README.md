## PrimeNumbersTrend

Run the PrimeNumbers.exe repeatedly for various inputs/complexity and dumps the output in csv parseable format. For given input, it runs `PrimeNumbers.exe` with different inputs ranging from `1 ~ maxInputSize` , `1 ~ maxComplexity` and reports back the counters. For each input, it executes 5 iterations and reports back the average counters as final output.

### Usage

```
Usage: PrimeNumbersTrend.exe 

  -p, --pathToPrimeNumberTrend    Required. Path to Prime Number Trend.

  -i, --maxInputSizeRange         Required. Range of Max input size in the form of start-stop-increment e.g., 100-1000-5
  -c, --maxComplexityRange        Range of Max Complexity in the form of start-stop-increment e.g., 15-25-2

  -t, --maxThreads                Range of Max Number of Threads e.g., 2-20-1

  -j, --joinType                  The Join Type as a Comma Separated List e.g., 1,2,4

  -h, --ht                        Specification if hyperthread is on or off. This must match the machine configuration.

  -a, --affinitizeRange           Range of Affinitized value in the form of start-stop-increment e.g., 0-2-1

  -m, --mwaitTimeoutRange         Range of The Timeout for mwait in the form of start-stop-increment e.g.,
                                  1000-3000-500.

  -o, --outputPath                Output path of the markdown.

  --help                          Display this help screen.

  --version                       Display version information.
```


### Update PrimeNumbers
Just compile PrimeNumbers in "Release" mode with following change:

```diff
-#define PRINT_STATS(msg, ...) printf(msg ".\n", __VA_ARGS__);
-//#define PRINT_ONELINE_STATS(msg, ...) printf(msg "\n", __VA_ARGS__);
+//#define PRINT_STATS(msg, ...) printf(msg ".\n", __VA_ARGS__);
+#define PRINT_ONELINE_STATS(msg, ...) printf(msg "\n", __VA_ARGS__);
```

### Output

It displays following data:
Column name | Meaning
--|--
HT| If hyperthreading is enabled or not. 
Affinity| Choice between affinity, affinity physical core, no affinity.
Input_Count| Number of input numbers each thread will operate upon
Complexity| Cost of execution for each input number. More complexity means bigger input number and hence more time each thread will take to complete
join_type| The type of join used.
Spin_count | The spin count used.
Thread_count| Number of threads. If <maxThreads> is not passed, this will always be same as no. of logical processors
totalSoftWaits | The total number of softwaits. 
Soft Wait : Total| Total number of softwaits.
Soft Wait: #/input| Total number of softwaits / (Input_Count * (Thread_count - 1))
Soft Wait: Iterations/wait| Average Iterations Per Soft Wait.
Soft Wait: Spin time / wait| Average Spin Loop Time Per Soft Wait.
Soft Wait: wakeup latency / wait| Average Softwait Wake up Time 
Hard Wait : Total| Total number of softwaits.
Hard Wait: #/input| Total number of softwaits / (Input_Count * (Thread_count - 1))
Hard Wait: Iterations/wait| Average Iterations Per Hard Wait.
Hard Wait: Spin time / wait| Average Spin Loop Time Per Hard Wait.
Hard Wait: wakeup latency / wait| Average Hardwait Wake up Time 
Total cycles spinning | Total Spin Loop Time 
Cycles spin per thread| Total Spin Loop Time / Thread_Count
Elapsed time| Total time in microseconds taken to process entire input.
Elapsed cycles| Elapsed Ticks
Timeout | Either MwaitX CPU cycles or UMWAIT timestamp at which these instructions timeout.

Note:
- `*_per_number` shows metrics to process each number.
- `*_number_allthreads` shows metrics to process `per input number` for all threads combined.
- `*_thread_allnumbers` shows metrics to process `per thread` for all input numbers.


### Sample output

|HT|Affinity|Input_count|Complexity|join_type|Spin_count|Thread_count|Soft Wait : Total|Soft Wait: #/input|Soft Wait: Iterations/wait|Soft Wait: Spin time / wait|Soft Wait: wakeup latency / wait|Hard Wait : Total|Hard Wait: #/input|Hard Wait: Iterations/wait|Hard Wait: Spin time / wait|Hard Wait: wakeup latency / wait|Total cycles spinning |Cycles spin per thread|Elapsed time|Elapsed cycles|MWaitx Cycles
|--|--|--|--|--|--|--|--|--|--|--|--|--|--|--|--|--|--|--|--|--|--|--|
|no HT|affinity|1000|0||128000|2|1000|1|583.2|20884.2|248|0|0|0|0|0|20883836.8|10441918.4|26|0
|no HT|affinity|1000|0||128000|2|14|0.014799999999999999|2|2681.4|1106.6|985|0.9852000000000001|2|2192.4|20008.6|2198054.4|1099027.2|37|0
|no HT|affinity|1000|0||128000|2|1000|1|581.4|20755.8|196.6|0|0|0|0|0|20755276.8|10377638.4|26|0
|no HT|affinity|1000|0||128000|2|19|0.0196|2|2035.4|330.8|980|0.9802|2|2386.6|17929.2|2378608|1189304|35.6|0
|no HT|affinity|1000|0||128000|2|1000|1|583.4|20867.4|231.4|0|0|0|0|0|20866812.8|10433406.4|26|0
|no HT|affinity|1000|0||128000|2|24|0.024200000000000003|2|2193.4|422.4|975|0.9757999999999999|2|2561|17839.6|2551513.6|1275756.8|35.6|0
|no HT|affinity|1000|0||128000|2|1000|1|582.8|20857.6|230|0|0|0|0|0|20857062.4|10428531.2|26|0
