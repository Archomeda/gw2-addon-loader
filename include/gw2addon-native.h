/**
This is an include file specifically for creating Guild Wars 2 addons with the GW2 Addon Loader.
Visit https://github.com/Archomeda/gw2-addon-loader for more information.
*/

#pragma once
#include <d3d9.h>

#ifndef GW2ADDON_API

#ifdef _GW2ADDON_IMPORTS
#define GW2ADDON_API __declspec(dllimport)
#else
#define GW2ADDON_API __declspec(dllexport)
#endif

#endif

#define GW2ADDON_RESULT int
#define GW2ADDON_CALL __stdcall

/**
Native addons can be loaded multiple times.
Therefore, don't depend on DllMain. Use GW2_Load instead for initialization if required.
And don't forget GW2_Unload to clean up your mess.

Native addons support versioning in order to keep older compiled addons from breaking whenever something gets added.
Please always use the newest version whenever you compile your addon by using the latest GW2AddonInfo.
Required functions need to be exported in order for the addon to work properly.
Optional functions can be left out of the exported functions if you don't need its functionality.

Please see the examples on how to use this.
*/


/**
GW2AddonInfo is defined by the addon to specify addon information for the loader.
The addon info is requested by the loader through GW2_GetAddonInfo.
*/
typedef struct {
    const int ver = 0;
} GW2AddonInfoHeader;

typedef struct {
    // Versioning, automatically defined.
    const int ver = 1;

    // Size of the addon ID wide C-string.
    size_t idSize;

    // The addon ID (wide C-string).
    // The ID must be unique and fixed as long as it is the same addon.
    const wchar_t* id;

    // Size of the addon name wide C-string.
    size_t nameSize;

    // The addon name (wide C-string).
    const wchar_t* name;

    // Size of the addon author wide C-string.
    size_t authorSize;

    // The addon author (wide C-string).
    const wchar_t* author;

    // Size of the addon description wide C-string.
    size_t descriptionSize;

    // The addon description (wide C-string).
    const wchar_t* description;

    // Size of the addon version (wide C-string).
    size_t versionSize;

    // The addon version (wide C-string).
    const wchar_t* version;

    // Size of the addon homepage URL (wide C-string).
    size_t homepageSize;

    // The addon homepage URL.
    const wchar_t* homepage;
} GW2AddonInfoV1;

typedef GW2AddonInfoV1 GW2AddonInfo;


/**
Exported C-function: GW2_GetAddonInfo. Required.
Gets the addon info.
Note: This function needs to be accessible regardless if the addon is initialized or not.
Make sure to not allocate memory you can't deallocate later. The callee does not clean up any allocations made by the caller.
This function should return 0 on success. Any other value will be treated as an error.
*/
typedef GW2ADDON_RESULT(GW2ADDON_CALL *GW2GetAddonInfo)(GW2AddonInfo** addonInfo);
#define GW2_DLL_GetAddonInfo "GW2_GetAddonInfo"

/**
Exported C-function: GW2_Load. Optional.
Gets called whenever the addon is loading.
Can be used to initialize the addon when it gets enabled before it gets used.
This function should return 0 on success. Any other value will be treated as an error.
*/
typedef GW2ADDON_RESULT(GW2ADDON_CALL *GW2Load)(HWND hFocusWindow, IDirect3DDevice9* pDev);
#define GW2_DLL_Load "GW2_Load"

/**
Exported C-function: GW2_Unload. Optional.
Gets called whenever the addon is unloading.
Can be used to uninitialize the addon when it gets disabled or when Guild Wars 2 shuts down.
This function should return 0 on success. Any other value will be treated as an error.
*/
typedef GW2ADDON_RESULT(GW2ADDON_CALL *GW2Unload)();
#define GW2_DLL_Unload "GW2_Unload"

/**
Exported C-function: GW2_DrawFrameBeforePostProcessing. Optional.
Gets called every frame right before the post processing gets drawn.
Can be used to draw custom elements in the world that needs to be behind the GUI, or to do some additional postprocessing.
*/
typedef void(GW2ADDON_CALL *GW2DrawFrameBeforePostProcessing)(IDirect3DDevice9* pDev);
#define GW2_DLL_DrawFrameBeforePostProcessing "GW2_DrawFrameBeforePostProcessing"

/**
Exported C-function: GW2_DrawFrameBeforeGui. Optional.
Gets called every frame right before the GUI gets drawn.
Can be used to draw custom elements in the world that needs to be behind the GUI, or to do some additional postprocessing.
*/
typedef void(GW2ADDON_CALL *GW2DrawFrameBeforeGui)(IDirect3DDevice9* pDev);
#define GW2_DLL_DrawFrameBeforeGui "GW2_DrawFrameBeforeGui"

/**
Exported C-function: GW2_DrawFrame. Optional.
Gets called every frame whenever the frame needs to be drawn on the D3D9 device.
Can be used to draw custom elements on the game as an overlay.
*/
typedef void(GW2ADDON_CALL *GW2DrawFrame)(IDirect3DDevice9* pDev);
#define GW2_DLL_DrawFrame "GW2_DrawFrame"

/**
Exported C-function: GW2_HandleWndProc. Optional.
Gets called whenever a message in the message loop is being handled by the Guild Wars 2 client.
Can be used to e.g. intercept mouse and keyboard events.
This function should return true if it's handled and should not be handled by any other addon or the game itself.
Return false whenever it should be handled by other addons and the game itself as well.
*/
typedef bool(GW2ADDON_CALL *GW2HandleWndProc)(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
#define GW2_DLL_HandleWndProc "GW2_HandleWndProc"


/****************************
  Advanced functions follow
****************************/

/**
Exported C-function: GW2_AdvPreBeginScene. Optional.
Advanced function that gets called before D3D9 processes the BeginScene call.
Do not call BeginScene yourself. The addon loader does that already after this call.
*/
typedef void(GW2ADDON_CALL *GW2AdvPreBeginScene)(IDirect3DDevice9* pDev);
#define GW2_DLL_AdvPreBeginScene "GW2_AdvPreBeginScene"

/**
Exported C-function: GW2_AdvPostBeginScene. Optional.
Advanced function that gets called after D3D9 has processed the BeginScene call.
*/
typedef void(GW2ADDON_CALL *GW2AdvPostBeginScene)(IDirect3DDevice9* pDev);
#define GW2_DLL_AdvPostBeginScene "GW2_AdvPostBeginScene"

/**
Exported C-function: GW2_AdvPreEndScene. Optional.
Advanced function that gets called before D3D9 processes the EndScene call.
Do not call EndScene yourself. The addon loader does that already after this call.
*/
typedef void(GW2ADDON_CALL *GW2AdvPreEndScene)(IDirect3DDevice9* pDev);
#define GW2_DLL_AdvPreEndScene "GW2_AdvPreEndScene"

/**
Exported C-function: GW2_AdvPostEndScene. Optional.
Advanced function that gets called after D3D9 has processed the EndScene call.
*/
typedef void(GW2ADDON_CALL *GW2AdvPostEndScene)(IDirect3DDevice9* pDev);
#define GW2_DLL_AdvPostEndScene "GW2_AdvPostEndScene"

/**
Exported C-function: GW2_AdvPreClear. Optional.
Advanced function that gets called before D3D9 processes the Clear call.
Do not call Clear yourself. The addon loader does that already after this call.
*/
typedef void(GW2ADDON_CALL *GW2AdvPreClear)(IDirect3DDevice9* pDev, DWORD Count, CONST D3DRECT* pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil);
#define GW2_DLL_AdvPreClear "GW2_AdvPreClear"

/**
Exported C-function: GW2_AdvPostClear. Optional.
Advanced function that gets called after D3D9 has processed the Clear call.
*/
typedef void(GW2ADDON_CALL *GW2AdvPostClear)(IDirect3DDevice9* pDev, DWORD Count, CONST D3DRECT* pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil);
#define GW2_DLL_AdvPostClear "GW2_AdvPostClear"

/**
Exported C-function: GW2_AdvPreReset. Optional.
Advanced function that gets called before D3D9 processes the Reset call.
Do not call Reset yourself. The addon loader does that already after this call.
*/
typedef void(GW2ADDON_CALL *GW2AdvPreReset)(IDirect3DDevice9* pDev, D3DPRESENT_PARAMETERS* pPresentationParameters);
#define GW2_DLL_AdvPreReset "GW2_AdvPreReset"

/**
Exported C-function: GW2_AdvPostReset. Optional.
Advanced function that gets called after D3D9 has processed the Reset call.
*/
typedef void(GW2ADDON_CALL *GW2AdvPostReset)(IDirect3DDevice9* pDev, D3DPRESENT_PARAMETERS* pPresentationParameters);
#define GW2_DLL_AdvPostReset "GW2_AdvPostReset"

/**
Exported C-function: GW2_AdvPrePresent. Optional.
Advanced function that gets called before D3D9 processes the Present call.
Do not call Present yourself. The addon loader does that already after this call.
*/
typedef void(GW2ADDON_CALL *GW2AdvPrePresent)(IDirect3DDevice9* pDev, CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion);
#define GW2_DLL_AdvPrePresent "GW2_AdvPrePresent"

/**
Exported C-function: GW2_AdvPostPresent. Optional.
Advanced function that gets called after D3D9 has processed the Present call.
*/
typedef void(GW2ADDON_CALL *GW2AdvPostPresent)(IDirect3DDevice9* pDev, CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion);
#define GW2_DLL_AdvPostPresent "GW2_AdvPostPresent"

/**
Exported C-function: GW2_AdvPreCreateTexture. Optional.
Advanced function that gets called before D3D9 processes the CreateTexture call.
If the return value does not equal D3D_OK, the call fails.

You can call CreateTexture yourself. If the value of ppTexture is not NULL after the function returns,
it is assumed that the addon has created (overridden) the texture instead.
Calls to remaining addons will be skipped, and the addon loader will not call CreateTexture either.
*/
typedef HRESULT(GW2ADDON_CALL *GW2AdvPreCreateTexture)(IDirect3DDevice9* pDev, UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DTexture9** ppTexture, HANDLE* pSharedHandle);
#define GW2_DLL_AdvPreCreateTexture "GW2_AdvPreCreateTexture"

/**
Exported C-function: GW2_AdvPostCreateTexture. Optional.
Advanced function that gets called after D3D9 has processed the CreateTexture call.
*/
typedef void(GW2ADDON_CALL *GW2AdvPostCreateTexture)(IDirect3DDevice9* pDev, IDirect3DTexture9* pTexture, UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, HANDLE* pSharedHandle);
#define GW2_DLL_AdvPostCreateTexture "GW2_AdvPostCreateTexture"

/**
Exported C-function: GW2_AdvPreCreateVertexShader. Optional.
Advanced function that gets called before D3D9 processes the CreateVertexShader call.
If the return value does not equal D3D_OK, the call fails.

You can call CreateVertexShader yourself. If the value of ppShader is not NULL after the function returns,
it is assumed that the addon has created (overridden) the shader instead.
Calls to remaining addons will be skipped, and the addon loader will not call CreateVertexShader either.
*/
typedef HRESULT(GW2ADDON_CALL *GW2AdvPreCreateVertexShader)(IDirect3DDevice9* pDev, CONST DWORD* pFunction, IDirect3DVertexShader9** ppShader);
#define GW2_DLL_AdvPreCreateVertexShader "GW2_AdvPreCreateVertexShader"

/**
Exported C-function: GW2_AdvPostCreateVertexShader. Optional.
Advanced function that gets called after D3D9 has processed the CreateVertexShader call.
*/
typedef void(GW2ADDON_CALL *GW2AdvPostCreateVertexShader)(IDirect3DDevice9* pDev, IDirect3DVertexShader9* ppShader, CONST DWORD* pFunction);
#define GW2_DLL_AdvPostCreateVertexShader "GW2_AdvPostCreateVertexShader"

/**
Exported C-function: GW2_AdvPreCreatePixelShader. Optional.
Advanced function that gets called before D3D9 processes the CreatePixelShader call.
If the return value does not equal D3D_OK, the call fails.

You can call CreatePixelShader yourself. If the value of ppShader is not NULL after the function returns,
it is assumed that the addon has created (overridden) the shader instead.
Calls to remaining addons will be skipped, and the addon loader will not call CreatePixelShader either.
*/
typedef HRESULT(GW2ADDON_CALL *GW2AdvPreCreatePixelShader)(IDirect3DDevice9* pDev, CONST DWORD* pFunction, IDirect3DPixelShader9** ppShader);
#define GW2_DLL_AdvPreCreatePixelShader "GW2_AdvPreCreatePixelShader"

/**
Exported C-function: GW2_AdvPostCreatePixelShader. Optional.
Advanced function that gets called after D3D9 has processed the CreatePixelShader call.
*/
typedef void(GW2ADDON_CALL *GW2AdvPostCreatePixelShader)(IDirect3DDevice9* pDev, IDirect3DPixelShader9* ppShader, CONST DWORD* pFunction);
#define GW2_DLL_AdvPostCreatePixelShader "GW2_AdvPostCreatePixelShader"

/**
Exported C-function: GW2_AdvPreCreateRenderTarget. Optional.
Advanced function that gets called before D3D9 processes the CreateRenderTarget call.
If the return value does not equal D3D_OK, the call fails.

You can call CreateRenderTarget yourself. If the value of ppSurface is not NULL after the function returns,
it is assumed that the addon has created (overridden) the surface instead.
Calls to remaining addons will be skipped, and the addon loader will not call CreateRenderTarget either.
*/
typedef HRESULT(GW2ADDON_CALL *GW2AdvPreCreateRenderTarget)(IDirect3DDevice9* pDev, UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Lockable, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle);
#define GW2_DLL_AdvPreCreateRenderTarget "GW2_AdvPreCreateRenderTarget"

/**
Exported C-function: GW2_AdvPostCreateRenderTarget. Optional.
Advanced function that gets called after D3D9 has processed the CreateRenderTarget call.
*/
typedef void(GW2ADDON_CALL *GW2AdvPostCreateRenderTarget)(IDirect3DDevice9* pDev, IDirect3DSurface9* ppSurface, UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Lockable, HANDLE* pSharedHandle);
#define GW2_DLL_AdvPostCreateRenderTarget "GW2_AdvPostCreateRenderTarget"

/**
Exported C-function: GW2_AdvPreSetTexture. Optional.
Advanced function that gets called before D3D9 processes the SetTexture call.
Do not call SetTexture yourself. The addon loader does that already after this call.
*/
typedef void(GW2ADDON_CALL *GW2AdvPreSetTexture)(IDirect3DDevice9* pDev, DWORD Stage, IDirect3DBaseTexture9* pTexture);
#define GW2_DLL_AdvPreSetTexture "GW2_AdvPreSetTexture"

/**
Exported C-function: GW2_AdvPostSetTexture. Optional.
Advanced function that gets called after D3D9 has processed the SetTexture call.
*/
typedef void(GW2ADDON_CALL *GW2AdvPostSetTexture)(IDirect3DDevice9* pDev, DWORD Stage, IDirect3DBaseTexture9* pTexture);
#define GW2_DLL_AdvPostSetTexture "GW2_AdvPostSetTexture"

/**
Exported C-function: GW2_AdvPreSetVertexShader. Optional.
Advanced function that gets called before D3D9 processes the SetVertexShader call.
Do not call SetVertexShader yourself. The addon loader does that already after this call.
*/
typedef void(GW2ADDON_CALL *GW2AdvPreSetVertexShader)(IDirect3DDevice9* pDev, IDirect3DVertexShader9* pShader);
#define GW2_DLL_AdvPreSetVertexShader "GW2_AdvPreSetVertexShader"

/**
Exported C-function: GW2_AdvPostSetVertexShader. Optional.
Advanced function that gets called after D3D9 has processed the SetVertexShader call.
*/
typedef void(GW2ADDON_CALL *GW2AdvPostSetVertexShader)(IDirect3DDevice9* pDev, IDirect3DVertexShader9* pShader);
#define GW2_DLL_AdvPostSetVertexShader "GW2_AdvPostSetVertexShader"

/**
Exported C-function: GW2_AdvPreSetPixelShader. Optional.
Advanced function that gets called before D3D9 processes the SetPixelShader call.
Do not call SetPixelShader yourself. The addon loader does that already after this call.
*/
typedef void(GW2ADDON_CALL *GW2AdvPreSetPixelShader)(IDirect3DDevice9* pDev, IDirect3DPixelShader9* pShader);
#define GW2_DLL_AdvPreSetPixelShader "GW2_AdvPreSetPixelShader"

/**
Exported C-function: GW2_AdvPostSetPixelShader. Optional.
Advanced function that gets called after D3D9 has processed the SetPixelShader call.
*/
typedef void(GW2ADDON_CALL *GW2AdvPostSetPixelShader)(IDirect3DDevice9* pDev, IDirect3DPixelShader9* pShader);
#define GW2_DLL_AdvPostSetPixelShader "GW2_AdvPostSetPixelShader"

/**
Exported C-function: GW2_AdvPreSetRenderTarget. Optional.
Advanced function that gets called before D3D9 processes the SetRenderTarget call.
Do not call SetRenderTarget yourself. The addon loader does that already after this call.
*/
typedef void(GW2ADDON_CALL *GW2AdvPreSetRenderTarget)(IDirect3DDevice9* pDev, DWORD RenderTargetIndex, IDirect3DSurface9* pRenderTarget);
#define GW2_DLL_AdvPreSetRenderTarget "GW2_AdvPreSetRenderTarget"

/**
Exported C-function: GW2_AdvPostSetRenderTarget. Optional.
Advanced function that gets called after D3D9 has processed the SetRenderTarget call.
*/
typedef void(GW2ADDON_CALL *GW2AdvPostSetRenderTarget)(IDirect3DDevice9* pDev, DWORD RenderTargetIndex, IDirect3DSurface9* pRenderTarget);
#define GW2_DLL_AdvPostSetRenderTarget "GW2_AdvPostSetRenderTarget"

/**
Exported C-function: GW2_AdvPreSetRenderState. Optional.
Advanced function that gets called before D3D9 processes the SetRenderState. call.
Do not call SetRenderState yourself. The addon loader does that already after this call.
*/
typedef void(GW2ADDON_CALL *GW2AdvPreSetRenderState)(IDirect3DDevice9* pDev, D3DRENDERSTATETYPE State, DWORD Value);
#define GW2_DLL_AdvPreSetRenderState "GW2_AdvPreSetRenderState"

/**
Exported C-function: GW2_AdvPostSetRenderState. Optional.
Advanced function that gets called after D3D9 has processed the SetRenderState call.
*/
typedef void(GW2ADDON_CALL *GW2AdvPostSetRenderState)(IDirect3DDevice9* pDev, D3DRENDERSTATETYPE State, DWORD Value);
#define GW2_DLL_AdvPostSetRenderState "GW2_AdvPostSetRenderState"

/**
Exported C-function: GW2_AdvPreDrawIndexedPrimitive. Optional.
Advanced function that gets called before D3D9 processes the DrawIndexedPrimitive call.
Do not call DrawIndexedPrimitive yourself. The addon loader does that already after this call.
*/
typedef void(GW2ADDON_CALL *GW2AdvPreDrawIndexedPrimitive)(IDirect3DDevice9* pDev, D3DPRIMITIVETYPE PrimitiveType, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount);
#define GW2_DLL_AdvPreDrawIndexedPrimitive "GW2_AdvPreDrawIndexedPrimitive"

/**
Exported C-function: GW2_AdvPostDrawIndexedPrimitive. Optional.
Advanced function that gets called after D3D9 has processed the DrawIndexedPrimitive call.
*/
typedef void(GW2ADDON_CALL *GW2AdvPostDrawIndexedPrimitive)(IDirect3DDevice9* pDev, D3DPRIMITIVETYPE PrimitiveType, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount);
#define GW2_DLL_AdvPostDrawIndexedPrimitive "GW2_AdvPostDrawIndexedPrimitive"
