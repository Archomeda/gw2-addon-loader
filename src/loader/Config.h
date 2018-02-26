#pragma once
#include <SimpleIni.h>
#include <set>
#include <stdint.h>
#include <string>

namespace loader {
    class Config {
    public:
        Config() { }

        void Initialize();

        const std::wstring GetConfigPath() const { return this->configPath; }
        const std::string GetImGuiConfigPath() const { return this->configImGuiPath; }

        bool GetAddonEnabled(const std::wstring& fileName) const;
        void SetAddonEnabled(const std::wstring& fileName, bool enabled);
        int GetAddonOrder(const std::wstring& fileName) const;
        void SetAddonOrder(const std::wstring& fileName, int order);

        const std::set<uint_fast8_t> ParseKeybindString(const std::wstring& keys) const;
        const std::wstring ToKeybindString(const std::set<uint_fast8_t>& keys) const;


        const std::set<uint_fast8_t> GetSettingsKeybind() const { return this->settingsKeybind; }
        bool GetShowUnsupportedAddons() const { return this->showUnsupportedAddons; }
        bool GetShowDebugFeatures() const { return this->showDebugFeatures; }
        
        void SetSettingsKeybind(const std::set<uint_fast8_t>& keys);
        void SetShowUnsupportedAddons(bool showUnsupportedAddons);
        void SetShowDebugFeatures(bool showDebugFeatures);

    protected:
        const std::wstring configFolder = L"addons/loader/";
        const std::wstring configName = L"loader.ini";
        const std::wstring configImGuiName = L"loader_imgui.ini";

        std::wstring configPath;
        std::string configImGuiPath;

        std::set<uint_fast8_t> settingsKeybind;
        bool showUnsupportedAddons = false;
        bool showDebugFeatures = false;

        CSimpleIni ini;
    };

    extern Config AppConfig;
}
