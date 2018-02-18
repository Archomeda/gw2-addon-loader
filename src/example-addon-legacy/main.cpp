/**
This example is currently non-functional.
*/

#include <list>
#include <stdint.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <d3d9.h>

#include "../loader/d3d9types.h"

using namespace std;

HMODULE SystemD3D9 = nullptr;

IDirect3DDevice9* D3D9Device = nullptr;
IDirect3DDevice9Ex* D3DDevice9Ex = nullptr;

CreateDevice_t CreateDevice = nullptr;
CreateDeviceEx_t CreateDeviceEx = nullptr;
Present_t Present = nullptr;
PresentEx_t PresentEx = nullptr;
Reset_t Reset = nullptr;
ResetEx_t ResetEx = nullptr;
AddRef_t AddRef = nullptr;
Release_t Release = nullptr;

HRESULT WINAPI ID3D9_CreateDevice(IDirect3D9* _this, UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DDevice9** ppReturnedDeviceInterface) {
    // Get ourselves the device
    HRESULT hr = CreateDevice(_this, Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, &D3D9Device);
    if (hr != D3D_OK) { return hr; }
    *ppReturnedDeviceInterface = D3D9Device;

    return hr;
}

HRESULT WINAPI ID3D9Ex_CreateDevice(IDirect3D9Ex* _this, UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, D3DDISPLAYMODEEX* pFullscreenDisplayMode, IDirect3DDevice9Ex** ppReturnedDeviceInterface) {
    // Get ourselves the device
    HRESULT hr = CreateDeviceEx(_this, Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, pFullscreenDisplayMode, &D3DDevice9Ex);
    if (hr != D3D_OK) { return hr; }
    *ppReturnedDeviceInterface = D3DDevice9Ex;

    return hr;
}

HRESULT WINAPI ID3D9_Present(IDirect3DDevice9* _this, CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion) {
    HRESULT hr = Present(_this, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
    return hr;
}

HRESULT WINAPI ID3D9Ex_Present(IDirect3DDevice9Ex* _this, CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion, DWORD dwFlags) {
    HRESULT hr = PresentEx(_this, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion, dwFlags);
    return hr;
}

HRESULT WINAPI ID3D9_Reset(IDirect3DDevice9* _this, D3DPRESENT_PARAMETERS* pPresentationParameters) {
    HRESULT hr = Reset(_this, pPresentationParameters);
    return hr;
}

HRESULT WINAPI ID3D9Ex_Reset(IDirect3DDevice9Ex* _this, D3DPRESENT_PARAMETERS* pPresentationParameters, D3DDISPLAYMODEEX *pFullscreenDisplayMode) {
    HRESULT hr = ResetEx(_this, pPresentationParameters, pFullscreenDisplayMode);
    return hr;
}


IDirect3D9* WINAPI Direct3DCreate9(UINT sdkVersion) {
    // Our entry point, along with Direct3DCreate9Ex
    // The game calls this when a D3D9 device gets created
    // You can initialize various things you need

    // Create our stuff
    auto fDirect3DCreate9 = reinterpret_cast<Direct3DCreate9_t>(GetProcAddress(SystemD3D9, "Direct3DCreate9"));
    if (!fDirect3DCreate9) {
        return nullptr;
    }
    auto d3d9 = fDirect3DCreate9(sdkVersion);

    // Hook the D3D9 entry point, aka CreateDevice
    auto vft = GetVftD3D9(d3d9);
    if (!CreateAndEnableHook("CreateDevice", vft.CreateDevice, &ID3D9_CreateDevice, &CreateDevice)) {
        return false;
    }

    // Do device hooks
    HWND hWnd;
    auto d3dpar = SetupHookDevice(hWnd);
    IDirect3DDevice9* pDev;
    CreateDevice(d3d9, D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpar, &pDev);
    auto vftd = GetVftD3DDevice9(pDev);
    DeleteHookDevice(pDev, hWnd);

    // Hook device functions
    if (!CreateAndEnableHook("Reset", vftd.Reset, &ID3D9_Reset, &Reset) ||
        !CreateAndEnableHook("Present", vftd.Present, &ID3D9_Present, &Present)) {
        return false;
    }

    return d3d9;
}

IDirect3D9Ex* WINAPI Direct3DCreate9Ex(UINT sdkVersion) {
    // Our entry point, along with Direct3DCreate9
    // The game calls this when a D3D9Ex device gets created
    // You can initialize various things you need

    // Create our stuff
    auto fDirect3DCreate9 = reinterpret_cast<Direct3DCreate9Ex_t>(GetProcAddress(SystemD3D9, "Direct3DCreate9Ex"));
    if (!fDirect3DCreate9) {
        return nullptr;
    }
    auto d3d9 = fDirect3DCreate9(sdkVersion);

    // Detour
    if (!detours::CreateDetours(d3d9)) {
        return nullptr;
    }

    return d3d9;
}


bool WINAPI DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved) {
    switch (fdwReason) {
    case DLL_PROCESS_ATTACH:
        // This is where the addon gets loaded by the game, do some stuff if needed
        // But generally initialize your stuff in Direct3DCreate9 or Direct3DCreate9Ex
        break;
    case DLL_PROCESS_DETACH:
        // This is where the addon gets unloaded by the game, clean up all your loaded stuff
        break;
    }
    return true;
}
