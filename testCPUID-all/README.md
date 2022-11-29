This is a simple test to get a rough idea how the following instructions behave -

`tpause, umonitor/umwait` on Intel

`monitorx/mwaitx` on AMD

I also have `inc_only` and `inc_with_pause` (but they are commented out currently) to see the latency of the `pause` instruction as that's what we are using currently. `pause` gives no control how long you want to pause for. If you uncomment it you can specify the number of iterations to run with the '-ti' commandline arg. For example on my i9 machines with HT -

```
c:\tools>testCPUID.exe -ti 50000000
[       inc_only] 50000000 iterations took      64,483,172 cycles(    1/iter), sum is          2,499,999,950,000,000
[ inc_with_pause] 50000000 iterations took   5,285,730,269 cycles(  105/iter), sum is          2,499,999,950,000,000
[       inc_only] 50000000 iterations took      51,975,608 cycles(    1/iter), sum is          2,499,999,950,000,000
[ inc_with_pause] 50000000 iterations took   5,250,022,361 cycles(  105/iter), sum is          2,499,999,950,000,000

```

For the other instructions, use the '-proc' commandline arg to specify which instruction you want to test -

0 - pause

1 - tpause (only supported on (new enough) Intel machines)

2 - umonitor/umwait (only supported on (new enough) Intel machines)

3 - monitorx/mwaitx (only supported on AMD machines)

for 1/2/3 you can specify a timeout value with the '-timeout' commandline arg. Example -

```
C:\tools>testCPUID.exe -proc 4 -timeout 80000000
thread 17344 created
original value is 10, timeout is 80000000
end...
[    mwaitx] changed to 5 and waited 703 times!
other thread exited
umode cpu 10015ms, kmode cpu 0ms, elapsed time 10014ms
15,988,972,192 cycles total, 22743914/iteration

C:\tools>testCPUID.exe -proc 0
thread 29232 created
eflags are 582, IF bit is enabled
original value is 10
end...
other thread exited
umode cpu 10015ms, kmode cpu 0ms, elapsed time 10006ms
15,975,658,032 cycles total, 35/iteration
```

It also prints out some time/cycles info from the Win32 `GetThreadTimes` and `GetThreadCycleTime` APIs.