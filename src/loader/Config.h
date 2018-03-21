#pragma once
#include <SimpleIni.h>
#include <chrono>
#include <map>
#include <set>
#include <stdint.h>
#include <string>

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

        const std::set<uint_fast8_t> GetSettingsKeybind() const { return this->settingsKeybind; }
        bool GetOBSCompatibilityMode() const { return this->obsCompatibilityMode; }
        bool GetShowIncompatibleAddons() const { return this->showIncompatibleAddons; }
        bool GetShowDebugFeatures() const { return this->showDebugFeatures; }
        timestamp GetLastUpdateCheck() const { return this->lastUpdateCheck; }
        std::string GetLastestVersion() const { return this->lastestVersion; }
        std::string GetLastestVersionInfoUrl() const { return this->lastestVersionInfoUrl; }
        std::string GetLastestVersionDownloadUrl() const { return this->lastestVersionDownloadUrl; }

        void SetSettingsKeybind(const std::set<uint_fast8_t>& keys);
        void SetOBSCompatibilityMode(bool compatibilityMode);
        void SetShowIncompatibleAddons(bool showIncompatibleAddons);
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

        /** Addon settings */

        bool GetAddonEnabled(addons::Addon* const addon);
        int GetAddonOrder(addons::Addon* const addon);
        std::string GetLastestAddonVersion(addons::Addon* const addon);
        std::string GetLastestAddonVersionInfoUrl(addons::Addon* const addon);
        std::string GetLastestAddonVersionDownloadUrl(addons::Addon* const addon);

        void SetAddonEnabled(const addons::Addon* const addon, bool enabled);
        void SetAddonOrder(const addons::Addon* const addon, int order);
        void SetLastestAddonVersion(const addons::Addon* const addon, const std::string& version);
        void SetLastestAddonVersionInfoUrl(const addons::Addon* const addon, const std::string& url);
        void SetLastestAddonVersionDownloadUrl(const addons::Addon* const addon, const std::string& url);

    private:
        const std::string configName = "loader.ini";
        const std::string configImGuiName = "loader_imgui.ini";

        std::string configPath;
        std::string configImGuiPath;

        std::set<uint_fast8_t> settingsKeybind;
        bool obsCompatibilityMode = false;
        bool showIncompatibleAddons = false;
        bool showDebugFeatures = false;
        timestamp lastUpdateCheck;
        std::string lastestVersion;
        std::string lastestVersionInfoUrl;
        std::string lastestVersionDownloadUrl;

        std::map<addons::Addon*, bool> addonEnabled;
        std::map<addons::Addon*, int> addonOrder;
        std::map<addons::Addon*, std::string> lastestAddonVersion;
        std::map<addons::Addon*, std::string> lastestAddonVersionInfoUrl;
        std::map<addons::Addon*, std::string> lastestAddonVersionDownloadUrl;

        CSimpleIni ini;
    };

    extern Config AppConfig;
}
