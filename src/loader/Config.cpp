#include "Config.h"
#include "windows.h"
#include <ShlObj.h>
#include <Shlwapi.h>
#include <filesystem> 
#include <sstream>
#include "log.h"

using namespace std;
using namespace std::experimental::filesystem::v1;

namespace loader {

    Config AppConfig;
    

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

        this->settingsKeybind = this->ParseKeybindString(this->ini.GetValue(L"addons", L"window_keybind", L"16+18+122")); // Alt + Shift + F11
        this->showUnsupportedAddons = this->ini.GetBoolValue(L"addons", L"show_unsupported_addons", false);
    }


    void Config::SetSettingsKeybind(const set<uint_fast8_t>& keys) {
        this->settingsKeybind = keys;
        wstring keybind = this->ToKeybindString(keys);
        this->ini.SetValue(L"addons", L"window_keybind", keybind.c_str());
        this->ini.SaveFile(this->configPath.c_str());
    }


    void Config::SetShowUnsupportedAddons(bool showUnsupportedAddons) {
        this->showUnsupportedAddons = showUnsupportedAddons;
        this->ini.SetBoolValue(L"addons", L"show_unsupported_addons", showUnsupportedAddons);
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
        return this->ini.GetLongValue(fileName.c_str(), L"order", -1);
    }

    void Config::SetAddonOrder(const wstring& fileName, int order) {
        this->ini.SetLongValue(fileName.c_str(), L"order", order);
        this->ini.SaveFile(this->configPath.c_str());
    }


    const set<uint_fast8_t> Config::ParseKeybindString(const wstring& keys) const {
        set<uint_fast8_t> result;
        if (keys.length() > 0) {
            wstringstream ss(keys);

            while (ss.good()) {
                wstring substr;
                getline(ss, substr, L'+');
                int val = stoi(substr);
                if (val >= 0 && val <= 0xFF) {
                    result.insert(static_cast<uint_fast8_t>(val));
                }
            }
        }
        return result;
    }

    const wstring Config::ToKeybindString(const set<uint_fast8_t>& keys) const {
        wstring result;
        for (auto it = keys.begin(); it != keys.end(); ++it) {
            result += to_wstring(*it);
            if (next(it) != keys.end()) {
                result += L"+";
            }
        }
        return result;
    }

}
