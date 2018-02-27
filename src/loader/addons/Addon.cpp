#include "Addon.h"
#include <Strsafe.h>
#include <filesystem>
#include <string>
#include <gw2addon-native.h>
#include "addons_manager.h"
#include "exceptions.h"
#include "types/DummyAddonImpl.h"
#include "types/NativeAddonImpl.h"
#include "../log.h"
#include "../utils.h"
#include "../Config.h"

using namespace std;
using namespace std::experimental::filesystem::v1;
using namespace loader::addons::types;

namespace loader {
    namespace addons {

        Addon::Addon(const std::wstring& fullFileName) {
            this->filePath = fullFileName;
            this->fileName = path(fullFileName).filename();
        }


        bool Addon::IsEnabledByConfig() {
            return AppConfig.GetAddonEnabled(this->GetFileName());
        }


        const wstring Addon::GetID() {
            return this->GetTypeImpl()->GetID();
        }

        const wstring Addon::GetName() {
            return this->GetTypeImpl()->GetName();
        }

        const wstring Addon::GetAuthor() {
            return this->GetTypeImpl()->GetAuthor();
        }

        const wstring Addon::GetDescription() {
            return this->GetTypeImpl()->GetDescription();
        }

        const wstring Addon::GetVersion() {
            return this->GetTypeImpl()->GetVersion();
        }

        const wstring Addon::GetHomepage() {
            return this->GetTypeImpl()->GetHomepage();
        }


        const AddonType Addon::GetAddonType() {
            return this->addonType;
        }

        const wstring Addon::GetAddonTypeString() {
            auto impl = this->GetTypeImpl();
            wstring subType = impl ? impl->GetAddonSubTypeString() : L"";
            switch (this->addonType) {
            case AddonType::NativeAddon:
                return L"Native" + subType;
            case AddonType::LegacyAddon:
                return L"Legacy" + subType;
            case AddonType::ChainAddon:
                return L"Chain" + subType;
            default:
                return L"Unknown";
            }
        }


        bool Addon::IsLoaded() const {
            auto impl = this->GetTypeImpl();
            return impl && impl->GetAddonState() == AddonState::LoadedState;
        }


        void Addon::SetTypeImpl(const AddonType addonType) {
            switch (addonType) {
            case AddonType::NativeAddon:
                this->typeImpl = make_shared<types::NativeAddonImpl>(this->filePath);
                break;
            case AddonType::LegacyAddon:
            case AddonType::ChainAddon:
                // Our addon is not native
                //TODO: Reserved to possible future use
            default:
                this->typeImpl = make_shared<types::DummyAddonImpl>(this->filePath);
            }
            this->typeImpl->SetAddon(this->shared_from_this());           
        }


        bool Addon::Initialize() {
            // Preload stuff
            this->LoadAddonType();
            this->SetTypeImpl(this->GetAddonType());

            // Actually initialize addon
            try {
                this->GetTypeImpl()->Initialize();
            }
            catch (const exceptions::AddonInitializationException& ex) {
                GetLog()->error("Failed to initialize addon {0}: {1}", ws2s(this->GetFileName()), ex.what());
                return false;
            }

            return true;
        }

        bool Addon::Uninitialize() {
            GetLog()->debug("loader::addons::Addon({0})::Uninitialize()", ws2s(this->GetFileName()));
            this->GetTypeImpl()->Uninitialize();
            return true;
        }

        bool Addon::Load() {
            GetLog()->debug("loader::addons::Addon({0})::Load()", ws2s(this->GetFileName()));
            try {
                auto start = chrono::steady_clock::now();
                this->GetTypeImpl()->Load();
                this->durationLoad = (chrono::steady_clock::now() - start).count();
            }
            catch (const exceptions::AddonLoadingException& ex) {
                GetLog()->error("Failed to load addon: {0}: {1}", ws2s(this->GetFileName()), ex.what());
                return false;
            }
            return true;
        }

        bool Addon::Unload() {
            GetLog()->debug("loader::addons::Addon({0})::Unload()", ws2s(this->GetFileName()));
            try {
                this->GetTypeImpl()->Unload();
            }
            catch (const exceptions::AddonUnloadingException& ex) {
                GetLog()->error("Failed to unload addon: {0}: {1}", ws2s(this->GetFileName()), ex.what());
                return false;
            }
            return true;
        }


        bool Addon::SupportsLoading() {
            switch (this->GetAddonType()) {
            case AddonType::NativeAddon:
                return true;
            }
            return false;
        }

        bool Addon::SupportsSettings() {
            return false;
        }

        bool Addon::SupportsHomepage() {
            return !this->GetHomepage().empty();
        }

        bool Addon::HandleWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
            if (this->GetTypeImpl()->GetAddonState() != AddonState::LoadedState) {
                return false;
            }

            try {
                return this->GetTypeImpl()->HandleWndProc(hWnd, msg, wParam, lParam);
            }
            catch (const exceptions::AddonWndProcException& ex) {
                GetLog()->error("Failed to handle WndProc in addon: {0}: {1}", ws2s(this->GetFileName()), ex.what());
                GetLog()->error("Addon will be disabled on next restart");
                AppConfig.SetAddonEnabled(this->GetFileName(), false);
            }
            return false;
        }


        void Addon::LoadAddonType() {
            HMODULE hAddon = LoadLibraryEx(this->filePath.c_str(), NULL, DONT_RESOLVE_DLL_REFERENCES);
            
            // First check if the addon is native
            if (GetProcAddress(hAddon, GW2ADDON_DLL_Initialize) != nullptr) {
                // Our addon is native
                this->addonType = AddonType::NativeAddon;
            }
            else {
                // Our addon is not native
                //TODO: Reserved for possible future use
                this->addonType = AddonType::UnknownAddon;
            }

            FreeLibrary(hAddon);
        }
  
  }
}
