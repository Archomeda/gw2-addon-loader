#pragma once
#include "../windows.h"
#include <d3d9.h>

namespace loader {
    namespace hooks {

        typedef HRESULT(*PreCreateDevice_t)(IDirect3D9* d3d9, UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DDevice9** ppReturnedDeviceInterface);
        typedef void(*PostCreateDevice_t)(IDirect3D9* d3d9, IDirect3DDevice9* pDeviceInterface, HWND hFocusWindow);
        typedef HRESULT(*PreCreateDeviceEx_t)(IDirect3D9Ex* d3d9, UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, D3DDISPLAYMODEEX* pFullscreenDisplayMode, IDirect3DDevice9Ex** ppReturnedDeviceInterface);
        typedef void(*PostCreateDeviceEx_t)(IDirect3D9Ex* d3d9, IDirect3DDevice9Ex* pDeviceInterface, HWND hFocusWindow);

        extern PreCreateDevice_t PreCreateDeviceHook;
        extern PostCreateDevice_t PostCreateDeviceHook;
        extern PreCreateDeviceEx_t PreCreateDeviceExHook;
        extern PostCreateDeviceEx_t PostCreateDeviceExHook;


        class LoaderDirect3D9 : public IDirect3D9
        {
        public:
            LoaderDirect3D9(IDirect3D9* d3d9) : d3d9(d3d9) { }
            LoaderDirect3D9() { }

            /*** IUnknown methods ***/
            STDMETHOD(QueryInterface)(REFIID riid, void** ppvObj);
            STDMETHOD_(ULONG, AddRef)();
            STDMETHOD_(ULONG, Release)();

            /*** IDirect3D9 methods ***/
            STDMETHOD(RegisterSoftwareDevice)(void* pInitializeFunction);
            STDMETHOD_(UINT, GetAdapterCount)();
            STDMETHOD(GetAdapterIdentifier)(UINT Adapter, DWORD Flags, D3DADAPTER_IDENTIFIER9* pIdentifier);
            STDMETHOD_(UINT, GetAdapterModeCount)(UINT Adapter, D3DFORMAT Format);
            STDMETHOD(EnumAdapterModes)(UINT Adapter, D3DFORMAT Format, UINT Mode, D3DDISPLAYMODE* pMode);
            STDMETHOD(GetAdapterDisplayMode)(UINT Adapter, D3DDISPLAYMODE* pMode);
            STDMETHOD(CheckDeviceType)(UINT Adapter, D3DDEVTYPE DevType, D3DFORMAT AdapterFormat, D3DFORMAT BackBufferFormat, BOOL bWindowed);
            STDMETHOD(CheckDeviceFormat)(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, DWORD Usage, D3DRESOURCETYPE RType, D3DFORMAT CheckFormat);
            STDMETHOD(CheckDeviceMultiSampleType)(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT SurfaceFormat, BOOL Windowed, D3DMULTISAMPLE_TYPE MultiSampleType, DWORD* pQualityLevels);
            STDMETHOD(CheckDepthStencilMatch)(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, D3DFORMAT RenderTargetFormat, D3DFORMAT DepthStencilFormat);
            STDMETHOD(CheckDeviceFormatConversion)(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT SourceFormat, D3DFORMAT TargetFormat);
            STDMETHOD(GetDeviceCaps)(UINT Adapter, D3DDEVTYPE DeviceType, D3DCAPS9* pCaps);
            STDMETHOD_(HMONITOR, GetAdapterMonitor)(UINT Adapter);
            STDMETHOD(CreateDevice)(UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DDevice9** ppReturnedDeviceInterface);

        private:
            IDirect3D9* d3d9;
        };

        class LoaderDirect3D9Ex : public IDirect3D9Ex
        {
        public:
            LoaderDirect3D9Ex(IDirect3D9Ex* d3d9Ex) : d3d9(d3d9) { }
            LoaderDirect3D9Ex() { }

            /*** IUnknown methods ***/
            STDMETHOD(QueryInterface)(REFIID riid, void** ppvObj);
            STDMETHOD_(ULONG, AddRef)();
            STDMETHOD_(ULONG, Release)();

            /*** IDirect3D9 methods ***/
            STDMETHOD(RegisterSoftwareDevice)(void* pInitializeFunction);
            STDMETHOD_(UINT, GetAdapterCount)();
            STDMETHOD(GetAdapterIdentifier)(UINT Adapter, DWORD Flags, D3DADAPTER_IDENTIFIER9* pIdentifier);
            STDMETHOD_(UINT, GetAdapterModeCount)(UINT Adapter, D3DFORMAT Format);
            STDMETHOD(EnumAdapterModes)(UINT Adapter, D3DFORMAT Format, UINT Mode, D3DDISPLAYMODE* pMode);
            STDMETHOD(GetAdapterDisplayMode)(UINT Adapter, D3DDISPLAYMODE* pMode);
            STDMETHOD(CheckDeviceType)(UINT Adapter, D3DDEVTYPE DevType, D3DFORMAT AdapterFormat, D3DFORMAT BackBufferFormat, BOOL bWindowed);
            STDMETHOD(CheckDeviceFormat)(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, DWORD Usage, D3DRESOURCETYPE RType, D3DFORMAT CheckFormat);
            STDMETHOD(CheckDeviceMultiSampleType)(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT SurfaceFormat, BOOL Windowed, D3DMULTISAMPLE_TYPE MultiSampleType, DWORD* pQualityLevels);
            STDMETHOD(CheckDepthStencilMatch)(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, D3DFORMAT RenderTargetFormat, D3DFORMAT DepthStencilFormat);
            STDMETHOD(CheckDeviceFormatConversion)(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT SourceFormat, D3DFORMAT TargetFormat);
            STDMETHOD(GetDeviceCaps)(UINT Adapter, D3DDEVTYPE DeviceType, D3DCAPS9* pCaps);
            STDMETHOD_(HMONITOR, GetAdapterMonitor)(UINT Adapter);
            STDMETHOD(CreateDevice)(UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DDevice9** ppReturnedDeviceInterface);
            STDMETHOD_(UINT, GetAdapterModeCountEx)(UINT Adapter, CONST D3DDISPLAYMODEFILTER* pFilter);
            STDMETHOD(EnumAdapterModesEx)(UINT Adapter, CONST D3DDISPLAYMODEFILTER* pFilter, UINT Mode, D3DDISPLAYMODEEX* pMode);
            STDMETHOD(GetAdapterDisplayModeEx)(UINT Adapter, D3DDISPLAYMODEEX* pMode, D3DDISPLAYROTATION* pRotation);
            STDMETHOD(CreateDeviceEx)(UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, D3DDISPLAYMODEEX* pFullscreenDisplayMode, IDirect3DDevice9Ex** ppReturnedDeviceInterface);
            STDMETHOD(GetAdapterLUID)(UINT Adapter, LUID* pLUID);

        private:
            IDirect3D9Ex* d3d9;
        };

    }
}
