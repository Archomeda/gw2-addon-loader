#pragma once
#include <d3d9.h>
#include "../hooks/vftable.h"

namespace loader {
    namespace addons {

        class AddonDirect3D9 : public IDirect3D9
        {
        public:
            AddonDirect3D9(IDirect3D9* d3d9);
            AddonDirect3D9() { }

            /*** IUnknown methods ***/
            STDMETHOD(QueryInterface)(THIS_ REFIID riid, void** ppvObj);
            STDMETHOD_(ULONG, AddRef)(THIS);
            STDMETHOD_(ULONG, Release)(THIS);

            /*** IDirect3D9 methods ***/
            STDMETHOD(RegisterSoftwareDevice)(THIS_ void* pInitializeFunction);
            STDMETHOD_(UINT, GetAdapterCount)(THIS);
            STDMETHOD(GetAdapterIdentifier)(THIS_ UINT Adapter, DWORD Flags, D3DADAPTER_IDENTIFIER9* pIdentifier);
            STDMETHOD_(UINT, GetAdapterModeCount)(THIS_ UINT Adapter, D3DFORMAT Format);
            STDMETHOD(EnumAdapterModes)(THIS_ UINT Adapter, D3DFORMAT Format, UINT Mode, D3DDISPLAYMODE* pMode);
            STDMETHOD(GetAdapterDisplayMode)(THIS_ UINT Adapter, D3DDISPLAYMODE* pMode);
            STDMETHOD(CheckDeviceType)(THIS_ UINT Adapter, D3DDEVTYPE DevType, D3DFORMAT AdapterFormat, D3DFORMAT BackBufferFormat, BOOL bWindowed);
            STDMETHOD(CheckDeviceFormat)(THIS_ UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, DWORD Usage, D3DRESOURCETYPE RType, D3DFORMAT CheckFormat);
            STDMETHOD(CheckDeviceMultiSampleType)(THIS_ UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT SurfaceFormat, BOOL Windowed, D3DMULTISAMPLE_TYPE MultiSampleType, DWORD* pQualityLevels);
            STDMETHOD(CheckDepthStencilMatch)(THIS_ UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, D3DFORMAT RenderTargetFormat, D3DFORMAT DepthStencilFormat);
            STDMETHOD(CheckDeviceFormatConversion)(THIS_ UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT SourceFormat, D3DFORMAT TargetFormat);
            STDMETHOD(GetDeviceCaps)(THIS_ UINT Adapter, D3DDEVTYPE DeviceType, D3DCAPS9* pCaps);
            STDMETHOD_(HMONITOR, GetAdapterMonitor)(THIS_ UINT Adapter);
            STDMETHOD(CreateDevice)(THIS_ UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DDevice9** ppReturnedDeviceInterface);

        private:
            D3D9_vft vft = { 0 };
        };

        class AddonDirect3D9Ex : public IDirect3D9Ex
        {
        public:
            AddonDirect3D9Ex(IDirect3D9Ex* d3d9Ex);
            AddonDirect3D9Ex() { }

            /*** IUnknown methods ***/
            STDMETHOD(QueryInterface)(THIS_ REFIID riid, void** ppvObj);
            STDMETHOD_(ULONG, AddRef)(THIS);
            STDMETHOD_(ULONG, Release)(THIS);

            /*** IDirect3D9 methods ***/
            STDMETHOD(RegisterSoftwareDevice)(THIS_ void* pInitializeFunction);
            STDMETHOD_(UINT, GetAdapterCount)(THIS);
            STDMETHOD(GetAdapterIdentifier)(THIS_ UINT Adapter, DWORD Flags, D3DADAPTER_IDENTIFIER9* pIdentifier);
            STDMETHOD_(UINT, GetAdapterModeCount)(THIS_ UINT Adapter, D3DFORMAT Format);
            STDMETHOD(EnumAdapterModes)(THIS_ UINT Adapter, D3DFORMAT Format, UINT Mode, D3DDISPLAYMODE* pMode);
            STDMETHOD(GetAdapterDisplayMode)(THIS_ UINT Adapter, D3DDISPLAYMODE* pMode);
            STDMETHOD(CheckDeviceType)(THIS_ UINT Adapter, D3DDEVTYPE DevType, D3DFORMAT AdapterFormat, D3DFORMAT BackBufferFormat, BOOL bWindowed);
            STDMETHOD(CheckDeviceFormat)(THIS_ UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, DWORD Usage, D3DRESOURCETYPE RType, D3DFORMAT CheckFormat);
            STDMETHOD(CheckDeviceMultiSampleType)(THIS_ UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT SurfaceFormat, BOOL Windowed, D3DMULTISAMPLE_TYPE MultiSampleType, DWORD* pQualityLevels);
            STDMETHOD(CheckDepthStencilMatch)(THIS_ UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, D3DFORMAT RenderTargetFormat, D3DFORMAT DepthStencilFormat);
            STDMETHOD(CheckDeviceFormatConversion)(THIS_ UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT SourceFormat, D3DFORMAT TargetFormat);
            STDMETHOD(GetDeviceCaps)(THIS_ UINT Adapter, D3DDEVTYPE DeviceType, D3DCAPS9* pCaps);
            STDMETHOD_(HMONITOR, GetAdapterMonitor)(THIS_ UINT Adapter);
            STDMETHOD(CreateDevice)(THIS_ UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DDevice9** ppReturnedDeviceInterface);
            STDMETHOD_(UINT, GetAdapterModeCountEx)(THIS_ UINT Adapter, CONST D3DDISPLAYMODEFILTER* pFilter);
            STDMETHOD(EnumAdapterModesEx)(THIS_ UINT Adapter, CONST D3DDISPLAYMODEFILTER* pFilter, UINT Mode, D3DDISPLAYMODEEX* pMode);
            STDMETHOD(GetAdapterDisplayModeEx)(THIS_ UINT Adapter, D3DDISPLAYMODEEX* pMode, D3DDISPLAYROTATION* pRotation);
            STDMETHOD(CreateDeviceEx)(THIS_ UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, D3DDISPLAYMODEEX* pFullscreenDisplayMode, IDirect3DDevice9Ex** ppReturnedDeviceInterface);
            STDMETHOD(GetAdapterLUID)(THIS_ UINT Adapter, LUID* pLUID);

        private:
            D3D9Ex_vft vft = { 0 };
        };

    }
}
