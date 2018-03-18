#pragma once
#include <SimpleIni.h>
#include <chrono>
#include <set>
#include <stdint.h>
#include <string>
#include "addons/Addon.h"

namespace loader {
    typedef std::chrono::time_point<std::chrono::seconds> timestamp;

    class Config {
    public:
        Config() { }

        void Initialize();

        const std::string GetConfigPath() const { return this->configPath; }
        const std::string GetImGuiConfigPath() const { return this->configImGuiPath; }

        bool GetAddonEnabled(const std::string& addonId) const;
        bool GetAddonEnabled(const addons::Addon* const addon) const;
        void SetAddonEnabled(const std::string& addonId, bool enabled);
        void SetAddonEnabled(const addons::Addon* const addon, bool enabled);
        int GetAddonOrder(const std::string& addonId) const;
        int GetAddonOrder(const addons::Addon* const addon) const;
        void SetAddonOrder(const std::string& addonId, int order);
        void SetAddonOrder(const addons::Addon* const addon, int order);

        const std::set<uint_fast8_t> ParseKeybindString(const std::string& keys) const;
        const std::string ToKeybindString(const std::set<uint_fast8_t>& keys) const;


        const std::set<uint_fast8_t> GetSettingsKeybind() const { return this->settingsKeybind; }
        bool GetOBSCompatibilityMode() const { return this->obsCompatibilityMode; }
        bool GetShowUnsupportedAddons() const { return this->showUnsupportedAddons; }
        bool GetShowDebugFeatures() const { return this->showDebugFeatures; }
        timestamp GetLastUpdateCheck() const { return this->lastUpdateCheck; }
        std::string GetLastestVersion() const { return this->lastestVersion; }
        std::string GetLastestVersionInfoUrl() const { return this->lastestVersionInfoUrl; }
        std::string GetLastestVersionDownloadUrl() const { return this->lastestVersionDownloadUrl; }

        void SetSettingsKeybind(const std::set<uint_fast8_t>& keys);
        void SetOBSCompatibilityMode(bool compatibilityMode);
        void SetShowUnsupportedAddons(bool showUnsupportedAddons);
        void SetShowDebugFeatures(bool showDebugFeatures);
        template<class Clock>
        void SetLastUpdateCheck(std::chrono::time_point<Clock, std::chrono::seconds> lastUpdate) {
            using namespace std::chrono;
            auto duration = duration_cast<seconds>(lastUpdate.time_since_epoch());
            this->lastUpdateCheck = timestamp(seconds(duration.count()));
            this->ini.SetLongValue(L"general", L"last_update_check", static_cast<long>(duration.count()));
            this->ini.SaveFile(this->configPath.c_str());
        }
        void SetLastestVersion(const std::string& version);
        void SetLastestVersionInfoUrl(const std::string& url);
        void SetLastestVersionDownloadUrl(const std::string& url);

    private:
        const std::string configFolder = "addons/loader/";
        const std::string configName = "loader.ini";
        const std::string configImGuiName = "loader_imgui.ini";

        std::string configPath;
        std::string configImGuiPath;

        std::set<uint_fast8_t> settingsKeybind;
        bool obsCompatibilityMode = false;
        bool showUnsupportedAddons = false;
        bool showDebugFeatures = false;
        timestamp lastUpdateCheck;
        std::string lastestVersion;
        std::string lastestVersionInfoUrl;
        std::string lastestVersionDownloadUrl;

        CSimpleIni ini;
    };

    extern Config AppConfig;
}
