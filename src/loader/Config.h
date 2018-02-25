#pragma once
#include <set>
#include <SimpleIni.h>

namespace loader {
    class Config {
    public:
        Config();

        void Initialize();

        const std::wstring GetConfigPath() const { return this->configPath; }
        const std::string GetImGuiConfigPath() const { return this->configImGuiPath; }

        const std::set<uint32_t> GetSettingsKeybind() const { return this->settingsKeybind; }
        void SetSettingsKeybind(const std::wstring& keys);
        void SetSettingsKeybind(const std::set<uint32_t>& keys);

        bool GetShowUnsupportedAddons() const { return this->showUnsupportedAddons; }
        void SetShowUnsupportedAddons(bool showUnsupportedAddons);

        bool GetAddonEnabled(const std::wstring& fileName) const;
        void SetAddonEnabled(const std::wstring& fileName, bool enabled);
        int GetAddonOrder(const std::wstring& fileName) const;
        void SetAddonOrder(const std::wstring& fileName, int order);

        const std::set<uint32_t> ParseKeybindString(const std::wstring& keys) const;
        const std::wstring ToKeybindString(const std::set<uint32_t>& keys) const;

    protected:
        const std::wstring configFolder = L"addons/loader/";
        const std::wstring configName = L"loader.ini";
        const std::wstring configImGuiName = L"loader_imgui.ini";

        std::wstring configPath;
        std::string configImGuiPath;

        std::set<uint32_t> settingsKeybind;
        bool showUnsupportedAddons = false;

        CSimpleIni ini;
    };

    extern Config AppConfig;
}
