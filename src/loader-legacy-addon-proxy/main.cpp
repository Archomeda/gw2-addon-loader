/**
This is a proxy DLL that impersonates the real d3d9.dll when legacy addons are loading.
The goal is to support legacy addons without having them to be ported to native addons.
This DLL will intercept all necessary D3D9 functions and redirect them to the real D3D9
library. It does nothing else.

However, if you're an addon developer, please consider making a native addon. An
example is available as the example-addon-native project. If needed, it's possible to
make a native + legacy hybrid. This means it's compatible with the addon loader, while
also being able to be used on its own without the addon loader. While an example is
out-of-scope for this project, you can refer to the example-addon-native project that
shows how you can detect the addon loader in the DllMain function.

This project creates a file called d3d9-proxy.dll. The addon loader expects this file
to be present inside the addon folder.
*/

#include <d3d9.h>
#include <d3d9-defines.h>
#include <gw2addon-legacy.h>
#include <filesystem>
#include "ProxyDirect3D9.h"

using namespace std;
using namespace std::experimental::filesystem;


HMODULE SystemD3D9Module;
D3D9Exports SystemD3D9;
const GW2LegacyAddonProxyAPI* LoaderApi;

#pragma region D3D9 exports

void* WINAPI Direct3DShaderValidatorCreate9() {
    if (!SystemD3D9.Direct3DShaderValidatorCreate9) {
        return nullptr;
    }
    return SystemD3D9.Direct3DShaderValidatorCreate9();
}

void WINAPI PSGPError() {
    if (!SystemD3D9.PSGPError) {
        return;
    }
    SystemD3D9.PSGPError();
}

void WINAPI PSGPSampleTexture() {
    if (!SystemD3D9.PSGPSampleTexture) {
        return;
    }
    SystemD3D9.PSGPSampleTexture();
}

int WINAPI D3DPERF_BeginEvent(D3DCOLOR col, LPCWSTR wszName) {
    if (!SystemD3D9.D3DPERF_BeginEvent) {
        return 0;
    }
    return SystemD3D9.D3DPERF_BeginEvent(col, wszName);
}

int WINAPI D3DPERF_EndEvent() {
    if (!SystemD3D9.D3DPERF_EndEvent) {
        return 0;
    }
    return SystemD3D9.D3DPERF_EndEvent();
}

DWORD WINAPI D3DPERF_GetStatus() {
    if (!SystemD3D9.D3DPERF_GetStatus) {
        return 0;
    }
    return SystemD3D9.D3DPERF_GetStatus();
}

BOOL WINAPI D3DPERF_QueryRepeatFrame() {
    if (!SystemD3D9.D3DPERF_QueryRepeatFrame) {
        return FALSE;
    }
    return SystemD3D9.D3DPERF_QueryRepeatFrame();
}

void WINAPI D3DPERF_SetMarker(D3DCOLOR col, LPCWSTR wszName) {
    if (!SystemD3D9.D3DPERF_SetMarker) {
        return;
    }
    SystemD3D9.D3DPERF_SetMarker(col, wszName);
}

void WINAPI D3DPERF_SetOptions(DWORD dwOptions) {
    if (!SystemD3D9.D3DPERF_SetOptions) {
        return;
    }
    SystemD3D9.D3DPERF_SetOptions(dwOptions);
}

void WINAPI D3DPERF_SetRegion(D3DCOLOR col, LPCWSTR wszName) {
    if (!SystemD3D9.D3DPERF_SetRegion) {
        return;
    }
    SystemD3D9.D3DPERF_SetRegion(col, wszName);
}

void WINAPI DebugSetLevel(LONG level) {
    if (!SystemD3D9.DebugSetLevel) {
        return;
    }
    SystemD3D9.DebugSetLevel(level);
}

void WINAPI DebugSetMute() {
    if (!SystemD3D9.DebugSetMute) {
        return;
    }
    SystemD3D9.DebugSetMute();
}

void WINAPI Direct3D9EnableMaximizedWindowedModeShim() {
    if (!SystemD3D9.Direct3D9EnableMaximizedWindowedModeShim) {
        return;
    }
    SystemD3D9.Direct3D9EnableMaximizedWindowedModeShim();
}

IDirect3D9* WINAPI Direct3DCreate9(UINT sdkVersion) {
    if (!SystemD3D9.Direct3DCreate9) {
        return nullptr;
    }
    return new ProxyDirect3D9(SystemD3D9.Direct3DCreate9(sdkVersion), LoaderApi);
}

IDirect3D9Ex* WINAPI Direct3DCreate9Ex(UINT sdkVersion) {
    if (!SystemD3D9.Direct3DCreate9Ex) {
        return nullptr;
    }
    return new ProxyDirect3D9Ex(SystemD3D9.Direct3DCreate9Ex(sdkVersion), LoaderApi);
}

#pragma endregion

bool GW2PROXY_CALL ProxyInitialize(const GW2LegacyAddonProxyAPI* const api) {
    wchar_t systemDir[MAX_PATH];
    GetSystemDirectory(systemDir, MAX_PATH);
    path systemPath(systemDir);
    systemPath /= "d3d9";
    SystemD3D9Module = LoadLibrary(systemPath.c_str());
    if (SystemD3D9Module == NULL) {
        return false;
    }

    if (api == nullptr) {
        return false;
    }

    SystemD3D9.Direct3DShaderValidatorCreate9 = reinterpret_cast<Direct3DShaderValidatorCreate9_t*>(GetProcAddress(SystemD3D9Module, "Direct3DShaderValidatorCreate9"));
    SystemD3D9.PSGPError = reinterpret_cast<PSGPError_t*>(GetProcAddress(SystemD3D9Module, "PSGPError"));
    SystemD3D9.PSGPSampleTexture = reinterpret_cast<PSGPSampleTexture_t*>(GetProcAddress(SystemD3D9Module, "PSGPSampleTexture"));
    SystemD3D9.D3DPERF_BeginEvent = reinterpret_cast<D3DPERF_BeginEvent_t*>(GetProcAddress(SystemD3D9Module, "D3DPERF_BeginEvent"));
    SystemD3D9.D3DPERF_EndEvent = reinterpret_cast<D3DPERF_EndEvent_t*>(GetProcAddress(SystemD3D9Module, "D3DPERF_EndEvent"));
    SystemD3D9.D3DPERF_GetStatus = reinterpret_cast<D3DPERF_GetStatus_t*>(GetProcAddress(SystemD3D9Module, "D3DPERF_GetStatus"));
    SystemD3D9.D3DPERF_QueryRepeatFrame = reinterpret_cast<D3DPERF_QueryRepeatFrame_t*>(GetProcAddress(SystemD3D9Module, "D3DPERF_QueryRepeatFrame"));
    SystemD3D9.D3DPERF_SetMarker = reinterpret_cast<D3DPERF_SetMarker_t*>(GetProcAddress(SystemD3D9Module, "D3DPERF_SetMarker"));
    SystemD3D9.D3DPERF_SetOptions = reinterpret_cast<D3DPERF_SetOptions_t*>(GetProcAddress(SystemD3D9Module, "D3DPERF_SetOptions"));
    SystemD3D9.D3DPERF_SetRegion = reinterpret_cast<D3DPERF_SetRegion_t*>(GetProcAddress(SystemD3D9Module, "D3DPERF_SetRegion"));
    SystemD3D9.DebugSetLevel = reinterpret_cast<DebugSetLevel_t*>(GetProcAddress(SystemD3D9Module, "DebugSetLevel"));
    SystemD3D9.DebugSetMute = reinterpret_cast<DebugSetMute_t*>(GetProcAddress(SystemD3D9Module, "DebugSetMute"));
    SystemD3D9.Direct3D9EnableMaximizedWindowedModeShim = reinterpret_cast<Direct3D9EnableMaximizedWindowedModeShim_t*>(GetProcAddress(SystemD3D9Module, "Direct3D9EnableMaximizedWindowedModeShim"));
    SystemD3D9.Direct3DCreate9 = reinterpret_cast<Direct3DCreate9_t*>(GetProcAddress(SystemD3D9Module, "Direct3DCreate9"));
    SystemD3D9.Direct3DCreate9Ex = reinterpret_cast<Direct3DCreate9Ex_t*>(GetProcAddress(SystemD3D9Module, "Direct3DCreate9Ex"));
    LoaderApi = api;

    return true;
}

bool GW2PROXY_CALL ProxyRelease() {
    FreeLibrary(SystemD3D9Module);
    SystemD3D9Module = NULL;
    LoaderApi = nullptr;
    return true;
}
