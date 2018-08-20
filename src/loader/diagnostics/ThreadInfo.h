#pragma once
#include "../stdafx.h"

namespace loader::diagnostics {

    enum ProcessPriority {
        ProcessIdle = 4,
        ProcessBelowNormal = 6,
        ProcessNormal = 8,
        ProcessAboveNormal = 10,
        ProcessHigh = 13,
        ProcessRealtime = 24
    };

    enum ThreadPriority {
        ThreadIdle = -7,
        ThreadLowest,
        ThreadBelowNormal,
        ThreadNormal,
        ThreadAboveNormal,
        ThreadHighest,
        ThreadTimeCritical
    };

    const std::string ProcessPriorityToString(ProcessPriority priority);
    const std::string ThreadPriorityToString(ThreadPriority priority);

    class ThreadInfo {
    public:
        static ThreadPriority DetermineThreadPriority(ProcessPriority processPriority, DWORD basePriority);

        DWORD processId;
        DWORD threadId;
        ProcessPriority processPriority;
        ThreadPriority basePriority;
        ThreadPriority priority;
        std::string processName;
        std::string threadName;
        std::string threadDescription;
        int64_t prevKernelTime;
        int64_t prevUserTime;
        double kernelTimePercentage;
        double userTimePercentage;
        int order;

        const std::string GetProcessPriorityString() const { return ProcessPriorityToString(this->processPriority); }
        const std::string GetBasePriorityString() const { return ThreadPriorityToString(this->basePriority); }
        const std::string GetPriorityString() const { return ThreadPriorityToString(this->priority); }
    };

}
