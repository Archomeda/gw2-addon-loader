#include "hook.h"

#include <vector>
#include <tchar.h>
#include "MinHook.h"

#include "d3d9.h"
#include "vftable.h"

#ifdef _WIN64
#define CHAINLOAD_PATH "bin64\\addons"
#else
#define CHAINLOAD_PATH "bin\\addons"
#endif

using namespace std;

HMODULE SystemD3D9 = nullptr;
vector<HMODULE> ChainedD3D9();
bool HookedD3D = false;


bool InitializeHook() {
    if (MH_Initialize() != MH_OK) {
        MessageBox(NULL, TEXT("Failed to initialize hook library."), TEXT("GW2 Addon Loader"), MB_OK);
        return false;
    }
    return true;
}

void UninitializeHook() {
    MH_Uninitialize();
    if (SystemD3D9)
    {
        FreeLibrary(SystemD3D9);
        SystemD3D9 = nullptr;
    }
}


#pragma region D3D9 hooks

typedef IDirect3D9* (WINAPI *Direct3DCreate9_t)(UINT SDKVersion);
typedef IDirect3D9Ex* (WINAPI *Direct3DCreate9Ex_t)(UINT SDKVersion);

CreateDevice_t CreateDevice_real = nullptr;
CreateDeviceEx_t CreateDeviceEx_real = nullptr;
Present_t Present_real = nullptr;
PresentEx_t PresentEx_real = nullptr;
Reset_t Reset_real = nullptr;
ResetEx_t ResetEx_real = nullptr;
Release_t Release_real = nullptr;
AddRef_t AddRef_real = nullptr;

HRESULT WINAPI CreateDevice_hook(IDirect3D9* _this, UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DDevice9** ppReturnedDeviceInterface)
{
    PreCreateDevice(hFocusWindow);

    IDirect3DDevice9* temp_device = nullptr;
    HRESULT hr = CreateDevice_real(_this, Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, &temp_device);
    if (hr != D3D_OK)
        return hr;

    *ppReturnedDeviceInterface = temp_device;

    PostCreateDevice(temp_device, pPresentationParameters);

    return hr;
}

HRESULT WINAPI CreateDeviceEx_hook(IDirect3D9Ex* _this, UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, D3DDISPLAYMODEEX* pFullscreenDisplayMode, IDirect3DDevice9** ppReturnedDeviceInterface)
{
    PreCreateDevice(hFocusWindow);

    IDirect3DDevice9Ex* temp_device = nullptr;
    HRESULT hr = CreateDeviceEx_real(_this, Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, pFullscreenDisplayMode, &temp_device);
    if (hr != D3D_OK)
        return hr;

    *ppReturnedDeviceInterface = temp_device;

    PostCreateDevice(temp_device, pPresentationParameters);

    return hr;
}

HRESULT WINAPI Present_hook(IDirect3DDevice9* _this, CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion)
{
    Draw(_this);

    return Present_real(_this, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
}

HRESULT WINAPI PresentEx_hook(IDirect3DDevice9Ex* _this, CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion, DWORD dwFlags)
{
    Draw(_this);

    return PresentEx_real(_this, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion, dwFlags);
}

HRESULT WINAPI Reset_hook(IDirect3DDevice9* _this, D3DPRESENT_PARAMETERS* pPresentationParameters)
{
    PreReset();

    HRESULT hr = Reset_real(_this, pPresentationParameters);
    if (hr != D3D_OK)
        return hr;

    PostReset(_this, pPresentationParameters);

    return D3D_OK;
}

HRESULT WINAPI ResetEx_hook(IDirect3DDevice9Ex* _this, D3DPRESENT_PARAMETERS* pPresentationParameters, D3DDISPLAYMODEEX *pFullscreenDisplayMode)
{
    PreReset();

    HRESULT hr = ResetEx_real(_this, pPresentationParameters, pFullscreenDisplayMode);
    if (hr != D3D_OK)
        return hr;

    PostReset(_this, pPresentationParameters);

    return D3D_OK;
}

ULONG WINAPI Release_hook(IDirect3DDevice9* _this)
{
    ULONG refcount = Release_real(_this);

    return refcount;
}

ULONG WINAPI AddRef_hook(IDirect3DDevice9* _this)
{
    return AddRef_real(_this);
}

#pragma endregion


bool OnCreate() {
    if (!SystemD3D9) {
        TCHAR path[MAX_PATH];

        // Load system D3D9 library
        GetSystemDirectory(path, MAX_PATH);
        _tcscat_s(path, TEXT("\\d3d9.dll"));
        SystemD3D9 = LoadLibrary(path);

        if (!SystemD3D9) {
            MessageBox(NULL, TEXT("Failed to load the system d3d9.dll. Unexpected behavior."), TEXT("GW2 Addon Loader"), MB_OK);
            return false;
        }
    }

    return true;
}

D3DPRESENT_PARAMETERS SetupHookDevice(HWND &hWnd) {
    WNDCLASSEXA wc = { 0 };
    wc.cbSize = sizeof(wc);
    wc.style = CS_CLASSDC;
    wc.lpfnWndProc = DefWindowProc;
    wc.hInstance = GetModuleHandleA(NULL);
    wc.lpszClassName = "DXTMP";
    RegisterClassExA(&wc);

    hWnd = CreateWindowA("DXTMP", 0, WS_OVERLAPPEDWINDOW, 100, 100, 300, 300, GetDesktopWindow(), 0, wc.hInstance, 0);

    D3DPRESENT_PARAMETERS d3dPar = { 0 };
    d3dPar.Windowed = TRUE;
    d3dPar.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dPar.hDeviceWindow = hWnd;
    d3dPar.BackBufferCount = 1;
    d3dPar.BackBufferFormat = D3DFMT_X8R8G8B8;
    d3dPar.BackBufferHeight = 300;
    d3dPar.BackBufferHeight = 300;

    return d3dPar;
}

void DeleteHookDevice(IDirect3DDevice9* pDev, HWND hWnd) {
    if (pDev) {
        pDev->Release();
    }

    if (hWnd) {
        DestroyWindow(hWnd);
        UnregisterClassA("DXTMP", GetModuleHandleA(NULL));
    }
}

template <typename T>
bool CreateHook(TCHAR* targetName, LPVOID pTarget, LPVOID pDetour, T** ppOriginal) {
    MH_STATUS result = MH_CreateHook(pTarget, pDetour, reinterpret_cast<LPVOID*>(ppOriginal));
    if (result != MH_OK && result != MH_ERROR_ALREADY_CREATED) {
        TCHAR message[256];
        _sntprintf_s(message, sizeof(message), _TRUNCATE, TEXT("Failed to hook %s"), targetName);
        MessageBox(NULL, message, TEXT("GW2 Addon Loader"), MB_OK);
        return false;
    }
    return true;
}

IDirect3D9 *WINAPI Direct3DCreate9(UINT SDKVersion) {
    if (!OnCreate()) {
        return nullptr;
    }

    auto fDirect3DCreate9 = (Direct3DCreate9_t)GetProcAddress(SystemD3D9, "Direct3DCreate9");
    auto d3d = fDirect3DCreate9(SDKVersion);

    if (!HookedD3D) {
        auto vft = GetVftD3D9(d3d);

        if (!CreateHook(TEXT("CreateDevice"), vft.CreateDevice, &CreateDevice_hook, &CreateDevice_real)) {
            return nullptr;
        }
        if (MH_EnableHook(MH_ALL_HOOKS) != MH_OK) {
            MessageBox(NULL, TEXT("Failed to enable the Direct3D9 hooks"), TEXT("GW2 Addon Loader"), MB_OK);
            return nullptr;
        }
    }

    //if (ChainD3D9Module)
    //{
    //    d3d9->Release();

    //    fDirect3DCreate9 = (Direct3DCreate9_t)GetProcAddress(ChainD3D9Module, "Direct3DCreate9");
    //    d3d9 = fDirect3DCreate9(SDKVersion);
    //}

    if (!HookedD3D)     {
        HWND hWnd;
        auto d3dpar = SetupHookDevice(hWnd);
        IDirect3DDevice9* pDev;
        CreateDevice_real(d3d, D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpar, &pDev);

        auto vftd = GetVftD3DDevice9(pDev);

        DeleteHookDevice(pDev, hWnd);

        if (!CreateHook(TEXT("Reset"), vftd.Reset, &Reset_hook, &Reset_real) ||
            !CreateHook(TEXT("Present"), vftd.Present, &Present_hook, &Present_real) ||
            !CreateHook(TEXT("Release"), vftd.Release, &Release_hook, &Release_real) ||
            !CreateHook(TEXT("AddRef"), vftd.AddRef, &AddRef_hook, &AddRef_real)) {
            return nullptr;
        }
        if (MH_EnableHook(MH_ALL_HOOKS) != MH_OK) {
            MessageBox(NULL, TEXT("Failed to enable the Direct3DDevice9 hooks"), TEXT("GW2 Addon Loader"), MB_OK);
            return nullptr;
        }

        HookedD3D = true;
    }

    return d3d;
}

IDirect3D9Ex *WINAPI Direct3DCreate9Ex(UINT SDKVersion) {
    if (!OnCreate()) {
        return nullptr;
    }

    auto fDirect3DCreate9 = (Direct3DCreate9Ex_t)GetProcAddress(SystemD3D9, "Direct3DCreate9Ex");
    auto d3d = fDirect3DCreate9(SDKVersion);

    if (!HookedD3D) {
        auto vft = GetVftD3D9Ex(d3d);

        if (!CreateHook(TEXT("CreateDevice"), vft.CreateDevice, &CreateDevice_hook, &CreateDevice_real) ||
            !CreateHook(TEXT("CreateDeviceEx"), vft.CreateDeviceEx, &CreateDeviceEx_hook, &CreateDeviceEx_real)) {
            return nullptr;
        }
        if (MH_EnableHook(MH_ALL_HOOKS) != MH_OK) {
            MessageBox(NULL, TEXT("Failed to enable the Direct3D9Ex hooks"), TEXT("GW2 Addon Loader"), MB_OK);
            return nullptr;
        }
    }

    //if (ChainD3D9Module)
    //{
    //    d3d->Release();

    //    fDirect3DCreate9 = (Direct3DCreate9Ex_t)GetProcAddress(ChainD3D9Module, "Direct3DCreate9Ex");
    //    d3d = fDirect3DCreate9(SDKVersion);
    //}

    if (!HookedD3D) {
        HWND hWnd;
        auto d3dpar = SetupHookDevice(hWnd);
        IDirect3DDevice9Ex* pDev;
        CreateDeviceEx_real(d3d, D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpar, nullptr, &pDev);

        auto vftd = GetVftD3DDevice9Ex(pDev);

        DeleteHookDevice(pDev, hWnd);

        if (!CreateHook(TEXT("Reset"), vftd.Reset, &Reset_hook, &Reset_real) ||
            !CreateHook(TEXT("ResetEx"), vftd.ResetEx, &ResetEx_hook, &ResetEx_real) ||
            !CreateHook(TEXT("Present"), vftd.Present, &Present_hook, &Present_real) ||
            !CreateHook(TEXT("PresentEx"), vftd.PresentEx, &PresentEx_hook, &PresentEx_real) ||
            !CreateHook(TEXT("Release"), vftd.Release, &Release_hook, &Release_real) ||
            !CreateHook(TEXT("AddRef"), vftd.AddRef, &AddRef_hook, &AddRef_real)) {
            return nullptr;
        }
        if (MH_EnableHook(MH_ALL_HOOKS) != MH_OK) {
            MessageBox(NULL, TEXT("Failed to enable the Direct3DDevice9 hooks"), TEXT("GW2 Addon Loader"), MB_OK);
            return nullptr;
        }

        HookedD3D = true;
    }

    return d3d;
}
