#pragma once
#include "../stdafx.h"

namespace addon::qol {

    bool ConfineCursor(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    void DisableConfineCursor();
    void EnableConfineCursor(HWND hWnd);

}
