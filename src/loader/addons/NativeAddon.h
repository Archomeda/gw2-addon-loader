#pragma once
#include "../stdafx.h"
#include "Addon.h"

namespace loader::addons {

    class NativeAddon : public Addon {
    public:
        NativeAddon() : Addon() { }
        NativeAddon(const std::string& filePath) : Addon(filePath) { }
        NativeAddon(const std::filesystem::path& filePath) : Addon(filePath) { }

        virtual bool Initialize() override;
        virtual bool Uninitialize() override;
        virtual bool Load() override;
        virtual bool Unload() override;

        virtual bool SupportsLoading() const override { return this->AddonInitialize != nullptr; }
        virtual bool SupportsHotLoading() const override { return true; }
        virtual bool SupportsSettings() const override { return this->AddonOpenSettings != nullptr; }
        virtual bool SupportsUpdating() const override { return this->updateMethod != AddonUpdateMethod::NoUpdateMethod; }
        virtual AddonUpdateMethod GetUpdateMethod() const override { return this->updateMethod; }

        virtual AddonType GetType() const override { return AddonType::AddonTypeNative; }

        virtual const std::string GetID() const override { return this->id; }
        virtual const std::string GetName() const override { return this->name; }
        virtual const std::string GetAuthor() const override { return this->author; }
        virtual const std::string GetDescription() const override { return this->description; }
        virtual const std::string GetVersion() const override { return this->version; }
        virtual const std::string GetHomepage() const override { return this->homepage; }
        virtual IDirect3DTexture9* GetIcon() const override { return this->icon; }

        virtual std::unique_ptr<updaters::Updater> GetUpdater() override;
        virtual std::unique_ptr<updaters::Downloader> GetDownloader() override;

        virtual void OpenSettings() override;

    private:
        bool InitializeV1(GW2AddonAPIBase* base);

        HMODULE addonHandle = NULL;

        std::string id;
        std::string name;
        std::string author;
        std::string description;
        std::string version;
        std::string homepage;
        IDirect3DTexture9* icon = nullptr;
        bool iconManaged = false;

        AddonUpdateMethod updateMethod = AddonUpdateMethod::NoUpdateMethod;
        std::string githubRepo;
        GW2AddonCheckUpdate_t* AddonCheckUpdate = nullptr;
        GW2AddonDownloadUpdate_t* AddonDownloadUpdate = nullptr;

        GW2AddonInitialize_t* AddonInitialize = nullptr;
        GW2AddonRelease_t* AddonRelease = nullptr;
        GW2AddonLoad_t* AddonLoad = nullptr;
        GW2AddonOpenSettings_t* AddonOpenSettings = nullptr;
    };

}
