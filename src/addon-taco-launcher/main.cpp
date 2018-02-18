#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <Shlwapi.h>
#include <filesystem>
#include <string>
#include "../../include/gw2addon-native.h"

using namespace std;
using namespace std::experimental::filesystem::v1;

GW2AddonInfo info = {};
#ifdef _WIN64
const std::wstring tacoFolder = L"bin64/addons/taco/";
#else
const std::wstring tacoFolder = L"bin/addons/taco";
#endif
PROCESS_INFORMATION tacoProcess;

#ifdef __cplusplus
extern "C" {
#endif

    GW2ADDON_API GW2ADDON_RESULT GW2ADDON_CALL GW2_GetAddonInfo(GW2AddonInfo** addonInfo) {
        if (!info.idSize) {
            info.id = L"taco-launcher";
            info.idSize = wcslen(info.id);
            info.name = L"Tactical Overlay Launcher";
            info.nameSize = wcslen(info.name);
            info.author = L"TacO by BoyC - Addon by Archomeda";
            info.authorSize = wcslen(info.author);
            info.description = L"A small launcher addon for TacO that automatically launches TacO whenever Guild Wars 2 is starting up.";
            info.descriptionSize = wcslen(info.description);
            info.version = L"1.0";
            info.versionSize = wcslen(info.version);
            info.homepage = L"https://github.com/Archomeda/gw2-addon-loader";
            info.homepageSize = wcslen(info.homepage);
        }
        *addonInfo = &info;
        return 0;
    }

    GW2ADDON_API GW2ADDON_RESULT GW2ADDON_CALL GW2_Load(HWND hFocusWindow, IDirect3DDevice9* pDev) {
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

    GW2ADDON_API GW2ADDON_RESULT GW2ADDON_CALL GW2_Unload() {
        if (tacoProcess.hProcess) {
            TerminateProcess(tacoProcess.hProcess, 0);
            CloseHandle(tacoProcess.hProcess);
            CloseHandle(tacoProcess.hThread);
            tacoProcess.hProcess = NULL;
            tacoProcess.hThread = NULL;
        }
        return 0;
    }

#ifdef __cplusplus
}
#endif
