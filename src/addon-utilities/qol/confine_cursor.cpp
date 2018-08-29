#include "confine_cursor.h"

namespace addon::qol {

    bool ConfineCursor(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        switch (msg) {
        case WM_SETFOCUS: {
            EnableConfineCursor(hWnd);
            break;
        }
        case WM_KILLFOCUS:
            DisableConfineCursor();
            break;
        }
        return false;
    }

    void DisableConfineCursor() {
        ClipCursor(NULL);
    }

    void EnableConfineCursor(HWND hWnd) {
        RECT rect;
        if (GetWindowRect(hWnd, &rect)) {
            ClipCursor(&rect);
        }
    }

}
