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
