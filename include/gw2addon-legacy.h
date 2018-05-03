/**
This is an include file specifically for the legacy proxy DLL for the GW2 Addon Loader.
*/

#pragma once
#include <d3d9.h>

#define GW2PROXY_CALL __stdcall

typedef IDirect3DDevice9* (GW2PROXY_CALL GetDirect3DDevice9_t)(UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters);
typedef IDirect3DDevice9* (GW2PROXY_CALL CreateProxyDirect3DDevice9_t)(IDirect3DDevice9* sourceDev);
typedef IDirect3DDevice9Ex* (GW2PROXY_CALL CreateProxyDirect3DDevice9Ex_t)(IDirect3DDevice9Ex* sourceDev);

typedef struct {
    const char* name;
    const char* description;
} GW2LegacyAddonProxyAPIMetadata;

struct GW2LegacyAddonProxyAPI {
    GetDirect3DDevice9_t* GetDirect3DDevice;
    CreateProxyDirect3DDevice9_t* CreateProxyDirect3DDevice9;
    CreateProxyDirect3DDevice9Ex_t* CreateProxyDirect3DDevice9Ex;
};


typedef GW2LegacyAddonProxyAPIMetadata(GW2PROXY_CALL ProxyInitialize_t)(const GW2LegacyAddonProxyAPI* const api);
#define GW2PROXY_DLL_Initialize "ProxyInitialize"
typedef void(GW2PROXY_CALL ProxyRelease_t)();
#define GW2PROXY_DLL_Release "ProxyRelease"


/**
Ensure compiler errors when exported addon functions are wrong.
*/
#ifndef _GW2ADDON_IMPORTS
GW2LegacyAddonProxyAPIMetadata GW2PROXY_CALL ProxyInitialize(const GW2LegacyAddonProxyAPI* const api);
void GW2PROXY_CALL ProxyRelease();
#endif
