/**
This is an include file specifically for creating Guild Wars 2 addons with the GW2 Addon Loader.
Visit https://github.com/Archomeda/gw2-addon-loader for more information.
*/

#pragma once
#include <d3d9.h>

#define GW2ADDON_RESULT int
#define GW2ADDON_CALL __stdcall
#define GW2ADDON_VER 1

/**
THE STRUCT TO IMPLEMENT CAN BE FOUND AFTER THE FUNCTION TYPEDEFS.

It's not defined how and when native addons are loaded.
Therefore, do not depend on DllMain for initialization. Use Load instead.
And don't forget to use Unload to clean up your mess.
*/


/**
The addon update method. Defaults to NoUpdateMethod.
*/
enum AddonUpdateMethod {
    NoUpdateMethod = 0,
    CustomUpdateMethod = 1, // Reserved
    GithubReleasesUpdateMethod = 2
};


/**
Gets called whenever the addon is loading.
Can be used to initialize the addon when it gets enabled before it gets used.
This function should return 0 on success. Any other value will be treated as an error.
*/
typedef GW2ADDON_RESULT(GW2ADDON_CALL GW2AddonLoad_t)(HWND hFocusWindow, IDirect3DDevice9* pDev);

/**
Gets called every frame right before the post processing gets drawn.
Can be used to draw custom elements in the world that needs to be behind the GUI, or to do some additional postprocessing.
*/
typedef void(GW2ADDON_CALL GW2AddonDrawFrameBeforePostProcessing_t)(IDirect3DDevice9* pDev);

/**
Gets called every frame right before the GUI gets drawn.
Can be used to draw custom elements in the world that needs to be behind the GUI, or to do some additional postprocessing.
*/
typedef void(GW2ADDON_CALL GW2AddonDrawFrameBeforeGui_t)(IDirect3DDevice9* pDev);

/**
Gets called every frame whenever the frame needs to be drawn on the D3D9 device.
Can be used to draw custom elements on the game as an overlay.
*/
typedef void(GW2ADDON_CALL GW2AddonDrawFrame_t)(IDirect3DDevice9* pDev);

/**
Gets called whenever a message in the message loop is being handled by the Guild Wars 2 client.
Can be used to e.g. intercept mouse and keyboard events.
This function should return true if it's handled and should not be handled by any other addon or the game itself.
Return false whenever it should be handled by other addons and the game itself as well.
*/
typedef bool(GW2ADDON_CALL GW2AddonHandleWndProc_t)(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

/**
Gets called whenever the user clicks on the settings button in the addon loader.
Can be used as an alternative to or together with keybinds.
*/
typedef void(GW2ADDON_CALL GW2AddonOpenSettings_t)();

/**
Gets called whenever the shared API key changes, and immediately after loading.
Parameter key is a null-terminated C-string. The pointer can be NULL if no API key is set.
Copy the key to a local variable, because it will be cleaned up after this call.
*/
typedef void(GW2ADDON_CALL GW2AddonApiKeyChange_t)(const char* key);


/****************************
  Advanced functions follow
****************************/

/**
Advanced function that gets called before D3D9 processes the BeginScene call.
Do not call BeginScene yourself. The addon loader does that already after this call.
*/
typedef void(GW2ADDON_CALL GW2AddonAdvPreBeginScene_t)(IDirect3DDevice9* pDev);

/**
Advanced function that gets called after D3D9 has processed the BeginScene call.
*/
typedef void(GW2ADDON_CALL GW2AddonAdvPostBeginScene_t)(IDirect3DDevice9* pDev);

/**
Advanced function that gets called before D3D9 processes the EndScene call.
Do not call EndScene yourself. The addon loader does that already after this call.
*/
typedef void(GW2ADDON_CALL GW2AddonAdvPreEndScene_t)(IDirect3DDevice9* pDev);

/**
Advanced function that gets called after D3D9 has processed the EndScene call.
*/
typedef void(GW2ADDON_CALL GW2AddonAdvPostEndScene_t)(IDirect3DDevice9* pDev);

/**
Advanced function that gets called before D3D9 processes the Clear call.
Do not call Clear yourself. The addon loader does that already after this call.
*/
typedef void(GW2ADDON_CALL GW2AddonAdvPreClear_t)(IDirect3DDevice9* pDev, DWORD Count, CONST D3DRECT* pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil);

/**
Advanced function that gets called after D3D9 has processed the Clear call.
*/
typedef void(GW2ADDON_CALL GW2AddonAdvPostClear_t)(IDirect3DDevice9* pDev, DWORD Count, CONST D3DRECT* pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil);

/**
Advanced function that gets called before D3D9 processes the Reset call.
Do not call Reset yourself. The addon loader does that already after this call.
*/
typedef void(GW2ADDON_CALL GW2AddonAdvPreReset_t)(IDirect3DDevice9* pDev, D3DPRESENT_PARAMETERS* pPresentationParameters);

/**
Advanced function that gets called after D3D9 has processed the Reset call.
*/
typedef void(GW2ADDON_CALL GW2AddonAdvPostReset_t)(IDirect3DDevice9* pDev, D3DPRESENT_PARAMETERS* pPresentationParameters);

/**
Advanced function that gets called before D3D9 processes the Present call.
Do not call Present yourself. The addon loader does that already after this call.
*/
typedef void(GW2ADDON_CALL GW2AddonAdvPrePresent_t)(IDirect3DDevice9* pDev, CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion);

/**
Advanced function that gets called after D3D9 has processed the Present call.
*/
typedef void(GW2ADDON_CALL GW2AddonAdvPostPresent_t)(IDirect3DDevice9* pDev, CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion);

/**
Advanced function that gets called before D3D9 processes the CreateTexture call.
If the return value does not equal D3D_OK, the call fails.

This function is disabled in competitive areas.

You can call CreateTexture yourself. If the value of ppTexture is not NULL after the function returns,
it is assumed that the addon has created (overridden) the texture instead.
Calls to remaining addons will be skipped, and the addon loader will not call CreateTexture either.
*/
typedef HRESULT(GW2ADDON_CALL GW2AddonAdvPreCreateTexture_t)(IDirect3DDevice9* pDev, UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DTexture9** ppTexture, HANDLE* pSharedHandle);

/**
Advanced function that gets called after D3D9 has processed the CreateTexture call.

This function is disabled in competitive areas.
*/
typedef void(GW2ADDON_CALL GW2AddonAdvPostCreateTexture_t)(IDirect3DDevice9* pDev, IDirect3DTexture9* pTexture, UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, HANDLE* pSharedHandle);

/**
Advanced function that gets called before D3D9 processes the CreateVertexShader call.
If the return value does not equal D3D_OK, the call fails.

This function is disabled in competitive areas.

You can call CreateVertexShader yourself. If the value of ppShader is not NULL after the function returns,
it is assumed that the addon has created (overridden) the shader instead.
Calls to remaining addons will be skipped, and the addon loader will not call CreateVertexShader either.
*/
typedef HRESULT(GW2ADDON_CALL GW2AddonAdvPreCreateVertexShader_t)(IDirect3DDevice9* pDev, CONST DWORD* pFunction, IDirect3DVertexShader9** ppShader);

/**
Advanced function that gets called after D3D9 has processed the CreateVertexShader call.

This function is disabled in competitive areas.
*/
typedef void(GW2ADDON_CALL GW2AddonAdvPostCreateVertexShader_t)(IDirect3DDevice9* pDev, IDirect3DVertexShader9* ppShader, CONST DWORD* pFunction);

/**
Advanced function that gets called before D3D9 processes the CreatePixelShader call.
If the return value does not equal D3D_OK, the call fails.

This function is disabled in competitive areas.

You can call CreatePixelShader yourself. If the value of ppShader is not NULL after the function returns,
it is assumed that the addon has created (overridden) the shader instead.
Calls to remaining addons will be skipped, and the addon loader will not call CreatePixelShader either.
*/
typedef HRESULT(GW2ADDON_CALL GW2AddonAdvPreCreatePixelShader_t)(IDirect3DDevice9* pDev, CONST DWORD* pFunction, IDirect3DPixelShader9** ppShader);

/**
Advanced function that gets called after D3D9 has processed the CreatePixelShader call.

This function is disabled in competitive areas.
*/
typedef void(GW2ADDON_CALL GW2AddonAdvPostCreatePixelShader_t)(IDirect3DDevice9* pDev, IDirect3DPixelShader9* ppShader, CONST DWORD* pFunction);

/**
Advanced function that gets called before D3D9 processes the CreateRenderTarget call.
If the return value does not equal D3D_OK, the call fails.

This function is disabled in competitive areas.

You can call CreateRenderTarget yourself. If the value of ppSurface is not NULL after the function returns,
it is assumed that the addon has created (overridden) the surface instead.
Calls to remaining addons will be skipped, and the addon loader will not call CreateRenderTarget either.
*/
typedef HRESULT(GW2ADDON_CALL GW2AddonAdvPreCreateRenderTarget_t)(IDirect3DDevice9* pDev, UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Lockable, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle);

/**
Advanced function that gets called after D3D9 has processed the CreateRenderTarget call.

This function is disabled in competitive areas.
*/
typedef void(GW2ADDON_CALL GW2AddonAdvPostCreateRenderTarget_t)(IDirect3DDevice9* pDev, IDirect3DSurface9* ppSurface, UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Lockable, HANDLE* pSharedHandle);

/**
Advanced function that gets called before D3D9 processes the SetTexture call.
Do not call SetTexture yourself. The addon loader does that already after this call.

This function is disabled in competitive areas.
*/
typedef void(GW2ADDON_CALL GW2AddonAdvPreSetTexture_t)(IDirect3DDevice9* pDev, DWORD Stage, IDirect3DBaseTexture9* pTexture);

/**
Advanced function that gets called after D3D9 has processed the SetTexture call.

This function is disabled in competitive areas.
*/
typedef void(GW2ADDON_CALL GW2AddonAdvPostSetTexture_t)(IDirect3DDevice9* pDev, DWORD Stage, IDirect3DBaseTexture9* pTexture);

/**
Advanced function that gets called before D3D9 processes the SetVertexShader call.
Do not call SetVertexShader yourself. The addon loader does that already after this call.

This function is disabled in competitive areas.
*/
typedef void(GW2ADDON_CALL GW2AddonAdvPreSetVertexShader_t)(IDirect3DDevice9* pDev, IDirect3DVertexShader9* pShader);

/**
Advanced function that gets called after D3D9 has processed the SetVertexShader call.

This function is disabled in competitive areas.
*/
typedef void(GW2ADDON_CALL GW2AddonAdvPostSetVertexShader_t)(IDirect3DDevice9* pDev, IDirect3DVertexShader9* pShader);

/**
Advanced function that gets called before D3D9 processes the SetPixelShader call.
Do not call SetPixelShader yourself. The addon loader does that already after this call.

This function is disabled in competitive areas.
*/
typedef void(GW2ADDON_CALL GW2AddonAdvPreSetPixelShader_t)(IDirect3DDevice9* pDev, IDirect3DPixelShader9* pShader);

/**
Advanced function that gets called after D3D9 has processed the SetPixelShader call.

This function is disabled in competitive areas.
*/
typedef void(GW2ADDON_CALL GW2AddonAdvPostSetPixelShader_t)(IDirect3DDevice9* pDev, IDirect3DPixelShader9* pShader);

/**
Advanced function that gets called before D3D9 processes the SetRenderTarget call.
Do not call SetRenderTarget yourself. The addon loader does that already after this call.

This function is disabled in competitive areas.
*/
typedef void(GW2ADDON_CALL GW2AddonAdvPreSetRenderTarget_t)(IDirect3DDevice9* pDev, DWORD RenderTargetIndex, IDirect3DSurface9* pRenderTarget);

/**
Advanced function that gets called after D3D9 has processed the SetRenderTarget call.

This function is disabled in competitive areas.
*/
typedef void(GW2ADDON_CALL GW2AddonAdvPostSetRenderTarget_t)(IDirect3DDevice9* pDev, DWORD RenderTargetIndex, IDirect3DSurface9* pRenderTarget);

/**
Advanced function that gets called before D3D9 processes the SetRenderState. call.
Do not call SetRenderState yourself. The addon loader does that already after this call.

This function is disabled in competitive areas.
*/
typedef void(GW2ADDON_CALL GW2AddonAdvPreSetRenderState_t)(IDirect3DDevice9* pDev, D3DRENDERSTATETYPE State, DWORD Value);

/**
Advanced function that gets called after D3D9 has processed the SetRenderState call.

This function is disabled in competitive areas.
*/
typedef void(GW2ADDON_CALL GW2AddonAdvPostSetRenderState_t)(IDirect3DDevice9* pDev, D3DRENDERSTATETYPE State, DWORD Value);

/**
Advanced function that gets called before D3D9 processes the DrawIndexedPrimitive call.
Do not call DrawIndexedPrimitive yourself. The addon loader does that already after this call.

This function is disabled in competitive areas.
*/
typedef void(GW2ADDON_CALL GW2AddonAdvPreDrawIndexedPrimitive_t)(IDirect3DDevice9* pDev, D3DPRIMITIVETYPE PrimitiveType, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount);

/**
Advanced function that gets called after D3D9 has processed the DrawIndexedPrimitive call.

This function is disabled in competitive areas.
*/
typedef void(GW2ADDON_CALL GW2AddonAdvPostDrawIndexedPrimitive_t)(IDirect3DDevice9* pDev, D3DPRIMITIVETYPE PrimitiveType, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount);


/**********************
 END FUNCTION TYPEDEFS
**********************/


/********************************************************************************************************************
THE FOLLOWING NEEDS TO BE IMPLEMENTED IN YOUR ADDON

Native addons support versioning in order to keep older compiled addons from breaking whenever something gets added.
Please always use the newest version whenever you compile your addon by using the latest GW2AddonAPI struct.
Required functions need to be exported in order for the addon to work properly.
Optional functions can be left out of the exported functions if you don't need its functionality.

Please see the examples on how to use this.

*********************************************************************************************************************/

/**
GW2AddonAPI is the addon export table. Use the latest version.
Previous versions are kept for compatibility reasons only.
*/
typedef struct {
    // Versioning, automatically defined
    const int ver = 0;
} GW2AddonAPIBase;

typedef struct {
    // Versioning, automatically defined
    const int ver = 1;

    /** ADDON INFORMATION **/

    // The addon ID, must be unique and fixed as long as it's the same addon
    const char* id;

    // The addon name
    const char* name;

    // The addon author
    const char* author;

    // The addon description
    const char* description;

    // The addon version
    const char* version;

    // The addon homepage URL
    const char* homepage;

    // The addon icon, should be 32x32 pixels
    // This is a pointer that should point to either:
    //  - The already loaded image in memory
    //    In this case set the iconSize to the image size in bytes
    //    NOTE: You are responsible for releasing the original image data in memory, if needed
    //  - The already loaded texture in memory (type should be IDirect3DTexture9*)
    //    In this case leave the iconSize as -1
    //    NOTE: You are responsible for releasing the D3D9 texture
    // Supported image types can be found at https://msdn.microsoft.com/en-US/library/windows/desktop/bb172803(v=vs.85).aspx under the Remarks section
    void* icon;
    int iconSize = -1;

    struct {
        // The addon update method
        // If none, the addon doesn't support updating
        AddonUpdateMethod method = NoUpdateMethod;

        union {
            // Additional method information
            const char* methodInfo;

            // Alias; the GitHub repository (e.g. Archomeda/gw2-addon-loader) if using the GithubReleases method
            const char* githubRepo;
        };
    } updateInfo;


    /** ADDON FUNCTION EXPORTS **/
    GW2AddonOpenSettings_t* OpenSettings;

    void* CheckUpdate; // Reserved
    void* DownloadUpdate; // Reserved

    GW2AddonLoad_t* Load;
    GW2AddonDrawFrameBeforePostProcessing_t* DrawFrameBeforePostProcessing;
    GW2AddonDrawFrameBeforeGui_t* DrawFrameBeforeGui;
    GW2AddonDrawFrame_t* DrawFrame;
    GW2AddonHandleWndProc_t* HandleWndProc;

    GW2AddonAdvPreBeginScene_t* AdvPreBeginScene;
    GW2AddonAdvPostBeginScene_t* AdvPostBeginScene;
    GW2AddonAdvPreEndScene_t* AdvPreEndScene;
    GW2AddonAdvPostEndScene_t* AdvPostEndScene;
    GW2AddonAdvPreClear_t* AdvPreClear;
    GW2AddonAdvPostClear_t* AdvPostClear;
    GW2AddonAdvPreReset_t* AdvPreReset;
    GW2AddonAdvPostReset_t* AdvPostReset;
    GW2AddonAdvPrePresent_t* AdvPrePresent;
    GW2AddonAdvPostPresent_t* AdvPostPresent;
    GW2AddonAdvPreCreateTexture_t* AdvPreCreateTexture;
    GW2AddonAdvPostCreateTexture_t* AdvPostCreateTexture;
    GW2AddonAdvPreCreateVertexShader_t* AdvPreCreateVertexShader;
    GW2AddonAdvPostCreateVertexShader_t* AdvPostCreateVertexShader;
    GW2AddonAdvPreCreatePixelShader_t* AdvPreCreatePixelShader;
    GW2AddonAdvPostCreatePixelShader_t* AdvPostCreatePixelShader;
    GW2AddonAdvPreCreateRenderTarget_t* AdvPreCreateRenderTarget;
    GW2AddonAdvPostCreateRenderTarget_t* AdvPostCreateRenderTarget;
    GW2AddonAdvPreSetTexture_t* AdvPreSetTexture;
    GW2AddonAdvPostSetTexture_t* AdvPostSetTexture;
    GW2AddonAdvPreSetVertexShader_t* AdvPreSetVertexShader;
    GW2AddonAdvPostSetVertexShader_t* AdvPostSetVertexShader;
    GW2AddonAdvPreSetPixelShader_t* AdvPreSetPixelShader;
    GW2AddonAdvPostSetPixelShader_t* AdvPostSetPixelShader;
    GW2AddonAdvPreSetRenderTarget_t* AdvPreSetRenderTarget;
    GW2AddonAdvPostSetRenderTarget_t* AdvPostSetRenderTarget;
    GW2AddonAdvPreSetRenderState_t* AdvPreSetRenderState;
    GW2AddonAdvPostSetRenderState_t* AdvPostSetRenderState;
    GW2AddonAdvPreDrawIndexedPrimitive_t* AdvPreDrawIndexedPrimitive;
    GW2AddonAdvPostDrawIndexedPrimitive_t* AdvPostDrawIndexedPrimitive;

    GW2AddonApiKeyChange_t* ApiKeyChange;
} GW2AddonAPIV1;

/**
Exported C-function: GW2AddonInitialize. Required.
Gets called as soon as the addon needs to be initialized.
This is not the same as the addon being loaded or activated.
This function provides all the necessary information for the addon loader.
*/
typedef GW2AddonAPIBase* (GW2ADDON_CALL GW2AddonInitialize_t)(int loaderVersion);
#define GW2ADDON_DLL_Initialize "GW2AddonInitialize"

/**
Exported C-function: GW2AddonRelease. Required.
Gets called as soon as the addon needs to be released.
This is not the same as the addon being unloaded or deactivated.
This function is your last chance to clean up your stuff.
*/
typedef void(GW2ADDON_CALL GW2AddonRelease_t)();
#define GW2ADDON_DLL_Release "GW2AddonRelease"


/**
Ensure compiler errors when exported addon functions are wrong.
*/
#ifndef _GW2ADDON_IMPORTS
#if GW2ADDON_VER == 1
typedef GW2AddonAPIV1 GW2AddonAPI;
#endif
GW2AddonAPI* GW2ADDON_CALL GW2AddonInitialize(int loaderVersion);
void GW2ADDON_CALL GW2AddonRelease();
#endif
