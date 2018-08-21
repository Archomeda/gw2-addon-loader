#pragma once
#include "stdafx.h"

namespace loader {

    namespace addons {
        class Addon;
    }

    typedef std::chrono::time_point<std::chrono::seconds> timestamp;

    class Config {
    public:
        Config() { }

        void Initialize();

        const std::string GetConfigPath() const { return this->configPath; }
        const std::string GetImGuiConfigPath() const { return this->configImGuiPath; }

        const std::set<uint_fast8_t> ParseKeybindString(const std::string& keys) const;
        const std::string ToKeybindString(const std::set<uint_fast8_t>& keys) const;

        /** Global settings */

        const bool GetDisclaimerAccepted() const { return this->disclaimerAccepted; }
        const std::set<uint_fast8_t> GetSettingsKeybind() const { return this->settingsKeybind; }
        bool GetOBSCompatibilityMode() const { return this->obsCompatibilityMode; }
        bool GetShowHiddenAddons() const { return this->showHiddenAddons; }
        bool GetDiagnostics() const { return this->diagnostics; }
        std::string GetApiKey() const { return this->apiKey; }
        timestamp GetLastUpdateCheck() const { return this->lastUpdateCheck; }
        std::string GetLatestVersion() const { return this->latestVersion; }
        std::string GetLatestVersionInfoUrl() const { return this->latestVersionInfoUrl; }
        std::string GetLatestVersionDownloadUrl() const { return this->latestVersionDownloadUrl; }

        void SetDisclaimerAccepted(bool disclaimerAccepted);
        void SetSettingsKeybind(const std::set<uint_fast8_t>& keys);
        void SetOBSCompatibilityMode(bool compatibilityMode);
        void SetShowHiddenAddons(bool showHiddenAddons);
        void SetDiagnostics(bool diagnostics);
        void SetApiKey(const std::string& apiKey);
        template<class Clock>
        void SetLastUpdateCheck(std::chrono::time_point<Clock, std::chrono::seconds> lastUpdate) {
            using namespace std::chrono;
            auto duration = duration_cast<seconds>(lastUpdate.time_since_epoch());
            this->lastUpdateCheck = timestamp(seconds(duration.count()));
            this->ini.SetLongValue(L"general", L"last_update_check", static_cast<long>(duration.count()));
            this->ini.SaveFile(this->configPath.c_str());
        }
        void SetLatestVersion(const std::string& version);
        void SetLatestVersionInfoUrl(const std::string& url);
        void SetLatestVersionDownloadUrl(const std::string& url);

        /** Add-on settings */

        bool GetAddonEnabled(const addons::Addon* const addon);
        int GetAddonOrder(const addons::Addon* const addon);
        std::string GetLatestAddonVersion(const addons::Addon* const addon);
        std::string GetLatestAddonVersionInfoUrl(const addons::Addon* const addon);
        std::string GetLatestAddonVersionDownloadUrl(const addons::Addon* const addon);

        void SetAddonEnabled(const addons::Addon* const addon, bool enabled);
        void SetAddonOrder(const addons::Addon* const addon, int order);
        void SetLatestAddonVersion(const addons::Addon* const addon, const std::string& version);
        void SetLatestAddonVersionInfoUrl(const addons::Addon* const addon, const std::string& url);
        void SetLatestAddonVersionDownloadUrl(const addons::Addon* const addon, const std::string& url);

    private:
        const std::string configName = "loader.ini";
        const std::string configImGuiName = "loader_imgui.ini";

        std::string configPath;
        std::string configImGuiPath;

        bool disclaimerAccepted = false;
        std::set<uint_fast8_t> settingsKeybind;
        bool obsCompatibilityMode = false;
        bool showHiddenAddons = false;
        bool diagnostics = false;
        std::string apiKey;
        timestamp lastUpdateCheck;
        std::string latestVersion;
        std::string latestVersionInfoUrl;
        std::string latestVersionDownloadUrl;

        std::map<addons::Addon*, bool> addonEnabled;
        std::map<addons::Addon*, int> addonOrder;
        std::map<addons::Addon*, std::string> latestAddonVersion;
        std::map<addons::Addon*, std::string> latestAddonVersionInfoUrl;
        std::map<addons::Addon*, std::string> latestAddonVersionDownloadUrl;

        CSimpleIni ini;
    };

    extern Config AppConfig;

}
