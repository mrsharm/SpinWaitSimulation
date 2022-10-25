using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Numerics;
using System.Text;
using System.Threading;
//using System.Linq;

public class PrimeNumbersTrend {

    private static ProcessStartInfo startInfo;

    public static void Main(String[] args) {
        if ((args.Length != 3) && (args.Length != 4)) {
            Console.WriteLine("Usage: PrimeNumbersTrend.exe <Path> <maxInputSize> <maxComplexity> <maxThreads>?");
            Console.WriteLine("<Path>: Full path to PrimeNumbers.exe");

            Console.WriteLine("<maxInputSize>: Max Input size.");
            Console.WriteLine("<maxComplexity>: Max Complexity.");
            Console.WriteLine("<maxThreads>: Max Threads.");
            Environment.Exit(1);
        }

        string primeNumbersExe = args[0];
        if (!File.Exists(primeNumbersExe)) {
            Console.WriteLine($"{primeNumbersExe} not present.");
            Environment.Exit(1);
        }

        int maxInputSize = Int32.Parse(args[1]);
        int maxComplexity = Int32.Parse(args[2]);
        int maxThreads = 0;
        if (args.Length > 3) {
            maxThreads = Int32.Parse(args[3]);
        }

        startInfo = new ProcessStartInfo() {
            FileName = primeNumbersExe,
            Arguments = "",
            RedirectStandardOutput = true,
            UseShellExecute = false,
        };

        StringBuilder results = new StringBuilder();
        results.Append("inputSize|complex|thread");
        results.Append("|iters_number");
        results.Append("|hardwait_number");
        results.Append("|softwait_number");
        results.Append("|wakeup_number");
        results.Append("|iters_thread");
        results.Append("|hardwait_thread");
        results.Append("|softwait_thread");
        results.Append("|wakeup_thread");
        results.Append("|ticks");
        results.Append("|totalTime");
        results.AppendLine();

        for (int inputSize = 1; inputSize <= maxInputSize; inputSize++) {
            for (int complex = 0; complex <= maxComplexity; complex++) {
                for (int thread = 0; thread <= maxThreads; thread++) {
                    
                    List<long> iters_number = new();
                    List<long> hardwait_number = new();
                    List<long> softwait_number = new();
                    List<long> wakeup_number = new();
                    List<long> iters_thread = new();
                    List<long> hardwait_thread = new();
                    List<long> softwait_thread = new();
                    List<long> wakeup_thread = new();
                    List<long> ticks = new();
                    List<long> totalTime = new();

                    // Retry 5 times and take average.
                    for (int iter = 0; iter < 5; iter++) {
                        var result = RunAndGetResult(inputSize, complex, thread);
                        iters_number.Add(result[0]);
                        hardwait_number.Add(result[1]);
                        softwait_number.Add(result[2]);
                        wakeup_number.Add(result[3]);
                        iters_thread.Add(result[4]);
                        hardwait_thread.Add(result[5]);
                        softwait_thread.Add(result[6]);
                        wakeup_thread.Add(result[7]);
                        ticks.Add(result[8]);
                        totalTime.Add(result[9]);
                    }

                    results.Append($"{inputSize}|{complex}|{thread}");
                    results.Append($"|{iters_number.Average()}");
                    results.Append($"|{hardwait_number.Average()}");
                    results.Append($"|{softwait_number.Average()}");
                    results.Append($"|{wakeup_number.Average()}");
                    results.Append($"|{iters_thread.Average()}");
                    results.Append($"|{hardwait_thread.Average()}");
                    results.Append($"|{softwait_thread.Average()}");
                    results.Append($"|{wakeup_thread.Average()}");
                    results.Append($"|{ticks.Average()}");
                    results.Append($"|{totalTime.Average()}");
                    results.AppendLine();
                }

            }
        }
        Console.WriteLine(results.ToString());
    }


    private static List<long> RunAndGetResult(int inputSize, int complexity, int threads) {
        List<long> result = null;
        startInfo.Arguments = $"{inputSize} {complexity} ";

        if (threads != 0) {
            startInfo.Arguments += $" {threads}";
        }

        Process? process = Process.Start(startInfo);

        if (process != null) {
            process.WaitForExit();

            if (process.ExitCode == 0) {
                var output = process.StandardOutput.ReadToEnd().Split('|', StringSplitOptions.TrimEntries | StringSplitOptions.RemoveEmptyEntries);
                result = output.Select(x => long.Parse(x)).ToList();
            }
        }

        return result;
    }
}
