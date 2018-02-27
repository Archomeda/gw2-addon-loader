#include "Config.h"
#include "windows.h"
#include <ShlObj.h>
#include <Shlwapi.h>
#include <filesystem> 
#include <sstream>
#include "log.h"
#include "utils/encoding.h"

using namespace std;
using namespace std::experimental::filesystem::v1;
using namespace loader::utils;

namespace loader {

    Config AppConfig;
    

    void Config::Initialize() {
        GetLog()->debug("loader::Config::Initialize()");

        // Create path
        wchar_t fileName[MAX_PATH];
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
        GetLog()->info("Using config file {0}", configPath.u8string());
        this->configPath = configPath.u8string();

        // ImGUI config file
        path configImGuiPath(configFolder);
        configImGuiPath /= this->configImGuiName;
        GetLog()->info("Using ImGUI config file {0}", configImGuiPath.u8string());
        this->configImGuiPath = configImGuiPath.u8string();

        // Load settings
        this->ini.SetUnicode();
        this->ini.LoadFile(this->configPath.c_str());

        this->settingsKeybind = this->ParseKeybindString(u8(this->ini.GetValue(L"addons", L"window_keybind", L"16+18+122"))); // Alt + Shift + F11
        this->showUnsupportedAddons = this->ini.GetBoolValue(L"addons", L"show_unsupported_addons", false);
        this->showDebugFeatures = this->ini.GetBoolValue(L"general", L"show_advanced", false);
    }


    bool Config::GetAddonEnabled(const string& fileName) const {
        wstring wFileName = u16(fileName);
        return this->ini.GetBoolValue(wFileName.c_str(), L"enabled", false);
    }

    void Config::SetAddonEnabled(const string& fileName, bool enabled) {
        wstring wFileName = u16(fileName);
        this->ini.SetBoolValue(wFileName.c_str(), L"enabled", enabled);
        this->ini.SaveFile(this->configPath.c_str());
    }

    int Config::GetAddonOrder(const string& fileName) const {
        wstring wFileName = u16(fileName);
        return this->ini.GetLongValue(wFileName.c_str(), L"order", -1);
    }

    void Config::SetAddonOrder(const string& fileName, int order) {
        wstring wFileName = u16(fileName);
        this->ini.SetLongValue(wFileName.c_str(), L"order", order);
        this->ini.SaveFile(this->configPath.c_str());
    }


    const set<uint_fast8_t> Config::ParseKeybindString(const string& keys) const {
        set<uint_fast8_t> result;
        if (keys.length() > 0) {
            stringstream ss(keys);

            while (ss.good()) {
                string substr;
                getline(ss, substr, '+');
                int val = stoi(substr);
                if (val >= 0 && val <= 0xFF) {
                    result.insert(static_cast<uint_fast8_t>(val));
                }
            }
        }
        return result;
    }

    const string Config::ToKeybindString(const set<uint_fast8_t>& keys) const {
        string result;
        for (auto it = keys.begin(); it != keys.end(); ++it) {
            result += to_string(*it);
            if (next(it) != keys.end()) {
                result += "+";
            }
        }
        return result;
    }


    void Config::SetSettingsKeybind(const set<uint_fast8_t>& keys) {
        this->settingsKeybind = keys;
        wstring keybind = u16(this->ToKeybindString(keys));
        this->ini.SetValue(L"addons", L"window_keybind", keybind.c_str());
        this->ini.SaveFile(this->configPath.c_str());
    }

    void Config::SetShowUnsupportedAddons(bool showUnsupportedAddons) {
        this->showUnsupportedAddons = showUnsupportedAddons;
        this->ini.SetBoolValue(L"addons", L"show_unsupported_addons", showUnsupportedAddons);
        this->ini.SaveFile(this->configPath.c_str());
    }

    void Config::SetShowDebugFeatures(bool showDebugFeatures) {
        this->showDebugFeatures = showDebugFeatures;
        this->ini.SetBoolValue(L"general", L"show_debug_features", showDebugFeatures);
        this->ini.SaveFile(this->configPath.c_str());
    }

}
