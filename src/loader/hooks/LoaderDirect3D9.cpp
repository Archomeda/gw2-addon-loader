#include "LoaderDirect3D9.h"
#include <map>
#include "LoaderDirect3DDevice9.h"

using namespace std;

namespace loader {
    namespace hooks {

        PreCreateDevice_t* PreCreateDeviceHook = nullptr;
        PostCreateDevice_t* PostCreateDeviceHook = nullptr;
        PreCreateDeviceEx_t* PreCreateDeviceExHook = nullptr;
        PostCreateDeviceEx_t* PostCreateDeviceExHook = nullptr;


        struct Direct3DDevice9Id {
            UINT adapter;
            D3DDEVTYPE deviceType;
            HWND hFocusWindow;
            DWORD behaviorFlags;
            D3DPRESENT_PARAMETERS presentationParameters;
        };
        bool operator<(const D3DPRESENT_PARAMETERS& l, const D3DPRESENT_PARAMETERS& r) {
            return
                l.AutoDepthStencilFormat < r.AutoDepthStencilFormat ||
                (l.AutoDepthStencilFormat == r.AutoDepthStencilFormat && l.BackBufferCount < r.BackBufferCount) ||
                (l.AutoDepthStencilFormat == r.AutoDepthStencilFormat && l.BackBufferCount == r.BackBufferCount &&
                    l.BackBufferFormat < r.BackBufferFormat) ||
                (l.AutoDepthStencilFormat == r.AutoDepthStencilFormat && l.BackBufferCount == r.BackBufferCount &&
                    l.BackBufferFormat == r.BackBufferFormat && l.BackBufferHeight < r.BackBufferHeight) ||
                (l.AutoDepthStencilFormat == r.AutoDepthStencilFormat && l.BackBufferCount == r.BackBufferCount &&
                    l.BackBufferFormat == r.BackBufferFormat && l.BackBufferHeight == r.BackBufferHeight && l.BackBufferWidth < r.BackBufferWidth) ||
                (l.AutoDepthStencilFormat == r.AutoDepthStencilFormat && l.BackBufferCount == r.BackBufferCount &&
                    l.BackBufferFormat == r.BackBufferFormat && l.BackBufferHeight == r.BackBufferHeight && l.BackBufferWidth == r.BackBufferWidth &&
                    l.EnableAutoDepthStencil < r.EnableAutoDepthStencil) ||
                (l.AutoDepthStencilFormat == r.AutoDepthStencilFormat && l.BackBufferCount == r.BackBufferCount &&
                    l.BackBufferFormat == r.BackBufferFormat && l.BackBufferHeight == r.BackBufferHeight && l.BackBufferWidth == r.BackBufferWidth &&
                    l.EnableAutoDepthStencil == r.EnableAutoDepthStencil && l.Flags < r.Flags) ||
                (l.AutoDepthStencilFormat == r.AutoDepthStencilFormat && l.BackBufferCount == r.BackBufferCount &&
                    l.BackBufferFormat == r.BackBufferFormat && l.BackBufferHeight == r.BackBufferHeight && l.BackBufferWidth == r.BackBufferWidth &&
                    l.EnableAutoDepthStencil == r.EnableAutoDepthStencil && l.Flags == r.Flags && l.FullScreen_RefreshRateInHz < r.FullScreen_RefreshRateInHz) ||
                (l.AutoDepthStencilFormat == r.AutoDepthStencilFormat && l.BackBufferCount == r.BackBufferCount &&
                    l.BackBufferFormat == r.BackBufferFormat && l.BackBufferHeight == r.BackBufferHeight && l.BackBufferWidth == r.BackBufferWidth &&
                    l.EnableAutoDepthStencil == r.EnableAutoDepthStencil && l.Flags == r.Flags && l.FullScreen_RefreshRateInHz == r.FullScreen_RefreshRateInHz &&
                    l.hDeviceWindow < r.hDeviceWindow) ||
                (l.AutoDepthStencilFormat == r.AutoDepthStencilFormat && l.BackBufferCount == r.BackBufferCount &&
                    l.BackBufferFormat == r.BackBufferFormat && l.BackBufferHeight == r.BackBufferHeight && l.BackBufferWidth == r.BackBufferWidth &&
                    l.EnableAutoDepthStencil == r.EnableAutoDepthStencil && l.Flags == r.Flags && l.FullScreen_RefreshRateInHz < r.FullScreen_RefreshRateInHz &&
                    l.hDeviceWindow == r.hDeviceWindow && l.MultiSampleQuality < r.MultiSampleQuality) ||
                (l.AutoDepthStencilFormat == r.AutoDepthStencilFormat && l.BackBufferCount == r.BackBufferCount &&
                    l.BackBufferFormat == r.BackBufferFormat && l.BackBufferHeight == r.BackBufferHeight && l.BackBufferWidth == r.BackBufferWidth &&
                    l.EnableAutoDepthStencil == r.EnableAutoDepthStencil && l.Flags == r.Flags && l.FullScreen_RefreshRateInHz < r.FullScreen_RefreshRateInHz &&
                    l.hDeviceWindow == r.hDeviceWindow && l.MultiSampleQuality == r.MultiSampleQuality && l.MultiSampleType < r.MultiSampleType) ||
                (l.AutoDepthStencilFormat == r.AutoDepthStencilFormat && l.BackBufferCount == r.BackBufferCount &&
                    l.BackBufferFormat == r.BackBufferFormat && l.BackBufferHeight == r.BackBufferHeight && l.BackBufferWidth == r.BackBufferWidth &&
                    l.EnableAutoDepthStencil == r.EnableAutoDepthStencil && l.Flags == r.Flags && l.FullScreen_RefreshRateInHz < r.FullScreen_RefreshRateInHz &&
                    l.hDeviceWindow == r.hDeviceWindow && l.MultiSampleQuality == r.MultiSampleQuality && l.MultiSampleType == r.MultiSampleType &&
                    l.PresentationInterval < r.PresentationInterval) ||
                (l.AutoDepthStencilFormat == r.AutoDepthStencilFormat && l.BackBufferCount == r.BackBufferCount &&
                    l.BackBufferFormat == r.BackBufferFormat && l.BackBufferHeight == r.BackBufferHeight && l.BackBufferWidth == r.BackBufferWidth &&
                    l.EnableAutoDepthStencil == r.EnableAutoDepthStencil && l.Flags == r.Flags && l.FullScreen_RefreshRateInHz < r.FullScreen_RefreshRateInHz &&
                    l.hDeviceWindow == r.hDeviceWindow && l.MultiSampleQuality == r.MultiSampleQuality && l.MultiSampleType == r.MultiSampleType &&
                    l.PresentationInterval == r.PresentationInterval && l.SwapEffect < r.SwapEffect) ||
                (l.AutoDepthStencilFormat == r.AutoDepthStencilFormat && l.BackBufferCount == r.BackBufferCount &&
                    l.BackBufferFormat == r.BackBufferFormat && l.BackBufferHeight == r.BackBufferHeight && l.BackBufferWidth == r.BackBufferWidth &&
                    l.EnableAutoDepthStencil == r.EnableAutoDepthStencil && l.Flags == r.Flags && l.FullScreen_RefreshRateInHz < r.FullScreen_RefreshRateInHz &&
                    l.hDeviceWindow == r.hDeviceWindow && l.MultiSampleQuality == r.MultiSampleQuality && l.MultiSampleType == r.MultiSampleType &&
                    l.PresentationInterval == r.PresentationInterval && l.SwapEffect == r.SwapEffect && l.Windowed < r.Windowed);
        }
        bool operator<(const Direct3DDevice9Id& l, const Direct3DDevice9Id& r) {
            return
                l.adapter < r.adapter ||
                (l.adapter == r.adapter && l.behaviorFlags < r.behaviorFlags) ||
                (l.adapter == r.adapter && l.behaviorFlags == r.behaviorFlags && l.deviceType < r.deviceType) ||
                (l.adapter == r.adapter && l.behaviorFlags == r.behaviorFlags && l.deviceType == r.deviceType && l.hFocusWindow < r.hFocusWindow) ||
                (l.adapter == r.adapter && l.behaviorFlags == r.behaviorFlags && l.deviceType == r.deviceType && l.hFocusWindow == r.hFocusWindow && l.presentationParameters < r.presentationParameters);
        }
        map<Direct3DDevice9Id, IDirect3DDevice9*> devices;

        Direct3DDevice9Information globalDeviceInformation = {};
        IDirect3DDevice9* GetCreatedDevice(UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters) {
            Direct3DDevice9Id id = {
                Adapter,
                DeviceType,
                hFocusWindow,
                BehaviorFlags,
                *pPresentationParameters
            };
            try {
                return devices.at(id);
            }
            catch (const out_of_range&) {
                return nullptr;
            }
        }

        Direct3DDevice9Information GetGlobalDeviceInformation() {
            return globalDeviceInformation;
        }


        HRESULT LoaderDirect3D9::QueryInterface(REFIID riid, void** ppvObj) {
            return this->d3d9->QueryInterface(riid, ppvObj);
        }

        ULONG LoaderDirect3D9::AddRef() {
            return this->d3d9->AddRef();
        }

        ULONG LoaderDirect3D9::Release() {
            ULONG count = this->d3d9->Release();
            if (count == 0) {
                delete this;
            }
            return count;
        }

        HRESULT LoaderDirect3D9::RegisterSoftwareDevice(void* pInitializeFunction) {
            return this->d3d9->RegisterSoftwareDevice(pInitializeFunction);
        }

        UINT LoaderDirect3D9::GetAdapterCount() {
            return this->d3d9->GetAdapterCount();
        }

        HRESULT LoaderDirect3D9::GetAdapterIdentifier(UINT Adapter, DWORD Flags, D3DADAPTER_IDENTIFIER9* pIdentifier) {
            return this->d3d9->GetAdapterIdentifier(Adapter, Flags, pIdentifier);
        }

        UINT LoaderDirect3D9::GetAdapterModeCount(UINT Adapter, D3DFORMAT Format) {
            return this->d3d9->GetAdapterModeCount(Adapter, Format);
        }

        HRESULT LoaderDirect3D9::EnumAdapterModes(UINT Adapter, D3DFORMAT Format, UINT Mode, D3DDISPLAYMODE* pMode) {
            return this->d3d9->EnumAdapterModes(Adapter, Format, Mode, pMode);
        }

        HRESULT LoaderDirect3D9::GetAdapterDisplayMode(UINT Adapter, D3DDISPLAYMODE *pMode) {
            return this->d3d9->GetAdapterDisplayMode(Adapter, pMode);
        }

        HRESULT LoaderDirect3D9::CheckDeviceType(UINT Adapter, D3DDEVTYPE CheckType, D3DFORMAT DisplayFormat, D3DFORMAT BackBufferFormat, BOOL Windowed) {
            return this->d3d9->CheckDeviceType(Adapter, CheckType, DisplayFormat, BackBufferFormat, Windowed);
        }

        HRESULT LoaderDirect3D9::CheckDeviceFormat(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, DWORD Usage, D3DRESOURCETYPE RType, D3DFORMAT CheckFormat) {
            return this->d3d9->CheckDeviceFormat(Adapter, DeviceType, AdapterFormat, Usage, RType, CheckFormat);
        }

        HRESULT LoaderDirect3D9::CheckDeviceMultiSampleType(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT SurfaceFormat, BOOL Windowed, D3DMULTISAMPLE_TYPE MultiSampleType, DWORD* pQualityLevels) {
            return this->d3d9->CheckDeviceMultiSampleType(Adapter, DeviceType, SurfaceFormat, Windowed, MultiSampleType, pQualityLevels);
        }

        HRESULT LoaderDirect3D9::CheckDepthStencilMatch(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, D3DFORMAT RenderTargetFormat, D3DFORMAT DepthStencilFormat) {
            return this->d3d9->CheckDepthStencilMatch(Adapter, DeviceType, AdapterFormat, RenderTargetFormat, DepthStencilFormat);
        }

        HRESULT LoaderDirect3D9::CheckDeviceFormatConversion(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT SourceFormat, D3DFORMAT TargetFormat) {
            return this->d3d9->CheckDeviceFormatConversion(Adapter, DeviceType, SourceFormat, TargetFormat);
        }

        HRESULT LoaderDirect3D9::GetDeviceCaps(UINT Adapter, D3DDEVTYPE DeviceType, D3DCAPS9* pCaps) {
            return this->d3d9->GetDeviceCaps(Adapter, DeviceType, pCaps);
        }

        HMONITOR LoaderDirect3D9::GetAdapterMonitor(UINT Adapter) {
            return this->d3d9->GetAdapterMonitor(Adapter);
        }

        HRESULT LoaderDirect3D9::CreateDevice(UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DDevice9** ppReturnedDeviceInterface) {
            HRESULT hr;
            if (PreCreateDeviceHook != nullptr) {
                hr = PreCreateDeviceHook(this, Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, ppReturnedDeviceInterface);
                if (hr != D3D_OK) {
                    // Fail
                    return hr;
                }
            }

            if (!*ppReturnedDeviceInterface) {
                // The pre hook didn't return a device interface, let's get one ourselves
                hr = this->d3d9->CreateDevice(Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, ppReturnedDeviceInterface);
                if (hr != D3D_OK) {
                    // Fail
                    return hr;
                }
            }

            // Save reference to the created device with parameters
            Direct3DDevice9Id id = {
                Adapter,
                DeviceType,
                hFocusWindow,
                BehaviorFlags,
                *pPresentationParameters
            };
            devices[id] = *ppReturnedDeviceInterface;
            if (globalDeviceInformation.device == nullptr) {
                globalDeviceInformation = {
                    Adapter,
                    DeviceType,
                    hFocusWindow,
                    BehaviorFlags,
                    *pPresentationParameters,
                    *ppReturnedDeviceInterface
                };
            }

            LoaderDirect3DDevice9* device = new LoaderDirect3DDevice9(*ppReturnedDeviceInterface);
            *ppReturnedDeviceInterface = device;

            if (PostCreateDeviceHook != nullptr) {
                PostCreateDeviceHook(this, device, hFocusWindow);
            }

            return hr;
        }


        HRESULT LoaderDirect3D9Ex::QueryInterface(REFIID riid, void** ppvObj) {
            return this->d3d9->QueryInterface(riid, ppvObj);
        }

        ULONG LoaderDirect3D9Ex::AddRef() {
            return this->d3d9->AddRef();
        }

        ULONG LoaderDirect3D9Ex::Release() {
            ULONG count = this->d3d9->Release();
            if (count == 0) {
                delete this;
            }
            return count;
        }

        HRESULT LoaderDirect3D9Ex::RegisterSoftwareDevice(void* pInitializeFunction) {
            return this->d3d9->RegisterSoftwareDevice(pInitializeFunction);
        }

        UINT LoaderDirect3D9Ex::GetAdapterCount() {
            return this->d3d9->GetAdapterCount();
        }

        HRESULT LoaderDirect3D9Ex::GetAdapterIdentifier(UINT Adapter, DWORD Flags, D3DADAPTER_IDENTIFIER9* pIdentifier) {
            return this->d3d9->GetAdapterIdentifier(Adapter, Flags, pIdentifier);
        }

        UINT LoaderDirect3D9Ex::GetAdapterModeCount(UINT Adapter, D3DFORMAT Format) {
            return this->d3d9->GetAdapterModeCount(Adapter, Format);
        }

        HRESULT LoaderDirect3D9Ex::EnumAdapterModes(UINT Adapter, D3DFORMAT Format, UINT Mode, D3DDISPLAYMODE* pMode) {
            return this->d3d9->EnumAdapterModes(Adapter, Format, Mode, pMode);
        }

        HRESULT LoaderDirect3D9Ex::GetAdapterDisplayMode(UINT Adapter, D3DDISPLAYMODE *pMode) {
            return this->d3d9->GetAdapterDisplayMode(Adapter, pMode);
        }

        HRESULT LoaderDirect3D9Ex::CheckDeviceType(UINT Adapter, D3DDEVTYPE CheckType, D3DFORMAT DisplayFormat, D3DFORMAT BackBufferFormat, BOOL Windowed) {
            return this->d3d9->CheckDeviceType(Adapter, CheckType, DisplayFormat, BackBufferFormat, Windowed);
        }

        HRESULT LoaderDirect3D9Ex::CheckDeviceFormat(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, DWORD Usage, D3DRESOURCETYPE RType, D3DFORMAT CheckFormat) {
            return this->d3d9->CheckDeviceFormat(Adapter, DeviceType, AdapterFormat, Usage, RType, CheckFormat);
        }

        HRESULT LoaderDirect3D9Ex::CheckDeviceMultiSampleType(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT SurfaceFormat, BOOL Windowed, D3DMULTISAMPLE_TYPE MultiSampleType, DWORD* pQualityLevels) {
            return this->d3d9->CheckDeviceMultiSampleType(Adapter, DeviceType, SurfaceFormat, Windowed, MultiSampleType, pQualityLevels);
        }

        HRESULT LoaderDirect3D9Ex::CheckDepthStencilMatch(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, D3DFORMAT RenderTargetFormat, D3DFORMAT DepthStencilFormat) {
            return this->d3d9->CheckDepthStencilMatch(Adapter, DeviceType, AdapterFormat, RenderTargetFormat, DepthStencilFormat);
        }

        HRESULT LoaderDirect3D9Ex::CheckDeviceFormatConversion(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT SourceFormat, D3DFORMAT TargetFormat) {
            return this->d3d9->CheckDeviceFormatConversion(Adapter, DeviceType, SourceFormat, TargetFormat);
        }

        HRESULT LoaderDirect3D9Ex::GetDeviceCaps(UINT Adapter, D3DDEVTYPE DeviceType, D3DCAPS9* pCaps) {
            return this->d3d9->GetDeviceCaps(Adapter, DeviceType, pCaps);
        }

        HMONITOR LoaderDirect3D9Ex::GetAdapterMonitor(UINT Adapter) {
            return this->d3d9->GetAdapterMonitor(Adapter);
        }

        HRESULT LoaderDirect3D9Ex::CreateDevice(UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DDevice9** ppReturnedDeviceInterface) {
            HRESULT hr;
            if (PreCreateDeviceHook != nullptr) {
                hr = PreCreateDeviceHook(dynamic_cast<LoaderDirect3D9*>(this), Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, ppReturnedDeviceInterface);
                if (hr != D3D_OK) {
                    // Fail
                    return hr;
                }
            }

            if (!*ppReturnedDeviceInterface) {
                // The pre hook didn't return a device interface, let's get one ourselves
                hr = this->d3d9->CreateDevice(Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, ppReturnedDeviceInterface);
                if (hr != D3D_OK) {
                    // Fail
                    return hr;
                }
            }

            // Save reference to the created device with parameters
            Direct3DDevice9Id id = {
                Adapter,
                DeviceType,
                hFocusWindow,
                BehaviorFlags,
                *pPresentationParameters
            };
            devices[id] = *ppReturnedDeviceInterface;
            if (globalDeviceInformation.device == nullptr) {
                globalDeviceInformation = {
                    Adapter,
                    DeviceType,
                    hFocusWindow,
                    BehaviorFlags,
                    *pPresentationParameters,
                    *ppReturnedDeviceInterface
                };
            }

            LoaderDirect3DDevice9* device = new LoaderDirect3DDevice9(*ppReturnedDeviceInterface);
            *ppReturnedDeviceInterface = device;

            if (PostCreateDeviceHook != nullptr) {
                PostCreateDeviceHook(dynamic_cast<LoaderDirect3D9*>(this), device, hFocusWindow);
            }

            return hr;
        }

        UINT LoaderDirect3D9Ex::GetAdapterModeCountEx(UINT Adapter, CONST D3DDISPLAYMODEFILTER* pFilter) {
            return this->d3d9->GetAdapterModeCountEx(Adapter, pFilter);
        }

        HRESULT LoaderDirect3D9Ex::EnumAdapterModesEx(UINT Adapter, CONST D3DDISPLAYMODEFILTER* pFilter, UINT Mode, D3DDISPLAYMODEEX* pMode) {
            return this->d3d9->EnumAdapterModesEx(Adapter, pFilter, Mode, pMode);
        }

        HRESULT LoaderDirect3D9Ex::GetAdapterDisplayModeEx(UINT Adapter, D3DDISPLAYMODEEX* pMode, D3DDISPLAYROTATION* pRotation) {
            return this->d3d9->GetAdapterDisplayModeEx(Adapter, pMode, pRotation);
        }

        HRESULT LoaderDirect3D9Ex::CreateDeviceEx(UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, D3DDISPLAYMODEEX* pFullscreenDisplayMode, IDirect3DDevice9Ex** ppReturnedDeviceInterface) {
            HRESULT hr;
            if (PreCreateDeviceExHook != nullptr) {
                hr = PreCreateDeviceExHook(this, Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, pFullscreenDisplayMode, ppReturnedDeviceInterface);
                if (hr != D3D_OK) {
                    // Fail
                    return hr;
                }
            }

            if (!*ppReturnedDeviceInterface) {
                // The pre hook didn't return a device interface, let's get one ourselves
                hr = this->d3d9->CreateDeviceEx(Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, pFullscreenDisplayMode, ppReturnedDeviceInterface);
                if (hr != D3D_OK) {
                    // Fail
                    return hr;
                }
            }

            // Save reference to the created device with parameters
            Direct3DDevice9Id id = {
                Adapter,
                DeviceType,
                hFocusWindow,
                BehaviorFlags,
                *pPresentationParameters
            };
            devices[id] = *ppReturnedDeviceInterface;
            if (globalDeviceInformation.device == nullptr) {
                globalDeviceInformation = {
                    Adapter,
                    DeviceType,
                    hFocusWindow,
                    BehaviorFlags,
                    *pPresentationParameters,
                    *ppReturnedDeviceInterface
                };
            }

            LoaderDirect3DDevice9Ex* device = new LoaderDirect3DDevice9Ex(*ppReturnedDeviceInterface);
            *ppReturnedDeviceInterface = device;

            if (PostCreateDeviceExHook != nullptr) {
                PostCreateDeviceExHook(this, device, hFocusWindow);
            }

            return hr;
        }

        HRESULT LoaderDirect3D9Ex::GetAdapterLUID(UINT Adapter, LUID* pLUID) {
            return this->d3d9->GetAdapterLUID(Adapter, pLUID);
        }

    }
}
