using CommandLine;
using DataModel;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Text;

public class Options
{
    [Option('p', "path", Required = true, HelpText = "Path to Prime Number Trend.")]
    public string Path { get; set; }

    [Option('i', "maxInputSize", Required = true, HelpText = "Max input size.")]
    public int MaxInputSize { get; set; }

    [Option('c', "maxComplexity", Required = true, HelpText = "Max Complexity.")]
    public int MaxComplexity { get; set; }

    [Option('t', "maxThreads", Required = false, HelpText = "Max Number of Threads.")]
    public int? MaxThreads { get; set; }
}

public class PrimeNumbersTrend {

    private static ProcessStartInfo startInfo;

    public static void Main(String[] args) 
    {
        Parser.Default.ParseArguments<Options>(args)
            .WithParsed<Options>(o =>
            {
                bool useDefaultThreads = true;
                if (string.IsNullOrEmpty(o.Path) || !File.Exists(o.Path))
                {
                    Console.WriteLine($"{o.Path} not present.");
                    Environment.Exit(1);
                }

                if (!o.MaxThreads.HasValue)
                {
                    o.MaxThreads = 1;
                }
                else
                {
                    useDefaultThreads = false;
                }

                startInfo = new ProcessStartInfo() {
                    FileName = o.Path,
                    Arguments = "",
                    RedirectStandardOutput = true,
                    RedirectStandardError = true,
                    UseShellExecute = false,
                };

                StringBuilder results = new StringBuilder();
                results.Append("inputSize|complexity|thread");
                results.Append("| HT|Affinity|Input_count");
                results.Append("|join_type");
                results.Append("|Spin_count");
                results.Append("|Thread_count");
                results.Append("|Soft Wait : Total");
                results.Append("|Soft Wait: #/input");
                results.Append("|Soft Wait: Iterations/wait");
                results.Append("|Soft Wait: Spin time / wait");
                results.Append("|Soft Wait: wakeup latency / wait");
                results.Append("|Hard Wait : Total");
                results.Append("|Hard Wait: #/input");
                results.Append("|Hard Wait: Iterations/wait");
                results.Append("|Hard Wait: Spin time / wait");
                results.Append("|Hard Wait: wakeup latency / wait");
                results.Append("|Total cycles spinning ");
                results.Append("|Cycles spin per thread");
                results.Append("|Elapsed time");
                results.Append("|Elapsed cycles");
                results.AppendLine();
                int columns = results.ToString().Count(c => c == '|');
                for (int col = 0; col <= columns; col++) {
                    results.Append("--|");
                }
                results.AppendLine();

                int progressIteration = 0;
                int totalIterations = 5 * o.MaxInputSize * (o.MaxComplexity + 1);
                if (!useDefaultThreads) {
                    totalIterations *= o.MaxThreads.Value;
                }
                int percentComplete = 5;
                Console.Write("Progress: 0%");
                for (int inputSize = 1; inputSize <= o.MaxInputSize; inputSize++) {
                    for (int complex = 0; complex <= o.MaxComplexity; complex++) {
                        for (int thread = 1; thread <= o.MaxThreads; thread++) {

                            List<ResultItem> iterations = new();

                            // Retry 5 times and take average.
                            for (int iter = 0; iter < 5; iter++) {
                                ResultItem? result = RunAndGetResult(iter, inputSize, complex, useDefaultThreads ? 0 : thread);
                                if (result == null) continue;

                                iterations.Add(result);

                                progressIteration++;
                                int percent = (progressIteration * 100) / totalIterations;
                                if (percent == percentComplete) {
                                    Console.Write($" {percentComplete}%");
                                    percentComplete += 5;
                                }
                            }

                            results.Append($"{inputSize}|{complex}|{thread}");
                            results.Append($"|{ResultItem.ConstructAveragedResultItems(iterations).ToMarkDownRow()}");
                            results.AppendLine();
                        }

                    }
                }
                Console.WriteLine();
                Console.WriteLine("-------------------------");
                Console.WriteLine(results.ToString());
            });
    }


    private static ResultItem? RunAndGetResult(int iter, int inputSize, int complexity, int threads) {
        StringBuilder stdoutBuilder = new StringBuilder();
        StringBuilder stderrBuilder = new StringBuilder();
        ResultItem? result = null;

        startInfo.Arguments = $"--input_count {inputSize} --complexity {complexity} ";

        if (threads != 0) {
            startInfo.Arguments += $" --thread_count {threads}";
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
                    result = new ResultItem(parsedOutput);
                    return result;
                }
            }
        }

        Console.WriteLine("*** Exitcode " + process?.ExitCode);
        Console.WriteLine("[OUT]: ");
        Console.WriteLine(output);
        Console.WriteLine("[ERR]: ");
        Console.WriteLine(error);
        return result;
    }
} 