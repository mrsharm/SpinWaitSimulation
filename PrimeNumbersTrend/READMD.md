## PrimeNumbersTrend

Run the PrimeNumbers.exe repeatedly for various inputs/complexity and dumps the output in csv parseable format. For given input, it runs `PrimeNumbers.exe` with different inputs ranging from `1 ~ maxInputSize` , `1 ~ maxComplexity` and reports back the counters. For each input, it executes 5 iterations and reports back the average counters as final output.

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
1|0|1|1|0|20|2016102.2|3|17.4|66107.4|100805.6|1|1
1|1|1|1|1|20|1967887|3|17.6|89463|98395|1|1
1|2|1|1|2|20|1348815|2.4|19|55391.2|67441.2|1|1.4
1|3|1|1|3|20|273400.2|1|21|10291|13670.6|1|2
1|4|1|1|4|20|389826|1|21|9133.4|19491.8|1|2
2|0|1|2|0|20|552828.8|1.8|19.6|1601306.8|55283.4|1|2.8
2|1|1|2|1|20|109988.8|1|21|7628.6|10999.4|1|3
2|2|1|2|2|20|408425.2|1.6|19.8|1556383.4|40843|1|2.8
2|3|1|2|3|20|282226.4|1|21|715838.2|28223.2|1|3
2|4|1|2|4|20|1051499.6|3|18|3156052.8|105150.2|1|2.6
3|0|1|3|0|20|1314457.8|3|18.2|4803923|197169.2|1|3.2
3|1|1|3|1|20|960450.6|3.2|18.4|4490452|144068|1|3.6
3|2|1|3|2|20|2259999.6|6.4|14.2|11639146.6|339000.2|1|3
3|3|1|3|3|20|2143994.4|5.8|15.4|14300595.8|321599.6|1.4|2.8
3|4|1|3|4|20|2011732.6|5.8|14.8|18416786.2|301760.2|1.4|2.8
4|0|1|4|0|20|1233517.2|4|17.2|7630524.4|246703.8|1.4|3.8
4|1|1|4|1|20|1183803.8|2.4|18.6|5131362.8|236761.2|1|4
4|2|1|4|2|20|348488|1.2|20.4|661973.2|69697.8|1|4.6
4|3|1|4|3|20|695976.2|2|19.8|2616369.8|139195.6|1|4.6
4|4|1|4|4|20|1379759|3.4|18|6943355.6|275952.4|1.2|4.2
5|0|1|5|0|20|278933.8|1.2|20.6|1020621|69733.8|1|5.8
5|1|1|5|1|20|609381.6|2.2|19.6|2741682.8|152346|1|5.6
5|2|1|5|2|20|2191181.8|5.6|15.6|13769743.6|547796|2|4
5|3|1|5|3|20|2050008.6|5.8|15.2|14191421.6|512502.6|2|4
5|4|1|5|4|20|1036645.8|3.6|17.8|10611428|259161.6|1.4|5
6|0|1|6|0|20|104484.4|1|20.8|11496.2|31345.6|1|6.8
6|1|1|6|1|20|120314|1|21|8615.6|36094.6|1|7
6|2|1|6|2|20|1196220.4|3.8|17.6|7170024.2|358866.6|1.6|5.8
6|3|1|6|3|20|276686.2|1.2|20.6|846497.2|83006.2|1|6.8
6|4|1|6|4|20|88163|1|21|7853.6|26449.4|1|7
7|0|1|7|0|20|172010.6|1|20.8|452702.2|60204.2|1|7.8
7|1|1|7|1|20|149023.2|1.2|20.6|394933.2|52158.2|1|7.8
7|2|1|7|2|20|496686.4|2|19.6|3204353.6|173840.8|1.4|7.4
7|3|1|7|3|20|2020701|5.8|15.2|12598595.4|707245.8|2.4|5.6
7|4|1|7|4|20|2185055.2|6.8|14.2|18854525.4|764769.6|2.8|5.2
8|0|1|8|0|20|113271.8|1|20.8|52555.2|45309|1|8.8
8|1|1|8|1|20|364676.4|1.8|19.6|1093763.6|145871.2|1.2|8.2
8|2|1|8|2|20|795928|3.4|18|4267865.6|318371.4|1.8|7.8
8|3|1|8|3|20|264594.4|1.2|20.6|1232133.4|105838|1|8.8
8|4|1|8|4|20|84840.6|1|20.8|9589.4|33936.6|1|8.8
9|0|1|9|0|20|172383.6|1.2|20.6|779645|77573.2|1|9.8
9|1|1|9|1|20|263359.2|1.4|20.2|966113|118512|1|9.6
9|2|1|9|2|20|1549975.4|4.4|16.6|10485426|697489.4|2.4|7.8
9|3|1|9|3|20|2121624.6|6|14.8|17554769|954731.4|2.8|7.2
9|4|1|9|4|20|428668.4|1.6|20|2547759.8|192901.2|1.2|9.4
10|0|1|10|0|20|530485.2|1.8|19.4|2513580.2|265243|1.2|10
10|1|1|10|1|20|79020.8|1|20.8|82092|39510.6|1|10.8
10|2|1|10|2|20|61602|1|21|7697.4|30801.2|1|11
10|3|1|10|3|20|105745.6|1|20.6|82213.6|52873.2|1|10.6
10|4|1|10|4|20|63263|1|20.8|8727.6|31631.8|1|10.8