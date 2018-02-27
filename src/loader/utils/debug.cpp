#include "debug.h"
#include "../windows.h"
#include <filesystem>
#include <sstream>
#include "../log.h"

namespace loader {
    namespace utils {

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

    }
}
