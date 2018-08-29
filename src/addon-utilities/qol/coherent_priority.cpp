#include "coherent_priority.h"
#include "../minhook.h"
#include "../utils/encoding.h"

using namespace std;
using namespace std::filesystem;
using namespace addon::utils;

namespace addon::qol {

#pragma region Loop varables
    std::thread loopThread;
    std::mutex loopMutex;
    std::condition_variable loopCv;
#pragma endregion

    atomic_bool isEnabled = false;
    CoherentPriorityLevel priorityLevel = CoherentPriorityLevel::CoherentPriorityNormal;
    BOOL(WINAPI *SystemCreateProcessA)(LPCSTR, LPSTR, LPSECURITY_ATTRIBUTES, LPSECURITY_ATTRIBUTES, BOOL, DWORD, LPVOID, LPCSTR, LPSTARTUPINFOA, LPPROCESS_INFORMATION);
    BOOL(WINAPI *SystemCreateProcessW)(LPCWSTR, LPWSTR, LPSECURITY_ATTRIBUTES, LPSECURITY_ATTRIBUTES, BOOL, DWORD, LPVOID, LPCWSTR, LPSTARTUPINFOW, LPPROCESS_INFORMATION);

    bool IsCoherent(const LPWSTR fileName) {
        return _wcsicmp(fileName, L"CoherentUI_Host.exe") == 0;
    }

    bool IsCoherentCommandLine(const wchar_t* commandLine) {
        int args;
        LPWSTR* argsList = CommandLineToArgvW(commandLine, &args);
        if (argsList == NULL || args == 0) {
            return false;
        }

        path filePath(argsList[0]);
        LocalFree(argsList);
        return filePath.filename() == "CoherentUI_Host.exe";
    }

    bool IsCoherentCommandLine(const char* commandLine) {
        wstring commandLineW = u16(commandLine);
        return IsCoherentCommandLine(commandLineW.c_str());
    }

    DWORD GetWinPriorityClass(CoherentPriorityLevel level) {
        switch (level) {
        case CoherentPriorityLevel::CoherentPriorityHigh:
            return HIGH_PRIORITY_CLASS;
        case CoherentPriorityLevel::CoherentPriorityAboveNormal:
            return ABOVE_NORMAL_PRIORITY_CLASS;
        case CoherentPriorityLevel::CoherentPriorityNormal:
        default:
            return NORMAL_PRIORITY_CLASS;
        }
    }


    void SetCoherentPriority(HANDLE hProcess, CoherentPriorityLevel level) {
        if (hProcess) {
            DWORD oldPriority = GetPriorityClass(hProcess);
            DWORD newPriority = GetWinPriorityClass(level);
            if (oldPriority != newPriority) {
                SetPriorityClass(hProcess, newPriority);
            }
        }
    }

    void SetCoherentPriority(CoherentPriorityLevel level) {
        priorityLevel = level;

        HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
        PROCESSENTRY32 process;
        process.dwSize = sizeof(PROCESSENTRY32);
        if (Process32First(hSnapshot, &process)) {
            do {
                // This also changes the process priority of other similarly named processes that might not be related to Guild Wars 2 or Coherent UI at all.
                //TODO: Update this to make sure that only GW2 related processes are included.
                if (IsCoherent(process.szExeFile)) {
                    HANDLE hProcess = OpenProcess(PROCESS_SET_INFORMATION, true, process.th32ProcessID);
                    SetCoherentPriority(hProcess, level);
                    CloseHandle(hProcess);
                }
            } while (Process32Next(hSnapshot, &process));
        }
    }


    void Loop() {
        SetThreadDescription(GetCurrentThread(), L"[ADDON] Utility: Coherent Priority Monitor");
        SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_BELOW_NORMAL);

        while (isEnabled) {
            SetCoherentPriority(priorityLevel);

            unique_lock<mutex> lock(loopMutex);
            loopCv.wait_for(lock, 1s, [=] { return !isEnabled; });
        }
    }


    BOOL WINAPI HookCreateProcessA(LPCSTR lpApplicationName, LPSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes, LPSECURITY_ATTRIBUTES lpThreadAttributes,
        BOOL bInheritHandles, DWORD dwCreationFlags, LPVOID lpEnvironment, LPCSTR lpCurrentDirectory, LPSTARTUPINFOA lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation) {
        BOOL result = SystemCreateProcessA(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation);
        if (lpCommandLine != NULL && IsCoherentCommandLine(lpCommandLine)) {
            SetCoherentPriority(lpProcessInformation->hProcess, priorityLevel);
        }
        return result;
    }

    BOOL WINAPI HookCreateProcessW(LPCWSTR lpApplicationName, LPWSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes, LPSECURITY_ATTRIBUTES lpThreadAttributes,
        BOOL bInheritHandles, DWORD dwCreationFlags, LPVOID lpEnvironment, LPCWSTR lpCurrentDirectory, LPSTARTUPINFOW lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation) {
        BOOL result = SystemCreateProcessW(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation);
        if (lpCommandLine != NULL && IsCoherentCommandLine(lpCommandLine)) {
            SetCoherentPriority(lpProcessInformation->hProcess, priorityLevel);
        }
        return result;
    }


    void DisableCoherentPriority() {
        if (!isEnabled) {
            return;
        }

        isEnabled = false;
        loopCv.notify_all();
        loopThread.join();

        MH_QueueDisableHook(&CreateProcessA);
        MH_QueueDisableHook(&CreateProcessW);
        MH_ApplyQueued();
        MH_RemoveHook(&CreateProcessA);
        MH_RemoveHook(&CreateProcessW);
        SystemCreateProcessA = nullptr;
        SystemCreateProcessW = nullptr;
        SetCoherentPriority(CoherentPriorityLevel::CoherentPriorityNormal);
    }

    void EnableCoherentPriority(CoherentPriorityLevel level) {
        SetCoherentPriority(level);
        if (isEnabled) {
            return;
        }

        auto result = MH_CreateHookEx(&CreateProcessA, HookCreateProcessA, &SystemCreateProcessA);
        result = MH_CreateHookEx(&CreateProcessW, HookCreateProcessW, &SystemCreateProcessW);
        result = MH_QueueEnableHook(&CreateProcessA);
        result = MH_QueueEnableHook(&CreateProcessW);
        result = MH_ApplyQueued();
        isEnabled = true;
        loopThread = thread(&Loop);
    }

}
