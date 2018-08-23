/**
This is an include file specifically for creating Guild Wars 2 add-ons with the GW2 Add-on Loader.
Visit https://github.com/Archomeda/gw2-addon-loader for more information.
*/

#pragma once
#include <d3d9.h>

#define GW2ADDON_RESULT int
#define GW2ADDON_CALL __stdcall
#define GW2ADDON_VER 1

/**
THE STRUCT TO IMPLEMENT CAN BE FOUND AFTER THE FUNCTION TYPEDEFS.

It's not defined how and when native add-ons are loaded.
Therefore, do not depend on DllMain for initialization. Use Load instead.
And don't forget to use Unload to clean up your mess.
*/


/**
The add-on update method. Defaults to NoUpdateMethod.
*/
enum AddonUpdateMethod {
    NoUpdateMethod = 0,
    CustomUpdateMethod = 1,
    GithubReleasesUpdateMethod = 2
};

/**
The add-on check update details when using a custom updater.
*/
struct UpdateCheckDetails {
    // The new version.
    // This will get matched against the version inside the current add-on instance,
    // if they differ, the add-on loader treats it as a new version.
    char* version;

    // The version maximum character buffer size, includes the null terminator.
    // Filled in by the add-on loader.
    int versionSize;

    // The release notes URL, e.g. add-on homepage or changelog.
    char* infoUrl;

    // The release notes URL maximum character buffer size, includes the null terminator.
    // Filled in by the add-on loader.
    int infoUrlSize;

    // The download URL for the new version.
    // If NULL, the add-on loader won't download it with the built-in HTTP downloader.
    // and you will have to provide the DownloadUpdate function.
    char* downloadUrl;

    // The download URL maximum character buffer size, includes the null terminuator.
    // Filled in by the add-on loader.
    int downloadUrlSize;
};


/**
Gets called whenever the add-on is loading.
Can be used to initialize the add-on when it gets enabled before it gets used.
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
This function should return true if it's handled and should not be handled by any other add-on or the game itself.
Return false whenever it should be handled by other add-ons and the game itself as well.
*/
typedef bool(GW2ADDON_CALL GW2AddonHandleWndProc_t)(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

/**
Gets called whenever the user clicks on the settings button in the add-on loader.
Can be used as an alternative to or together with keybinds.
*/
typedef void(GW2ADDON_CALL GW2AddonOpenSettings_t)();

/**
Gets called whenever the add-on loader wants the add-on to check for an update.
Only used for add-ons that implement custom updaters.
Check the struct documentation for more information about the returned struct as parameter.
All strings inside this struct will get deallocated after call.
This function should return 0 on success. Any other value will be treated as an error.
*/
typedef GW2ADDON_RESULT(GW2ADDON_CALL GW2AddonCheckUpdate_t)(UpdateCheckDetails* const details);

/**
A function that must be periodically called to feed the add-on loader with the data from the buffer.
ptr is the instance pointer that is passed to AddonDownloadUpdate; bytesWritten is the amount of bytes written in the buffer in this call; totalSize is the total file size in bytes.
Don't call this too often, but every time when your buffer is full, or when it's the last bits of data.
Do call this at the start to show the total size at the beginning, but with bytesWritten set to 0 (optional, but helpful in the UI).
If you do not know the total size, set it to 0.
*/
typedef void(GW2ADDON_CALL GW2AddonLoaderWriteBufferCallback_t)(void* ptr, unsigned int bytesWritten, unsigned int totalSize);

/**
Gets called whenever the add-on loader wants the add-on to download an update and the CheckUpdate function didn't return an update url.
Only used for add-ons that implement custom updaters and don't use HTTP hosting for their files.
This function is running asynchronously, wrapping it yourself is not needed.
Write the downloaded data back into the buffer, and call writeBufferCallback every time your buffer is full, and when the download is complete.
Don't exceed the buffer size. Writing past the allocated buffer causes undefined behavior and possibly crashes.
This function should return 0 on success. Any other value will be treated as an error.
*/
typedef GW2ADDON_RESULT(GW2ADDON_CALL GW2AddonDownloadUpdate_t)(void* ptr, char* buffer, int bufferSize, GW2AddonLoaderWriteBufferCallback_t* writeBufferCallback);

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
Do not call BeginScene yourself. The add-on loader does that already after this call.
*/
typedef void(GW2ADDON_CALL GW2AddonAdvPreBeginScene_t)(IDirect3DDevice9* pDev);

/**
Advanced function that gets called after D3D9 has processed the BeginScene call.
*/
typedef void(GW2ADDON_CALL GW2AddonAdvPostBeginScene_t)(IDirect3DDevice9* pDev);

/**
Advanced function that gets called before D3D9 processes the EndScene call.
Do not call EndScene yourself. The add-on loader does that already after this call.
*/
typedef void(GW2ADDON_CALL GW2AddonAdvPreEndScene_t)(IDirect3DDevice9* pDev);

/**
Advanced function that gets called after D3D9 has processed the EndScene call.
*/
typedef void(GW2ADDON_CALL GW2AddonAdvPostEndScene_t)(IDirect3DDevice9* pDev);

/**
Advanced function that gets called before D3D9 processes the Clear call.
Do not call Clear yourself. The add-on loader does that already after this call.
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
Do not call Present yourself. The add-on loader does that already after this call.
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
it is assumed that the add-on has created (overridden) the texture instead.
Calls to remaining add-ons will be skipped, and the add-on loader will not call CreateTexture either.
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
it is assumed that the add-on has created (overridden) the shader instead.
Calls to remaining add-ons will be skipped, and the add-on loader will not call CreateVertexShader either.
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
it is assumed that the add-on has created (overridden) the shader instead.
Calls to remaining add-ons will be skipped, and the add-on loader will not call CreatePixelShader either.
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
it is assumed that the add-on has created (overridden) the surface instead.
Calls to remaining add-ons will be skipped, and the add-on loader will not call CreateRenderTarget either.
*/
typedef HRESULT(GW2ADDON_CALL GW2AddonAdvPreCreateRenderTarget_t)(IDirect3DDevice9* pDev, UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Lockable, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle);

/**
Advanced function that gets called after D3D9 has processed the CreateRenderTarget call.

This function is disabled in competitive areas.
*/
typedef void(GW2ADDON_CALL GW2AddonAdvPostCreateRenderTarget_t)(IDirect3DDevice9* pDev, IDirect3DSurface9* ppSurface, UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Lockable, HANDLE* pSharedHandle);

/**
Advanced function that gets called before D3D9 processes the SetTexture call.
Do not call SetTexture yourself. The add-on loader does that already after this call.

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
Do not call SetVertexShader yourself. The add-on loader does that already after this call.

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
Do not call SetPixelShader yourself. The add-on loader does that already after this call.

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
Do not call SetRenderTarget yourself. The add-on loader does that already after this call.

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
Do not call SetRenderState yourself. The add-on loader does that already after this call.

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
Do not call DrawIndexedPrimitive yourself. The add-on loader does that already after this call.

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
THE FOLLOWING NEEDS TO BE IMPLEMENTED IN YOUR ADD-ON

Native add-ons support versioning in order to keep older compiled add-ons from breaking whenever something gets added.
Please always use the newest version whenever you compile your add-on by using the latest GW2AddonAPI struct.
Required functions need to be exported in order for the add-on to work properly.
Optional functions can be left out of the exported functions if you don't need its functionality.

Please see the examples on how to use this.

*********************************************************************************************************************/

/**
GW2AddonAPI is the add-on export table. Use the latest version.
Previous versions are kept for compatibility reasons only.
*/
typedef struct {
    // Versioning, automatically defined.
    const int ver = 0;
} GW2AddonAPIBase;

typedef struct {
    // Versioning, automatically defined.
    const int ver = 1;

    /** ADD-ON INFORMATION **/

    // The add-on ID, must be unique and fixed as long as it's the same add-on. Null terminated.
    const char* id;

    // The add-on name. Null terminated.
    const char* name;

    // The add-on author. Null terminated.
    const char* author;

    // The add-on description. Null terminated.
    const char* description;

    // The add-on version. Null terminated.
    const char* version;

    // The add-on homepage URL. Null terminated.
    const char* homepage;

    // The add-on icon, should be 32x32 pixels.
    // This is a pointer that should point to either:
    //  - The already loaded image in memory.
    //    In this case set the iconSize to the image size in bytes.
    //    NOTE: You are responsible for releasing the original image data in memory, if needed.
    //  - The already loaded texture in memory (type should be IDirect3DTexture9*).
    //    In this case leave the iconSize as -1.
    //    NOTE: You are responsible for releasing the D3D9 texture.
    // Supported image types can be found at https://msdn.microsoft.com/en-US/library/windows/desktop/bb172803(v=vs.85).aspx under the Remarks section.
    void* icon;
    int iconSize = -1;

    struct {
        // The add-on update method.
        // - NoUpdateMethod: Add-on doesn't support updating.
        // - CustomUpdateMethod: Add-on provides custom functions to check for updates and to download updates.
        // - GithubReleasesUpdateMethod: Add-on uses GitHub releases for updating.
        //   Add-on should provide the GitHub repository name in the githubRepo field.
        //   Make sure that the binaries in the releases are either in .dll or .zip format
        //   and have the architecture somewhere in the file name (x86 for 32-bit and x64 for 64-bit builds).
        AddonUpdateMethod method = NoUpdateMethod;

        union {
            // Additional method information. Reserved, unused directly.
            const char* methodInfo;

            // The GitHub repository (e.g. Archomeda/gw2-addon-loader) if using the GithubReleases method. Null terminated.
            const char* githubRepo;
        };
    } updateInfo;


    /** ADD-ON FUNCTION EXPORTS **/
    GW2AddonOpenSettings_t* OpenSettings;

    GW2AddonCheckUpdate_t* CheckUpdate; // Only used in combination with CustomUpdateMethod
    GW2AddonDownloadUpdate_t* DownloadUpdate; // Only used in combination with CustomUpdateMethod and when no downloadUrl is provided in the CheckUpdate returned struct

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
Gets called as soon as the add-on needs to be initialized.
This is not the same as the add-on being loaded or activated.
This function provides all the necessary information for the add-on loader.
*/
typedef GW2AddonAPIBase* (GW2ADDON_CALL GW2AddonInitialize_t)(int loaderVersion);
#define GW2ADDON_DLL_Initialize "GW2AddonInitialize"

/**
Exported C-function: GW2AddonRelease. Required.
Gets called as soon as the add-on needs to be released.
This is not the same as the add-on being unloaded or deactivated.
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
