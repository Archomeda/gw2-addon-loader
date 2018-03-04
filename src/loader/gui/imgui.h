#pragma once
#include "../windows.h"
#include <d3d9.h>
#include <IconsMaterialDesign.h>
#include <imgui.h>

namespace loader {
    namespace gui {
        namespace imgui {

            static const ImWchar FontMaterialIconsRange[] = { ICON_MIN_MD, ICON_MAX_MD, 0 };
            extern ImFont* FontMain;
            extern ImFont* FontIconButtons;

            bool ProcessWndProc(UINT msg, WPARAM wParam, LPARAM lParam);

            bool Initialize(HMODULE hModule, HWND hWnd, IDirect3DDevice9* device);
            void Shutdown();
            void NewFrame();
            bool UpdateMouseCursor();
            bool CreateDeviceObjects();
            void InvalidateDeviceObjects();

        }
    }
}
