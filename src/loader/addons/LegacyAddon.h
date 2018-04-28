#pragma once
#include "../windows.h"
#include <d3d9.h>
#include <functional>
#include <string>
#include "Addon.h"
#include "ProxyDirect3DDevice9.h"
#include "../hooks/vftable.h"

namespace loader {
    namespace addons {

        typedef IDirect3D9*(WINAPI Direct3DCreate9_t)(UINT sdkVersion);

        class LegacyAddon : public Addon {
        public:
            enum SafeEnvType {
                Initialization,
                Loading
            };

            LegacyAddon() : Addon() { }
            LegacyAddon(const std::string& filePath) : Addon(filePath) { }
            LegacyAddon(const std::experimental::filesystem::path& filePath) : Addon(filePath) { }

            bool ApplySafeEnv(SafeEnvType envType);
            bool RevertSafeEnv(SafeEnvType envType);
            void SetNextAddonChain(LegacyAddon* addon);

            virtual bool Initialize() override;
            virtual bool Uninitialize() override;
            virtual bool Load() override;
            virtual bool Unload() override;

            virtual bool SupportsLoading() const override { return this->AddonCreate != nullptr; }

            virtual AddonType GetType() const override { return AddonType::AddonTypeLegacy; }

            IDirect3D9* AddonD3D9 = nullptr;
            IDirect3DDevice9* AddonD3DDevice9 = nullptr;
            ProxyDirect3DDevice9* ProxyD3DDevice9 = nullptr;

        private:
            HMODULE addonHandle = NULL;
            DWORD proxyAddonNumberOfExports = 0;
            D3DDevice9_vft proxyVft = { 0 };

            Direct3DCreate9_t* AddonCreate = nullptr;
        };

    }
}
