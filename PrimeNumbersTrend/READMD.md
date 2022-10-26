Run the PrimeNumbers.exe long enough for various inputs/complexity and dumps the output in csv parseable format.

### Usage

```
Usage: PrimeNumbersTrend.exe <Path> <maxInputSize> <maxComplexity> <maxThreads>?
<Path>: Full path to PrimeNumbers.exe
<maxInputSize>: Max Input size.
<maxComplexity>: Max Complexity.
<maxThreads>: Optional max threads. By default, it will just run with threads equal to number of logical processors.
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
inputSize| Number of input numbers each thread will operate upon
complex| Cost of execution for each input number. More complexity means bigger input number and hence more time each thread will take to complete
thread| Number of threads. If <maxThreads> is not passed, this will always be same as no. of logical processors
iters_number| Average iterations threads had to spin for processing **each input number**
hardwait_number| Average hard waits threads made for processing **each input number**
softwait_number| Average soft waits threads made for processing **each input number**
wakeup_number| Average clock cycles threads took to wake-up since restart while processing **each input number**
iters_thread| Average iterations taken to process input numbers by **each thread**
hardwait_thread| Average hard waits made to process input numbers by **each thread**
softwait_thread| Average soft waits made to process input numbers by **each thread**
wakeup_thread| Average clock cycles took to wake-up since restart to process input numbers by **each thread**
ticks| Total clock cycles taken to process entire input.
totalTime | Total time in microseconds taken to process entire input.

Note: `*_number` shows metrics to process `per input number` while `*_thread` shows metrics to process `per thread`.

### Sample output

Here is the output of sample executions:

`PrimeNumbersTrend.exe path\to\PrimeNumbers.exe 10 4`

inputSize|complex|thread|iters_number|hardwait_number|softwait_number|wakeup_number|iters_thread|hardwait_thread|softwait_thread|wakeup_thread|ticks|totalTime
--|--|--|--|--|--|--|--|--|--|--|--|--|
1|0|0|1|0|20|1901892.6|2.2|19|47315|95095|1|1.2
1|1|0|1|1|20|648925.6|1.2|20.8|13841|32446.6|1|1.8
1|2|0|1|2|20|225070.2|1|21|10347|11254|1|2
1|3|0|1|3|20|1021824.8|1.8|19.6|46455.6|51091.6|1|1.6
1|4|0|1|4|20|541229.2|1|21|9865|27062|1|2
2|0|0|2|0|20|2213484.4|3.8|17.8|9458270.8|221349|1|2
2|1|0|2|1|20|2031321.2|2.6|18.4|10678683.4|203132.4|1|2
2|2|0|2|2|20|2053332|3.8|16|12521553|205333.4|1|2
2|3|0|2|3|20|1234212.4|2.2|19.2|3419397.4|123421.6|1|2.4
2|4|0|2|4|20|277983.8|1|21|9978.4|27798.8|1|3
3|0|0|3|0|20|1079915.6|1.6|20|3243649.4|161987.6|1|3.4
3|1|0|3|1|20|78570.2|1|21|10383|11786|1|4
3|2|0|3|2|20|435101.6|1.6|20.4|1148238.8|65265.6|1|3.8
3|3|0|3|3|20|221521.4|1.2|20.6|32032.6|33228.6|1|3.6
3|4|0|3|4|20|258763.4|1.2|20.4|300385.8|38814.8|1|3.6
4|0|0|4|0|20|155956.6|1|21|9309.2|31192|1|5
4|1|0|4|1|20|1771571.6|3.4|17.6|9520564.4|354314.6|1.2|3.8
4|2|0|4|2|20|2202142.8|3.4|17.8|12189596.8|440429|1|4
4|3|0|4|3|20|2011147.2|4.2|16.6|14707487|402229.8|1.2|3.8
4|4|0|4|4|20|68786.6|1|21|9584.6|13757.4|1|5
5|0|0|5|0|20|255282.2|1.2|20.6|838821.2|63820.8|1|5.8
5|1|0|5|1|20|147673.8|1|20.8|11049.6|36918.8|1|5.8
5|2|0|5|2|20|128132.6|1.2|20.8|27441|32033.4|1|5.8
5|3|0|5|3|20|282373.8|1|20.8|609927.8|70593.8|1|5.8
5|4|0|5|4|20|239807.4|1.2|20.6|632718.4|59952.2|1|5.8
6|0|0|6|0|20|1003940.6|1.8|19.6|5120189.6|301182.6|1|6.4
6|1|0|6|1|20|114779.2|1|20.8|51809.8|34434.2|1|6.8
6|2|0|6|2|20|2059560.6|5.8|15.2|20568980.4|617868.4|1.8|5
6|3|0|6|3|20|1879702.6|3.4|17.8|11363675.8|563911|1.6|5.6
6|4|0|6|4|20|559313.6|1.8|19.8|3492807.2|167794.4|1|6.6
7|0|0|7|0|20|251819.6|1|20.6|1023158.2|88137.2|1|7.6
7|1|0|7|1|20|250279.2|1|20.6|331778|87598.2|1|7.6
7|2|0|7|2|20|37429.2|1|21|7919.2|13100.6|1|8
7|3|0|7|3|20|447785|1|20.4|1600564|156725|1|7.4
7|4|0|7|4|20|832524.6|2|19|3907514.6|291384|1.2|6.8
8|0|0|8|0|20|1943366.4|4.4|16.6|19388900.2|777347|2|7
8|1|0|8|1|20|2228988.4|4.2|16.8|17134633.4|891595.8|2|7
8|2|0|8|2|20|69093.8|1|21|9370|27637.6|1|9
8|3|0|8|3|20|233057.6|1|20.8|682248.2|93223.2|1|8.8
8|4|0|8|4|20|229072.6|1|20.6|690470.4|91629.4|1|8.6
9|0|0|9|0|20|85013.4|1|21|221692.8|38256.2|1|10
9|1|0|9|1|20|151051.4|1|20.8|580700|67973.4|1|9.8
9|2|0|9|2|20|816586|1.6|19.8|5531957.6|367463.8|1.2|9.2
9|3|0|9|3|20|161609.4|1|20.8|330465|72724.4|1|9.8
9|4|0|9|4|20|1596462.4|2.6|18.8|10283338.4|718408.4|1.6|8.8
10|0|0|10|0|20|1661975.2|2.8|18.2|14073149.2|830987.8|1.8|9.2
10|1|0|10|1|20|483645.6|1.4|20|2331835.2|241823.2|1|10.4
10|2|0|10|2|20|62826.6|1|20.8|9169.2|31413.4|1|10.8
10|3|0|10|3|20|136959|1|20.8|302595.4|68479.8|1|10.8
10|4|0|10|4|20|47007.6|1|21|8424.2|23504|1|11
