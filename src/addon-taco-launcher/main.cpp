#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <Shlwapi.h>
#include <filesystem>
#include <string>
#include "../../include/gw2addon-native.h"

using namespace std;
using namespace std::experimental::filesystem::v1;

#ifdef _WIN64
const std::wstring tacoFolder = L"bin64/addons/taco/";
#else
const std::wstring tacoFolder = L"bin/addons/taco";
#endif
PROCESS_INFORMATION tacoProcess;

GW2ADDON_RESULT GW2ADDON_CALL Load(HWND hFocusWindow, IDirect3DDevice9* pDev) {
    // Create path
    TCHAR fileName[MAX_PATH];
    GetModuleFileName(NULL, fileName, sizeof(fileName));
    PathRemoveFileSpec(fileName);
    path folder(fileName);
    folder /= tacoFolder;
    path file(folder);
    file /= L"GW2TacO.exe";
    if (!PathFileExists(file.c_str())) {
        return 1;
    }
    wchar_t args[] = L"GW2TacO.exe -launcher";
    STARTUPINFO startupInfo = {};
    startupInfo.cb = sizeof(startupInfo);

    if (!CreateProcess(file.c_str(), args, NULL, NULL, FALSE, 0, NULL, folder.c_str(), &startupInfo, &tacoProcess)) {
        return 2;
    }

    return 0;
}

#ifdef __cplusplus
extern "C" {
#endif

    GW2ADDON_API GW2AddonAPIV1* GW2ADDON_CALL GW2AddonInitialize(int loaderVersion) {
        static GW2AddonAPIV1 addon;
        addon.id = "taco-launcher";
        addon.name = "Tactical Overlay Launcher";
        addon.author = "TacO by BoyC - Addon by Archomeda";
        addon.version = "1.0";
        addon.description = "A small launcher addon for TacO that automatically launches TacO whenever Guild Wars 2 is starting up.";
        addon.homepage = "https://github.com/Archomeda/gw2-addon-loader";
        addon.Load = &Load;
        return &addon;
    }

    GW2ADDON_API void GW2ADDON_CALL GW2AddonRelease() {
        if (tacoProcess.hProcess) {
            TerminateProcess(tacoProcess.hProcess, 0);
            CloseHandle(tacoProcess.hProcess);
            CloseHandle(tacoProcess.hThread);
            tacoProcess.hProcess = NULL;
            tacoProcess.hThread = NULL;
        }
    }

#ifdef __cplusplus
}
#endif
