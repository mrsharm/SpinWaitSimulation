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
hardWaitWakeup_per_number| Average clock cycles took to wake-up from hard-wait since restart while processing **each input number**
softWaitWakeup_per_number| Average clock cycles took to wake-up from soft-wait since restart while processing **each input number**
iters_number_allthreads| Average iterations threads had to spin for processing **each input number**
hardwait_number_allthreads| Average hard waits threads made for processing **each input number**
softwait_number_allthreads| Average soft waits threads made for processing **each input number**
hardWaitWakeup_number_allthreads| Average clock cycles threads took to wake-up from hard-wait since restart while processing **each input number**
softWaitWakeup_number_allthreads| Average clock cycles threads took to wake-up from soft-wait since restart while processing **each input number**
iters_thread_allnumbers| Average iterations taken to process input numbers by **each thread**
hardwait_thread_allnumbers| Average hard waits made to process input numbers by **each thread**
softwait_thread_allnumbers| Average soft waits made to process input numbers by **each thread**
hardWaitWakeup_thread_allnumbers| Average clock cycles took to wake-up from hard-wait since restart to process input numbers by **each thread**
softWaitWakeup_thread_allnumbers| Average clock cycles took to wake-up from soft-wait since restart to process input numbers by **each thread**
ticks| Total clock cycles taken to process entire input.
totalTime | Total time in microseconds taken to process entire input.

Note:
- `*_per_number` shows metrics to process each number.
- `*_number_allthreads` shows metrics to process `per input number` for all threads combined.
- `*_thread_allnumbers` shows metrics to process `per thread` for all input numbers.


### Sample output

Here is the output of sample executions:

`PrimeNumbersTrend.exe path\to\PrimeNumbers.exe 10 5

inputSize|complex|thread|iters_per_number|hardwait_per_number|softwait_per_number|hardWaitWakeup_per_number|softWaitWakeup_per_number|iters_number_allthreads|hardwait_number_allthreads|softwait_number_allthreads|hardWaitWakeup_number_allthreads|softWaitWakeup_number_allthreads|iters_thread_allnumbers|hardwait_thread_allnumbers|softwait_thread_allnumbers|hardWaitWakeup_thread_allnumbers|softWaitWakeup_thread_allnumbers|ticks|totalTime
 --|--|--|--|--|--|--|--|--|--|--|--|--|
--|--|--|--|--|--|--|--|--|--|--|--|--|--|--|--|--|--|--|--|
1|0|1|33174.8|1|1.8|2127.2|2268.6|663492|1.6|20.4|42528.8|45359.2|33174.8|1|1.8|2127.2|2268.6|6591518.4|1783.2
1|1|1|51735.4|1|1.6|1707.6|1858.8|1034702.2|1.8|20.2|34141.6|37171.6|51735.4|1|1.6|1707.6|1858.8|8458439.8|2288.4
1|2|1|66043.2|1|1.4|1985|2119.8|1320856.8|1.6|19.8|39692|42387.2|66043.2|1|1.4|1985|2119.8|10977395.2|2970
1|3|1|60630.6|1|1.6|1377.4|1533|1212604.6|1.6|20.2|27541.4|30647.8|60630.6|1|1.6|1377.4|1533|9773289.4|2644.2
1|4|1|30607.4|1|1.8|1847.6|1968.4|612140|1.4|20.4|36941.8|39360.2|30607.4|1|1.8|1847.6|1968.4|6113238.2|1654.2
1|5|1|46379.4|1|1.8|1376.6|1510|927577|1.4|20.6|27521.8|30187|46379.4|1|1.8|1376.6|1510|7915056.6|2141.4
2|0|1|62998|1|1.4|178983|179092|1259947.6|4|17.8|3579650|3581833|125995.4|1|2.4|357965.4|358183.6|17748031.4|4801.8
2|1|1|17856.8|1|1.8|431|568.2|357123.8|1|20.8|8611.8|11353.8|35713|1|2.8|861.6|1135.8|6421296.8|1737.2
2|2|1|79261.4|1|1|377432.8|377563|1585219.2|4.4|16.6|7548644.6|7551254.2|158522.4|1|2|754865|755125.8|23066830.4|6241
2|3|1|38291.6|1|1.6|50304.4|50431.8|765821.2|1.2|20.6|1006072.8|1008627.4|76582.6|1|2.6|100607.8|100863.2|11754507.4|3180.2
2|4|1|6927.4|1|2|3077.8|3208|138537.4|1|21|61548.8|64149|13854.2|1|3|6155.4|6415.4|3653244.4|988.4
2|5|1|28473.8|1|1.8|77376.6|77520.8|569467.4|1.6|20.4|1547520.2|1550405.6|56947|1|2.8|154752.4|155040.8|9282621.4|2511.4
3|0|1|10058.2|1|2|36654.6|36750.8|201151.6|1|21|733085.8|735004|30173.4|1|4|109963.2|110251|5641777.4|1526.2
3|1|1|5416.6|1|2|5479.2|5601|108320.8|1|21|109569.8|112011.2|16248.8|1|4|16436|16802.4|3923121.6|1061.4
3|2|1|27568.8|1|1.4|35027.2|35119.2|551366.2|1.4|20.2|700534.2|702373.6|82705.4|1|3.4|105080.4|105356.4|12622886.2|3415.2
3|3|1|90327.8|1|1|502794.2|502919.4|1806544|5.2|16|10055874.4|10058379.6|270982.2|1.4|2.8|1508381.6|1508757.4|37383301.2|10114.6
3|4|1|62363.4|1|1.2|212498.6|212635.4|1247261.8|3.4|18|4249968.4|4252701.6|187089.6|1|3.2|637495.6|637905.4|25880752.6|7002.2
3|5|1|77682.8|1|1.2|393775.8|393910|1553645.8|5.4|16|7875512.2|7878188.4|233047.4|1.2|3|1181327|1181728.8|32548579|8806.2
4|0|1|4463|1|2|547|632.8|89253.2|1|21|10933.6|12645.4|17851|1|5|2187|2529.6|4257465|1151.6
4|1|1|37901.4|1|1.4|137620.4|137729.6|758015.4|2.6|18.8|2752397.2|2754585.6|151603.6|1.2|4.2|550479.8|550917.4|21639908.8|5855
4|2|1|8772.4|1|1.8|4834.6|4959.8|175443.4|1.2|20.8|96678.6|99185|35089|1|4.8|19336.2|19837.4|6258067.4|1693.2
4|3|1|51719.2|1|1.6|411358|390480.6|1034372.4|3.6|18.2|8227151.6|7809605|206875|1.2|4.4|1645430.8|1561921.4|29166723|7891.2
4|4|1|100649.8|1|1|772786|772920.2|2012985.6|7|13.8|15455712.4|15458392|402597.4|2|3|3091142.8|3091678.8|56061719|15168.6
4|5|1|110115.8|1|1|787811.4|787957.4|2202302.6|6.4|15|15756217.4|15759141.2|440461|1.8|3.4|3151243.8|3151828.8|59866787.2|16197.6
5|0|1|92958.8|1|1|1000910.2|1001032.6|1859163|6.2|15|20018195.4|20020639.8|464791.2|2|4|5004549.4|5005160.2|66866065.8|18092
5|1|1|112057.2|1|1|1065393.8|1034872.2|2241130.8|8.2|13|21307866.4|20697434|560283.2|2.2|3.8|5326967|5174359|77673621.8|21015.8
5|2|1|112093.4|1|1|1174882.6|1175009.6|2241857.8|7.6|13.4|23497644|23500183|560464.6|2|4|5874411.4|5875046.2|77568205|20987.4
5|3|1|114088.6|1|1|774643.6|774769.4|2281759|7.2|14.2|15492859.4|15495374.6|570440.2|2|4|3873215.2|3873844.2|75672412.4|20474.4
5|4|1|35107.6|1|1.6|147083.8|147221.4|702144.2|2.2|19.4|2941666.8|2944422|175536.6|1.2|5.4|735417|736105.8|23925841.6|6473.4
5|5|1|9803.8|1|1.8|21423.8|21547.8|196065.8|1|20.8|428464.2|430945.6|49016.6|1|5.8|107116.4|107737|8179850.4|2213
6|0|1|2186.4|1|2|396.6|500.2|43724|1|21|7923|9991.4|13117.8|1|7|2377.2|2997.8|3498976.4|946.6
6|1|1|4683.2|1|2|328|453.6|93654|1|21|6549|9058.2|28096.8|1|7|1965|2718|5401648.6|1461.2
6|2|1|39375|1|1.2|296501.8|296618.6|787492.8|2.4|18.8|5930027.2|5932361.6|236248|1.2|6|1779008.4|1779708.8|33362637.8|9026.6
6|3|1|9606|1|1.6|5823.2|5910.2|192112.6|1|20.6|116452.6|118193.6|57634.2|1|6.6|34936.4|35458.4|9242102.2|2500.2
6|4|1|21958.4|1|1.8|90558.8|90687|439154.4|2|19.8|1811169.2|1813728|131746.6|1.2|6.6|543351.2|544118.8|18911560.6|5116.8
6|5|1|20681.8|1|1.6|103753.8|103897.6|413625|2|19.8|2075066.2|2077944.8|124087.8|1.2|6.4|622520.2|623383.8|18194686|4922.6
7|0|1|6483.4|1|1.8|12788.6|12879.4|129659.4|1|20.8|255759.2|257579.2|45381|1|7.8|89516|90153|7749003.6|2096.6
7|1|1|33437.8|1|1.2|223228.2|223333.4|668749.6|2|19.2|4464554|4466661.4|234062.8|1.2|7|1562594.2|1563331.8|32532179.8|8802
7|2|1|7400.8|1|1.8|17895.6|17996.2|148005.2|1|20.8|357903.6|359912.4|51802|1|7.8|125266.6|125969.8|8412488.8|2276
7|3|1|6084.2|1|1.8|5708.2|5829.8|121671.4|1|20.8|114152|116582|42585.4|1|7.8|39953.6|40804|7346333.6|1987.4
7|4|1|6472|1|1.8|15442|15576|129428.4|1|20.8|308830|311510.8|45300.2|1|7.8|108091|109029.2|7631721.6|2064.6
7|5|1|6730|1|1.8|34399|34524.2|134593|1.4|20.6|687970.6|690473.4|47108|1|7.8|240790|241666|7850431.2|2124
8|0|1|8017.8|1|1.8|40472.2|40573.8|160343|1|20.8|809438.8|811469.4|64137.6|1|8.8|323775.6|324588.2|10451313.2|2827.6
8|1|1|66230.2|1|1|424305|424423.6|1324591.2|4.2|17|8486090.4|8488461.2|529837|2|7.2|3394436.4|3395385|71616735.4|19377
8|2|1|11054.6|1|1.8|31533.8|31657.4|221082|1.4|20.4|630663.8|633139|88432.8|1|8.8|252265.8|253256|13072790.4|3536.8
8|3|1|7279.4|1|1.8|20749|20877.8|145579.4|1.2|20.6|414971.4|417541.6|58232.2|1|8.8|165989|167017|9497989.6|2569.6
8|4|1|3790.6|1|2|347.2|446.8|75804.2|1|21|6933.8|8925|30322.2|1|9|2773.8|3570.2|5872084.8|1588.6
8|5|1|1737.6|1|2|323.6|442.2|34748|1|21|6465|8831.6|13899.4|1|9|2586.2|3532.8|4013504.2|1085.8
9|0|1|16934|1|1.6|107977.2|107948|338671.6|1.2|20.4|2159538.2|2158947.8|152402.6|1|9.6|971792.6|971526.8|22097372.2|5978.8
9|1|1|9409.2|1|1.8|29097.8|29215.8|188175|1.2|20.6|581950.2|584306.6|84679|1|9.8|261877.8|262938.2|12884150.4|3486.2

9|3|1|10299.2|1|1.6|33944.2|34039|205974.8|1.2|20.4|678875.4|680772.6|92689|1|9.6|305494.4|306347.6|13942545.2|3772
9|4|1|29471.6|1|1.6|183150.6|182799.8|589418.2|2.2|19.6|3663000.2|3655989.8|265238.4|1.4|9.2|1648350.2|1645195.4|37691082.4|10197.8
9|5|1|33479.2|1|1.6|359511.4|359253.8|669573.2|2.8|18.8|7190215.6|7185064.4|301308.2|1.8|8.8|3235597.2|3233279.4|43580664.2|11791.6
10|0|1|29528.8|1|1.4|224524.8|217105.2|590569|2.2|19.4|4490487.6|4342088.8|295284.6|1.4|10.2|2245244|2171044.6|40201652.8|10877
10|1|1|3625|1|1.8|2593.2|2723.8|72490.4|1|20.8|51855.2|54467|36245.6|1|10.8|25928|27233.6|6558488.8|1774.4
10|2|1|1263.8|1|2|367.4|486.4|25266|1|21|7343.4|9720|12633.4|1|11|3672|4860.2|3494764|945.4
10|3|1|1730.2|1|2|341.8|443.8|34597|1|21|6827|8867.8|17298.6|1|11|3414|4434.2|4173412.8|1129.2
10|4|1|9226.8|1|1.6|23592.6|23703.2|184526.8|1.2|20.4|471843.6|474057|92263.6|1|10.6|235922.2|237028.8|13571182.2|3671.6
10|5|1|18231.2|1|1.8|97647.8|97757.4|364618.6|1.8|20|1952947.2|1955131|182309.6|1.4|10.4|976473.6|977565.6|25275170.2|6838.2