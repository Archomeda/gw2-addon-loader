#include "Addon.h"
#include <Strsafe.h>
#include <Winver.h>
#include <filesystem>
#include <string>
#include <gw2addon-native.h>
#include "addons_manager.h"
#include "exceptions.h"
#include "types/DummyAddonImpl.h"
#include "types/LegacyAddonImpl.h"
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
            if (!this->metaDataLoaded) {
                this->LoadMetaData();
            }
            return this->id;
        }

        const wstring Addon::GetProductName() {
            if (!this->metaDataLoaded) {
                this->LoadMetaData();
            }
            return this->productName;
        }

        const wstring Addon::GetAuthor() {
            if (!this->metaDataLoaded) {
                this->LoadMetaData();
            }
            return this->author;
        }

        const wstring Addon::GetDescription() {
            if (!this->metaDataLoaded) {
                this->LoadMetaData();
            }
            return this->description;
        }

        const wstring Addon::GetVersion() {
            if (!this->metaDataLoaded) {
                this->LoadMetaData();
            }
            return this->version;
        }

        const wstring Addon::GetHomepage() {
            if (!this->metaDataLoaded) {
                this->LoadMetaData();
            }
            return this->homepage;
        }


        const AddonType Addon::GetAddonType() {
            if (!this->metaDataLoaded) {
                this->LoadMetaData();
            }
            return this->addonType;
        }

        const wstring Addon::GetAddonTypeString() {
            if (!this->metaDataLoaded) {
                this->LoadMetaData();
            }
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
                this->typeImpl = make_shared<types::LegacyAddonImpl>(this->filePath);
                break;
            case AddonType::ChainAddon:
                //TODO: Implement chain mode
            default:
                this->typeImpl = make_shared<types::DummyAddonImpl>(this->filePath);
            }
            this->typeImpl->SetAddon(this->shared_from_this());           
        }


        bool Addon::Initialize() {
            // Preload stuff
            this->LoadMetaData();
            this->SetTypeImpl(this->GetAddonType());

            // Actually initialize addon
            try {
                auto impl = this->GetTypeImpl();
                if (impl) {
                    impl->Initialize();
                }
            }
            catch (const exceptions::AddonInitializationException& ex) {
                GetLog()->error("Failed to initialize addon {0}: {1}", ws2s(this->GetFileName()), ex.what());
                return false;
            }

            return true;
        }

        bool Addon::Uninitialize() {
            GetLog()->debug("loader::addons::Addon({0})::Uninitialize()", ws2s(this->GetFileName()));
            auto impl = this->GetTypeImpl();
            if (impl) {
                impl->Uninitialize();
            }
            return true;
        }

        bool Addon::Load() {
            GetLog()->debug("loader::addons::Addon({0})::Load()", ws2s(this->GetFileName()));
            try {
                auto impl = this->GetTypeImpl();
                if (impl) {
                    impl->Load();
                }
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
                auto impl = this->GetTypeImpl();
                if (impl) {
                    impl->Unload();
                }
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

        void Addon::LoadMetaData() {
            GetLog()->debug("loader::addons::Addon({0})::LoadMetaData()", ws2s(this->GetFileName()));
            this->metaDataLoaded = true;
                        
            struct LANGANDCODEPAGE {
                WORD wLanguage;
                WORD wCodePage;
            } *lpTranslate;

            const wchar_t* fileName = this->filePath.c_str();
            HMODULE hAddon = LoadLibraryEx(fileName, NULL, DONT_RESOLVE_DLL_REFERENCES);
            
            // First check if the addon is native
            GW2_GetAddonInfo_t getAddonInfo = reinterpret_cast<GW2_GetAddonInfo_t>(GetProcAddress(hAddon, GW2_DLL_GetAddonInfo));
            if (getAddonInfo != NULL) {
                // Our addon is native
                FreeLibrary(hAddon);
                hAddon = LoadLibrary(fileName);
                getAddonInfo = reinterpret_cast<GW2_GetAddonInfo_t>(GetProcAddress(hAddon, GW2_DLL_GetAddonInfo));
                void* addonInfoGeneric;

                GW2ADDON_RESULT result = getAddonInfo((GW2AddonInfo**)&addonInfoGeneric);
                if (!result) {
                    GW2AddonInfoHeader* header = (GW2AddonInfoHeader*)addonInfoGeneric;
                    if (header->ver >= 1) {
                        GW2AddonInfoV1* v1 = (GW2AddonInfoV1*)addonInfoGeneric;
                        if (v1->idSize > 0) {
                            this->id = wstring(v1->id, v1->idSize);
                        }
                        if (v1->nameSize > 0) {
                            this->productName = wstring(v1->name, v1->nameSize);
                        }
                        if (v1->authorSize > 0) {
                            this->author = wstring(v1->author, v1->authorSize);
                        }
                        if (v1->descriptionSize > 0) {
                            this->description = wstring(v1->description, v1->descriptionSize);
                        }
                        if (v1->versionSize > 0) {
                            this->version = wstring(v1->version, v1->versionSize);
                        }
                        if (v1->homepageSize > 0) {
                            this->homepage = wstring(v1->homepage, v1->homepageSize);
                        }
                    }
                }

                this->addonType = AddonType::NativeAddon;
            }
            else {
                // Our addon is not native
                this->id = path(this->fileName).stem();

                DWORD dwSize = GetFileVersionInfoSize(fileName, NULL);
                if (dwSize > 0) {
                    BYTE* pbVersionInfo = new BYTE[dwSize];

                    // Get version
                    if (GetFileVersionInfo(fileName, 0, dwSize, pbVersionInfo)) {
                        VS_FIXEDFILEINFO* pFileInfo = NULL;
                        UINT puLenFileInfo = 0;
                        if (VerQueryValue(pbVersionInfo, L"\\", (LPVOID*)&pFileInfo, &puLenFileInfo)) {
                            this->version = to_wstring((pFileInfo->dwFileVersionMS >> 16 & 0xFFFF)) + L"." +
                                to_wstring((pFileInfo->dwFileVersionMS >> 0 & 0xFFFF)) + L"." +
                                to_wstring((pFileInfo->dwFileVersionLS >> 16 & 0xFFFF)) + L"." +
                                to_wstring((pFileInfo->dwFileVersionLS >> 0 & 0xFFFF));
                        }
                    }

                    // Get product name and description by getting the first available locale
                    UINT cbTranslate;
                    VerQueryValue(pbVersionInfo, L"\\VarFileInfo\\Translation", (LPVOID*)&lpTranslate, &cbTranslate);
                    wchar_t* lpBuffer;
                    UINT dwBytes;
                    for (size_t i = 0; i < (cbTranslate / sizeof(struct LANGANDCODEPAGE)); i++)
                    {
                        wchar_t subBlock[50];
                        StringCchPrintf(subBlock, sizeof(subBlock), L"\\StringFileInfo\\%04x%04x\\ProductName", lpTranslate[i].wLanguage, lpTranslate[i].wCodePage);
                        if (VerQueryValue(pbVersionInfo, subBlock, (LPVOID*)&lpBuffer, &dwBytes)) {
                            this->productName = wstring(lpBuffer);
                        }
                        StringCchPrintf(subBlock, sizeof(subBlock), L"\\StringFileInfo\\%04x%04x\\FileDescription", lpTranslate[i].wLanguage, lpTranslate[i].wCodePage);
                        if (VerQueryValue(pbVersionInfo, subBlock, (LPVOID*)&lpBuffer, &dwBytes)) {
                            this->description = wstring(lpBuffer);
                        }
                        if (!this->productName.empty() || !this->description.empty()) {
                            break;
                        }
                    }

                    delete[] pbVersionInfo;
                }

                // Get preliminary type
                if (hAddon) {
                    if (productName == L"ReShade") {
                        // Force chain mode for ReShade
                        this->addonType = AddonType::ChainAddon;
                    }
                    else if (GetProcAddress(hAddon, "Direct3DCreate9") || GetProcAddress(hAddon, "Direct3DCreate9Ex")) {
                        // Force legacy mode
                        this->addonType = AddonType::LegacyAddon;
                    }
                }
            }

            FreeLibrary(hAddon);

            GetLog()->info("Addon {0} type: {1}", ws2s(this->GetFileName()), ws2s(this->GetAddonTypeString()));
            if (this->GetID() != L"") {
                GetLog()->info("Addon {0} ID: {1}", ws2s(this->GetFileName()), ws2s(this->GetID()));
            }
            if (this->GetProductName() != L"") {
                GetLog()->info("Addon {0} product name: {1}", ws2s(this->GetFileName()), ws2s(this->GetProductName()));
            }
            if (this->GetAuthor() != L"") {
                GetLog()->info("Addon {0} author: {1}", ws2s(this->GetFileName()), ws2s(this->GetAuthor()));
            }
            if (this->GetDescription() != L"") {
                GetLog()->info("Addon {0} description: {1}", ws2s(this->GetFileName()), ws2s(this->GetDescription()));
            }
            if (this->GetVersion() != L"") {
                GetLog()->info("Addon {0} version: {1}", ws2s(this->GetFileName()), ws2s(this->GetVersion()));
            }
            if (this->GetHomepage() != L"") {
                GetLog()->info("Addon {0} homepage: {1}", ws2s(this->GetFileName()), ws2s(this->GetHomepage()));
            }
        }
  
  }
}
