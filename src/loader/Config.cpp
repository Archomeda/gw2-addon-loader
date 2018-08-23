#include "Config.h"
#include "globals.h"
#include "log.h"
#include "addons/Addon.h"
#include "addons/addons_manager.h"
#include "utils/encoding.h"
#include "utils/file.h"

using namespace std;
using namespace std::filesystem;
using namespace loader::addons;
using namespace loader::utils;

namespace loader {

    Config AppConfig;


    void Config::Initialize() {
        CONFIG_LOG()->info("Initializing config");

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
        CONFIG_LOG()->info("Using config file {0}", configPath.u8string());
        this->configPath = configPath.u8string();

        // ImGUI config file
        path configImGuiPath(configFolder);
        configImGuiPath /= this->configImGuiName;
        CONFIG_LOG()->info("Using ImGUI config file {0}", configImGuiPath.u8string());
        this->configImGuiPath = configImGuiPath.u8string();

        // Load settings
        this->ini.SetUnicode();
        this->ini.LoadFile(this->configPath.c_str());

        this->disclaimerAccepted = this->ini.GetBoolValue(L"general", L"disclaimer_accepted", false);
        this->settingsKeybind = this->ParseKeybindString(u8(this->ini.GetValue(L"addons", L"window_keybind", L"16+18+122"))); // Alt + Shift + F11
        this->obsCompatibilityMode = this->ini.GetBoolValue(L"general", L"obs_compatibility_mode", false);
        this->showHiddenAddons = this->ini.GetBoolValue(L"addons", L"show_hidden_addons", false);
        this->diagnostics = this->ini.GetBoolValue(L"general", L"diagnostics", false);
        this->apiKey = u8(this->ini.GetValue(L"general", L"api_key", L""));
        this->lastUpdateCheck = timestamp(chrono::seconds(this->ini.GetLongValue(L"general", L"last_update_check", 0)));
        this->latestVersion = u8(this->ini.GetValue(L"general", L"latest_version", L""));
        this->latestVersionInfoUrl = u8(this->ini.GetValue(L"general", L"latest_version_info_url", L""));
        this->latestVersionDownloadUrl = u8(this->ini.GetValue(L"general", L"latest_version_download_url", L""));
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


    void Config::SetDisclaimerAccepted(bool disclaimerAccepted) {
        this->disclaimerAccepted = disclaimerAccepted;
        this->ini.SetBoolValue(L"general", L"disclaimer_accepted", disclaimerAccepted);
        this->ini.SaveFile(this->configPath.c_str());
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

    void Config::SetShowHiddenAddons(bool showHiddenAddons) {
        this->showHiddenAddons = showHiddenAddons;
        this->ini.SetBoolValue(L"addons", L"show_hidden_addons", showHiddenAddons);
        this->ini.SaveFile(this->configPath.c_str());
    }

    void Config::SetDiagnostics(bool diagnostics) {
        this->diagnostics = diagnostics;
        this->ini.SetBoolValue(L"general", L"diagnostics", diagnostics);
        this->ini.SaveFile(this->configPath.c_str());
    }

    void Config::SetApiKey(const string& apiKey) {
        this->apiKey = apiKey;
        this->ini.SetValue(L"general", L"api_key", u16(apiKey).c_str());
        this->ini.SaveFile(this->configPath.c_str());

        // Update the add-ons
        if (!apiKey.empty()) {
            addons::ApiKeyChange(apiKey.c_str(), static_cast<int>(apiKey.length()));
        }
        else {
            addons::ApiKeyChange(nullptr, 0);
        }
    }

    void Config::SetLatestVersion(const string& version) {
        this->latestVersion = version;
        this->ini.SetValue(L"general", L"latest_version", u16(version).c_str());
        this->ini.SaveFile(this->configPath.c_str());
    }

    void Config::SetLatestVersionInfoUrl(const string& url) {
        this->latestVersionInfoUrl = url;
        this->ini.SetValue(L"general", L"latest_version_info_url", u16(url).c_str());
        this->ini.SaveFile(this->configPath.c_str());
    }

    void Config::SetLatestVersionDownloadUrl(const string& url) {
        this->latestVersionDownloadUrl = url;
        this->ini.SetValue(L"general", L"latest_version_download_url", u16(url).c_str());
        this->ini.SaveFile(this->configPath.c_str());
    }


    bool Config::GetAddonEnabled(const Addon* const addon) {
        bool value;
        auto a = const_cast<Addon*>(addon);
        if (this->addonEnabled.count(a) == 0) {
            wstring key = u16("addon-" + addon->GetID());
            value = this->ini.GetBoolValue(key.c_str(), L"enabled", false);
            this->addonEnabled[a] = value;
        }
        else {
            value = this->addonEnabled[a];
        }
        return value;
    }

    int Config::GetAddonOrder(const Addon* const addon) {
        int value;
        auto a = const_cast<Addon*>(addon);
        if (this->addonOrder.count(a) == 0) {
            wstring key = u16("addon-" + addon->GetID());
            value = this->ini.GetLongValue(key.c_str(), L"order", -1);
            this->addonOrder[a] = value;
        }
        else {
            value = this->addonOrder[a];
        }
        return value;
    }

    string Config::GetLatestAddonVersion(const Addon* const addon) {
        string value;
        auto a = const_cast<Addon*>(addon);
        if (this->latestAddonVersion.count(a) == 0) {
            wstring key = u16("addon-" + addon->GetID());
            value = u8(this->ini.GetValue(key.c_str(), L"latest_version", L""));
            this->latestAddonVersion[a] = value;
        }
        else {
            value = this->latestAddonVersion[a];
        }
        return value;
    }

    string Config::GetLatestAddonVersionInfoUrl(const Addon* const addon) {
        string value;
        auto a = const_cast<Addon*>(addon);
        if (this->latestAddonVersionInfoUrl.count(a) == 0) {
            wstring key = u16("addon-" + addon->GetID());
            value = u8(this->ini.GetValue(key.c_str(), L"latest_version_info_url", L""));
            this->latestAddonVersionInfoUrl[a] = value;
        }
        else {
            value = this->latestAddonVersionInfoUrl[a];
        }
        return value;
    }

    string Config::GetLatestAddonVersionDownloadUrl(const Addon* const addon) {
        string value;
        auto a = const_cast<Addon*>(addon);
        if (this->latestAddonVersionDownloadUrl.count(a) == 0) {
            wstring key = u16("addon-" + addon->GetID());
            value = u8(this->ini.GetValue(key.c_str(), L"latest_version_download_url", L""));
            this->latestAddonVersionDownloadUrl[a] = value;
        }
        else {
            value = this->latestAddonVersionDownloadUrl[a];
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

    void Config::SetLatestAddonVersion(const Addon* const addon, const string& version) {
        wstring key = u16("addon-" + addon->GetID());
        this->ini.SetValue(key.c_str(), L"latest_version", u16(version).c_str());
        this->ini.SaveFile(this->configPath.c_str());
    }

    void Config::SetLatestAddonVersionInfoUrl(const Addon* const addon, const string& url) {
        wstring key = u16("addon-" + addon->GetID());
        this->ini.SetValue(key.c_str(), L"latest_version_info_url", u16(url).c_str());
        this->ini.SaveFile(this->configPath.c_str());
    }

    void Config::SetLatestAddonVersionDownloadUrl(const Addon* const addon, const string& url) {
        wstring key = u16("addon-" + addon->GetID());
        this->ini.SetValue(key.c_str(), L"latest_version_download_url", u16(url).c_str());
        this->ini.SaveFile(this->configPath.c_str());
    }

}
