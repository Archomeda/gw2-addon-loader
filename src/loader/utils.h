#pragma once
#include <string>
#include <MinHook.h>

namespace loader {

    bool LaunchDebugger();
    const std::wstring s2ws(const std::string& str);
    const std::string ws2s(const std::wstring& wstr);

    const std::wstring GetSystemPath();
    const std::string GetSystemPath(const std::string& append);
    const std::wstring GetSystemPath(const std::wstring& append);
    HMODULE LoadSystemD3D9();


    template <typename T>
    bool CreateHook(const std::string& hookName, LPVOID pTarget, LPVOID pDetour, T** ppOriginal) {
        GetLog()->debug("loader::CreateHook({0})", hookName);
        MH_STATUS status = MH_CreateHook(pTarget, pDetour, reinterpret_cast<LPVOID*>(ppOriginal));
        if (status != MH_OK) {
            GetLog()->error("Failed to create hook {0}: {1}", hookName, MH_StatusToString(status));
            return false;
        }
        GetLog()->info("Hook {0} created", hookName);
        return true;
    }
    bool EnableHook(const std::string& hookName, LPVOID pTarget);
    template <typename T>
    bool CreateAndEnableHook(const std::string& hookName, LPVOID pTarget, LPVOID pDetour, T** ppOriginal) {
        return CreateHook(hookName, pTarget, pDetour, ppOriginal) && EnableHook(hookName, pTarget);
    }

}

