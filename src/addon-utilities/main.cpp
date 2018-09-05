//TODO: Split this add-on from the main loader repository and move it into its own repository

#include "main.h"
#include "Config.h"
#include "fixes/mouse_movement.h"
#include "qol/coherent_priority.h"
#include "qol/confine_cursor.h"
#include "qol/highlight_cursor.h"

using namespace std;
using namespace addon;

HWND hWindow;
HMODULE dllModule;
IDirect3DDevice9* device;


GW2ADDON_RESULT GW2ADDON_CALL Load(HWND hFocusWindow, IDirect3DDevice9* pDev) {
    hWindow = hFocusWindow;
    device = pDev;
    AppConfig.Initialize();

    MH_Initialize();

    if (AppConfig.GetConfineCursor()) {
        qol::EnableConfineCursor(hWindow);
    }
    if (AppConfig.GetCoherentPriority() > 0) {
        qol::EnableCoherentPriority(AppConfig.GetCoherentPriority());
    }
    if (AppConfig.GetHighlightCursor()) {
        qol::EnableHighlightCursor(dllModule, pDev);
    }

    return 0;
}

void GW2ADDON_CALL DrawFrame(IDirect3DDevice9* pDev) {
    if (AppConfig.GetHighlightCursor()) {
        qol::RenderHighlightCursor(hWindow, pDev);
    }
}

bool GW2ADDON_CALL WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (AppConfig.GetHighlightCursor() && msg == WM_KEYDOWN && wParam == VK_CONTROL) {
        qol::TriggerHighlightCursor();
    }

    bool result = false;
    if (!result && AppConfig.GetCursorMovementFix()) {
        result = fixes::FixMouseMovement(hWnd, msg, wParam, lParam);
    }
    if (!result && AppConfig.GetConfineCursor()) {
        result = qol::ConfineCursor(hWnd, msg, wParam, lParam);
    }
    return result;
}

AddonSettings* GW2ADDON_CALL GetSettings(AddonSettings* const settings) {
    static bool settingCursorMovementFix = false;
    static CoherentPriorityLevel settingCoherentPriority = CoherentPriorityLevel::CoherentPriorityDontChange;
    static bool settingConfineCursor = false;
    static bool settingHighlightCursor = false;

    if (settings == nullptr) {
        static AddonSettingsEntry entries[9];

        entries[0].definition.type = AddonSettingsEntryType::SettingsTypeText;
        entries[0].definition.name = "Fixes";

        entries[1].definition.type = AddonSettingsEntryType::SettingsTypeBoolean;
        entries[1].definition.name = "Enable mouse cursor camera movement fix";
        entries[1].definition.hint = "When the left or right mouse button is pressed to rotate the camera,\nthe game does not confine the mouse cursor to its current location,\ncausing it to still move in the background.\nBecause of this, the game can accept input on odd places like\nscrolling in a UI window, while the cursor wasn't initially there.\n\nEnabling this will fix this issue by confining the cursor properly.";
        settingCursorMovementFix = AppConfig.GetCursorMovementFix();
        entries[1].boolValue = &settingCursorMovementFix;

        entries[2].definition.type = AddonSettingsEntryType::SettingsTypeSeparator;

        entries[3].definition.type = AddonSettingsEntryType::SettingsTypeText;
        entries[3].definition.name = "Quality of Life";

        entries[4].definition.type = AddonSettingsEntryType::SettingsTypeOption;
        entries[4].definition.name = "Coherent UI process priority";
        entries[4].definition.hint = "Coherent UI renders the trading post, gem store and various guild hall related UI windows.\nOn computers with 4 or less logical CPU cores, Coherent UI might not get enough\nprocessing time to render these windows properly without performance issues.\nIf this is the case, consider changing the priority to a higher value.";
        entries[4].definition.listSize = 4;
        static const char* coherentPriorities[4];
        coherentPriorities[0] = "Don't change (default)";
        coherentPriorities[1] = "Normal";
        coherentPriorities[2] = "Above Normal";
        coherentPriorities[3] = "High";
        entries[4].definition.listNames = &coherentPriorities[0];
        settingCoherentPriority = AppConfig.GetCoherentPriority();
        entries[4].optionValue = reinterpret_cast<int*>(&settingCoherentPriority);

        entries[5].definition.type = AddonSettingsEntryType::SettingsTypeBoolean;
        entries[5].definition.name = "Confine cursor to window";
        entries[5].definition.hint = "When playing in windowed fullscreen, the game does not restrict the mouse cursor to the game window.\nThis might unintentionally cause the mouse cursor to go outside the window borders when using multiple monitors.\n\nEnabling this will confine the mouse cursor to the game window at all times.";
        settingConfineCursor = AppConfig.GetConfineCursor();
        entries[5].boolValue = &settingConfineCursor;

        entries[6].definition.type = AddonSettingsEntryType::SettingsTypeSeparator;

        entries[7].definition.type = AddonSettingsEntryType::SettingsTypeText;
        entries[7].definition.name = "Testing";

        entries[8].definition.type = AddonSettingsEntryType::SettingsTypeBoolean;
        entries[8].definition.name = "Highlight cursor";
        entries[8].definition.hint = "Enabling this will highlight the cursor to make it easier to locate.";
        settingHighlightCursor = AppConfig.GetHighlightCursor();
        entries[8].boolValue = &settingHighlightCursor;

        static AddonSettings addonSettings;
        addonSettings.entriesSize = 9;
        addonSettings.entries = &entries[0];

        return &addonSettings;
    }
    else {
        AppConfig.SetCursorMovementFix(settingCursorMovementFix);
        AppConfig.SetCoherentPriority(settingCoherentPriority);
        if (settingCoherentPriority > 0) {
            qol::EnableCoherentPriority(settingCoherentPriority);
        }
        else {
            qol::DisableCoherentPriority();
        }
        AppConfig.SetConfineCursor(settingConfineCursor);
        if (settingConfineCursor) {
            qol::EnableConfineCursor(hWindow);
        }
        else {
            qol::DisableConfineCursor();
        }
        AppConfig.SetHighlightCursor(settingHighlightCursor);
        if (settingHighlightCursor) {
            qol::EnableHighlightCursor(dllModule, device);
        }
        else {
            qol::DisableHighlightCursor(device);
        }
        return nullptr;
    }
}


void GW2ADDON_CALL PreReset(IDirect3DDevice9* pDev, D3DPRESENT_PARAMETERS* pPresentationParameters) {
    if (AppConfig.GetHighlightCursor()) {
        qol::DisableHighlightCursor(pDev);
    }
}

void GW2ADDON_CALL PostReset(IDirect3DDevice9* pDev, D3DPRESENT_PARAMETERS* pPresentationParameters) {
    if (AppConfig.GetHighlightCursor()) {
        qol::EnableHighlightCursor(dllModule, pDev);
    }
}



GW2AddonAPIV1* GW2ADDON_CALL GW2AddonInitialize(int loaderVersion) {
    static GW2AddonAPIV1 addon;
    addon.id = "utilities";
    addon.name = "Utilities";
    addon.author = "Archomeda";
    addon.version = "1.0";
    addon.description = "An add-on that contains a collection of various Guild Wars 2 utilities.";
    addon.homepage = "https://github.com/Archomeda/gw2-addon-loader";
    addon.Load = &Load;
    addon.HandleWndProc = &WndProc;
    addon.DrawFrame = &DrawFrame;
    addon.OpenSettings = &GetSettings;
    addon.AdvPreReset = &PreReset;
    addon.AdvPostReset = &PostReset;
    return &addon;
}

void GW2ADDON_CALL GW2AddonRelease() {
    if (AppConfig.GetConfineCursor()) {
        qol::DisableConfineCursor();
    }
    if (AppConfig.GetCoherentPriority() > 0) {
        qol::DisableCoherentPriority();
    }
    if (AppConfig.GetHighlightCursor()) {
        qol::DisableHighlightCursor(device);
    }

    MH_Uninitialize();
}

bool WINAPI DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved) {
    switch (fdwReason) {
    case DLL_PROCESS_ATTACH:
        dllModule = hModule;
        break;
    case DLL_PROCESS_DETACH:
        // Force release it here, because the add-on will unload earlier than the loader
        GW2AddonRelease();
        break;
    }
    return true;
}
