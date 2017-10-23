#include "main.h"

#include <tchar.h>
#include <fstream>

#include "hook/hook.h"


#define CONFIG_PATH "%DOCUMENTS%\\Guild Wars 2\\addons\\Loader"

using namespace std;


void Draw(IDirect3DDevice9* dev) {
    dev->BeginScene();

    LPD3DXFONT font = nullptr;
    D3DXCreateFont(dev, 17, 0, FW_BOLD, 0, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH | FF_DONTCARE, TEXT("Arial"), &font);
    D3DCOLOR color = D3DCOLOR_ARGB(255, 200, 200, 200);
    RECT rect;
    rect.left = 0;
    rect.right = 1920;
    rect.top = 0;
    rect.bottom = 100;
    font->DrawTextA(NULL, TEXT("Testing Loader!"), -1, &rect, 0, color);
    font->Release();

    dev->EndScene();
}


void PreCreateDevice(HWND hFocusWindow) {

}

void PostCreateDevice(IDirect3DDevice9* temp_device, D3DPRESENT_PARAMETERS *pPresentationParameters) {
    
}

void PreReset() {

}

void PostReset(IDirect3DDevice9* dev, D3DPRESENT_PARAMETERS *pPresentationParameters) {

}


void Shutdown() {
    PreReset();
}


bool WINAPI DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved) {
    switch (fdwReason) {
        case DLL_PROCESS_ATTACH: {
            if (!InitializeHook()) {
                return false;
            }
        }
        break;
        case DLL_PROCESS_DETACH: {
            UninitializeHook();
        }
        break;
    }
    return true;
}
