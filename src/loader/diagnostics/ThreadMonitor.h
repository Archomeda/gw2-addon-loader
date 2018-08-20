#pragma once
#include "../stdafx.h"
#include <winternl.h>
#include "ThreadInfo.h"

namespace loader::diagnostics {

    typedef NTSTATUS(WINAPI NtQuerySystemInformation_t)(SYSTEM_INFORMATION_CLASS, PVOID, ULONG, PULONG);

    class ThreadMonitor {
    public:
        ThreadMonitor(ThreadMonitor const&) = delete;
        void operator=(ThreadMonitor const&) = delete;

        static ThreadMonitor& GetInstance() {
            static ThreadMonitor instance;
            return instance;
        }

        void Start();
        void Stop();

        std::vector<std::shared_ptr<ThreadInfo>> GetThreadInfos();

    private:
        ThreadMonitor() { };

        void Loop();
        std::set<DWORD> CacheThreadsFromProcess(PVOID process, int64_t deltaNanoTime);

        NtQuerySystemInformation_t* NtQueryInfo;
        BYTE* queryData;
        DWORD queryDataSize = 1000;

        std::map<DWORD, std::shared_ptr<ThreadInfo>> threads;
        int cpus = 1;

        bool active = false;
        std::thread loopThread;
    };

}
