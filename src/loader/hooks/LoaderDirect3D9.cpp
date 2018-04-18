#include "LoaderDirect3D9.h"
#include "LoaderDirect3DDevice9.h"

namespace loader {
    namespace hooks {

        PreCreateDevice_t* PreCreateDeviceHook = nullptr;
        PostCreateDevice_t* PostCreateDeviceHook = nullptr;
        PreCreateDeviceEx_t* PreCreateDeviceExHook = nullptr;
        PostCreateDeviceEx_t* PostCreateDeviceExHook = nullptr;


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
                hr = PreCreateDeviceHook(this->d3d9, Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, ppReturnedDeviceInterface);
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

            *ppReturnedDeviceInterface = new LoaderDirect3DDevice9(*ppReturnedDeviceInterface);

            if (PostCreateDeviceHook != nullptr) {
                PostCreateDeviceHook(this->d3d9, *ppReturnedDeviceInterface, hFocusWindow);
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
            return this->d3d9->CreateDevice(Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, ppReturnedDeviceInterface);
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
                hr = PreCreateDeviceExHook(this->d3d9, Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, pFullscreenDisplayMode, ppReturnedDeviceInterface);
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

            *ppReturnedDeviceInterface = new LoaderDirect3DDevice9Ex(*ppReturnedDeviceInterface);

            if (PostCreateDeviceExHook != nullptr) {
                PostCreateDeviceExHook(this->d3d9, *ppReturnedDeviceInterface, hFocusWindow);
            }

            return hr;
        }

        HRESULT LoaderDirect3D9Ex::GetAdapterLUID(UINT Adapter, LUID* pLUID) {
            return this->d3d9->GetAdapterLUID(Adapter, pLUID);
        }

    }
}
