#pragma once
#include "../windows.h"
#include <gw2addon-legacy.h>
#include "Addon.h"
#include "ProxyDirect3DDevice9.h"

namespace loader {
    namespace addons {

        class ProxyAddon : public Addon {
        public:
            ProxyAddon() : Addon() {
                ProxyAddon::Instance = this;
            }
            ProxyAddon(const std::string& filePath) : Addon(filePath) { 
                ProxyAddon::Instance = this; 
            }
            ProxyAddon(const std::experimental::filesystem::path& filePath) : Addon(filePath) { 
                ProxyAddon::Instance = this; 
            }

            static ProxyAddon* Instance;
            ProxyDirect3DDevice9* LastProxiedDevice = nullptr;

            virtual bool Initialize() override;
            virtual bool Uninitialize() override;
            virtual bool Load() override;
            virtual bool Unload() override;

            virtual bool IsForced() const override { return true; }
            virtual bool SupportsLoading() const override { return this->ProxyInitialize != nullptr; }

            virtual AddonType GetType() const override { return AddonType::AddonTypeLoaderProxy; }

            virtual const std::string GetName() const override { return this->proxyMetadata.name; }
            virtual const std::string GetDescription() const override { return this->proxyMetadata.description; }

        private:
            HMODULE addonHandle = NULL;

            GW2LegacyAddonProxyAPI proxyApi;
            GW2LegacyAddonProxyAPIMetadata proxyMetadata = {};

            ProxyInitialize_t* ProxyInitialize = nullptr;
            ProxyRelease_t* ProxyRelease = nullptr;
        };

    }
}
