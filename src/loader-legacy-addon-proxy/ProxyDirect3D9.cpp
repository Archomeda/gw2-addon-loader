#include "ProxyDirect3D9.h"

HRESULT ProxyDirect3D9::QueryInterface(REFIID riid, void** ppvObj) {
    return this->d3d9->QueryInterface(riid, ppvObj);
}

ULONG ProxyDirect3D9::AddRef() {
    return this->d3d9->AddRef();
}

ULONG ProxyDirect3D9::Release() {
    return this->d3d9->Release();
}

HRESULT ProxyDirect3D9::RegisterSoftwareDevice(void* pInitializeFunction) {
    return this->d3d9->RegisterSoftwareDevice(pInitializeFunction);
}

UINT ProxyDirect3D9::GetAdapterCount() {
    return this->d3d9->GetAdapterCount();
}

HRESULT ProxyDirect3D9::GetAdapterIdentifier(UINT Adapter, DWORD Flags, D3DADAPTER_IDENTIFIER9* pIdentifier) {
    return this->d3d9->GetAdapterIdentifier(Adapter, Flags, pIdentifier);
}

UINT ProxyDirect3D9::GetAdapterModeCount(UINT Adapter, D3DFORMAT Format) {
    return this->d3d9->GetAdapterModeCount(Adapter, Format);
}

HRESULT ProxyDirect3D9::EnumAdapterModes(UINT Adapter, D3DFORMAT Format, UINT Mode, D3DDISPLAYMODE* pMode) {
    return this->d3d9->EnumAdapterModes(Adapter, Format, Mode, pMode);
}

HRESULT ProxyDirect3D9::GetAdapterDisplayMode(UINT Adapter, D3DDISPLAYMODE *pMode) {
    return this->d3d9->GetAdapterDisplayMode(Adapter, pMode);
}

HRESULT ProxyDirect3D9::CheckDeviceType(UINT Adapter, D3DDEVTYPE CheckType, D3DFORMAT DisplayFormat, D3DFORMAT BackBufferFormat, BOOL Windowed) {
    return this->d3d9->CheckDeviceType(Adapter, CheckType, DisplayFormat, BackBufferFormat, Windowed);
}

HRESULT ProxyDirect3D9::CheckDeviceFormat(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, DWORD Usage, D3DRESOURCETYPE RType, D3DFORMAT CheckFormat) {
    return this->d3d9->CheckDeviceFormat(Adapter, DeviceType, AdapterFormat, Usage, RType, CheckFormat);
}

HRESULT ProxyDirect3D9::CheckDeviceMultiSampleType(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT SurfaceFormat, BOOL Windowed, D3DMULTISAMPLE_TYPE MultiSampleType, DWORD* pQualityLevels) {
    return this->d3d9->CheckDeviceMultiSampleType(Adapter, DeviceType, SurfaceFormat, Windowed, MultiSampleType, pQualityLevels);
}

HRESULT ProxyDirect3D9::CheckDepthStencilMatch(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, D3DFORMAT RenderTargetFormat, D3DFORMAT DepthStencilFormat) {
    return this->d3d9->CheckDepthStencilMatch(Adapter, DeviceType, AdapterFormat, RenderTargetFormat, DepthStencilFormat);
}

HRESULT ProxyDirect3D9::CheckDeviceFormatConversion(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT SourceFormat, D3DFORMAT TargetFormat) {
    return this->d3d9->CheckDeviceFormatConversion(Adapter, DeviceType, SourceFormat, TargetFormat);
}

HRESULT ProxyDirect3D9::GetDeviceCaps(UINT Adapter, D3DDEVTYPE DeviceType, D3DCAPS9* pCaps) {
    return this->d3d9->GetDeviceCaps(Adapter, DeviceType, pCaps);
}

HMONITOR ProxyDirect3D9::GetAdapterMonitor(UINT Adapter) {
    return this->d3d9->GetAdapterMonitor(Adapter);
}

HRESULT ProxyDirect3D9::CreateDevice(UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DDevice9** ppReturnedDeviceInterface) {
    if (this->loaderApi->CreateProxyDirect3DDevice9 == nullptr) {
        return D3DERR_NOTFOUND;
    }

    // At this point, the device has already been created by the loader,
    // so we have to obtain the already created device from that.
    if (this->loaderApi->GetDirect3DDevice != nullptr) {
        *ppReturnedDeviceInterface = this->loaderApi->GetDirect3DDevice(Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters);
    }

    HRESULT hr = D3D_OK;
    if (*ppReturnedDeviceInterface == nullptr) {
        // Somehow we didn't get the created device, redirect to original D3D9 as a catch
        hr = this->d3d9->CreateDevice(Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, ppReturnedDeviceInterface);
    }

    *ppReturnedDeviceInterface = this->loaderApi->CreateProxyDirect3DDevice9(*ppReturnedDeviceInterface);
    return hr;
}


HRESULT ProxyDirect3D9Ex::QueryInterface(REFIID riid, void** ppvObj) {
    return this->d3d9->QueryInterface(riid, ppvObj);
}

ULONG ProxyDirect3D9Ex::AddRef() {
    return this->d3d9->AddRef();
}

ULONG ProxyDirect3D9Ex::Release() {
    return this->d3d9->Release();
}

HRESULT ProxyDirect3D9Ex::RegisterSoftwareDevice(void* pInitializeFunction) {
    return this->d3d9->RegisterSoftwareDevice(pInitializeFunction);
}

UINT ProxyDirect3D9Ex::GetAdapterCount() {
    return this->d3d9->GetAdapterCount();
}

HRESULT ProxyDirect3D9Ex::GetAdapterIdentifier(UINT Adapter, DWORD Flags, D3DADAPTER_IDENTIFIER9* pIdentifier) {
    return this->d3d9->GetAdapterIdentifier(Adapter, Flags, pIdentifier);
}

UINT ProxyDirect3D9Ex::GetAdapterModeCount(UINT Adapter, D3DFORMAT Format) {
    return this->d3d9->GetAdapterModeCount(Adapter, Format);
}

HRESULT ProxyDirect3D9Ex::EnumAdapterModes(UINT Adapter, D3DFORMAT Format, UINT Mode, D3DDISPLAYMODE* pMode) {
    return this->d3d9->EnumAdapterModes(Adapter, Format, Mode, pMode);
}

HRESULT ProxyDirect3D9Ex::GetAdapterDisplayMode(UINT Adapter, D3DDISPLAYMODE *pMode) {
    return this->d3d9->GetAdapterDisplayMode(Adapter, pMode);
}

HRESULT ProxyDirect3D9Ex::CheckDeviceType(UINT Adapter, D3DDEVTYPE CheckType, D3DFORMAT DisplayFormat, D3DFORMAT BackBufferFormat, BOOL Windowed) {
    return this->d3d9->CheckDeviceType(Adapter, CheckType, DisplayFormat, BackBufferFormat, Windowed);
}

HRESULT ProxyDirect3D9Ex::CheckDeviceFormat(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, DWORD Usage, D3DRESOURCETYPE RType, D3DFORMAT CheckFormat) {
    return this->d3d9->CheckDeviceFormat(Adapter, DeviceType, AdapterFormat, Usage, RType, CheckFormat);
}

HRESULT ProxyDirect3D9Ex::CheckDeviceMultiSampleType(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT SurfaceFormat, BOOL Windowed, D3DMULTISAMPLE_TYPE MultiSampleType, DWORD* pQualityLevels) {
    return this->d3d9->CheckDeviceMultiSampleType(Adapter, DeviceType, SurfaceFormat, Windowed, MultiSampleType, pQualityLevels);
}

HRESULT ProxyDirect3D9Ex::CheckDepthStencilMatch(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, D3DFORMAT RenderTargetFormat, D3DFORMAT DepthStencilFormat) {
    return this->d3d9->CheckDepthStencilMatch(Adapter, DeviceType, AdapterFormat, RenderTargetFormat, DepthStencilFormat);
}

HRESULT ProxyDirect3D9Ex::CheckDeviceFormatConversion(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT SourceFormat, D3DFORMAT TargetFormat) {
    return this->d3d9->CheckDeviceFormatConversion(Adapter, DeviceType, SourceFormat, TargetFormat);
}

HRESULT ProxyDirect3D9Ex::GetDeviceCaps(UINT Adapter, D3DDEVTYPE DeviceType, D3DCAPS9* pCaps) {
    return this->d3d9->GetDeviceCaps(Adapter, DeviceType, pCaps);
}

HMONITOR ProxyDirect3D9Ex::GetAdapterMonitor(UINT Adapter) {
    return this->d3d9->GetAdapterMonitor(Adapter);
}

HRESULT ProxyDirect3D9Ex::CreateDevice(UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DDevice9** ppReturnedDeviceInterface) {
    return this->d3d9->CreateDevice(Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, ppReturnedDeviceInterface);
}

UINT ProxyDirect3D9Ex::GetAdapterModeCountEx(UINT Adapter, CONST D3DDISPLAYMODEFILTER* pFilter) {
    return this->d3d9->GetAdapterModeCountEx(Adapter, pFilter);
}

HRESULT ProxyDirect3D9Ex::EnumAdapterModesEx(UINT Adapter, CONST D3DDISPLAYMODEFILTER* pFilter, UINT Mode, D3DDISPLAYMODEEX* pMode) {
    return this->d3d9->EnumAdapterModesEx(Adapter, pFilter, Mode, pMode);
}

HRESULT ProxyDirect3D9Ex::GetAdapterDisplayModeEx(UINT Adapter, D3DDISPLAYMODEEX* pMode, D3DDISPLAYROTATION* pRotation) {
    return this->d3d9->GetAdapterDisplayModeEx(Adapter, pMode, pRotation);
}

HRESULT ProxyDirect3D9Ex::CreateDeviceEx(UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, D3DDISPLAYMODEEX* pFullscreenDisplayMode, IDirect3DDevice9Ex** ppReturnedDeviceInterface) {
    if (this->loaderApi->CreateProxyDirect3DDevice9Ex == nullptr) {
        return D3DERR_NOTFOUND;
    }

    // At this point, the device has already been created by the loader,
    // so we have to obtain the already created device from that.
    // Since Guild Wars 2 doesn't use CreateDeviceEx, this implicitly calls CreateDevice instead.
    // If Guild Wars 2 (or any legacy add-on) starts using CreateDeviceEx and has issues, update this to reflect it
    if (this->loaderApi->GetDirect3DDevice != nullptr) {
        *ppReturnedDeviceInterface = reinterpret_cast<IDirect3DDevice9Ex*>(this->loaderApi->GetDirect3DDevice(Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters));
    }

    HRESULT hr;
    if (*ppReturnedDeviceInterface == nullptr) {
        // Somehow we didn't get the created device, redirect to original D3D9 as a catch
        hr = this->d3d9->CreateDeviceEx(Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, pFullscreenDisplayMode, ppReturnedDeviceInterface);
    }

    *ppReturnedDeviceInterface = this->loaderApi->CreateProxyDirect3DDevice9Ex(*ppReturnedDeviceInterface);
    return hr;
}

HRESULT ProxyDirect3D9Ex::GetAdapterLUID(UINT Adapter, LUID* pLUID) {
    return this->d3d9->GetAdapterLUID(Adapter, pLUID);
}

