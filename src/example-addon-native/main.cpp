/**
This example is here to show how you can create an addon that is supported by GW2 Addon Loader.
Compared to legacy addons, it's not needed to hook onto the D3D9 functions yourself at all, which saves a lot of time.
There are just a few functions that you have to create in your code that will be detected by the loader.
They can be found in the include/gw2addon-native.h header file in the root folder of this project.
Make sure to read the comments there as well.
*/

// We include some general things here to support our example.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <d3d9.h>
#include <d3dx9core.h>
#include <string>

// This is our addon include file
#include "../../include/gw2addon-native.h"

using namespace std;

// We defined our addon info struct globally
GW2AddonInfo info = {};

// Just a few states
bool loader = false;
HWND focusWindow;
IDirect3DDevice9* device;
int frame = 0;

#ifdef __cplusplus
extern "C" {
#endif

    GW2ADDON_API GW2ADDON_RESULT GW2ADDON_CALL GW2_GetAddonInfo(GW2AddonInfo** addonInfo) {
        // Export our info

        // Make sure to check if we've already populated our struct so we don't do this again
        if (!info.idSize) {
            info.id = L"example-native";
            info.idSize = wcslen(info.id);
            info.name = L"Example Native Addon";
            info.nameSize = wcslen(info.name);
            info.author = L"Archomeda";
            info.authorSize = wcslen(info.author);
            info.description = L"An example to show how native addons work.";
            info.descriptionSize = wcslen(info.description);
            info.version = L"1.0";
            info.versionSize = wcslen(info.version);
            info.homepage = L"https://github.com/Archomeda/gw2-addon-loader";
            info.homepageSize = wcslen(info.homepage);
        }
        *addonInfo = &info;
        return 0;
    }

    GW2ADDON_API GW2ADDON_RESULT GW2ADDON_CALL GW2_Load(HWND hFocusWindow, IDirect3DDevice9* pDev) {
        // Our loading entrypoint.
        // You can initialize whatever you want to initialize here.
        focusWindow = hFocusWindow;
        device = pDev;
        return 0;
    }

    GW2ADDON_API GW2ADDON_RESULT GW2ADDON_CALL GW2_Unload() {
        // Unload all used resources, do not neglect this.
        focusWindow = NULL;
        device = NULL;
        return 0;
    }

    GW2ADDON_API void GW2ADDON_CALL GW2_DrawFrameBeforePostProcessing(IDirect3DDevice9* pDev) {
        // One of our extra draw entrypoints.
        // This gets called before the game adds post processing.
        // Do NOT process longstanding tasks here.

        // Drawing this text is painfully slow; this is just an example.
        // Don't use this in your own addon.
        LPD3DXFONT font;
        RECT rect;
        rect.left = 10;
        rect.top = 50;
        rect.right = rect.left + 400;
        rect.bottom = rect.top + 16;
        D3DXCreateFont(pDev, 14, 0, 0, 0, false, DEFAULT_CHARSET, OUT_CHARACTER_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, L"Consolas", &font);
        wstring text = L"I am drawn behind the post processing layer";
        font->DrawText(0, text.c_str(), text.length(), &rect, DT_NOCLIP, D3DCOLOR_ARGB(255, 255, 255, 0));
        font->Release();
    }

    GW2ADDON_API void GW2ADDON_CALL GW2_DrawFrame(IDirect3DDevice9* pDev) {
        // Our main draw entrypoint.
        // Here you can draw whatever you want.
        // Do NOT process longstanding tasks here.

        // Drawing this text is painfully slow; this is just an example.
        // Don't use this in your own addon.
        frame++;
        LPD3DXFONT font;
        RECT rect;
        rect.left = 10;
        rect.top = 10;
        rect.right = rect.left + 400;
        rect.bottom = rect.top + 16;
        D3DXCreateFont(pDev, 14, 0, 0, 0, false, DEFAULT_CHARSET, OUT_CHARACTER_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, L"Consolas", &font);
        wstring text = L"Example add-on - frame " + to_wstring(frame) + L" (loader = " + (loader ? L"true" : L"false") + L")";
        font->DrawText(0, text.c_str(), text.length(), &rect, DT_NOCLIP, D3DCOLOR_ARGB(255, 0, 255, 0));
        font->Release();
    }

#ifdef __cplusplus
}
#endif

/**
For legacy purposes, if you want your addon to be compabible with both the addon loader and d3d9 calls,
you'll probably want to implement this as well.
How to write a legacy d3d9 addon is out of scope for this example.
*/
bool WINAPI DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved) {
    wstring var;
    switch (fdwReason) {
    case DLL_PROCESS_ATTACH:
        // This is where the addon gets loaded by the game or the addon loader.
        // In order to determine if the addon loader is active, you can look for the environment variable _IsGW2AddonLoaderActive.
        // If this variable exists, you know the addon is loaded by the addon loader, and not through conventional d3d9 means
        wchar_t buff[16];
        GetEnvironmentVariable(L"_IsGW2AddonLoaderActive", buff, 16);
        var = wstring(buff);
        if (!var.empty()) {
            // Loaded through GW2 Addon Loader.
            // The actual value will be "1", but it may change in the future.

            // For demo purposes, we set this variable to true and draw it on screen to show that it works.
            loader = true;
        }
        else {
            // Loaded through other means.
            loader = false;
        }

        break;
    case DLL_PROCESS_DETACH:
        // This is where the addon gets unloaded by the game.
        // Clean up all your loaded stuff if the addon wasn't loaded through the addon loader.
        break;
    }
    return true;
}
