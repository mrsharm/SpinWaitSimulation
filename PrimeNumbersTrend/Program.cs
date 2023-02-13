using CommandLine;
using DataModel;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Security.Cryptography;
using System.Text;
using System.Threading;

public class Options
{
    [Option('p', "pathToPrimeNumberTrend", Required = true, HelpText = "Path to Prime Number Trend.")]
    public string PathToPrimeNumberTrend { get; set; }

    [Option('i', "maxInputSizeRange", Required = false, HelpText = "Range of Max input size in the form of start-stop-increment e.g., 100-1000-5")]
    public string MaxInputSizeRange { get; set; } = "1000-100000-1000";

    [Option('c', "maxComplexityRange", Required = false, HelpText = "Range of Max Complexity in the form of start-stop-increment e.g., 15-25-2")]
    public string MaxComplexityRange { get; set; } = "15-25-2";

    [Option('t', "maxThreads", Required = false, HelpText = "Range of Max Number of Threads e.g., 2-20-1")]
    public string MaxThreadsRange { get; set; } = $"2-{Environment.ProcessorCount}-2";

    [Option('j', "joinType", Required = false, HelpText = "The Join Type as a Comma Separated List e.g., 1,2,4")]
    public string JoinTypeValues { get; set; } = "1,5";

    [Option('h', "ht", Required = false, HelpText = "Specification if hyperthread is on or off. This must match the machine configuration.")]
    public int HT { get; set; } = 0; 

    [Option('a', "affinitizeRange", Required = false, HelpText = "Range of Affinitized value in the form of start-stop-increment e.g., 0-2-1")]
    public string AffinitizeRange { get; set; } = "0-2-2";

    [Option('m', "mwaitTimeoutRange", Required = false, HelpText = "Range of The Timeout for mwait in the form of start-stop-increment only applicable for join type 5 e.g., 1000-3000-500.")]
    // TODO: Fix make the range variable.: [Maoni] I would do 1000 (I think the OS uses 1024 right?) to 40,000. 40,000 would be comparable to a context switch cost I believe. And I would increase with smaller steps when the values are small and bigger steps whe the values are big, so from 1000 to 5000 I would do 200 increments and 5000 to 10,000 500 increments and etc.
    public string MWaitTimeoutRange { get; set; } = "1000-40000-200";

    [Option('s', "spinCount", Required = false, HelpText = "Spin Count only applicable and used in the case where join_type == 1 e.g., 1000-3000-500.")]
    public string SpinCountRange { get; set; } = "64000-128000-64000";

    [Option('o', "outputPath", Required = false, HelpText = "Output path of the markdown.")]
    public string? OutputPath { get; set; }
}

public class PrimeNumbersTrend 
{
    // Range format is start-stop-increment
    private struct RangeValue
    {
        public static RangeValue Create(string line)
        {
            if (string.IsNullOrEmpty(line))
            {
                throw new ArgumentException("Range Values are empty!"); 
            }

            string[] splits = line.Split("-", StringSplitOptions.TrimEntries);
            if (splits.Length != 3 ) 
            {
                throw new ArgumentException("Range Values have the format of start-stop-increment. Please ensure that the input is in the following format.");
            }

            int start = int.Parse(splits[0]);
            int stop = int.Parse(splits[1]);
            int increment = int.Parse(splits[2]);

            List<int> range = new();

            for (int i = start; i <= stop; i += increment)
            {
                range.Add(i);
            }

            RangeValue value = new()
            {
                Start     = start,
                Stop      = stop,
                Increment = increment,
                Range     = range
            };

            return value;
        }

        public int Start { get; init; }
        public int Stop { get; init;  }
        public int Increment { get; init; }
        public IReadOnlyList<int> Range { get; init; }
    }

    private struct PrimeNumberInput
    {
        public PrimeNumberInput(int iteration, int input, int complexity, int thread, int affinitize, int joinType, int ht, int? spinCount, int? timeout)
        {
            Input      = input;
            Complexity = complexity;
            Thread     = thread;
            Affinitize = affinitize;
            Timeout    = timeout;
            JoinType   = joinType;
            Iteration  = iteration;
            SpinCount = spinCount;
            Ht         = ht;
        }

        public int Input      { get; }
        public int Complexity { get; }
        public int Thread     { get; }
        public int Affinitize { get; }
        public int JoinType   { get; }
        public int Iteration  { get; }
        public int? SpinCount { get; }
        public int? Timeout    { get; }
        public int Ht { get; }

        public string CommandLine
        {
            get
            {
                string commandLine = $"--input_count {Input} --complexity {Complexity} --thread_count {Thread} --affi {Affinitize} --join_type {JoinType} --ht {Ht}";

                if (SpinCount.HasValue)
                {
                    commandLine += $" --spin_count {SpinCount.Value}";
                }

                else
                {
                    commandLine += $" --mwaitx_cycle_count {Timeout.Value}";
                }

                return commandLine;
            }
        }
    }

    private static ProcessStartInfo startInfo;

    public static void Main(String[] args) 
    {
        Parser.Default.ParseArguments<Options>(args)
            .WithParsed<Options>(o =>
            {
                if (string.IsNullOrEmpty(o.PathToPrimeNumberTrend) || !File.Exists(o.PathToPrimeNumberTrend))
                {
                    Console.WriteLine($"{o.PathToPrimeNumberTrend} not present.");
                    Environment.Exit(1);
                }

                RangeValue inputs     = RangeValue.Create(o.MaxInputSizeRange);
                RangeValue complexity = RangeValue.Create(o.MaxComplexityRange);
                RangeValue threads    = RangeValue.Create(o.MaxThreadsRange);
                RangeValue affinitize = RangeValue.Create(o.AffinitizeRange);
                RangeValue spinCountRange = RangeValue.Create(o.SpinCountRange);
                RangeValue mwaitCountRange = RangeValue.Create(o.MWaitTimeoutRange);

                List<int> joinType    = o.JoinTypeValues.Split(",", StringSplitOptions.TrimEntries).Select(j => int.Parse(j)).ToList();

                startInfo = new ProcessStartInfo() {
                    FileName = o.PathToPrimeNumberTrend,
                    Arguments = "",
                    RedirectStandardOutput = true,
                    RedirectStandardError = true,
                    UseShellExecute = false,
                };

                StringBuilder results = new StringBuilder();
                results.Append("|HT|Affinity|Input_count");
                results.Append("|Complexity");
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
                results.Append("|MWaitx Cycles");
                results.AppendLine();
                int columns = results.ToString().Count(c => c == '|');
                for (int col = 0; col <= columns; col++) 
                {
                    results.Append("--|");
                }
                results.AppendLine();

                int progressIteration = 0;
                int totalIterations = 5 * inputs.Range.Count
                                        * complexity.Range.Count
                                        * threads.Range.Count
                                        * affinitize.Range.Count
                                        * joinType.Count;

                int percentComplete = 5;
                Console.Write("Progress: 0%");

                foreach (var input in inputs.Range)
                {
                    foreach (var complex in complexity.Range)
                    {
                        foreach (var thread in threads.Range)
                        {
                            foreach (var affinity in affinitize.Range)
                            {
                                foreach (var join in joinType)
                                {
                                    List<ResultItem> iterations = new();
                                    if (join == 1)
                                    {
                                        totalIterations *= spinCountRange.Range.Count;
                                        foreach (var spinCount in spinCountRange.Range)
                                        {
                                            // Retry 5 times and take average.
                                            for (int iter = 0; iter < 5; iter++)
                                            {
                                                PrimeNumberInput commandInput = new PrimeNumberInput(iteration: iter, input: input, complexity: complex, thread: thread, affinitize: affinity, timeout: null, joinType: join, spinCount: spinCount, ht: o.HT);
                                                ResultItem? result = RunAndGetResult(commandInput);
                                                if (result == null)
                                                {
                                                    Console.WriteLine($"Failed with arguments: Input: {input}, Complexity: {complexity}, thread: {thread}, affinity: {affinity}, timeout: {spinCount}, joinType: {join}.");
                                                }

                                                iterations.Add(result);

                                                progressIteration++;
                                                int percent = (progressIteration * 100) / totalIterations;
                                                if (percent == percentComplete)
                                                {
                                                    Console.Write($" {percentComplete}%");
                                                    percentComplete += 5;
                                                }
                                            }

                                            results.Append($"|{ResultItem.ConstructAveragedResultItems(iterations).ToMarkDownRow()}");
                                            results.AppendLine();
                                        }
                                    }

                                    else
                                    {
                                        totalIterations *= mwaitCountRange.Range.Count;

                                        foreach (var mwaitCount in mwaitCountRange.Range)
                                        {
                                            // Retry 5 times and take average.
                                            for (int iter = 0; iter < 5; iter++)
                                            {
                                                PrimeNumberInput commandInput = new PrimeNumberInput(iteration: iter, input: input, complexity: complex, thread: thread, affinitize: affinity, timeout: mwaitCount, joinType: join, spinCount: null, ht: o.HT);
                                                ResultItem? result = RunAndGetResult(commandInput);
                                                if (result == null)
                                                {
                                                    Console.WriteLine($"Failed with arguments: Input: {input}, Complexity: {complexity}, thread: {thread}, affinity: {affinity}, joinType: {join}, timeout: {mwaitCount}.");
                                                }

                                                iterations.Add(result);

                                                progressIteration++;
                                                int percent = (progressIteration * 100) / totalIterations;
                                                if (percent == percentComplete)
                                                {
                                                    Console.Write($" {percentComplete}%");
                                                    percentComplete += 5;
                                                }
                                            }
                                        }
                                    }

                                    results.Append($"|{ResultItem.ConstructAveragedResultItems(iterations).ToMarkDownRow()}");
                                    results.AppendLine();
                                }
                            }
                        }
                    }
                }

                Console.WriteLine();
                Console.WriteLine("-------------------------");
                Console.WriteLine(results.ToString());

                if (!string.IsNullOrEmpty(o.OutputPath))
                {
                    File.WriteAllText(o.OutputPath, results.ToString());
                }
            });
    }

    private static ResultItem? RunAndGetResult(PrimeNumberInput input)
    {
        StringBuilder stdoutBuilder = new StringBuilder();
        StringBuilder stderrBuilder = new StringBuilder();
        ResultItem? result = null;

        startInfo.Arguments = input.CommandLine;

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