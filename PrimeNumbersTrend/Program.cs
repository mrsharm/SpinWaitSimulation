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
        int maxThreads = 1;
        bool useDefaultThreads = true;
        if (args.Length > 3) {
            int inputThread = Int32.Parse(args[3]);
            // Ensure we process only valid thread count
            if (inputThread > 1) {
                maxThreads = inputThread;
                useDefaultThreads = false;
            }
        }

        startInfo = new ProcessStartInfo() {
            FileName = primeNumbersExe,
            Arguments = "",
            RedirectStandardOutput = true,
            RedirectStandardError = true,
            UseShellExecute = false,
        };

        StringBuilder results = new StringBuilder();
        results.Append("inputSize|complex|thread");
        results.Append("|iters_per_number");
        results.Append("|hardwait_per_number");
        results.Append("|softwait_per_number");
        results.Append("|hardWaitWakeup_per_number");
        results.Append("|softWaitWakeup_per_number");
        results.Append("|iters_number_allthreads");
        results.Append("|hardwait_number_allthreads");
        results.Append("|softwait_number_allthreads");
        results.Append("|hardWaitWakeup_number_allthreads");
        results.Append("|softWaitWakeup_number_allthreads");
        results.Append("|iters_thread_allnumbers");
        results.Append("|hardwait_thread_allnumbers");
        results.Append("|softwait_thread_allnumbers");
        results.Append("|hardWaitWakeup_thread_allnumbers");
        results.Append("|softWaitWakeup_thread_allnumbers");
        results.Append("|ticks");
        results.Append("|totalTime");
        results.AppendLine();
        int columns = results.ToString().Count(c => c == '|');
        for (int col = 0; col <= columns; col++) {
            results.Append("--|");
        }
        results.AppendLine();

        int progressIteration = 0;
        int totalIterations = 5 * maxInputSize * (maxComplexity+1);
        if (!useDefaultThreads) {
            totalIterations *= maxThreads;
        }
        int percentComplete = 5;
        Console.Write("Progress: 0%");
        for (int inputSize = 1; inputSize <= maxInputSize; inputSize++) {
            for (int complex = 0; complex <= maxComplexity; complex++) {
                for (int thread = 1; thread <= maxThreads; thread++) {
                    List<long> iters_pernumber = new();
                    List<long> hardwait_pernumber = new();
                    List<long> softwait_pernumber = new();
                    List<long> hardWaitWakeup_pernumber = new();
                    List<long> softWaitWakeup_pernumber = new();
                    List<long> iters_number = new();
                    List<long> hardwait_number = new();
                    List<long> softwait_number = new();
                    List<long> hardWaitWakeup_number = new();
                    List<long> softWaitWakeup_number = new();
                    List<long> iters_thread = new();
                    List<long> hardwait_thread = new();
                    List<long> softwait_thread = new();
                    List<long> hardWaitWakeup_thread = new();
                    List<long> softWaitWakeup_thread = new();
                    List<long> ticks = new();
                    List<long> totalTime = new();

                    // Retry 5 times and take average.
                    for (int iter = 0; iter < 5; iter++) {
                        var result = RunAndGetResult(iter, inputSize, complex, useDefaultThreads ? 0 : thread);
                        if (IsEmpty(result)) continue;

                        int col = 3;
                        iters_pernumber.Add(result[col++]);
                        hardwait_pernumber.Add(result[col++]);
                        softwait_pernumber.Add(result[col++]);
                        hardWaitWakeup_pernumber.Add(result[col++]);
                        softWaitWakeup_pernumber.Add(result[col++]);
                        iters_number.Add(result[col++]);
                        hardwait_number.Add(result[col++]);
                        softwait_number.Add(result[col++]);
                        hardWaitWakeup_number.Add(result[col++]);
                        softWaitWakeup_number.Add(result[col++]);
                        iters_thread.Add(result[col++]);
                        hardwait_thread.Add(result[col++]);
                        softwait_thread.Add(result[col++]);
                        hardWaitWakeup_thread.Add(result[col++]);
                        softWaitWakeup_thread.Add(result[col++]);
                        ticks.Add(result[col++]);
                        totalTime.Add(result[col]);
                        Debug.Assert(col == columns);

                        progressIteration++;
                        int percent = (progressIteration * 100) / totalIterations;
                        if (percent == percentComplete) {
                            Console.Write($" {percentComplete}%");
                            percentComplete += 5;
                        }
                    }

                    results.Append($"{inputSize}|{complex}|{thread}");
                    results.Append($"|{iters_pernumber.Average()}");
                    results.Append($"|{hardwait_pernumber.Average()}");
                    results.Append($"|{softwait_pernumber.Average()}");
                    results.Append($"|{hardWaitWakeup_pernumber.Average()}");
                    results.Append($"|{softWaitWakeup_pernumber.Average()}");
                    results.Append($"|{iters_number.Average()}");
                    results.Append($"|{hardwait_number.Average()}");
                    results.Append($"|{softwait_number.Average()}");
                    results.Append($"|{hardWaitWakeup_number.Average()}");
                    results.Append($"|{softWaitWakeup_number.Average()}");
                    results.Append($"|{iters_thread.Average()}");
                    results.Append($"|{hardwait_thread.Average()}");
                    results.Append($"|{softwait_thread.Average()}");
                    results.Append($"|{hardWaitWakeup_thread.Average()}");
                    results.Append($"|{softWaitWakeup_thread.Average()}");
                    results.Append($"|{ticks.Average()}");
                    results.Append($"|{totalTime.Average()}");
                    results.AppendLine();
                }

            }
        }
        Console.WriteLine();
        Console.WriteLine("-------------------------");
        Console.WriteLine(results.ToString());
    }


    private static List<long> RunAndGetResult(int iter, int inputSize, int complexity, int threads) {
        List<long> result = null;
        StringBuilder stdoutBuilder = new StringBuilder();
        StringBuilder stderrBuilder = new StringBuilder();

        startInfo.Arguments = $"{inputSize} {complexity} ";

        if (threads != 0) {
            startInfo.Arguments += $" {threads}";
        }

        string output = "", error = "";
        Process process = new Process() {
            StartInfo = startInfo
        };

        process.OutputDataReceived += (sender, args) => {
            if (args.Data != null) {
                string output = args.Data.ToString();
                if (!string.IsNullOrEmpty(output)) {
                    stdoutBuilder.AppendLine(output);
                }
                //Console.WriteLine(output);
            }
        };
        process.ErrorDataReceived += (sender, args) => {
            if (args.Data != null) {
                string error = args.Data?.ToString();
                if (!string.IsNullOrEmpty(error)) {
                    stdoutBuilder.AppendLine(error);
                }
                Console.WriteLine(error);
            }
        };

        process.Start();
        process.BeginOutputReadLine();
        process.BeginErrorReadLine();

        if (process != null) {
            process.WaitForExit();

            if (process.ExitCode == 0) {
                output = stdoutBuilder.ToString();
                error = stderrBuilder.ToString();

                string[] outputLines = output.Split(Environment.NewLine, StringSplitOptions.TrimEntries | StringSplitOptions.RemoveEmptyEntries);
                foreach (var outputLine in outputLines) {
                    if (!outputLine.StartsWith("OUT] ")) {
                        continue;
                    }
                    string resultOutput = outputLine.Replace("OUT] ", string.Empty);
                    string[] parsedOutput = resultOutput.Split('|', StringSplitOptions.TrimEntries | StringSplitOptions.RemoveEmptyEntries);
                    result = parsedOutput.Select(x => long.Parse(x)).ToList();
                }
            }
            else {
                Console.WriteLine("*** Exitcode " + process.ExitCode);
            }
        }

        if (IsEmpty(result)) {
            Console.WriteLine("[OUT]: ");
            Console.WriteLine(output);
            Console.WriteLine("[ERR]: ");
            Console.WriteLine(error);
        }
        return result;
    }

    private static bool IsEmpty(List<long> result) {
        return ((result == null) || (result.Count == 0));
    }
}
