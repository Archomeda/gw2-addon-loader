/**
This is an include file specifically for the legacy proxy DLL for the GW2 Addon Loader.
*/

#pragma once
#include <d3d9.h>

#define GW2PROXY_CALL __stdcall

typedef IDirect3DDevice9* (GW2PROXY_CALL GetDirect3DDevice9_t)(UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters);
typedef IDirect3DDevice9* (GW2PROXY_CALL CreateProxyDirect3DDevice9_t)(IDirect3DDevice9* sourceDev);
typedef IDirect3DDevice9Ex* (GW2PROXY_CALL CreateProxyDirect3DDevice9Ex_t)(IDirect3DDevice9Ex* sourceDev);

struct GW2LegacyAddonProxyAPI {
    GetDirect3DDevice9_t* GetDirect3DDevice;
    CreateProxyDirect3DDevice9_t* CreateProxyDirect3DDevice9;
    CreateProxyDirect3DDevice9Ex_t* CreateProxyDirect3DDevice9Ex;
};


typedef bool(GW2PROXY_CALL ProxyInitialize_t)(const GW2LegacyAddonProxyAPI* const api);
#define GW2PROXY_DLL_Initialize "ProxyInitialize"
typedef bool(GW2PROXY_CALL ProxyRelease_t)();
#define GW2PROXY_DLL_Release "ProxyRelease"


/**
Ensure compiler errors when exported addon functions are wrong.
*/
#ifndef _GW2ADDON_IMPORTS
bool GW2PROXY_CALL ProxyInitialize(const GW2LegacyAddonProxyAPI* const api);
bool GW2PROXY_CALL ProxyRelease();
#endif
