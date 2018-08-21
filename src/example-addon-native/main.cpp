/**
This example is here to show how you can create an add-on that is supported by GW2 Add-on Loader.
Compared to legacy add-ons, it's not needed to hook onto the D3D9 functions yourself at all, which saves a lot of time.
There are just a few functions that you have to create in your code that will be detected by the loader.
They can be found in the include/gw2addon-native.h header file in the root folder of this project.
Make sure to read the comments there as well.
*/

// Won't compile with C++17 otherwise, I'm not bothered to write my own converter
#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING

// We include some general things here to support our example.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <codecvt>
#include <d3d9.h>
#include <d3dx9core.h>
#include <string>
#include "resource.h"

// This is our add-on include file
#include "../../include/gw2addon-native.h"

using namespace std;

// Just a few states
HMODULE dllModule;
bool loader = false;
string apiKey;
HWND focusWindow;
IDirect3DDevice9* device;
int frame = 0;

// Forward declare a few functions in order to make this file an easier read
GW2ADDON_RESULT GW2ADDON_CALL Load(HWND hFocusWindow, IDirect3DDevice9* pDev);
void GW2ADDON_CALL Draw(IDirect3DDevice9* pDev);
void GW2ADDON_CALL DrawBeforePostProcessing(IDirect3DDevice9* pDev);
void GW2ADDON_CALL DrawBeforeGui(IDirect3DDevice9* pDev);
void GW2ADDON_CALL ApiKeyChange(const char* key);
GW2ADDON_RESULT GW2ADDON_CALL CheckUpdate(UpdateCheckDetails* details);

// A helper to convert a string to a wstring
wstring u16(const string& str) {
    return wstring_convert<codecvt_utf8<wchar_t>>().from_bytes(str);
}

//***************************************
// The actual implementation starts here
//***************************************

GW2AddonAPIV1* GW2ADDON_CALL GW2AddonInitialize(int loaderVersion) {
    // This is called whenever the add-on loader needs the information about your add-on.
    // So we export our information that's statically defined.
    // Refer to the header file for all possible fields.
    static GW2AddonAPIV1 addon;
    addon.id = "example-native";
    addon.name = "Example Native Add-on";
    addon.author = "Archomeda";
    addon.version = "1.0";
    addon.description = "An example to show how native addons work.";
    addon.homepage = "https://github.com/Archomeda/gw2-addon-loader";
    addon.Load = &Load;
    addon.DrawFrameBeforePostProcessing = &DrawBeforePostProcessing;
    addon.DrawFrameBeforeGui = &DrawBeforeGui;
    addon.DrawFrame = &Draw;
    addon.ApiKeyChange = &ApiKeyChange;
    addon.updateInfo.method = AddonUpdateMethod::CustomUpdateMethod;
    addon.CheckUpdate = &CheckUpdate;

    HRSRC hIconResInfo = FindResource(dllModule, MAKEINTRESOURCE(IDB_PNGICON), L"PNG");
    HGLOBAL hIconRes = hIconResInfo ? LoadResource(dllModule, hIconResInfo) : NULL;
    if (hIconRes) {
        addon.icon = LockResource(hIconRes);
        addon.iconSize = SizeofResource(dllModule, hIconResInfo);
    }

    return &addon;
}

void GW2ADDON_CALL GW2AddonRelease() {
    // This is called whenever the add-on is being unloaded.
    // So do not neglect this and clean up your stuff.
    focusWindow = NULL;
    device = NULL;
}

GW2ADDON_RESULT GW2ADDON_CALL Load(HWND hFocusWindow, IDirect3DDevice9* pDev) {
    // Our loading entrypoint.
    // This is called before the add-on is activated.
    // You can initialize whatever you want to initialize here.
    focusWindow = hFocusWindow;
    device = pDev;

    // We return 0 on success.
    return 0;
}

void GW2ADDON_CALL Draw(IDirect3DDevice9* pDev) {
    // Our main draw entrypoint.
    // Here you can draw whatever you want.
    // Do NOT process longstanding tasks here.

    // Drawing this text is painfully slow; this is just an example.
    // Don't use this in your own add-on.
    ++frame;
    LPD3DXFONT font;
    RECT rect;
    rect.left = 10;
    rect.top = 10;
    rect.right = rect.left + 400;
    rect.bottom = rect.top + 16;
    D3DXCreateFont(pDev, 14, 0, 0, 0, false, DEFAULT_CHARSET, OUT_CHARACTER_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, L"Consolas", &font);
    wstring text = L"Example add-on - frame " + to_wstring(frame) + L" (loader = " + (loader ? L"true" : L"false") + L", apiKey = " + u16(apiKey) + L")";
    font->DrawText(0, text.c_str(), static_cast<INT>(text.length()), &rect, DT_NOCLIP, D3DCOLOR_ARGB(255, 0, 255, 0));
    font->Release();
}

void GW2ADDON_CALL DrawBeforePostProcessing(IDirect3DDevice9* pDev) {
    // One of our extra draw entrypoints.
    // This gets called before the game adds post processing.
    // Do NOT process longstanding tasks here.

    // Drawing this text is painfully slow; this is just an example.
    // Don't use this in your own add-on.
    LPD3DXFONT font;
    RECT rect;
    rect.left = 10;
    rect.top = 50;
    rect.right = rect.left + 400;
    rect.bottom = rect.top + 16;
    D3DXCreateFont(pDev, 14, 0, 0, 0, false, DEFAULT_CHARSET, OUT_CHARACTER_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, L"Consolas", &font);
    wstring text = L"I am drawn behind the post processing layer";
    font->DrawText(0, text.c_str(), static_cast<INT>(text.length()), &rect, DT_NOCLIP, D3DCOLOR_ARGB(255, 255, 255, 0));
    font->Release();
}

void GW2ADDON_CALL DrawBeforeGui(IDirect3DDevice9* pDev) {
    // One of our extra draw entrypoints.
    // This gets called before the game draws its GUI.
    // Do NOT process longstanding tasks here.

    // Drawing this text is painfully slow; this is just an example.
    // Don't use this in your own add-on.
    LPD3DXFONT font;
    RECT rect;
    rect.left = 10;
    rect.top = 30;
    rect.right = rect.left + 400;
    rect.bottom = rect.top + 16;
    D3DXCreateFont(pDev, 14, 0, 0, 0, false, DEFAULT_CHARSET, OUT_CHARACTER_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, L"Consolas", &font);
    wstring text = L"I am drawn behind the GUI";
    font->DrawText(0, text.c_str(), static_cast<INT>(text.length()), &rect, DT_NOCLIP, D3DCOLOR_ARGB(255, 0, 255, 255));
    font->Release();
}

void GW2ADDON_CALL ApiKeyChange(const char* key) {
    // This is where we get the shared API key.
    // You can do whatever you want with the key, but you have to check its validity yourself.
    
    if (key == nullptr) {
        apiKey = "";
        return;
    }
    
    // Copy it because the parameter key will be cleaned up after this function.
    // 73 should be the max length of an API key + the null terminator.
    char k[73];
    strcpy_s(k, sizeof(k), key);
    apiKey = string(k);
}

GW2ADDON_RESULT GW2ADDON_CALL CheckUpdate(UpdateCheckDetails* details) {
    // In case you want to have your own custom update checker,
    // you can use this callback to determine if your add-on has an update available.
    // Check the header file under the AddonUpdateMethod enum for other built-in options.

    // The details parameter is the struct where we write our information in.
    // Every c-string has its size defined from the add-on loader.
    // You have to use this size to determine the maximum amount of characters you can write into the buffer.

    // Of course, this example has no valid download, but this is here to illustrate how it works.
    strcpy_s(details->version, details->versionSize, "2.0");
    strcpy_s(details->infoUrl, details->infoUrlSize, "https://github.com/Archomeda/gw2-addon-loader");
    strcpy_s(details->downloadUrl, details->downloadUrlSize, "https://github.com/Archomeda/gw2-addon-loader");

    // We return 0 on success.
    return 0;
}

/**
For legacy purposes.
If you want your add-on to be compatible with both the add-on loader and traditional d3d9 hooking,
you'll probably want to implement this as well.
How to write support for d3d9 hooking is out of scope for this example.
*/
bool WINAPI DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved) {
    string var;
    switch (fdwReason) {
    case DLL_PROCESS_ATTACH: {
        // This is where the add-on gets loaded by the game or the add-on loader.
        // In order to determine if the add-on loader is active, you can look for the environment variable _IsGW2AddonLoaderActive.
        // If this variable exists, you know the add-on is loaded by the add-on loader, and not through conventional d3d9 means
        char buff[16] = {};
        if (GetEnvironmentVariableA("_IsGW2AddonLoaderActive", buff, 16)) {
            var = string(buff);
            if (!var.empty()) {
                // Loaded through GW2 Add-on Loader.
                // The actual value will be "1", but it may change in the future.

                // For demo purposes, we set this variable to true and draw it on screen to show that it works.
                loader = true;
            }
        }
        if (!loader) {
            // Loaded through other means.
        }

        dllModule = hModule;

        break;
    }
    case DLL_PROCESS_DETACH:
        // This is where the add-on gets unloaded by the game.
        // Clean up all your loaded stuff if the add-on wasn't loaded through the add-on loader.
        break;
    }
    return true;
}
