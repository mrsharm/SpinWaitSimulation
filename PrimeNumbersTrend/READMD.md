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
iters_per_number| Average iterations to spin for processing **each input number**
hardwait_per_number| Average hard waits for processing **each input number**
softwait_per_number| Average soft waits for processing **each input number**
wakeup_per_number| Average clock cycles took to wake-up since restart while processing **each input number**
iters_number_allthreads| Average iterations threads had to spin for processing **each input number**
hardwait_number_allthreads| Average hard waits threads made for processing **each input number**
softwait_number_allthreads| Average soft waits threads made for processing **each input number**
wakeup_number_allthreads| Average clock cycles threads took to wake-up since restart while processing **each input number**
iters_thread_allnumbers| Average iterations taken to process input numbers by **each thread**
hardwait_thread_allnumbers| Average hard waits made to process input numbers by **each thread**
softwait_thread_allnumbers| Average soft waits made to process input numbers by **each thread**
wakeup_thread_allnumbers| Average clock cycles took to wake-up since restart to process input numbers by **each thread**
ticks| Total clock cycles taken to process entire input.
totalTime | Total time in microseconds taken to process entire input.

Note:
- `*_per_number` shows metrics to process each number.
- `*_number_allthreads` shows metrics to process `per input number` for all threads combined.
- `*_thread_allnumbers` shows metrics to process `per thread` for all input numbers.


### Sample output

Here is the output of sample executions:

`PrimeNumbersTrend.exe path\to\PrimeNumbers.exe 10 5

inputSize|complex|thread|iters_per_number|hardwait_per_number|softwait_per_number|wakeup_per_number|iters_number_allthreads|hardwait_number_allthreads|softwait_number_allthreads|wakeup_number_allthreads|iters_thread_allnumbers|hardwait_thread_allnumbers|softwait_thread_allnumbers|wakeup_thread_allnumbers|ticks|totalTime
1|0|1|1|0|20|67525.8|1|1.4|5053|1350505.6|2.6|19|101050.6|67525.8|1|1.4
1|1|1|1|1|20|68930.4|1|1.4|4390.4|1378592.6|2.8|19.2|87797.4|68930.4|1|1.4
1|2|1|1|2|20|70054.4|1|1.4|1849.6|1401076.6|2|19.8|36986.6|70054.4|1|1.4
1|3|1|1|3|20|32532|1|1.8|973.2|650630.4|1.4|20.6|19453.6|32532|1|1.8
1|4|1|1|4|20|20106|1|2|565|402109|1|21|11291.4|20106|1|2
1|5|1|1|5|20|14026.4|1|2|840|280515|1|21|16790.6|14026.4|1|2
2|0|1|2|0|20|19329|1|1.8|537.6|386572.2|1|20.8|10744.2|38657.6|1|2.8
2|1|1|2|1|20|67004.6|1|1.4|150473.6|1340087.2|3.6|17.8|3009462.2|134009|1|2.4
2|2|1|2|2|20|58975.4|1|1.2|80565|1179503.6|2.2|19.4|1611289.4|117950.8|1|2.2
2|3|1|2|3|20|65948.2|1|1.2|166614|1318953.6|3.6|18|3332268.6|131895.8|1|2.2
2|4|1|2|4|20|27096.2|1|1.8|79034|541916.8|2.2|19.6|1580672.8|54191.8|1|2.8
2|5|1|2|5|20|6631.2|1|2|433.6|132611.4|1|21|8661.6|13261.8|1|3
3|0|1|3|0|20|43322|1|1.6|217847|866432.6|2.8|18.8|4356930.4|129965.4|1|3.6
3|1|1|3|1|20|84642|1|1.2|345048.2|1692830.2|4.2|17.2|6900956.6|253925|1|3.2
3|2|1|3|2|20|70837.4|1|1|273067.8|1416738.2|4|17.4|5461342.4|212511.4|1|3
3|3|1|3|3|20|76143.2|1|1.2|360874.2|1522857.8|4.2|17.4|7217471|228429|1.4|3
3|4|1|3|4|20|72278.6|1|1.2|170444|1445563.2|3.8|17.8|3408873.4|216834.8|1|3.2
3|5|1|3|5|20|97416|1|1|682224.4|1948310|5.4|15.6|13644476.8|292247|1|3
4|0|1|4|0|20|66620.6|1|1.4|647551.4|1332405.6|5|16.6|12951017.2|266481.4|1.4|4
4|1|1|4|1|20|84456.6|1|1.2|432493.2|1689124.8|5.2|16.2|8649854.8|337825.4|1.4|3.8
4|2|1|4|2|20|51361.6|1|1.4|231401.6|1027216.6|3.4|18.2|4628025|205443.8|1.2|4.2
4|3|1|4|3|20|73663|1|1.2|342182|1473252.2|3.4|18|6843628.2|294651|1.2|4
4|4|1|4|4|20|36976.4|1|1.4|123696.8|739517.2|2.4|19|2473925|147904|1|4.4
4|5|1|4|5|20|18423.6|1|1.6|91069.2|368459.8|1.6|20|1821373|73692.4|1|4.6
5|0|1|5|0|20|7093.6|1|1.8|4238.4|141863.4|1|20.8|84756.6|35466.4|1|5.8
5|1|1|5|1|20|9118|1|1.8|32053.6|182347.4|1.4|20.6|641063.4|45587.4|1|5.8
5|2|1|5|2|20|42588.8|1|1.4|253624.6|851768.2|2.8|18.6|5072486.4|212942.4|1.2|5.2
5|3|1|5|3|20|46154|1|1.4|374643.2|923068.8|3.4|18|7492858.4|230767.6|1.6|4.8
5|4|1|5|4|20|2502.6|1|2|430|50038.2|1|21|8589.8|12510.2|1|6
5|5|1|5|5|20|8682|1|1.8|14302.6|173632.4|1|20.8|286039.8|43408.4|1|5.8
6|0|1|6|0|20|2254.8|1|2|1333.8|45085.6|1|21|26668.4|13526|1|7
6|1|1|6|1|20|2051|1|2|420|41008|1|21|8392|12302.8|1|7
6|2|1|6|2|20|3653|1|2|10502|73047.8|1|21|210032.8|21914.8|1|7
6|3|1|6|3|20|83274.8|1|1.2|617391.4|1665489.2|5.4|16|12347818.6|499646.8|2|5.2
6|4|1|6|4|20|47530.6|1|1.2|269128|950601|3.4|17.8|5382550|285180.8|1.6|5.8
6|5|1|6|5|20|73285.2|1|1|413561.2|1465691.6|4|17|8271220|439708|1.6|5.4
7|0|1|7|0|20|81028.4|1|1|500579.6|1620563.6|4.6|16.4|10011578.2|567197.6|1.8|6.2
7|1|1|7|1|20|14254.4|1|1.6|44041|285077|1.4|20.4|880808.8|99777.2|1|7.6
7|2|1|7|2|20|5709.6|1|1.8|3506|114184|1|20.8|70105.2|39964.8|1|7.8
7|3|1|7|3|20|5970.8|1|2|426.8|119408.8|1|21|8532.4|41793.4|1|8
7|4|1|7|4|20|25076.4|1|1.4|103321.8|501519|1.8|19.6|2066431.4|175531.8|1.2|7.2
7|5|1|7|5|20|72888.8|1|1|469644.6|1457767.2|4.2|17|9392885.2|510218.8|1.8|6.2
8|0|1|8|0|20|27662|1|1.4|132878|553233|1.8|19.6|2657553.4|221293.6|1.2|8.2
8|1|1|8|1|20|27417|1|1.6|129215.8|548330.2|2.2|19.4|2584311|219332.4|1.4|8.2
8|2|1|8|2|20|32496.8|1|1.2|140271.8|649926.2|2.2|19|2805429.4|259970.6|1.4|7.8
8|3|1|8|3|20|7067.2|1|1.8|14874|141328|1|20.8|297467.4|56531.6|1|8.8
8|4|1|8|4|20|50207.4|1|1.2|348526.6|1004140|3.6|17.6|6970520.6|401656.2|2|7.4
8|5|1|8|5|20|30523.4|1|1.2|124412.4|610458|2.2|19.2|2488243.4|244183.4|1.2|8.2
9|0|1|9|0|20|5453.4|1|1.8|15317.6|109058|1|20.8|306338|49076.4|1|9.8
9|1|1|9|1|20|9619.6|1|1.6|22274|192380.6|1.2|20.4|445466|86571.8|1|9.6
9|2|1|9|2|20|2115|1|2|2088|42288.4|1|21|41750.8|19030|1|10
9|3|1|9|3|20|31981.8|1|1.2|208905.2|639625.6|1.8|19.6|4178096.4|287831.8|1.2|9
9|4|1|9|4|20|35930.8|1|1.6|234454.8|718609.2|2.6|19.4|4689086|323374.6|1.4|9.2
9|5|1|9|5|20|27983.4|1|1.6|133774.2|559658|2.4|19.2|2675469.2|251846.2|1.6|9
10|0|1|10|0|20|23579.8|1|1.6|101589.4|471585.4|2|19.6|2031777.6|235793|1.4|10.2
10|1|1|10|1|20|2164.8|1|2|1393.6|43287.2|1|21|27863.6|21643.8|1|11
10|2|1|10|2|20|22002.2|1|1.6|110772.2|440034.6|1.8|19.8|2215434.2|220017.4|1.4|10.2
10|3|1|10|3|20|3185.6|1|1.6|9363.8|63705|1|20.6|187264.6|31852.8|1|10.6
10|4|1|10|4|20|2360.2|1|2|9582|47192|1|21|191629.8|23596.4|1|11
10|5|1|10|5|20|17475.2|1|1.4|83648.2|349500|1.8|19.6|1672957.6|174750.2|1.2|10.2