#pragma once
#include "../windows.h"
#include <d3d9.h>

namespace loader {
    namespace hooks {

        typedef void(*PreCreateDevice_t)(HWND hFocusWindow);
        typedef void(*PostCreateDevice_t)(HWND hFocusWindow, IDirect3DDevice9* pDev, D3DPRESENT_PARAMETERS* pPresentationParameters);
        typedef void(*PrePresent_t)(IDirect3DDevice9* pDev);
        typedef void(*PostPresent_t)(IDirect3DDevice9* pDev);
        typedef void(*PreReset_t)(IDirect3DDevice9* pDev, D3DPRESENT_PARAMETERS* pPresentationParameters);
        typedef void(*PostReset_t)(IDirect3DDevice9* pDev, D3DPRESENT_PARAMETERS* pPresentationParameters);

        struct D3D9Hooks {
            PreCreateDevice_t PreCreateDevice;
            PostCreateDevice_t PostCreateDevice;
            PrePresent_t PrePresent;
            PostPresent_t PostPresent;
            PreReset_t PreReset;
            PostReset_t PostReset;
        };

        bool InitializeHooks();
        void UninitializeHooks();

        void SetD3D9Hooks(const D3D9Hooks& hooks);
    }
}
