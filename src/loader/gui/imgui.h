#pragma once
#include "../windows.h"
#include <d3d9.h>
#include <imgui.h>

namespace loader {
    namespace gui {
        namespace imgui {

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
