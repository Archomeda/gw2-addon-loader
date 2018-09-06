#pragma once
#include "../stdafx.h"
#include "../Config.h"

namespace addon::qol {

    void DisableHighlightCursor(IDirect3DDevice9* const pDev);
    void EnableHighlightCursor(CursorHighlightType type, HMODULE hModule, IDirect3DDevice9* const pDev);
    void RenderHighlightCursor(HWND hWnd, IDirect3DDevice9* const pDev);
    void TriggerHighlightCursor();

}
