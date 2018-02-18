#include "hooks_manager.h"
#include <filesystem>
#include <MinHook.h>
#include "../addons/addons_manager.h"
#include "../d3d9types.h"
#include "../log.h"
#include "../utils.h"
#include "../Config.h"
#include "vftable.h"

using namespace std;
using namespace std::experimental::filesystem::v1;
using namespace loader::addons;
using namespace loader::hooks;

HMODULE SystemD3D9 = nullptr;
UINT d3d9SDKVersion;

namespace loader {
    namespace hooks {

        D3D9Hooks Hooks;

        bool LoaderReady = false;

        bool InitializeHooks() {
            GetLog()->debug("loader::hooks::InitializeHooks()");
            MH_STATUS status = MH_Initialize();
            if (status != MH_OK) {
                GetLog()->error("Failed to initialize hook library: {0}", MH_StatusToString(status));
                return false;
            }
            GetLog()->info("Hook library initialized");
            return true;
        }

        void UninitializeHooks() {
            GetLog()->debug("loader::hooks::UnintializeHooks()");

            MH_STATUS status = MH_Uninitialize();
            if (status != MH_OK) {
                GetLog()->error("Failed to uninitialize hook library: {0}", MH_StatusToString(status));
            }
            else {
                GetLog()->info("Hook library uninitialized");
            }

            if (SystemD3D9)
            {
                if (!FreeLibrary(SystemD3D9)) {
                    GetLog()->error("Failed to free system d3d9 library: {0}", LastErrorToString(GetLastError()));
                }
                SystemD3D9 = nullptr;
            }
        }

        void SetD3D9Hooks(const D3D9Hooks& hooks) {
            Hooks = hooks;
        }


        namespace detours {

            IDirect3DDevice9* D3DDevice9 = nullptr;
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
                if (LoaderReady && Hooks.PreCreateDevice != nullptr) {
                    Hooks.PreCreateDevice(hFocusWindow);
                }

                // Get ourselves the device
                HRESULT hr = CreateDevice(_this, Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, &D3DDevice9);
                if (hr != D3D_OK) { return hr; }
                *ppReturnedDeviceInterface = D3DDevice9;

                if (LoaderReady && Hooks.PostCreateDevice != nullptr) {
                    Hooks.PostCreateDevice(hFocusWindow, D3DDevice9, pPresentationParameters);
                }

                // Load enabled addons
                GetLog()->info("Loading enabled addons");
                for (auto it = AddonsList.begin(); it != AddonsList.end(); ++it) {
                    (*it)->SetFocusWindow(hFocusWindow);
                    (*it)->SetD3DDevice9(D3DDevice9);
                    if ((*it)->IsEnabledByConfig()) {
                        (*it)->Load();
                    }
                }

                return hr;
            }

            HRESULT WINAPI ID3D9Ex_CreateDevice(IDirect3D9Ex* _this, UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, D3DDISPLAYMODEEX* pFullscreenDisplayMode, IDirect3DDevice9Ex** ppReturnedDeviceInterface) {
                if (LoaderReady && Hooks.PreCreateDevice != nullptr) {
                    Hooks.PreCreateDevice(hFocusWindow);
                }

                // Get ourselves the device
                HRESULT hr = CreateDeviceEx(_this, Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, pFullscreenDisplayMode, &D3DDevice9Ex);
                if (hr != D3D_OK) { return hr; }
                *ppReturnedDeviceInterface = D3DDevice9Ex;

                if (LoaderReady && Hooks.PostCreateDevice != nullptr) {
                    Hooks.PostCreateDevice(hFocusWindow, D3DDevice9Ex, pPresentationParameters);
                }

                // Load enabled addons
                GetLog()->info("Loading enabled addons");
                for (auto it = AddonsList.begin(); it != AddonsList.end(); ++it) {
                    (*it)->SetFocusWindow(hFocusWindow);
                    (*it)->SetD3DDevice9(D3DDevice9Ex);
                    if ((*it)->IsEnabledByConfig()) {
                        (*it)->Load();
                    }
                }

                return hr;
            }

            HRESULT WINAPI ID3D9_Present(IDirect3DDevice9* _this, CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion) {
                _this->BeginScene();

                // Draw addon frames
                for (auto it = AddonsList.begin(); it != AddonsList.end(); ++it) {
                    (*it)->DrawFrame(_this);
                }

                _this->EndScene();

                // Draw our frames
                if (LoaderReady && Hooks.PrePresent != nullptr) {
                    Hooks.PrePresent(_this);
                }

                HRESULT hr = Present(_this, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
                
                if (LoaderReady && Hooks.PostPresent != nullptr) {
                    Hooks.PostPresent(_this);
                }

                return hr;
            }

            HRESULT WINAPI ID3D9Ex_Present(IDirect3DDevice9Ex* _this, CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion, DWORD dwFlags) {
                _this->BeginScene();

                // Draw addon frames
                for (auto it = AddonsList.begin(); it != AddonsList.end(); ++it) {
                    (*it)->DrawFrame(_this);
                }

                _this->EndScene();

                // Draw our frames
                if (LoaderReady && Hooks.PrePresent != nullptr) {
                    Hooks.PrePresent(_this);
                }

                HRESULT hr = PresentEx(_this, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion, dwFlags);

                if (LoaderReady && Hooks.PostPresent != nullptr) {
                    Hooks.PostPresent(_this);
                }

                return hr;
            }

            HRESULT WINAPI ID3D9_Reset(IDirect3DDevice9* _this, D3DPRESENT_PARAMETERS* pPresentationParameters) {
                if (LoaderReady && Hooks.PreReset != nullptr) {
                    Hooks.PreReset(_this, pPresentationParameters);
                }

                HRESULT hr = Reset(_this, pPresentationParameters);

                if (LoaderReady && Hooks.PostReset != nullptr) {
                    Hooks.PostReset(_this, pPresentationParameters);
                }
                
                return hr;
            }

            HRESULT WINAPI ID3D9Ex_Reset(IDirect3DDevice9Ex* _this, D3DPRESENT_PARAMETERS* pPresentationParameters, D3DDISPLAYMODEEX *pFullscreenDisplayMode) {
                if (LoaderReady && Hooks.PreReset != nullptr) {
                    Hooks.PreReset(_this, pPresentationParameters);
                }

                HRESULT hr = ResetEx(_this, pPresentationParameters, pFullscreenDisplayMode);

                if (LoaderReady && Hooks.PostReset != nullptr) {
                    Hooks.PostReset(_this, pPresentationParameters);
                }

                return hr;
            }


            D3DPRESENT_PARAMETERS SetupHookDevice(HWND &hWnd) {
                WNDCLASSEXA wc = { 0 };
                wc.cbSize = sizeof(wc);
                wc.style = CS_CLASSDC;
                wc.lpfnWndProc = DefWindowProc;
                wc.hInstance = GetModuleHandleA(nullptr);
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
                    UnregisterClassA("DXTMP", GetModuleHandleA(nullptr));
                }
            }

            bool CreateDetours(IDirect3D9* d3d9) {
                GetLog()->debug("loader::hooks::detours::CreateDetours(IDirect3D9*)");
                
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

                return true;
            }

            bool CreateDetours(IDirect3D9Ex* d3d9) {
                GetLog()->debug("loader::hooks::detours::CreateDetours(IDirect3D9Ex*)");

                // Hook the D3D9 entry point, aka CreateDevice
                auto vft = GetVftD3D9Ex(d3d9);
                if (!CreateAndEnableHook("CreateDevice", vft.CreateDevice, &ID3D9_CreateDevice, &CreateDevice) ||
                    !CreateAndEnableHook("CreateDeviceEx", vft.CreateDeviceEx, &ID3D9Ex_CreateDevice, &CreateDeviceEx)) {
                    return false;
                }
    
                // Get device
                HWND hWnd;
                auto d3dpar = SetupHookDevice(hWnd);
                IDirect3DDevice9Ex* pDev;
                CreateDeviceEx(d3d9, D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpar, nullptr, &pDev);
                auto vftd = GetVftD3DDevice9(pDev);
                DeleteHookDevice(pDev, hWnd);
      
                // Hook device functions
                if (!CreateAndEnableHook("Reset", vftd.Reset, &ID3D9_Reset, &Reset) ||
                    !CreateAndEnableHook("ResetEx", vftd.ResetEx, &ID3D9Ex_Reset, &ResetEx) ||
                    !CreateAndEnableHook("Present", vftd.Present, &ID3D9_Present, &Present) ||
                    !CreateAndEnableHook("PresentEx", vftd.PresentEx, &ID3D9Ex_Present, &PresentEx)) {
                    return false;
                }

                return true;
            }
        
        }


        bool BeforeDirect3DCreate9(UINT SDKVersion) {
            d3d9SDKVersion = SDKVersion;

            if (!SystemD3D9) {
                SystemD3D9 = LoadSystemD3D9();
            }
            if (!SystemD3D9) {
                GetLog()->error("Failed to load the system d3d9.dll: {0}", LastErrorToString(GetLastError()));
                return false;
            }
            return true;
        }

        IDirect3D9* WINAPI Direct3DCreate9(UINT sdkVersion) {
            GetLog()->debug("loader::hooks::Direct3DCreate9()");
            if (!BeforeDirect3DCreate9(sdkVersion)) {
                return nullptr;
            }

            // Create our stuff
            GetLog()->info("Creating D3D9");
            auto fDirect3DCreate9 = reinterpret_cast<Direct3DCreate9_t>(GetProcAddress(SystemD3D9, "Direct3DCreate9"));
            if (!fDirect3DCreate9) {
                GetLog()->error("System Direct3DCreate GetProcAddress failed");
                return nullptr;
            }
            auto d3d9 = fDirect3DCreate9(sdkVersion);

            // Detour
            GetLog()->info("Creating detours");
            if (!detours::CreateDetours(d3d9)) {
                return nullptr;
            }

            // Initialize addons
            GetLog()->info("Initializing addons");
            for (auto it = AddonsList.begin(); it != AddonsList.end(); ++it) {
                (*it)->SetSdkVersion(sdkVersion);
                (*it)->SetD3D9(d3d9);
                (*it)->Initialize();
            }
            
            LoaderReady = true;
            return d3d9;
        }

        IDirect3D9Ex* WINAPI Direct3DCreate9Ex(UINT sdkVersion) {
            GetLog()->debug("loader::hooks::Direct3DCreate9Ex()");
            if (!BeforeDirect3DCreate9(sdkVersion)) {
                return nullptr;
            }

            // Create our stuff
            GetLog()->info("Creating D3D9Ex");
            auto fDirect3DCreate9 = reinterpret_cast<Direct3DCreate9Ex_t>(GetProcAddress(SystemD3D9, "Direct3DCreate9Ex"));
            if (!fDirect3DCreate9) {
                GetLog()->error("System Direct3DCreate9Ex GetProcAddress failed");
                return nullptr;
            }
            auto d3d9 = fDirect3DCreate9(sdkVersion);

            // Detour
            GetLog()->info("Creating detours");
            if (!detours::CreateDetours(d3d9)) {
                return nullptr;
            }

            // Initialize addons
            GetLog()->info("Initializing addons");
            for (auto it = AddonsList.begin(); it != AddonsList.end(); ++it) {
                (*it)->SetSdkVersion(sdkVersion);
                (*it)->SetD3D9Ex(d3d9);
                (*it)->Initialize();
            }

            LoaderReady = true;
            return d3d9;
        }

    }
}
