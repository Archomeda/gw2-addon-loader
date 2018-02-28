#pragma once
#include <SimpleIni.h>
#include <set>
#include <stdint.h>
#include <string>
#include "addons/Addon.h"

namespace loader {
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
        bool GetShowUnsupportedAddons() const { return this->showUnsupportedAddons; }
        bool GetShowDebugFeatures() const { return this->showDebugFeatures; }
        
        void SetSettingsKeybind(const std::set<uint_fast8_t>& keys);
        void SetShowUnsupportedAddons(bool showUnsupportedAddons);
        void SetShowDebugFeatures(bool showDebugFeatures);

    protected:
        const std::string configFolder = "addons/loader/";
        const std::string configName = "loader.ini";
        const std::string configImGuiName = "loader_imgui.ini";

        std::string configPath;
        std::string configImGuiPath;

        std::set<uint_fast8_t> settingsKeybind;
        bool showUnsupportedAddons = false;
        bool showDebugFeatures = false;

        CSimpleIni ini;
    };

    extern Config AppConfig;
}
