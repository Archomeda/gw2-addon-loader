#pragma once
#include "../stdafx.h"
#include "../Config.h"

namespace addon::qol {

    void DisableHighlightCursor(IDirect3DDevice9* const pDev);
    void EnableHighlightCursor(CursorHighlightType type, HMODULE hModule, HWND hWnd, IDirect3DDevice9* const pDev);
    bool IsCursorHighlighted();
    void RenderHighlightCursor(IDirect3DDevice9* const pDev);
    void TriggerHighlightCursor();
    void UpdateCursorPos(POINT pos);

}
