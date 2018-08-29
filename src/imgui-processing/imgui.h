#pragma once
#include "stdafx.h"

namespace loader::imgui {

    extern ImFont* FontMain;
    extern ImFont* FontIconButtons;

    bool ProcessImGuiInput(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

    bool ProcessWndProc(UINT msg, WPARAM wParam, LPARAM lParam);

    bool Initialize(HMODULE hModule, HWND hWnd, IDirect3DDevice9* device);
    void Shutdown();
    void NewFrame();
    void Render();
    bool UpdateMouseCursor();
    void UpdateMousePos();
    bool CreateDeviceObjects();
    void InvalidateDeviceObjects();

}
