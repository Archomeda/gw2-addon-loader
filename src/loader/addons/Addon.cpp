#include "Addon.h"
#include "addons_manager.h"
#include "exceptions.h"
#include "LegacyAddon.h"
#include "NativeAddon.h"
#include "ProxyAddon.h"
#include "../Config.h"
#include "../log.h"
#include "../utils/encoding.h"

using namespace std;
using namespace std::experimental::filesystem;
using namespace loader::updaters;
using namespace loader::utils;

namespace loader {
    namespace addons {

        const string AddonTypeToString(AddonType type) {
            switch (type) {
            case AddonType::AddonTypeNative:
                return "Native";
            case AddonType::AddonTypeLegacy:
                return "Legacy";
            case AddonType::AddonTypeLoaderProxy:
                return "Proxy";
            default:
                return "Unknown";
            }
        }

        const string AddonStateToString(AddonState state) {
            switch (state) {
            case AddonState::DeactivatedOnRestartState:
                return "Deactivates on restart";
            case AddonState::ActivatedOnRestartState:
                return "Activates on restart";
            case AddonState::ErroredState:
                return "Errored";
            case AddonState::UnloadingState:
                return "Unloading";
            case AddonState::UnloadedState:
                return "Not active";
            case AddonState::LoadingState:
                return "Loading";
            case AddonState::LoadedState:
                return "Active";
            default:
                return "Unknown";
            }
        }


        unique_ptr<Addon> Addon::GetAddon(const string& filePath) {
            return Addon::GetAddon(u8path(filePath));
        }

        unique_ptr<Addon> Addon::GetAddon(const path& filePath) {
            unique_ptr<Addon> addon = make_unique<Addon>(filePath);
            HMODULE hAddon = LoadLibraryEx(filePath.c_str(), NULL, DONT_RESOLVE_DLL_REFERENCES);

            if (GetProcAddress(hAddon, "ProxyInitialize") != nullptr) {
                // Our add-on is our loader proxy
                addon = make_unique<ProxyAddon>(filePath);
            }
            else if (GetProcAddress(hAddon, GW2ADDON_DLL_Initialize) != nullptr) {
                // Our add-on is native
                addon = make_unique<NativeAddon>(filePath);
            }
            else if (GetProcAddress(hAddon, "Direct3DCreate9") != nullptr) {
                // Our add-on is legacy
                addon = make_unique<LegacyAddon>(filePath);
            }
            FreeLibrary(hAddon);

            return addon;
        }


        bool Addon::Load() {
            if (this->HandleWndProc) this->AddHook(ActiveAddonHooks.HandleWndProc);
            if (this->DrawFrameBeforeGui) this->AddHook(ActiveAddonHooks.DrawFrameBeforeGui);
            if (this->DrawFrameBeforePostProcessing) this->AddHook(ActiveAddonHooks.DrawFrameBeforePostProcessing);
            if (this->DrawFrame) this->AddHook(ActiveAddonHooks.DrawFrame);
            if (this->AdvPreBeginScene) this->AddHook(ActiveAddonHooks.AdvPreBeginScene);
            if (this->AdvPostBeginScene) this->AddHook(ActiveAddonHooks.AdvPostBeginScene);
            if (this->AdvPreEndScene) this->AddHook(ActiveAddonHooks.AdvPreEndScene);
            if (this->AdvPostEndScene) this->AddHook(ActiveAddonHooks.AdvPostEndScene);
            if (this->AdvPreClear) this->AddHook(ActiveAddonHooks.AdvPreClear);
            if (this->AdvPostClear) this->AddHook(ActiveAddonHooks.AdvPostClear);
            if (this->AdvPreReset) this->AddHook(ActiveAddonHooks.AdvPreReset);
            if (this->AdvPostReset) this->AddHook(ActiveAddonHooks.AdvPostReset);
            if (this->AdvPrePresent) this->AddHook(ActiveAddonHooks.AdvPrePresent);
            if (this->AdvPostPresent) this->AddHook(ActiveAddonHooks.AdvPostPresent);
            if (this->AdvPreCreateTexture) this->AddHook(ActiveAddonHooks.AdvPreCreateTexture);
            if (this->AdvPostCreateTexture) this->AddHook(ActiveAddonHooks.AdvPostCreateTexture);
            if (this->AdvPreCreateVertexShader) this->AddHook(ActiveAddonHooks.AdvPreCreateVertexShader);
            if (this->AdvPostCreateVertexShader) this->AddHook(ActiveAddonHooks.AdvPostCreateVertexShader);
            if (this->AdvPreCreatePixelShader) this->AddHook(ActiveAddonHooks.AdvPreCreatePixelShader);
            if (this->AdvPostCreatePixelShader) this->AddHook(ActiveAddonHooks.AdvPostCreatePixelShader);
            if (this->AdvPreCreateRenderTarget) this->AddHook(ActiveAddonHooks.AdvPreCreateRenderTarget);
            if (this->AdvPostCreateRenderTarget) this->AddHook(ActiveAddonHooks.AdvPostCreateRenderTarget);
            if (this->AdvPreSetTexture) this->AddHook(ActiveAddonHooks.AdvPreSetTexture);
            if (this->AdvPostSetTexture) this->AddHook(ActiveAddonHooks.AdvPostSetTexture);
            if (this->AdvPreSetVertexShader) this->AddHook(ActiveAddonHooks.AdvPreSetVertexShader);
            if (this->AdvPostSetVertexShader) this->AddHook(ActiveAddonHooks.AdvPostSetVertexShader);
            if (this->AdvPreSetPixelShader) this->AddHook(ActiveAddonHooks.AdvPreSetPixelShader);
            if (this->AdvPostSetPixelShader) this->AddHook(ActiveAddonHooks.AdvPostSetPixelShader);
            if (this->AdvPreSetRenderTarget) this->AddHook(ActiveAddonHooks.AdvPreSetRenderTarget);
            if (this->AdvPostSetRenderTarget) this->AddHook(ActiveAddonHooks.AdvPostSetRenderTarget);
            if (this->AdvPreSetRenderState) this->AddHook(ActiveAddonHooks.AdvPreSetRenderState);
            if (this->AdvPostSetRenderState) this->AddHook(ActiveAddonHooks.AdvPostSetRenderState);
            if (this->AdvPreDrawIndexedPrimitive) this->AddHook(ActiveAddonHooks.AdvPreDrawIndexedPrimitive);
            if (this->AdvPostDrawIndexedPrimitive) this->AddHook(ActiveAddonHooks.AdvPostDrawIndexedPrimitive);
            
            return true;
        }

        bool Addon::Unload() {
            if (this->HandleWndProc) this->RemoveHook(ActiveAddonHooks.HandleWndProc);
            if (this->DrawFrameBeforeGui) this->RemoveHook(ActiveAddonHooks.DrawFrameBeforeGui);
            if (this->DrawFrameBeforePostProcessing) this->RemoveHook(ActiveAddonHooks.DrawFrameBeforePostProcessing);
            if (this->DrawFrame) this->RemoveHook(ActiveAddonHooks.DrawFrame);
            if (this->AdvPreBeginScene) this->RemoveHook(ActiveAddonHooks.AdvPreBeginScene);
            if (this->AdvPostBeginScene) this->RemoveHook(ActiveAddonHooks.AdvPostBeginScene);
            if (this->AdvPreEndScene) this->RemoveHook(ActiveAddonHooks.AdvPreEndScene);
            if (this->AdvPostEndScene) this->RemoveHook(ActiveAddonHooks.AdvPostEndScene);
            if (this->AdvPreClear) this->RemoveHook(ActiveAddonHooks.AdvPreClear);
            if (this->AdvPostClear) this->RemoveHook(ActiveAddonHooks.AdvPostClear);
            if (this->AdvPreReset) this->RemoveHook(ActiveAddonHooks.AdvPreReset);
            if (this->AdvPostReset) this->RemoveHook(ActiveAddonHooks.AdvPostReset);
            if (this->AdvPrePresent) this->RemoveHook(ActiveAddonHooks.AdvPrePresent);
            if (this->AdvPostPresent) this->RemoveHook(ActiveAddonHooks.AdvPostPresent);
            if (this->AdvPreCreateTexture) this->RemoveHook(ActiveAddonHooks.AdvPreCreateTexture);
            if (this->AdvPostCreateTexture) this->RemoveHook(ActiveAddonHooks.AdvPostCreateTexture);
            if (this->AdvPreCreateVertexShader) this->RemoveHook(ActiveAddonHooks.AdvPreCreateVertexShader);
            if (this->AdvPostCreateVertexShader) this->RemoveHook(ActiveAddonHooks.AdvPostCreateVertexShader);
            if (this->AdvPreCreatePixelShader) this->RemoveHook(ActiveAddonHooks.AdvPreCreatePixelShader);
            if (this->AdvPostCreatePixelShader) this->RemoveHook(ActiveAddonHooks.AdvPostCreatePixelShader);
            if (this->AdvPreCreateRenderTarget) this->RemoveHook(ActiveAddonHooks.AdvPreCreateRenderTarget);
            if (this->AdvPostCreateRenderTarget) this->RemoveHook(ActiveAddonHooks.AdvPostCreateRenderTarget);
            if (this->AdvPreSetTexture) this->RemoveHook(ActiveAddonHooks.AdvPreSetTexture);
            if (this->AdvPostSetTexture) this->RemoveHook(ActiveAddonHooks.AdvPostSetTexture);
            if (this->AdvPreSetVertexShader) this->RemoveHook(ActiveAddonHooks.AdvPreSetVertexShader);
            if (this->AdvPostSetVertexShader) this->RemoveHook(ActiveAddonHooks.AdvPostSetVertexShader);
            if (this->AdvPreSetPixelShader) this->RemoveHook(ActiveAddonHooks.AdvPreSetPixelShader);
            if (this->AdvPostSetPixelShader) this->RemoveHook(ActiveAddonHooks.AdvPostSetPixelShader);
            if (this->AdvPreSetRenderTarget) this->RemoveHook(ActiveAddonHooks.AdvPreSetRenderTarget);
            if (this->AdvPostSetRenderTarget) this->RemoveHook(ActiveAddonHooks.AdvPostSetRenderTarget);
            if (this->AdvPreSetRenderState) this->RemoveHook(ActiveAddonHooks.AdvPreSetRenderState);
            if (this->AdvPostSetRenderState) this->RemoveHook(ActiveAddonHooks.AdvPostSetRenderState);
            if (this->AdvPreDrawIndexedPrimitive) this->RemoveHook(ActiveAddonHooks.AdvPreDrawIndexedPrimitive);
            if (this->AdvPostDrawIndexedPrimitive) this->RemoveHook(ActiveAddonHooks.AdvPostDrawIndexedPrimitive);

            return true;
        }

        bool Addon::LoadNextRestart() {
            this->ChangeState(AddonState::ActivatedOnRestartState);
            return true;
        }

        bool Addon::UnloadNextRestart() {
            this->ChangeState(AddonState::DeactivatedOnRestartState);
            return true;
        }

        
        bool Addon::HasUpdate() const {
            VersionInfo version = this->GetLatestVersion();
            return this->SupportsUpdating() && !version.downloadUrl.empty() && version.version != this->GetVersion();
        }


        VersionInfo Addon::GetLatestVersion() const {
            VersionInfo versionInfo;
            versionInfo.version = AppConfig.GetLatestAddonVersion(this);
            versionInfo.infoUrl = AppConfig.GetLatestAddonVersionInfoUrl(this);
            versionInfo.downloadUrl = AppConfig.GetLatestAddonVersionDownloadUrl(this);
            return versionInfo;
        }


        void Addon::CheckUpdate(const function<UpdateCheckCallback_t>& callback) {
            shared_ptr<Updater> updater = this->GetUpdater();
            if (updater != nullptr) {
                // Force updater to be capsured, otherwise it goes out of scope and this lambda expression will never get called
                updater->SetCheckCallback([updater, callback](const Updater* const updater, VersionInfo version) {
                    callback(updater, version);
                });
                updater->CheckForUpdateAsync();
            }
            else {
                callback(nullptr, {});
            }
        }


        void Addon::OnStartFrame(IDirect3DDevice9* device) {
            if (AppConfig.GetShowDebugFeatures()) {
                this->metricOverall.StartFrame();
                this->DrawFrameBeforeGui.GetMetric().StartFrame();
                this->DrawFrameBeforePostProcessing.GetMetric().StartFrame();
                this->DrawFrame.GetMetric().StartFrame();
                this->AdvPreBeginScene.GetMetric().StartFrame();
                this->AdvPostBeginScene.GetMetric().StartFrame();
                this->AdvPreEndScene.GetMetric().StartFrame();
                this->AdvPostEndScene.GetMetric().StartFrame();
                this->AdvPreClear.GetMetric().StartFrame();
                this->AdvPostClear.GetMetric().StartFrame();
                this->AdvPreReset.GetMetric().StartFrame();
                this->AdvPostReset.GetMetric().StartFrame();
                this->AdvPrePresent.GetMetric().StartFrame();
                this->AdvPostPresent.GetMetric().StartFrame();
                this->AdvPreCreateTexture.GetMetric().StartFrame();
                this->AdvPostCreateTexture.GetMetric().StartFrame();
                this->AdvPreCreateVertexShader.GetMetric().StartFrame();
                this->AdvPostCreateVertexShader.GetMetric().StartFrame();
                this->AdvPreCreatePixelShader.GetMetric().StartFrame();
                this->AdvPostCreatePixelShader.GetMetric().StartFrame();
                this->AdvPreCreateRenderTarget.GetMetric().StartFrame();
                this->AdvPostCreateRenderTarget.GetMetric().StartFrame();
                this->AdvPreSetTexture.GetMetric().StartFrame();
                this->AdvPostSetTexture.GetMetric().StartFrame();
                this->AdvPreSetVertexShader.GetMetric().StartFrame();
                this->AdvPostSetVertexShader.GetMetric().StartFrame();
                this->AdvPreSetPixelShader.GetMetric().StartFrame();
                this->AdvPostSetPixelShader.GetMetric().StartFrame();
                this->AdvPreSetRenderTarget.GetMetric().StartFrame();
                this->AdvPostSetRenderTarget.GetMetric().StartFrame();
                this->AdvPreSetRenderState.GetMetric().StartFrame();
                this->AdvPostSetRenderState.GetMetric().StartFrame();
                this->AdvPreDrawIndexedPrimitive.GetMetric().StartFrame();
                this->AdvPostDrawIndexedPrimitive.GetMetric().StartFrame();
            }
        }

        void Addon::OnEndFrame(IDirect3DDevice9* device) {
            if (AppConfig.GetShowDebugFeatures()) {
                this->metricOverall.EndFrame();
                this->DrawFrameBeforeGui.GetMetric().EndFrame();
                this->DrawFrameBeforePostProcessing.GetMetric().EndFrame();
                this->DrawFrame.GetMetric().EndFrame();
                this->AdvPreBeginScene.GetMetric().EndFrame();
                this->AdvPostBeginScene.GetMetric().EndFrame();
                this->AdvPreEndScene.GetMetric().EndFrame();
                this->AdvPostEndScene.GetMetric().EndFrame();
                this->AdvPreClear.GetMetric().EndFrame();
                this->AdvPostClear.GetMetric().EndFrame();
                this->AdvPreReset.GetMetric().EndFrame();
                this->AdvPostReset.GetMetric().EndFrame();
                this->AdvPrePresent.GetMetric().EndFrame();
                this->AdvPostPresent.GetMetric().EndFrame();
                this->AdvPreCreateTexture.GetMetric().EndFrame();
                this->AdvPostCreateTexture.GetMetric().EndFrame();
                this->AdvPreCreateVertexShader.GetMetric().EndFrame();
                this->AdvPostCreateVertexShader.GetMetric().EndFrame();
                this->AdvPreCreatePixelShader.GetMetric().EndFrame();
                this->AdvPostCreatePixelShader.GetMetric().EndFrame();
                this->AdvPreCreateRenderTarget.GetMetric().EndFrame();
                this->AdvPostCreateRenderTarget.GetMetric().EndFrame();
                this->AdvPreSetTexture.GetMetric().EndFrame();
                this->AdvPostSetTexture.GetMetric().EndFrame();
                this->AdvPreSetVertexShader.GetMetric().EndFrame();
                this->AdvPostSetVertexShader.GetMetric().EndFrame();
                this->AdvPreSetPixelShader.GetMetric().EndFrame();
                this->AdvPostSetPixelShader.GetMetric().EndFrame();
                this->AdvPreSetRenderTarget.GetMetric().EndFrame();
                this->AdvPostSetRenderTarget.GetMetric().EndFrame();
                this->AdvPreSetRenderState.GetMetric().EndFrame();
                this->AdvPostSetRenderState.GetMetric().EndFrame();
                this->AdvPreDrawIndexedPrimitive.GetMetric().EndFrame();
                this->AdvPostDrawIndexedPrimitive.GetMetric().EndFrame();
            }
        }


        void Addon::InitializeAddonFuncs() {
            this->HandleWndProc.GetMetric().SetMetricType(AddonMetricType::SingleMetric);
            this->DrawFrameBeforeGui.SetGlobalMetric(&this->metricOverall);
            this->DrawFrameBeforePostProcessing.SetGlobalMetric(&this->metricOverall);
            this->DrawFrame.SetGlobalMetric(&this->metricOverall);
            this->AdvPreBeginScene.SetGlobalMetric(&this->metricOverall);
            this->AdvPostBeginScene.SetGlobalMetric(&this->metricOverall);
            this->AdvPreEndScene.SetGlobalMetric(&this->metricOverall);
            this->AdvPostEndScene.SetGlobalMetric(&this->metricOverall);
            this->AdvPreClear.SetGlobalMetric(&this->metricOverall);
            this->AdvPostClear.SetGlobalMetric(&this->metricOverall);
            this->AdvPreReset.SetGlobalMetric(&this->metricOverall);
            this->AdvPostReset.SetGlobalMetric(&this->metricOverall);
            this->AdvPrePresent.SetGlobalMetric(&this->metricOverall);
            this->AdvPostPresent.SetGlobalMetric(&this->metricOverall);
            this->AdvPreCreateTexture.SetGlobalMetric(&this->metricOverall);
            this->AdvPostCreateTexture.SetGlobalMetric(&this->metricOverall);
            this->AdvPreCreateVertexShader.SetGlobalMetric(&this->metricOverall);
            this->AdvPostCreateVertexShader.SetGlobalMetric(&this->metricOverall);
            this->AdvPreCreatePixelShader.SetGlobalMetric(&this->metricOverall);
            this->AdvPostCreatePixelShader.SetGlobalMetric(&this->metricOverall);
            this->AdvPreCreateRenderTarget.SetGlobalMetric(&this->metricOverall);
            this->AdvPostCreateRenderTarget.SetGlobalMetric(&this->metricOverall);
            this->AdvPreSetTexture.SetGlobalMetric(&this->metricOverall);
            this->AdvPostSetTexture.SetGlobalMetric(&this->metricOverall);
            this->AdvPreSetVertexShader.SetGlobalMetric(&this->metricOverall);
            this->AdvPostSetVertexShader.SetGlobalMetric(&this->metricOverall);
            this->AdvPreSetPixelShader.SetGlobalMetric(&this->metricOverall);
            this->AdvPostSetPixelShader.SetGlobalMetric(&this->metricOverall);
            this->AdvPreSetRenderTarget.SetGlobalMetric(&this->metricOverall);
            this->AdvPostSetRenderTarget.SetGlobalMetric(&this->metricOverall);
            this->AdvPreSetRenderState.SetGlobalMetric(&this->metricOverall);
            this->AdvPostSetRenderState.SetGlobalMetric(&this->metricOverall);
            this->AdvPreDrawIndexedPrimitive.SetGlobalMetric(&this->metricOverall);
            this->AdvPostDrawIndexedPrimitive.SetGlobalMetric(&this->metricOverall);
        }


        void Addon::AddHook(vector<Addon*>& addons) {
            addons.push_back(this);
        }

        void Addon::RemoveHook(vector<Addon*>& addons) {
            addons.erase(remove(addons.begin(), addons.end(), this), addons.end());
        }

    }
}
