#include "Config.h"
#include "windows.h"
#include <ShlObj.h>
#include <Shlwapi.h>
#include <filesystem> 
#include <sstream>
#include "globals.h"
#include "log.h"
#include "addons/Addon.h"
#include "utils/encoding.h"
#include "utils/file.h"

using namespace std;
using namespace std::experimental::filesystem;
using namespace loader::addons;
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
        configFolder /= CONFIG_FOLDER;
        if (!FolderExists(configFolder.u8string())) {
            SHCreateDirectory(NULL, configFolder.c_str());
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
        this->obsCompatibilityMode = this->ini.GetBoolValue(L"general", L"obs_compatibility_mode", true);
        this->showIncompatibleAddons = this->ini.GetBoolValue(L"addons", L"show_incompatible_addons", false);
        this->showDebugFeatures = this->ini.GetBoolValue(L"general", L"show_debug_features", false);
        this->lastUpdateCheck = timestamp(chrono::seconds(this->ini.GetLongValue(L"general", L"last_update_check", 0)));
        this->lastestVersion = u8(this->ini.GetValue(L"general", L"lastest_version", L""));
        this->lastestVersionInfoUrl = u8(this->ini.GetValue(L"general", L"lastest_version_info_url", L""));
        this->lastestVersionDownloadUrl = u8(this->ini.GetValue(L"general", L"lastest_version_download_url", L""));
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

    void Config::SetOBSCompatibilityMode(bool compatibilityMode) {
        this->obsCompatibilityMode = compatibilityMode;
        this->ini.SetBoolValue(L"general", L"obs_compatibility_mode", compatibilityMode);
        this->ini.SaveFile(this->configPath.c_str());
    }

    void Config::SetShowIncompatibleAddons(bool showIncompatibleAddons) {
        this->showIncompatibleAddons = showIncompatibleAddons;
        this->ini.SetBoolValue(L"addons", L"show_incompatible_addons", showIncompatibleAddons);
        this->ini.SaveFile(this->configPath.c_str());
    }

    void Config::SetShowDebugFeatures(bool showDebugFeatures) {
        this->showDebugFeatures = showDebugFeatures;
        this->ini.SetBoolValue(L"general", L"show_debug_features", showDebugFeatures);
        this->ini.SaveFile(this->configPath.c_str());
    }

    void Config::SetLastestVersion(const string& version) {
        this->lastestVersion = version;
        this->ini.SetValue(L"general", L"lastest_version", u16(version).c_str());
        this->ini.SaveFile(this->configPath.c_str());
    }

    void Config::SetLastestVersionInfoUrl(const string& url) {
        this->lastestVersionInfoUrl = url;
        this->ini.SetValue(L"general", L"lastest_version_info_url", u16(url).c_str());
        this->ini.SaveFile(this->configPath.c_str());
    }

    void Config::SetLastestVersionDownloadUrl(const string& url) {
        this->lastestVersionDownloadUrl = url;
        this->ini.SetValue(L"general", L"lastest_version_download_url", u16(url).c_str());
        this->ini.SaveFile(this->configPath.c_str());
    }


    bool Config::GetAddonEnabled(Addon* const addon) {
        bool value;
        if (this->addonEnabled.count(addon) == 0) {
            wstring key = u16("addon-" + addon->GetID());
            value = this->ini.GetBoolValue(key.c_str(), L"enabled", false);
            this->addonEnabled[addon] = value;
        }
        else {
            value = this->addonEnabled[addon];
        }
        return value;
    }

    int Config::GetAddonOrder(Addon* const addon) {
        int value;
        if (this->addonOrder.count(addon) == 0) {
            wstring key = u16("addon-" + addon->GetID());
            value = this->ini.GetLongValue(key.c_str(), L"order", -1);
            this->addonOrder[addon] = value;
        }
        else {
            value = this->addonOrder[addon];
        }
        return value;
    }

    string Config::GetLastestAddonVersion(Addon* const addon) {
        string value;
        if (this->lastestAddonVersion.count(addon) == 0) {
            wstring key = u16("addon-" + addon->GetID());
            value = u8(this->ini.GetValue(key.c_str(), L"latest_version", L""));
            this->lastestAddonVersion[addon] = value;
        }
        else {
            value = this->lastestAddonVersion[addon];
        }
        return value;
    }

    string Config::GetLastestAddonVersionInfoUrl(Addon* const addon) {
        string value;
        if (this->lastestAddonVersionInfoUrl.count(addon) == 0) {
            wstring key = u16("addon-" + addon->GetID());
            value = u8(this->ini.GetValue(key.c_str(), L"lastest_version_info_url", L""));
            this->lastestAddonVersionInfoUrl[addon] = value;
        }
        else {
            value = this->lastestAddonVersionInfoUrl[addon];
        }
        return value;
    }

    string Config::GetLastestAddonVersionDownloadUrl(Addon* const addon) {
        string value;
        if (this->lastestAddonVersionDownloadUrl.count(addon) == 0) {
            wstring key = u16("addon-" + addon->GetID());
            value = u8(this->ini.GetValue(key.c_str(), L"lastest_version_download_url", L""));
            this->lastestAddonVersionDownloadUrl[addon] = value;
        }
        else {
            value = this->lastestAddonVersionDownloadUrl[addon];
        }
        return value;
    }


    void Config::SetAddonEnabled(const Addon* const addon, bool enabled) {
        wstring key = u16("addon-" + addon->GetID());
        this->ini.SetBoolValue(key.c_str(), L"enabled", enabled);
        this->ini.SaveFile(this->configPath.c_str());
    }

    void Config::SetAddonOrder(const Addon* const addon, int order) {
        wstring key = u16("addon-" + addon->GetID());
        this->ini.SetLongValue(key.c_str(), L"order", order);
        this->ini.SaveFile(this->configPath.c_str());
    }

    void Config::SetLastestAddonVersion(const Addon* const addon, const string& version) {
        wstring key = u16("addon-" + addon->GetID());
        this->ini.SetValue(key.c_str(), L"latest_version", u16(version).c_str());
        this->ini.SaveFile(this->configPath.c_str());
    }

    void Config::SetLastestAddonVersionInfoUrl(const Addon* const addon, const string& url) {
        wstring key = u16("addon-" + addon->GetID());
        this->ini.SetValue(key.c_str(), L"lastest_version_info_url", u16(url).c_str());
        this->ini.SaveFile(this->configPath.c_str());
    }

    void Config::SetLastestAddonVersionDownloadUrl(const Addon* const addon, const string& url) {
        wstring key = u16("addon-" + addon->GetID());
        this->ini.SetValue(key.c_str(), L"lastest_version_download_url", u16(url).c_str());
        this->ini.SaveFile(this->configPath.c_str());
    }

}
