using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;

namespace DataModel
{
    public sealed class ResultItem
    {
        // HT|affinity|5|3|t_join_pause|128000|5|16|     0.800|9170|2122538|911124|0|     0.000|0|0|0|33960602|6792120|3|11541562
        public ResultItem(string[] parsedOutput)
        {
            HT = parsedOutput[0];
            Affinity = parsedOutput[1];
            InputCount = Convert.ToInt32(parsedOutput[2]);
            Complexity = Convert.ToInt32(parsedOutput[3]);
            JoinType = parsedOutput[4];
            SpinCount = Convert.ToInt64(parsedOutput[5]);
            ThreadCount = Convert.ToInt64(parsedOutput[6]);
            SoftWaitTotal = Convert.ToInt64(parsedOutput[7]);
            SoftWaitNumPerInput = Convert.ToDouble(parsedOutput[8].Contains("nan") ? double.NaN : parsedOutput[8]);
            SoftWaitIterationsPerWait = Convert.ToDouble(parsedOutput[9]);
            SoftWaitSpinTimePerWait = Convert.ToDouble(parsedOutput[10]);
            SoftWaitWakeupLatencyPerWait = Convert.ToDouble(parsedOutput[11]);
            HardWaitTotal = Convert.ToInt64(parsedOutput[12]);
            HardWaitNumPerInput = Convert.ToDouble(parsedOutput[13].Contains("nan") ? double.NaN : parsedOutput[13]);
            HardWaitIterationsPerWait = Convert.ToDouble(parsedOutput[14]);
            HardWaitSpinTimePerWait = Convert.ToDouble(parsedOutput[15]);
            HardWaitWakeupLatencyPerWait = Convert.ToDouble(parsedOutput[16]);
            CyclesSpinPerThread = Convert.ToDouble(parsedOutput[17]);
            ElapsedTime = Convert.ToDouble(parsedOutput[18]);
            ElapsedCycles = Convert.ToDouble(parsedOutput[19]);
        }

        private ResultItem() { }
         
        public string HT { get; set; }
        public string Affinity { get; set; }
        public long InputCount { get; set; }
        public int Complexity { get; set; }
        public string JoinType { get; set; }
        public long SpinCount { get; set; }
        public long ThreadCount { get; set; }
        public long SoftWaitTotal { get; set; }
        public double SoftWaitNumPerInput { get; set; }
        public double SoftWaitIterationsPerWait { get; set; }
        public double SoftWaitSpinTimePerWait { get; set; }
        public double SoftWaitWakeupLatencyPerWait { get; set; }
        public long HardWaitTotal { get; set; }
        public double HardWaitNumPerInput { get; set; }
        public double HardWaitIterationsPerWait { get; set; }
        public double HardWaitSpinTimePerWait { get; set; }
        public double HardWaitWakeupLatencyPerWait { get; set; }
        public double CyclesSpinPerThread { get; set; }
        public double ElapsedTime { get; set; }
        public double ElapsedCycles { get; set; }

        public string ToMarkDownRow()
            => $"{HT}|{Affinity}|{InputCount}|{Complexity}|{JoinType}|{SpinCount}|{ThreadCount}|{SoftWaitTotal}|{SoftWaitNumPerInput}|{SoftWaitIterationsPerWait}|{SoftWaitSpinTimePerWait}|{SoftWaitWakeupLatencyPerWait}|{HardWaitTotal}|{HardWaitNumPerInput}|{HardWaitIterationsPerWait}|{HardWaitSpinTimePerWait}|{HardWaitWakeupLatencyPerWait}|{CyclesSpinPerThread}|{ElapsedTime}|{ElapsedCycles}";
        public string ToCSVRow()
            => $"{HT},{Affinity},{InputCount},{Complexity},{JoinType},{SpinCount},{ThreadCount},{SoftWaitTotal},{SoftWaitNumPerInput},{SoftWaitIterationsPerWait},{SoftWaitSpinTimePerWait},{SoftWaitWakeupLatencyPerWait},{HardWaitTotal},{HardWaitNumPerInput},{HardWaitIterationsPerWait},{HardWaitSpinTimePerWait},{HardWaitWakeupLatencyPerWait},{CyclesSpinPerThread},{ElapsedTime},{ElapsedCycles}";

        public static ResultItem ConstructAveragedResultItems(IReadOnlyList<ResultItem> others)
        {
            ResultItem result = new()
            {
                HT         = others[0].HT,
                Affinity   = others[0].Affinity,
                InputCount = others[0].InputCount,
            };

            result.SpinCount   = (long)others.Average(o => o.SpinCount);
            result.ThreadCount = (long)others.Average(o => o.ThreadCount);

            result.SoftWaitTotal = (long)others.Average(o => o.SoftWaitTotal);
            result.SoftWaitNumPerInput = others.Average(o => o.SoftWaitNumPerInput);
            result.SoftWaitIterationsPerWait = others.Average(o => o.SoftWaitIterationsPerWait);
            result.SoftWaitSpinTimePerWait = others.Average(o => o.SoftWaitSpinTimePerWait);
            result.SoftWaitWakeupLatencyPerWait = others.Average(o => o.SoftWaitWakeupLatencyPerWait);

            result.HardWaitTotal = (long)others.Average(o => o.HardWaitTotal);
            result.HardWaitNumPerInput = others.Average(o => o.HardWaitNumPerInput);
            result.HardWaitIterationsPerWait = others.Average(o => o.HardWaitIterationsPerWait);
            result.HardWaitSpinTimePerWait = others.Average(o => o.HardWaitSpinTimePerWait);
            result.HardWaitWakeupLatencyPerWait = others.Average(o => o.HardWaitWakeupLatencyPerWait);

            result.CyclesSpinPerThread = others.Average(o => o.CyclesSpinPerThread);
            result.ElapsedTime = others.Average(o => o.ElapsedTime);
            result.ElapsedCycles = others.Average(o => o.ElapsedCycles);

            return result;
        }
    }
}
