#pragma once
#include "windows.h"
#include <d3d9.h>

// D3D9 Classes
typedef IDirect3D9* (WINAPI* Direct3DCreate9_t)(UINT SDKVersion);
typedef IDirect3D9Ex* (WINAPI* Direct3DCreate9Ex_t)(UINT SDKVersion);

// IUnknown
typedef HRESULT(WINAPI *QueryInterface_t)(IUnknown* _this, REFIID riid, void** ppvObj);
typedef ULONG(WINAPI *AddRef_t)(IUnknown* _this);
typedef ULONG(WINAPI *Release_t)(IUnknown* _this);

// IDirect3D9
typedef HRESULT(WINAPI *RegisterSoftwareDevice_t)(IDirect3D9* _this, void* pInitializeFunction);
typedef UINT(WINAPI *GetAdapterCount_t)(IDirect3D9* _this);
typedef HRESULT(WINAPI *GetAdapterIdentifier_t)(IDirect3D9* _this, UINT Adapter, DWORD Flags, D3DADAPTER_IDENTIFIER9* pIdentifier);
typedef UINT(WINAPI *GetAdapterModeCount_t)(IDirect3D9* _this, UINT Adapter, D3DFORMAT Format);
typedef HRESULT(WINAPI *EnumAdapterModes_t)(IDirect3D9* _this, UINT Adapter, D3DFORMAT Format, UINT Mode, D3DDISPLAYMODE* pMode);
typedef HRESULT(WINAPI *GetAdapterDisplayMode_t)(IDirect3D9* _this, UINT Adapter, D3DDISPLAYMODE* pMode);
typedef HRESULT(WINAPI *CheckDeviceType_t)(IDirect3D9* _this, UINT Adapter, D3DDEVTYPE DevType, D3DFORMAT AdapterFormat, D3DFORMAT BackBufferFormat, BOOL bWindowed);
typedef HRESULT(WINAPI *CheckDeviceFormat_t)(IDirect3D9* _this, UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, DWORD Usage, D3DRESOURCETYPE RType, D3DFORMAT CheckFormat);
typedef HRESULT(WINAPI *CheckDeviceMultiSampleType_t)(IDirect3D9* _this, UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT SurfaceFormat, BOOL Windowed, D3DMULTISAMPLE_TYPE MultiSampleType, DWORD* pQualityLevels);
typedef HRESULT(WINAPI *CheckDepthStencilMatch_t)(IDirect3D9* _this, UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, D3DFORMAT RenderTargetFormat, D3DFORMAT DepthStencilFormat);
typedef HRESULT(WINAPI *CheckDeviceFormatConversion_t)(IDirect3D9* _this, UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT SourceFormat, D3DFORMAT TargetFormat);
typedef HRESULT(WINAPI *GetDeviceCaps_t)(IDirect3D9* _this, UINT Adapter, D3DDEVTYPE DeviceType, D3DCAPS9* pCaps);
typedef HMONITOR(WINAPI *GetAdapterMonitor_t)(IDirect3D9* _this, UINT Adapter);
typedef HRESULT(WINAPI *CreateDevice_t)(IDirect3D9* _this, UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DDevice9** ppReturnedDeviceInterface);

// IDirect3D9Ex
typedef UINT(WINAPI *GetAdapterModeCountEx_t)(IDirect3D9Ex* _this, UINT Adapter, CONST D3DDISPLAYMODEFILTER* pFilter);
typedef HRESULT(WINAPI *EnumAdapterModesEx_t)(IDirect3D9Ex* _this, UINT Adapter, CONST D3DDISPLAYMODEFILTER* pFilter, UINT Mode, D3DDISPLAYMODEEX* pMode);
typedef HRESULT(WINAPI *GetAdapterDisplayModeEx_t)(IDirect3D9Ex* _this, UINT Adapter, D3DDISPLAYMODEEX* pMode, D3DDISPLAYROTATION* pRotation);
typedef HRESULT(WINAPI *CreateDeviceEx_t)(IDirect3D9Ex* _this, UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, D3DDISPLAYMODEEX* pFullscreenDisplayMode, IDirect3DDevice9Ex** ppReturnedDeviceInterface);
typedef HRESULT(WINAPI *GetAdapterLUID_t)(IDirect3D9Ex* _this, UINT Adapter, LUID* pLUID);
