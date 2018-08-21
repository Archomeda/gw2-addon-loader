#include "ThreadMonitor.h"
#include "../log.h"
#include "../utils/encoding.h"

using namespace std;
using namespace std::filesystem;
using namespace loader::utils;

////
// Getting process + thread information taken from https://stackoverflow.com/a/22949726
////

#define STATUS_SUCCESS ((NTSTATUS)0x00000000)
#define STATUS_INFO_LENGTH_MISMATCH ((NTSTATUS)0xC0000004)
#define SYSTEM_EXTENDED_PROCESS_INFORMATION (SYSTEM_INFORMATION_CLASS)0x39

enum KWAIT_REASON
{
    Executive,
    FreePage,
    PageIn,
    PoolAllocation,
    DelayExecution,
    Suspended,
    UserRequest,
    WrExecutive,
    WrFreePage,
    WrPageIn,
    WrPoolAllocation,
    WrDelayExecution,
    WrSuspended,
    WrUserRequest,
    WrEventPair,
    WrQueue,
    WrLpcReceive,
    WrLpcReply,
    WrVirtualMemory,
    WrPageOut,
    WrRendezvous,
    Spare2,
    Spare3,
    Spare4,
    Spare5,
    Spare6,
    WrKernel,
    MaximumWaitReason
};

enum THREAD_STATE
{
    Running = 2,
    Waiting = 5,
};

#pragma pack(push, 8)

struct CLIENT_ID
{
    HANDLE UniqueProcess; // Process ID
    HANDLE UniqueThread;  // Thread ID
};

// http://www.geoffchappell.com/studies/windows/km/ntoskrnl/api/ex/sysinfo/thread.htm
// Size = 0x40 for Win32
// Size = 0x50 for Win64
struct SYSTEM_THREAD
{
    LARGE_INTEGER KernelTime;
    LARGE_INTEGER UserTime;
    LARGE_INTEGER CreateTime;
    ULONG WaitTime;
    PVOID StartAddress;
    CLIENT_ID ClientID; // process/thread ids
    LONG Priority;
    LONG BasePriority;
    ULONG ContextSwitches;
    THREAD_STATE ThreadState;
    KWAIT_REASON WaitReason;
};

struct SYSTEM_EXTENDED_THREAD {
    SYSTEM_THREAD ThreadInfo;
    PVOID StackBase;
    PVOID StackLimit;
    PVOID Win32StartAddress;
    PVOID TebBase;
    ULONG_PTR Reserved2;
    ULONG_PTR Reserved3;
    ULONG_PTR Reserved4;
};

struct VM_COUNTERS // virtual memory of process
{
    ULONG_PTR PeakVirtualSize;
    ULONG_PTR VirtualSize;
    ULONG PageFaultCount;
    ULONG_PTR PeakWorkingSetSize;
    ULONG_PTR WorkingSetSize;
    ULONG_PTR QuotaPeakPagedPoolUsage;
    ULONG_PTR QuotaPagedPoolUsage;
    ULONG_PTR QuotaPeakNonPagedPoolUsage;
    ULONG_PTR QuotaNonPagedPoolUsage;
    ULONG_PTR PagefileUsage;
    ULONG_PTR PeakPagefileUsage;
};

// http://www.geoffchappell.com/studies/windows/km/ntoskrnl/api/ex/sysinfo/process.htm
// See also SYSTEM_PROCESS_INFORMATION in Winternl.h
// Size = 0x00B8 for Win32
// Size = 0x0100 for Win64
struct SYSTEM_PROCESS
{
    ULONG NextEntryOffset; // relative offset
    ULONG ThreadCount;
    LARGE_INTEGER  WorkingSetPrivateSize;
    ULONG HardFaultCount;
    ULONG NumberOfThreadsHighWatermark;
    ULONGLONG CycleTime;
    LARGE_INTEGER CreateTime;
    LARGE_INTEGER UserTime;
    LARGE_INTEGER KernelTime;
    UNICODE_STRING ImageName;
    LONG BasePriority;
    PVOID UniqueProcessId;
    PVOID InheritedFromUniqueProcessId;
    ULONG HandleCount;
    ULONG SessionId;
    ULONG_PTR UniqueProcessKey;
    VM_COUNTERS VmCounters;
    ULONG_PTR PrivatePageCount;
    IO_COUNTERS IoCounters;
    SYSTEM_EXTENDED_THREAD Threads[1];
};

#pragma pack(pop)

namespace loader::diagnostics {

    void ThreadMonitor::Start() {
        if (this->active) {
            return;
        }

        DIAG_LOG()->info("Starting ThreadMonitor loop");

        this->active = true;
        this->NtQueryInfo = reinterpret_cast<NtQuerySystemInformation_t*>(GetProcAddress(GetModuleHandleA("Ntdll.dll"), "NtQuerySystemInformation"));
        if (this->NtQueryInfo == NULL) {
            DIAG_LOG()->info("Could not find NtQuerySystemInformation export in Ntdll.dll: {0}", GetLastError());
            this->active = false;
            return;
        }

        // Get CPU count
        int cpuCount = 0;
        SYSTEM_LOGICAL_PROCESSOR_INFORMATION* cpuInformation = NULL;
        DWORD bufferLength = 0;
        while (true) {
            if (!GetLogicalProcessorInformation(cpuInformation, &bufferLength)) {
                if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
                    if (cpuInformation) {
                        LocalFree(cpuInformation);
                    }
                    cpuInformation = static_cast<SYSTEM_LOGICAL_PROCESSOR_INFORMATION*>(LocalAlloc(LMEM_FIXED, bufferLength));
                    if (!cpuInformation) {
                        break;
                    }
                }
                else {
                    break;
                }
            }
            else {
                break;
            }
        }
        DWORD offset = 0;
        while (offset + sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION) <= bufferLength) {
            if (cpuInformation->Relationship == LOGICAL_PROCESSOR_RELATIONSHIP::RelationProcessorCore) {
                auto mask = cpuInformation->ProcessorMask;
                int logicalCores = 0;
                while (mask) {
                    logicalCores += mask & 1;
                    mask >>= 1;
                }
                cpuCount += logicalCores;
            }

            ++cpuInformation;
            offset += sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);
        }
        if (cpuCount > 0) {
            this->cpus = cpuCount;
        }

        this->loopThread = thread(&ThreadMonitor::Loop, this);
    }

    void ThreadMonitor::Stop() {
        if (!this->active) {
            return;
        }

        DIAG_LOG()->info("Stopping ThreadMonitor loop");

        this->active = false;
        this->loopThread.join();
    }

    vector<shared_ptr<ThreadInfo>> ThreadMonitor::GetThreadInfos() {
        vector<shared_ptr<ThreadInfo>> v;
        transform(this->threads.begin(), this->threads.end(), back_inserter(v), [](auto &kv) { return kv.second; });
        sort(v.begin(), v.end(), [](const auto& a, const auto& b) { 
            if (a->processId != b->processId) {
                return a->processId < b->processId;
            }
            return a->order < b->order;
        });
        return v;
    }

    void ThreadMonitor::Loop() {
        SetThreadDescription(GetCurrentThread(), L"[LOADER] ThreadMonitor");
        SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_BELOW_NORMAL);

        chrono::high_resolution_clock::time_point deltaTimePoint;

        while (this->active) {
            // Get process information
            while (true) {
                if (!this->queryData) {
                    this->queryData = static_cast<BYTE*>(LocalAlloc(LMEM_FIXED, queryDataSize));
                    if (!this->queryData) {
                        break;
                    }
                }

                ULONG bytesNeeded = 0;
                NTSTATUS status = this->NtQueryInfo(SYSTEM_EXTENDED_PROCESS_INFORMATION, this->queryData, queryDataSize, &bytesNeeded);
                if (status == STATUS_INFO_LENGTH_MISMATCH) {
                    this->queryDataSize = bytesNeeded + 4000;
                    LocalFree(this->queryData);
                    this->queryData = NULL;
                    continue;
                }
                break;
            }

            int64_t deltaTime = chrono::duration_cast<chrono::nanoseconds>(chrono::high_resolution_clock::now() - deltaTimePoint).count();
            deltaTimePoint = chrono::high_resolution_clock::now();

            // Get our process
            DWORD pid = GetCurrentProcessId();
            SYSTEM_PROCESS* process = reinterpret_cast<SYSTEM_PROCESS*>(this->queryData);
            while (true) {
                if (static_cast<DWORD>(reinterpret_cast<DWORD_PTR>(process->UniqueProcessId)) == pid) {
                    break;
                }
                if (!process->NextEntryOffset) {
                    process = NULL;
                    break;
                }
                process = reinterpret_cast<SYSTEM_PROCESS*>(reinterpret_cast<BYTE*>(process) + process->NextEntryOffset);
            }

            if (process == NULL) {
                // Somehow we didn't get the process
                break;
            }

            // Get all child processes
            map<DWORD, SYSTEM_PROCESS*> processes;
            processes[pid] = process;
            size_t oldSize = 0;
            while (oldSize != processes.size()) {
                oldSize = processes.size();
                process = reinterpret_cast<SYSTEM_PROCESS*>(this->queryData);
                while (true) {
                    if (processes.find(static_cast<DWORD>(reinterpret_cast<DWORD_PTR>(process->InheritedFromUniqueProcessId))) != processes.end()) {
                        processes[static_cast<DWORD>(reinterpret_cast<DWORD_PTR>(process->UniqueProcessId))] = process;
                    }
                    if (!process->NextEntryOffset) {
                        process = NULL;
                        break;
                    }
                    process = reinterpret_cast<SYSTEM_PROCESS*>(reinterpret_cast<BYTE*>(process) + process->NextEntryOffset);
                }
            }

            // Get all threads of all associated processes
            set<DWORD> activeThreads;
            for (const auto& process : processes) {
                const auto processedThreads = this->CacheThreadsFromProcess(process.second, deltaTime);
                activeThreads.insert(processedThreads.begin(), processedThreads.end());
            }

            // Remove old threads
            auto it = this->threads.begin();
            while (it != this->threads.end()) {
                if (activeThreads.find(it->first) == activeThreads.end()) {
                    it = this->threads.erase(it);
                }
                else {
                    ++it;
                }
            }

            this_thread::sleep_for(chrono::seconds(1));
        }
    }

    set<DWORD> ThreadMonitor::CacheThreadsFromProcess(PVOID process, int64_t deltaNanoTime) {
        SYSTEM_PROCESS* systemProcess = reinterpret_cast<SYSTEM_PROCESS*>(process);
        set<DWORD> threadIds;

        bool hasSymbols = false;
        HANDLE processHandle = NULL;
        ProcessPriority processPriority = static_cast<ProcessPriority>(systemProcess->BasePriority);
       
        // The first SYSTEM_THREAD structure comes immediately after the SYSTEM_PROCESS structure
        SYSTEM_EXTENDED_THREAD* thread = systemProcess->Threads;
        for (DWORD i = 0; i < systemProcess->ThreadCount; ++i) {
            DWORD threadId = static_cast<DWORD>(reinterpret_cast<DWORD_PTR>(thread->ThreadInfo.ClientID.UniqueThread));

            shared_ptr<ThreadInfo> threadInfo;
            if (this->threads.find(threadId) != this->threads.end()) {
                threadInfo = this->threads[threadId];
            }
            else {
                threadInfo = make_shared<ThreadInfo>();
                threadInfo->processId = static_cast<DWORD>(reinterpret_cast<DWORD_PTR>(thread->ThreadInfo.ClientID.UniqueProcess));
                threadInfo->threadId = threadId;
                threadInfo->processName = u8(wstring(systemProcess->ImageName.Buffer, systemProcess->ImageName.Length / sizeof(WCHAR)));

                if (processHandle == NULL) {
                    processHandle = OpenProcess(PROCESS_QUERY_INFORMATION, false, threadInfo->processId);
                    hasSymbols = processHandle != NULL && SymInitialize(processHandle, NULL, true);
                }

                HMODULE hModule = NULL;
                if (GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, reinterpret_cast<LPCTSTR>(thread->Win32StartAddress), &hModule)) {
                    wchar_t buff[MAX_PATH];
                    if (GetModuleFileName(hModule, buff, MAX_PATH)) {
                        DWORD_PTR address = reinterpret_cast<DWORD_PTR>(thread->Win32StartAddress);
                        DWORD relAddress = static_cast<DWORD>(address - reinterpret_cast<DWORD_PTR>(hModule));

                        path modulePath(buff);
                        stringstream ss;
                        ss << modulePath.filename().u8string();

                        if (hasSymbols) {
                            DWORD64 displacement = 0;
                            char buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)];
                            SYMBOL_INFO* symbol = reinterpret_cast<SYMBOL_INFO*>(buffer);
                            symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
                            symbol->MaxNameLen = MAX_SYM_NAME;
                            if (SymFromAddr(processHandle, address, &displacement, symbol)) {
                                ss << "!" << string(symbol->Name, symbol->NameLen);
                                relAddress = static_cast<DWORD>(address - symbol->Address);
                            }
                        }
                        
                        if (relAddress > 0) {
                            ss << "+0x" << setfill('0') << setw(8) << hex << relAddress;
                        }
                        threadInfo->threadName = ss.str();
                    }
                }

                HANDLE threadHandle = OpenThread(THREAD_QUERY_INFORMATION, false, threadInfo->threadId);
                if (threadHandle != NULL) {
                    PWSTR description;
                    if (SUCCEEDED(GetThreadDescription(threadHandle, &description))) {
                        threadInfo->threadDescription = u8(description);
                        LocalFree(description);
                    }
                    CloseHandle(threadHandle);
                }
           
                this->threads[threadInfo->threadId] = threadInfo;
            }

            threadInfo->processPriority = processPriority;
            threadInfo->basePriority = ThreadInfo::DetermineThreadPriority(static_cast<ProcessPriority>(systemProcess->BasePriority), thread->ThreadInfo.BasePriority);
            threadInfo->priority = ThreadInfo::DetermineThreadPriority(static_cast<ProcessPriority>(systemProcess->BasePriority), thread->ThreadInfo.Priority);
            threadInfo->order = i;

            if (threadInfo->prevKernelTime > 0) {
                threadInfo->kernelTimePercentage = (thread->ThreadInfo.KernelTime.QuadPart - threadInfo->prevKernelTime) * 100.0 / deltaNanoTime * 100 / this->cpus;
            }
            threadInfo->prevKernelTime = thread->ThreadInfo.KernelTime.QuadPart;
            if (threadInfo->prevUserTime > 0) {
                threadInfo->userTimePercentage = (thread->ThreadInfo.UserTime.QuadPart - threadInfo->prevUserTime) * 100.0 / deltaNanoTime * 100 / this->cpus;
            }
            threadInfo->prevUserTime = thread->ThreadInfo.UserTime.QuadPart;

            threadIds.insert(threadInfo->threadId);
            ++thread;
        }

        if (hasSymbols) {
            SymCleanup(processHandle);
        }
        if (processHandle != NULL) {
            CloseHandle(processHandle);
        }

        return threadIds;
    }

}
