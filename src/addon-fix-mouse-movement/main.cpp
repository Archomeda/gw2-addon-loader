#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "../../include/gw2addon-native.h"

using namespace std;

HWND focusWindow;
WNDPROC BaseWndProc;
POINT cursorPos;
bool lmbDown = false;
bool rmbDown = false;
bool isClipped = false;

bool GW2ADDON_CALL WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    // The way how Guild Wars 2 works with the mouse while the camera is being rotated,
    // is that it hides the cursor and confines the cursor to the Guild Wars 2 window.
    // However, the cursor still moves in the background and is only getting reset
    // to its original position when the user releases the mouse button.

    // You can easily notice this when you hold the right mouse button, move the cursor
    // towards the minimap (while invisible), and scroll up or down.
    // You'll notice that the minimap zooms in or out, instead of the camera itself.
    // The reason is because the hidden cursor is actually on top of the minimap,
    // even though it was last visible somewhere else.
    // This may cause some confusion and frustration.

    // What it should do instead, is that it confines the cursor to its current position while it's hidden.
    // This way, the cursor cannot move from its original spot while the user is rotating the camera.
    // Everyone happy!

    // P.S. ArenaNet, if you're reading this, please consider fixing this in the engine.
    // It's literally only a small piece of code.

    if (msg != WM_LBUTTONDOWN && msg != WM_LBUTTONUP && msg != WM_RBUTTONDOWN && msg != WM_RBUTTONUP && msg != WM_MOUSEMOVE) {
        return false;
    }

    CURSORINFO cursorInfo;
    cursorInfo.cbSize = sizeof(CURSORINFO);
    GetCursorInfo(&cursorInfo);

    switch (msg) {
    case WM_LBUTTONDOWN:
        lmbDown = true;
        cursorPos = cursorInfo.ptScreenPos;
        break;
    case WM_LBUTTONUP:
        lmbDown = false;
        break;
    case WM_RBUTTONDOWN:
        rmbDown = true;
        cursorPos = cursorInfo.ptScreenPos;
        break;
    case WM_RBUTTONUP:
        rmbDown = false;
        break;
    }

    if ((lmbDown || rmbDown) && !cursorInfo.hCursor && !isClipped) {
        // Set the space
        RECT rect;
        rect.left = cursorPos.x;
        rect.top = cursorPos.y;
        rect.right = cursorPos.x;
        rect.bottom = cursorPos.y;
        ClipCursor(&rect);
        isClipped = true;
    }
    else if (isClipped && cursorInfo.hCursor) {
        // Reset the space
        ClipCursor(NULL);
        isClipped = false;
    }

    return false;
}

GW2AddonAPIV1* GW2ADDON_CALL GW2AddonInitialize(int loaderVersion) {
    static GW2AddonAPIV1 addon;
    addon.id = "fix-mouse-movement";
    addon.name = "Fix Mouse Movement";
    addon.author = "Archomeda";
    addon.version = "1.0";
    addon.description = "A small add-on that fixes the mouse cursor movement while moving the camera by holding down the left or right mouse button.\nNormally the cursor still moves in the background, even though it's hidden. This causes the game to accept e.g. scroll wheel input in various GUI panels instead of using it for zooming. This add-on fixes that.";
    addon.homepage = "https://github.com/Archomeda/gw2-addon-loader";
    addon.HandleWndProc = &WndProc;
    return &addon;
}

void GW2ADDON_CALL GW2AddonRelease() {

}
