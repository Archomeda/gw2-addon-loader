#include "Config.h"
#include "globals.h"
#include "utils/file.h"

using namespace std;
using namespace std::filesystem;
using namespace addon::utils;

namespace addon {

    Config AppConfig;

    void Config::Initialize() {
        // Create path
        wchar_t fileName[MAX_PATH];
        GetModuleFileName(NULL, fileName, sizeof(fileName));
        PathRemoveFileSpec(fileName);
        path configFolder(fileName);
        configFolder /= CONFIG_FOLDER;
        if (!FolderExists(configFolder.u8string())) {
            SHCreateDirectory(NULL, configFolder.c_str());
        }

        // Our config file
        path configPath(configFolder);
        configPath /= this->configName;
        this->configPath = configPath.u8string();

        // Load settings
        this->ini.SetUnicode();
        this->ini.LoadFile(this->configPath.c_str());

        this->cursorMovementFix = this->ini.GetBoolValue(L"fixes", L"cursor_movement_fix", false);
        this->coherentPriority = static_cast<CoherentPriorityLevel>(this->ini.GetLongValue(L"qol", L"coherent_priority", 0));
        this->confineCursor = this->ini.GetBoolValue(L"qol", L"confine_cursor", false);
        this->cursorHighlightType = static_cast<CursorHighlightType>(this->ini.GetLongValue(L"testing", L"cursor_highlight_type", false));
        this->cursorHighlightKey = this->ini.GetLongValue(L"testing", L"cursor_highlight_key", VK_CONTROL);
        this->cursorHighlightColor = this->ini.GetLongValue(L"testing", L"cursor_highlight_color", 0xFFFFFF);
    }

    void Config::SetCursorMovementFix(bool cursorMovementFix) {
        this->cursorMovementFix = cursorMovementFix;
        this->ini.SetBoolValue(L"fixes", L"cursor_movement_fix", cursorMovementFix);
        this->ini.SaveFile(this->configPath.c_str());
    }

    void Config::SetCoherentPriority(CoherentPriorityLevel coherentPriority) {
        this->coherentPriority = coherentPriority;
        this->ini.SetLongValue(L"qol", L"coherent_priority", coherentPriority);
        this->ini.SaveFile(this->configPath.c_str());
    }

    void Config::SetConfineCursor(bool confineCursor) {
        this->confineCursor = confineCursor;
        this->ini.SetBoolValue(L"qol", L"confine_cursor", confineCursor);
        this->ini.SaveFile(this->configPath.c_str());
    }

    void Config::SetCursorHighlightType(CursorHighlightType cursorHighlightType) {
        this->cursorHighlightType = cursorHighlightType;
        this->ini.SetLongValue(L"testing", L"cursor_highlight_type", cursorHighlightType);
        this->ini.SaveFile(this->configPath.c_str());
    }

    void Config::SetCursorHighlightKey(int cursorHighlightKey) {
        this->cursorHighlightKey = cursorHighlightKey;
        this->ini.SetLongValue(L"testing", L"cursor_highlight_key", cursorHighlightKey);
        this->ini.SaveFile(this->configPath.c_str());
    }

    void Config::SetCursorHighlightColor(int cursorHighlightColor) {
        this->cursorHighlightColor = cursorHighlightColor;
        this->ini.SetLongValue(L"testing", L"cursor_highlight_color", cursorHighlightColor);
        this->ini.SaveFile(this->configPath.c_str());
    }

}
