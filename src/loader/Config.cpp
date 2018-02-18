#include "Config.h"
#include "windows.h"
#include <ShlObj.h>
#include <Shlwapi.h>
#include <filesystem> 
#include <sstream>
#include <vector>
#include "log.h"

using namespace std;
using namespace std::experimental::filesystem::v1;

namespace loader {

    Config AppConfig;
    
    Config::Config() {
        this->settingsKeybind.insert(VK_SHIFT);
        this->settingsKeybind.insert(VK_MENU);
        this->settingsKeybind.insert(VK_F11);
    }

    const set<uint32_t> Config::ParseKeybindString(const wstring& keys) const {
        set<uint32_t> result;
        if (keys.length() > 0) {
            wstringstream ss(keys);

            while (ss.good()) {
                wstring substr;
                getline(ss, substr, L'+');
                int val = stoi(substr);
                result.insert((uint32_t)val);
            }
        }
        return result;
    }

    const wstring Config::ToKeybindString(const set<uint32_t>& keys) const {
        wstring result;
        for (auto it = keys.begin(); it != keys.end(); ++it) {
            result += to_wstring(*it);
            if (next(it) != keys.end()) {
                result += L"+";
            }
        }
        return result;
    }

    void Config::Initialize() {
        GetLog()->debug("loader::Config::Initialize()");

        // Create path
        TCHAR fileName[MAX_PATH];
        GetModuleFileName(NULL, fileName, sizeof(fileName));
        PathRemoveFileSpec(fileName);
        path configFolder(fileName);
        configFolder /= this->configFolder;
        if (!PathFileExists(configFolder.c_str())) {
            SHCreateDirectoryEx(NULL, configFolder.c_str(), NULL);
        }

        // Our config file
        path configPath(configFolder);
        configPath /= this->configName;
        GetLog()->info("Using config file {0}", configPath.string());
        this->configPath = configPath.wstring();

        // ImGUI config file
        path configImGuiPath(configFolder);
        configImGuiPath /= this->configImGuiName;
        GetLog()->info("Using ImGUI config file {0}", configImGuiPath.string());
        this->configImGuiPath = configImGuiPath.string();

        // Load settings
        this->ini.SetUnicode();
        this->ini.LoadFile(this->configPath.c_str());
    }

    void Config::SetSettingsKeybind(const wstring& keys) {
        this->SetSettingsKeybind(this->ParseKeybindString(keys));
    }

    void Config::SetSettingsKeybind(const set<uint32_t>& keys) {
        wstring keybind = this->ToKeybindString(keys);
        this->ini.SetValue(L"keybinds", L"settings_window", keybind.c_str());
        this->ini.SaveFile(this->configPath.c_str());
    }

    bool Config::GetAddonEnabled(const wstring& fileName) const {
        return this->ini.GetBoolValue(fileName.c_str(), L"enabled", false);
    }

    void Config::SetAddonEnabled(const wstring& fileName, bool enabled) {
        this->ini.SetBoolValue(fileName.c_str(), L"enabled", enabled);
        this->ini.SaveFile(this->configPath.c_str());
    }

    int Config::GetAddonOrder(const wstring& fileName) const {
        return this->ini.GetLongValue(fileName.c_str(), L"order", 0);
    }

    void Config::SetAddonOrder(const wstring& fileName, int order) {
        this->ini.SetLongValue(fileName.c_str(), L"order", order);
        this->ini.SaveFile(this->configPath.c_str());
    }

}
