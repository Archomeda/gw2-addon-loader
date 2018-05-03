#pragma once
#include "../stdafx.h"

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
