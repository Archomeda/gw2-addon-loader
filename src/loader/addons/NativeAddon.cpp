#include "NativeAddon.h"
#include "addons_manager.h"
#include "../log.h"
#include "../updaters/GithubReleasesUpdater.h"

using namespace std;
using namespace loader::updaters;
using namespace loader::utils;

namespace loader {
    namespace addons {

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

            if (addonBase->ver == 1) {
                if (!this->InitializeV1(addonBase)) {
                    return false;
                }
            }
            else {
                this->ChangeState(AddonState::ErroredState);
                ADDONS_LOG()->error(("Could not initialize native add-on {0}: Addon uses a version (" + to_string(addonBase->ver) + ") that is not compatible. Are both the add-on and the add-on loader updated?").c_str(), this->GetFileName());
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

            if (v1->icon) {
                this->iconManaged = v1->iconSize > -1;
                if (this->iconManaged) {
                    // Icon is just image data, we have to create the texture
                    D3DXCreateTextureFromFileInMemory(this->D3DDevice9->GetSystemDevice(), v1->icon, v1->iconSize, &this->icon);
                    this->iconManaged = true;
                }
                else {
                    // Icon is already a loaded texture
                    this->icon = reinterpret_cast<IDirect3DTexture9*>(v1->icon);
                }
            }
            
            this->updateMethod = v1->updateInfo.method;
            switch (this->updateMethod) {
            case AddonUpdateMethod::GithubReleasesUpdateMethod:
                this->githubRepo = v1->updateInfo.githubRepo;
                break;
            }

            this->AddonLoad = v1->Load;
            this->AddonOpenSettings = v1->OpenSettings;

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

            if (this->AddonLoad != nullptr) {
                this->GetMetricLoad().StartMeasurement();
                GW2ADDON_RESULT result = this->AddonLoad(this->FocusWindow, this->D3DDevice9->GetSystemDevice());
                this->GetMetricLoad().EndMeasurement();

                if (result) {
                    this->ChangeState(AddonState::ErroredState);
                    ADDONS_LOG()->error("Could not load native add-on {0}: Add-on returned {1}", this->GetFileName(), to_string(result));
                    return false;
                }
            }

            bool result = Addon::Load();
            this->ChangeState(AddonState::LoadedState);

            // Let the add-on know about our shared API key
            if (this->ApiKeyChange) {
                this->ApiKeyChange(!AppConfig.GetApiKey().empty() ? AppConfig.GetApiKey().c_str() : nullptr);
            }

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


        void NativeAddon::OpenSettings() {
            if (this->AddonOpenSettings != nullptr) {
                this->AddonOpenSettings();
            }
        }

        unique_ptr<Updater> NativeAddon::GetUpdater() {
            switch (this->GetUpdateMethod()) {
            case AddonUpdateMethod::GithubReleasesUpdateMethod:
                return make_unique<GithubReleasesUpdater>(this->githubRepo);
            }
            return nullptr;
        }

    }
}
