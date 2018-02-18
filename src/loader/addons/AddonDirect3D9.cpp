#include "AddonDirect3D9.h"
#include "../d3d9types.h"

namespace loader {
    namespace addons {

        AddonDirect3D9::AddonDirect3D9(IDirect3D9* d3d9) {
            this->vft = GetVftD3D9(d3d9);
        }

        HRESULT AddonDirect3D9::QueryInterface(REFIID riid, void** ppvObj) {
            return reinterpret_cast<QueryInterface_t>(this->vft.QueryInterface)(this, riid, ppvObj);
        }

        ULONG AddonDirect3D9::AddRef() {
            return reinterpret_cast<AddRef_t>(this->vft.AddRef)(this);
        }

        ULONG AddonDirect3D9::Release() {
            return reinterpret_cast<Release_t>(this->vft.Release)(this);
        }

        HRESULT AddonDirect3D9::RegisterSoftwareDevice(void* pInitializeFunction) {
            return reinterpret_cast<RegisterSoftwareDevice_t>(this->vft.RegisterSoftwareDevice)(this, pInitializeFunction);
        }

        UINT AddonDirect3D9::GetAdapterCount() {
            return reinterpret_cast<GetAdapterCount_t>(this->vft.GetAdapterCount)(this);
        }

        HRESULT AddonDirect3D9::GetAdapterIdentifier(UINT Adapter, DWORD Flags, D3DADAPTER_IDENTIFIER9* pIdentifier) {
            return reinterpret_cast<GetAdapterIdentifier_t>(this->vft.GetAdapterIdentifier)(this, Adapter, Flags, pIdentifier);
        }

        UINT AddonDirect3D9::GetAdapterModeCount(THIS_ UINT Adapter, D3DFORMAT Format) {
            return reinterpret_cast<GetAdapterModeCount_t>(this->vft.GetAdapterModeCount)(this, Adapter, Format);
        }

        HRESULT AddonDirect3D9::EnumAdapterModes(THIS_ UINT Adapter, D3DFORMAT Format, UINT Mode, D3DDISPLAYMODE* pMode) {
            return reinterpret_cast<EnumAdapterModes_t>(this->vft.EnumAdapterModes)(this, Adapter, Format, Mode, pMode);
        }

        HRESULT AddonDirect3D9::GetAdapterDisplayMode(UINT Adapter, D3DDISPLAYMODE *pMode) {
            return reinterpret_cast<GetAdapterDisplayMode_t>(this->vft.GetAdapterDisplayMode)(this, Adapter, pMode);
        }

        HRESULT AddonDirect3D9::CheckDeviceType(UINT Adapter, D3DDEVTYPE CheckType, D3DFORMAT DisplayFormat, D3DFORMAT BackBufferFormat, BOOL Windowed) {
            return reinterpret_cast<CheckDeviceType_t>(this->vft.CheckDeviceType)(this, Adapter, CheckType, DisplayFormat, BackBufferFormat, Windowed);
        }

        HRESULT AddonDirect3D9::CheckDeviceFormat(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, DWORD Usage, D3DRESOURCETYPE RType, D3DFORMAT CheckFormat) {
            return reinterpret_cast<CheckDeviceFormat_t>(this->vft.CheckDeviceFormat)(this, Adapter, DeviceType, AdapterFormat, Usage, RType, CheckFormat);
        }

        HRESULT AddonDirect3D9::CheckDeviceMultiSampleType(THIS_ UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT SurfaceFormat, BOOL Windowed, D3DMULTISAMPLE_TYPE MultiSampleType, DWORD* pQualityLevels) {
            return reinterpret_cast<CheckDeviceMultiSampleType_t>(this->vft.CheckDeviceMultiSampleType)(this, Adapter, DeviceType, SurfaceFormat, Windowed, MultiSampleType, pQualityLevels);
        }

        HRESULT AddonDirect3D9::CheckDepthStencilMatch(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, D3DFORMAT RenderTargetFormat, D3DFORMAT DepthStencilFormat) {
            return reinterpret_cast<CheckDepthStencilMatch_t>(this->vft.CheckDepthStencilMatch)(this, Adapter, DeviceType, AdapterFormat, RenderTargetFormat, DepthStencilFormat);
        }

        HRESULT AddonDirect3D9::CheckDeviceFormatConversion(THIS_ UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT SourceFormat, D3DFORMAT TargetFormat) {
            return reinterpret_cast<CheckDeviceFormatConversion_t>(this->vft.CheckDeviceFormatConversion)(this, Adapter, DeviceType, SourceFormat, TargetFormat);
        }

        HRESULT AddonDirect3D9::GetDeviceCaps(UINT Adapter, D3DDEVTYPE DeviceType, D3DCAPS9* pCaps) {
            return reinterpret_cast<GetDeviceCaps_t>(this->vft.GetDeviceCaps)(this, Adapter, DeviceType, pCaps);
        }

        HMONITOR AddonDirect3D9::GetAdapterMonitor(UINT Adapter) {
            return reinterpret_cast<GetAdapterMonitor_t>(this->vft.GetAdapterMonitor)(this, Adapter);
        }

        HRESULT AddonDirect3D9::CreateDevice(UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DDevice9** ppReturnedDeviceInterface) {
            return reinterpret_cast<CreateDevice_t>(this->vft.CreateDevice)(this, Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, ppReturnedDeviceInterface);
        }


        AddonDirect3D9Ex::AddonDirect3D9Ex(IDirect3D9Ex* d3d9Ex) {
            this->vft = GetVftD3D9Ex(d3d9Ex);
        }

        HRESULT AddonDirect3D9Ex::QueryInterface(REFIID riid, void** ppvObj) {
            return reinterpret_cast<QueryInterface_t>(this->vft.QueryInterface)(this, riid, ppvObj);
        }

        ULONG AddonDirect3D9Ex::AddRef() {
            return reinterpret_cast<AddRef_t>(this->vft.AddRef)(this);
        }

        ULONG AddonDirect3D9Ex::Release() {
            return reinterpret_cast<Release_t>(this->vft.Release)(this);
        }

        HRESULT AddonDirect3D9Ex::RegisterSoftwareDevice(void* pInitializeFunction) {
            return reinterpret_cast<RegisterSoftwareDevice_t>(this->vft.RegisterSoftwareDevice)(this, pInitializeFunction);
        }

        UINT AddonDirect3D9Ex::GetAdapterCount() {
            return reinterpret_cast<GetAdapterCount_t>(this->vft.GetAdapterCount)(this);
        }

        HRESULT AddonDirect3D9Ex::GetAdapterIdentifier(UINT Adapter, DWORD Flags, D3DADAPTER_IDENTIFIER9* pIdentifier) {
            return reinterpret_cast<GetAdapterIdentifier_t>(this->vft.GetAdapterIdentifier)(this, Adapter, Flags, pIdentifier);
        }

        UINT AddonDirect3D9Ex::GetAdapterModeCount(THIS_ UINT Adapter, D3DFORMAT Format) {
            return reinterpret_cast<GetAdapterModeCount_t>(this->vft.GetAdapterModeCount)(this, Adapter, Format);
        }
        HRESULT AddonDirect3D9Ex::EnumAdapterModes(THIS_ UINT Adapter, D3DFORMAT Format, UINT Mode, D3DDISPLAYMODE* pMode) {
            return reinterpret_cast<EnumAdapterModes_t>(this->vft.EnumAdapterModes)(this, Adapter, Format, Mode, pMode);
        }

        HRESULT AddonDirect3D9Ex::GetAdapterDisplayMode(UINT Adapter, D3DDISPLAYMODE *pMode) {
            return reinterpret_cast<GetAdapterDisplayMode_t>(this->vft.GetAdapterDisplayMode)(this, Adapter, pMode);
        }

        HRESULT AddonDirect3D9Ex::CheckDeviceType(UINT Adapter, D3DDEVTYPE CheckType, D3DFORMAT DisplayFormat, D3DFORMAT BackBufferFormat, BOOL Windowed) {
            return reinterpret_cast<CheckDeviceType_t>(this->vft.CheckDeviceType)(this, Adapter, CheckType, DisplayFormat, BackBufferFormat, Windowed);
        }

        HRESULT AddonDirect3D9Ex::CheckDeviceFormat(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, DWORD Usage, D3DRESOURCETYPE RType, D3DFORMAT CheckFormat) {
            return reinterpret_cast<CheckDeviceFormat_t>(this->vft.CheckDeviceFormat)(this, Adapter, DeviceType, AdapterFormat, Usage, RType, CheckFormat);
        }

        HRESULT AddonDirect3D9Ex::CheckDeviceMultiSampleType(THIS_ UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT SurfaceFormat, BOOL Windowed, D3DMULTISAMPLE_TYPE MultiSampleType, DWORD* pQualityLevels) {
            return reinterpret_cast<CheckDeviceMultiSampleType_t>(this->vft.CheckDeviceMultiSampleType)(this, Adapter, DeviceType, SurfaceFormat, Windowed, MultiSampleType, pQualityLevels);
        }

        HRESULT AddonDirect3D9Ex::CheckDepthStencilMatch(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, D3DFORMAT RenderTargetFormat, D3DFORMAT DepthStencilFormat) {
            return reinterpret_cast<CheckDepthStencilMatch_t>(this->vft.CheckDepthStencilMatch)(this, Adapter, DeviceType, AdapterFormat, RenderTargetFormat, DepthStencilFormat);
        }

        HRESULT AddonDirect3D9Ex::CheckDeviceFormatConversion(THIS_ UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT SourceFormat, D3DFORMAT TargetFormat) {
            return reinterpret_cast<CheckDeviceFormatConversion_t>(this->vft.CheckDeviceFormatConversion)(this, Adapter, DeviceType, SourceFormat, TargetFormat);
        }

        HRESULT AddonDirect3D9Ex::GetDeviceCaps(UINT Adapter, D3DDEVTYPE DeviceType, D3DCAPS9* pCaps) {
            return reinterpret_cast<GetDeviceCaps_t>(this->vft.GetDeviceCaps)(this, Adapter, DeviceType, pCaps);
        }

        HMONITOR AddonDirect3D9Ex::GetAdapterMonitor(UINT Adapter) {
            return reinterpret_cast<GetAdapterMonitor_t>(this->vft.GetAdapterMonitor)(this, Adapter);
        }

        HRESULT AddonDirect3D9Ex::CreateDevice(UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DDevice9** ppReturnedDeviceInterface) {
            return reinterpret_cast<CreateDevice_t>(this->vft.CreateDevice)(this, Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, ppReturnedDeviceInterface);
        }

        UINT AddonDirect3D9Ex::GetAdapterModeCountEx(UINT Adapter, CONST D3DDISPLAYMODEFILTER* pFilter) {
            return reinterpret_cast<GetAdapterModeCountEx_t>(this->vft.GetAdapterModeCountEx)(this, Adapter, pFilter);
        }

        HRESULT AddonDirect3D9Ex::EnumAdapterModesEx(UINT Adapter, CONST D3DDISPLAYMODEFILTER* pFilter, UINT Mode, D3DDISPLAYMODEEX* pMode) {
            return reinterpret_cast<EnumAdapterModesEx_t>(this->vft.EnumAdapterModesEx)(this, Adapter, pFilter, Mode, pMode);
        }

        HRESULT AddonDirect3D9Ex::GetAdapterDisplayModeEx(UINT Adapter, D3DDISPLAYMODEEX* pMode, D3DDISPLAYROTATION* pRotation) {
            return reinterpret_cast<GetAdapterDisplayModeEx_t>(this->vft.GetAdapterDisplayModeEx)(this, Adapter, pMode, pRotation);
        }

        HRESULT AddonDirect3D9Ex::CreateDeviceEx(UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, D3DDISPLAYMODEEX* pFullscreenDisplayMode, IDirect3DDevice9Ex** ppReturnedDeviceInterface) {
            return reinterpret_cast<CreateDeviceEx_t>(this->vft.CreateDeviceEx)(this, Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, pFullscreenDisplayMode, ppReturnedDeviceInterface);
        }

        HRESULT AddonDirect3D9Ex::GetAdapterLUID(UINT Adapter, LUID* pLUID) {
            return reinterpret_cast<GetAdapterLUID_t>(this->vft.GetAdapterLUID)(this, Adapter, pLUID);
        }

    }
}
