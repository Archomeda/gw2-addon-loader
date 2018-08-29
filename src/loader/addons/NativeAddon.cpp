#include "NativeAddon.h"
#include "addons_manager.h"
#include "../gui/gui_manager.h"
#include "../gui/SettingsWindow.h"
#include "../gui/Window.h"
#include "../log.h"
#include "../updaters/CustomDownloader.h"
#include "../updaters/CustomUpdater.h"
#include "../updaters/GithubReleasesUpdater.h"
#include "../updaters/HttpDownloader.h"

using namespace std;
using namespace loader::gui;
using namespace loader::updaters;
using namespace loader::utils;

namespace loader::addons {

    bool NativeAddon::Initialize() {
        HMODULE h = LoadLibrary(this->GetFilePath().c_str());
        if (h == NULL) {
            this->ChangeState(AddonState::ErroredState);
            ADDONS_LOG()->error("Could not initialize native add-on {0}: Library handle is empty", this->GetFileName());
            return false;
        }
        this->addonHandle = h;

        this->AddonInitialize = reinterpret_cast<GW2AddonInitialize_t*>(GetProcAddress(h, GW2ADDON_DLL_Initialize));
        this->AddonRelease = reinterpret_cast<GW2AddonRelease_t*>(GetProcAddress(h, GW2ADDON_DLL_Release));
        ADDONS_LOG()->debug("Native add-on {0}:", this->GetFileName());
        ADDONS_LOG()->debug(" - " GW2ADDON_DLL_Initialize " - 0x{0:X}", reinterpret_cast<size_t>(this->AddonInitialize));
        ADDONS_LOG()->debug(" - " GW2ADDON_DLL_Release " - 0x{0:X}", reinterpret_cast<size_t>(this->AddonRelease));

        if (this->AddonInitialize == nullptr) {
            this->ChangeState(AddonState::ErroredState);
            ADDONS_LOG()->error("Could not initialize native add-on {0}: Addon doesn't contain a valid " GW2ADDON_DLL_Initialize " export", this->GetFileName());
            return false;
        }
        if (this->AddonRelease == nullptr) {
            this->ChangeState(AddonState::ErroredState);
            ADDONS_LOG()->error("Could not initialize native add-on {0}: Addon doesn't contain a valid " GW2ADDON_DLL_Release " export", this->GetFileName());
            return false;
        }

        GW2AddonAPIBase* addonBase = this->AddonInitialize(GW2ADDON_VER);
        if (addonBase == nullptr) {
            this->ChangeState(AddonState::ErroredState);
            ADDONS_LOG()->error("Could not initialize native add-on {0}: Addon didn't return a valid GW2AddonAPIBase pointer when calling " GW2ADDON_DLL_Initialize, this->GetFileName());
            return false;
        }

        ADDONS_LOG()->debug("Native add-on {0} is using compatibility version {1}", this->GetFileName(), addonBase->ver);
        if (addonBase->ver == 1) {
            if (!this->InitializeV1(addonBase)) {
                return false;
            }
        }
        else {
            this->ChangeState(AddonState::ErroredState);
            ADDONS_LOG()->error("Could not initialize native add-on {0}: Addon uses compatibility version {1} that is not supported. Are both the add-on and the add-on loader updated?", this->GetFileName(), addonBase->ver);
            return false;
        }

        return true;
    }

    bool NativeAddon::InitializeV1(GW2AddonAPIBase* base) {
        GW2AddonAPIV1* v1 = reinterpret_cast<GW2AddonAPIV1*>(base);
        this->id = v1->id;
        this->name = v1->name;
        this->author = v1->author;
        this->description = v1->description;
        this->version = v1->version;
        this->homepage = v1->homepage;

        ADDONS_LOG()->debug(" - id: {0}", v1->id);
        ADDONS_LOG()->debug(" - name: {0}", v1->name);
        ADDONS_LOG()->debug(" - author: {1}", v1->author);
        ADDONS_LOG()->debug(" - description: {1}", v1->description);
        ADDONS_LOG()->debug(" - version: {1}", v1->version);
        ADDONS_LOG()->debug(" - homepage: {1}", v1->homepage);

        if (v1->icon) {
            this->iconManaged = v1->iconSize > -1;
            if (this->iconManaged) {
                // Icon is just image data, we have to create the texture
                D3DXCreateTextureFromFileInMemory(this->D3DDevice9->GetSystemDevice(), v1->icon, v1->iconSize, &this->icon);
                this->iconManaged = true;
                ADDONS_LOG()->debug(" - icon: managed 0x{0:X}", reinterpret_cast<size_t>(this->icon));
                ADDONS_LOG()->debug(" - icon size: {0}", v1->iconSize);
            }
            else {
                // Icon is already a loaded texture
                this->icon = reinterpret_cast<IDirect3DTexture9*>(v1->icon);
                ADDONS_LOG()->debug(" - icon: unmanaged 0x{0:X}", reinterpret_cast<size_t>(this->icon));
            }
        }

        this->updateMethod = v1->updateInfo.method;
        ADDONS_LOG()->debug(" - update method: {0}", this->updateMethod);
        switch (this->updateMethod) {
        case AddonUpdateMethod::CustomUpdateMethod:
            this->AddonCheckUpdate = v1->CheckUpdate;
            if (this->AddonCheckUpdate) {
                ADDONS_LOG()->debug(" - CheckUpdate: 0x{0:X}", reinterpret_cast<size_t>(v1->CheckUpdate));
            }
            else {
                this->updateMethod = AddonUpdateMethod::NoUpdateMethod;
                ADDONS_LOG()->warn("No CheckUpdate function provided, add-on updater disabled");
            }
            this->AddonDownloadUpdate = v1->DownloadUpdate;
            if (this->AddonDownloadUpdate) {
                ADDONS_LOG()->debug(" - DownloadUpdate: 0x{0:X}", reinterpret_cast<size_t>(v1->DownloadUpdate));
            }
            break;
        case AddonUpdateMethod::GithubReleasesUpdateMethod:
            this->githubRepo = v1->updateInfo.githubRepo;
            ADDONS_LOG()->debug(" - GitHub repo - {0}", this->githubRepo);
            break;
        }

        this->AddonLoad = v1->Load;
        this->AddonOpenSettings = v1->OpenSettings;
        ADDONS_LOG()->debug(" - Load: 0x{0:X}", reinterpret_cast<size_t>(v1->Load));
        ADDONS_LOG()->debug(" - OpenSettings: 0x{0:X}", reinterpret_cast<size_t>(v1->OpenSettings));

        this->HandleWndProc.Func = v1->HandleWndProc;
        this->DrawFrameBeforePostProcessing.Func = v1->DrawFrameBeforePostProcessing;
        this->DrawFrameBeforeGui.Func = v1->DrawFrameBeforeGui;
        this->DrawFrame.Func = v1->DrawFrame;
        this->ApiKeyChange.Func = v1->ApiKeyChange;
        this->AdvPreBeginScene.Func = v1->AdvPreBeginScene;
        this->AdvPostBeginScene.Func = v1->AdvPostBeginScene;
        this->AdvPreEndScene.Func = v1->AdvPreEndScene;
        this->AdvPostEndScene.Func = v1->AdvPostEndScene;
        this->AdvPreClear.Func = v1->AdvPreClear;
        this->AdvPostClear.Func = v1->AdvPostClear;
        this->AdvPreReset.Func = v1->AdvPreReset;
        this->AdvPostReset.Func = v1->AdvPostReset;
        this->AdvPrePresent.Func = v1->AdvPrePresent;
        this->AdvPostPresent.Func = v1->AdvPostPresent;
        this->AdvPreCreateTexture.Func = v1->AdvPreCreateTexture;
        this->AdvPostCreateTexture.Func = v1->AdvPostCreateTexture;
        this->AdvPreCreateVertexShader.Func = v1->AdvPreCreateVertexShader;
        this->AdvPostCreateVertexShader.Func = v1->AdvPostCreateVertexShader;
        this->AdvPreCreatePixelShader.Func = v1->AdvPreCreatePixelShader;
        this->AdvPostCreatePixelShader.Func = v1->AdvPostCreatePixelShader;
        this->AdvPreCreateRenderTarget.Func = v1->AdvPreCreateRenderTarget;
        this->AdvPostCreateRenderTarget.Func = v1->AdvPostCreateRenderTarget;
        this->AdvPreSetTexture.Func = v1->AdvPreSetTexture;
        this->AdvPostSetTexture.Func = v1->AdvPostSetTexture;
        this->AdvPreSetVertexShader.Func = v1->AdvPreSetVertexShader;
        this->AdvPostSetVertexShader.Func = v1->AdvPostSetVertexShader;
        this->AdvPreSetPixelShader.Func = v1->AdvPreSetPixelShader;
        this->AdvPostSetPixelShader.Func = v1->AdvPostSetPixelShader;
        this->AdvPreSetRenderTarget.Func = v1->AdvPreSetRenderTarget;
        this->AdvPostSetRenderTarget.Func = v1->AdvPostSetRenderTarget;
        this->AdvPreSetRenderState.Func = v1->AdvPreSetRenderState;
        this->AdvPostSetRenderState.Func = v1->AdvPostSetRenderState;
        this->AdvPreDrawIndexedPrimitive.Func = v1->AdvPreDrawIndexedPrimitive;
        this->AdvPostDrawIndexedPrimitive.Func = v1->AdvPostDrawIndexedPrimitive;
        ADDONS_LOG()->debug(" - HandleWndProc: 0x{0:X}", reinterpret_cast<size_t>(v1->HandleWndProc));
        ADDONS_LOG()->debug(" - DrawFrameBeforePostProcessing: 0x{0:X}", reinterpret_cast<size_t>(v1->DrawFrameBeforePostProcessing));
        ADDONS_LOG()->debug(" - DrawFrameBeforeGui: 0x{0:X}", reinterpret_cast<size_t>(v1->DrawFrameBeforeGui));
        ADDONS_LOG()->debug(" - DrawFrame: 0x{0:X}", reinterpret_cast<size_t>(v1->DrawFrame));
        ADDONS_LOG()->debug(" - ApiKeyChange: 0x{0:X}", reinterpret_cast<size_t>(v1->ApiKeyChange));
        ADDONS_LOG()->debug(" - AdvPreBeginScene: 0x{0:X}", reinterpret_cast<size_t>(v1->AdvPreBeginScene));
        ADDONS_LOG()->debug(" - AdvPostBeginScene: 0x{0:X}", reinterpret_cast<size_t>(v1->AdvPostBeginScene));
        ADDONS_LOG()->debug(" - AdvPreEndScene: 0x{0:X}", reinterpret_cast<size_t>(v1->AdvPreEndScene));
        ADDONS_LOG()->debug(" - AdvPostEndScene: 0x{0:X}", reinterpret_cast<size_t>(v1->AdvPostEndScene));
        ADDONS_LOG()->debug(" - AdvPreClear: 0x{0:X}", reinterpret_cast<size_t>(v1->AdvPreClear));
        ADDONS_LOG()->debug(" - AdvPostClear: 0x{0:X}", reinterpret_cast<size_t>(v1->AdvPostClear));
        ADDONS_LOG()->debug(" - AdvPreReset: 0x{0:X}", reinterpret_cast<size_t>(v1->AdvPreReset));
        ADDONS_LOG()->debug(" - AdvPostReset: 0x{0:X}", reinterpret_cast<size_t>(v1->AdvPostReset));
        ADDONS_LOG()->debug(" - AdvPrePresent: 0x{0:X}", reinterpret_cast<size_t>(v1->AdvPrePresent));
        ADDONS_LOG()->debug(" - AdvPostPresent: 0x{0:X}", reinterpret_cast<size_t>(v1->AdvPostPresent));
        ADDONS_LOG()->debug(" - AdvPreCreateTexture: 0x{0:X}", reinterpret_cast<size_t>(v1->AdvPreCreateTexture));
        ADDONS_LOG()->debug(" - AdvPostCreateTexture: 0x{0:X}", reinterpret_cast<size_t>(v1->AdvPostCreateTexture));
        ADDONS_LOG()->debug(" - AdvPreCreateVertexShader: 0x{0:X}", reinterpret_cast<size_t>(v1->AdvPreCreateVertexShader));
        ADDONS_LOG()->debug(" - AdvPostCreateVertexShader: 0x{0:X}", reinterpret_cast<size_t>(v1->AdvPostCreateVertexShader));
        ADDONS_LOG()->debug(" - AdvPreCreatePixelShader: 0x{0:X}", reinterpret_cast<size_t>(v1->AdvPreCreatePixelShader));
        ADDONS_LOG()->debug(" - AdvPostCreatePixelShader: 0x{0:X}", reinterpret_cast<size_t>(v1->AdvPostCreatePixelShader));
        ADDONS_LOG()->debug(" - AdvPreCreateRenderTarget: 0x{0:X}", reinterpret_cast<size_t>(v1->AdvPreCreateRenderTarget));
        ADDONS_LOG()->debug(" - AdvPostCreateRenderTarget: 0x{0:X}", reinterpret_cast<size_t>(v1->AdvPostCreateRenderTarget));
        ADDONS_LOG()->debug(" - AdvPreSetTexture: 0x{0:X}", reinterpret_cast<size_t>(v1->AdvPreSetTexture));
        ADDONS_LOG()->debug(" - AdvPostSetTexture: 0x{0:X}", reinterpret_cast<size_t>(v1->AdvPostSetTexture));
        ADDONS_LOG()->debug(" - AdvPreSetVertexShader: 0x{0:X}", reinterpret_cast<size_t>(v1->AdvPreSetVertexShader));
        ADDONS_LOG()->debug(" - AdvPostSetVertexShader: 0x{0:X}", reinterpret_cast<size_t>(v1->AdvPostSetVertexShader));
        ADDONS_LOG()->debug(" - AdvPreSetPixelShader: 0x{0:X}", reinterpret_cast<size_t>(v1->AdvPreSetPixelShader));
        ADDONS_LOG()->debug(" - AdvPostSetPixelShader: 0x{0:X}", reinterpret_cast<size_t>(v1->AdvPostSetPixelShader));
        ADDONS_LOG()->debug(" - AdvPreSetRenderTarget: 0x{0:X}", reinterpret_cast<size_t>(v1->AdvPreSetRenderTarget));
        ADDONS_LOG()->debug(" - AdvPostSetRenderTarget: 0x{0:X}", reinterpret_cast<size_t>(v1->AdvPostSetRenderTarget));
        ADDONS_LOG()->debug(" - AdvPreSetRenderState: 0x{0:X}", reinterpret_cast<size_t>(v1->AdvPreSetRenderState));
        ADDONS_LOG()->debug(" - AdvPostSetRenderState: 0x{0:X}", reinterpret_cast<size_t>(v1->AdvPostSetRenderState));
        ADDONS_LOG()->debug(" - AdvPreDrawIndexedPrimitive: 0x{0:X}", reinterpret_cast<size_t>(v1->AdvPreDrawIndexedPrimitive));
        ADDONS_LOG()->debug(" - AdvPostDrawIndexedPrimitive: 0x{0:X}", reinterpret_cast<size_t>(v1->AdvPostDrawIndexedPrimitive));

        return true;
    }

    bool NativeAddon::Uninitialize() {
        this->Unload();

        this->AddonInitialize = nullptr;
        this->AddonRelease = nullptr;
        this->AddonLoad = nullptr;
        this->AddonOpenSettings = nullptr;

        this->HandleWndProc.Func = nullptr;
        this->DrawFrame.Func = nullptr;
        this->DrawFrameBeforeGui.Func = nullptr;
        this->DrawFrameBeforePostProcessing.Func = nullptr;
        this->AdvPreBeginScene.Func = nullptr;
        this->AdvPostBeginScene.Func = nullptr;
        this->AdvPreEndScene.Func = nullptr;
        this->AdvPostEndScene.Func = nullptr;
        this->AdvPreClear.Func = nullptr;
        this->AdvPostClear.Func = nullptr;
        this->AdvPreReset.Func = nullptr;
        this->AdvPostReset.Func = nullptr;
        this->AdvPrePresent.Func = nullptr;
        this->AdvPostPresent.Func = nullptr;
        this->AdvPreCreateTexture.Func = nullptr;
        this->AdvPostCreateTexture.Func = nullptr;
        this->AdvPreCreateVertexShader.Func = nullptr;
        this->AdvPostCreateVertexShader.Func = nullptr;
        this->AdvPreCreatePixelShader.Func = nullptr;
        this->AdvPostCreatePixelShader.Func = nullptr;
        this->AdvPreCreateRenderTarget.Func = nullptr;
        this->AdvPostCreateRenderTarget.Func = nullptr;
        this->AdvPreSetTexture.Func = nullptr;
        this->AdvPostSetTexture.Func = nullptr;
        this->AdvPreSetVertexShader.Func = nullptr;
        this->AdvPostSetVertexShader.Func = nullptr;
        this->AdvPreSetPixelShader.Func = nullptr;
        this->AdvPostSetPixelShader.Func = nullptr;
        this->AdvPreSetRenderTarget.Func = nullptr;
        this->AdvPostSetRenderTarget.Func = nullptr;
        this->AdvPreSetRenderState.Func = nullptr;
        this->AdvPostSetRenderState.Func = nullptr;
        this->AdvPreDrawIndexedPrimitive.Func = nullptr;
        this->AdvPostDrawIndexedPrimitive.Func = nullptr;

        if (this->iconManaged) {
            this->icon->Release();
            this->icon = nullptr;
            this->iconManaged = false;
        }

        if (this->addonHandle) {
            FreeLibrary(this->addonHandle);
            this->addonHandle = NULL;
        }

        return true;
    }

    bool NativeAddon::Load() {
        if (this->GetState() != AddonState::UnloadedState) {
            return false;
        }
        this->ChangeState(AddonState::LoadingState);

        this->GetMetricLoad().StartMeasurement();
        if (this->AddonLoad != nullptr) {
            GW2ADDON_RESULT result = this->AddonLoad(this->FocusWindow, this->D3DDevice9->GetSystemDevice());

            if (result) {
                this->GetMetricLoad().EndMeasurement();
                this->ChangeState(AddonState::ErroredState);
                ADDONS_LOG()->error("Could not load native add-on {0}: Add-on returned {1}", this->GetFileName(), to_string(result));
                return false;
            }
        }

        bool result = Addon::Load();

        this->GetMetricLoad().EndMeasurement();
        this->ChangeState(AddonState::LoadedState);

        return result;
    }

    bool NativeAddon::Unload() {
        if (this->GetState() != AddonState::LoadedState) {
            return false;
        }
        this->ChangeState(AddonState::UnloadingState);

        bool result = Addon::Unload();
        this->AddonRelease();

        this->ChangeState(AddonState::UnloadedState);
        return result;
    }


    unique_ptr<Updater> NativeAddon::GetUpdater() {
        switch (this->GetUpdateMethod()) {
        case AddonUpdateMethod::CustomUpdateMethod:
            if (this->AddonCheckUpdate) {
                return make_unique<CustomUpdater>(this->AddonCheckUpdate);
            }
            break;
        case AddonUpdateMethod::GithubReleasesUpdateMethod:
            return make_unique<GithubReleasesUpdater>(this->githubRepo);
        }
        return nullptr;
    }

    unique_ptr<Downloader> NativeAddon::GetDownloader() {
        const auto version = this->GetLatestVersion();
        if (version.downloadUrl.empty()) {
            if (this->AddonDownloadUpdate) {
                return make_unique<CustomDownloader>(this->AddonDownloadUpdate);
            }
        }
        else {
            return make_unique<HttpDownloader>(version.downloadUrl);
        }
        return nullptr;
    }

    void NativeAddon::OpenSettings() {
        if (this->AddonOpenSettings != nullptr && !this->lastSettings) {
            auto settings = this->AddonOpenSettings(nullptr);
            if (settings != nullptr) {
                // We supply the UI
                this->lastSettings = settings;
                this->settingsWindow = make_unique<AddonSettingsWindow>(shared_from_this());
                for (int i = 0; i < settings->entriesSize; ++i) {
                    this->settingsWindow->AddSetting(settings->entries[i]);
                }
                this->settingsWindow->Show();
            }
            else {
                // The add-on supplies the UI, so we just close the settings window
                SettingsWnd->Close();
            }
        }
    }

    void NativeAddon::CloseSettings() {
        if (this->lastSettings != nullptr) {
            this->AddonOpenSettings(this->lastSettings);
            this->lastSettings = nullptr;
            this->settingsWindow.reset();
        }
    }

}
