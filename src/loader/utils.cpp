#include "utils.h"
#include "windows.h"
#include <algorithm>
#include <codecvt>
#include <filesystem>
#include <locale>
#include <sstream>
#include "log.h"

using namespace std;
using namespace std::experimental::filesystem::v1;

namespace loader {

    bool LaunchDebugger() {
#ifdef _DEBUG
        GetLog()->debug("Attempting to attach debugger");

        if (IsDebuggerPresent()) {
            GetLog()->debug("Debugger already attached");
            return true;
        }

        // https://stackoverflow.com/a/20387632
        // Get System directory, typically c:\windows\system32
        std::wstring systemDir(MAX_PATH + 1, '\0');
        UINT nChars = GetSystemDirectoryW(&systemDir[0], static_cast<UINT>(systemDir.length()));
        if (nChars == 0) return false; // failed to get system directory
        systemDir.resize(nChars);

        // Get process ID and create the command line
        DWORD pid = GetCurrentProcessId();
        std::wostringstream s;
        s << systemDir << L"\\vsjitdebugger.exe -p " << pid;
        std::wstring cmdLine = s.str();

        // Start debugger process
        STARTUPINFOW si;
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);

        PROCESS_INFORMATION pi;
        ZeroMemory(&pi, sizeof(pi));

        if (!CreateProcessW(NULL, &cmdLine[0], NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) return false;

        // Wait for the debugger to attach
        GetLog()->debug("Waiting for debugger");
        while (true) {
            DWORD exitCode;
            if (GetExitCodeProcess(pi.hProcess, &exitCode) && exitCode != STILL_ACTIVE) {
                break;
            }
            Sleep(100);
        };

        // Close debugger process handles to eliminate resource leak
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);

        GetLog()->debug("Debugger attched? {0}", IsDebuggerPresent() ? "YES" : "NO");
#endif
        return true;
    }

    const wstring s2ws(const string& str) {
        return wstring_convert<codecvt_utf8<wchar_t>>().from_bytes(str);
    }

    const string ws2s(const wstring& wstr) {
        return wstring_convert<codecvt_utf8<wchar_t>>().to_bytes(wstr);
    }

    const wstring GetSystemPath() {
        TCHAR systemDir[MAX_PATH];
        GetSystemDirectory(systemDir, MAX_PATH);
        return wstring(systemDir);
    }

    const string GetSystemPath(const string& append) {
        path result(GetSystemPath());
        result /= append;
        return result.string();
    }

    const wstring GetSystemPath(const wstring& append) {
        path result(GetSystemPath());
        result /= append;
        return result.wstring();
    }

    HMODULE LoadSystemD3D9() {
        return LoadLibrary(GetSystemPath(TEXT("d3d9")).c_str());
    }

    bool EnableHook(const string& hookName, LPVOID pTarget) {
        GetLog()->debug("loader::EnableHook({0})", hookName);
        MH_STATUS status = MH_EnableHook(pTarget);
        if (status != MH_OK) {
            GetLog()->error("Failed to enable hook {0}: {1}", hookName, MH_StatusToString(status));
            return false;
        }
        GetLog()->info("Hook {0} enabled", hookName);
        return true;
    }

}