#pragma once
#include "../windows.h"
#include <d3d9.h>

namespace loader {
    namespace gui {
        namespace imgui {

            bool ProcessWndProc(UINT msg, WPARAM wParam, LPARAM lParam);

            bool Initialize(HWND hWnd, IDirect3DDevice9* device);
            void Shutdown();
            void NewFrame();
            bool UpdateMouseCursor();
            bool CreateDeviceObjects();
            void InvalidateDeviceObjects();

        }
    }
}
