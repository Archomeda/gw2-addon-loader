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
using namespace std::filesystem;
using namespace loader::updaters;
using namespace loader::utils;

namespace loader::addons {

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

        if (GetProcAddress(hAddon, GW2PROXY_DLL_Initialize) != nullptr) {
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
        if (this->ApiKeyChange) this->AddHook(ActiveAddonHooks.ApiKeyChange);
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

        this->UpdateHasRenderingHooks();

        // Let the add-on know about our shared API key
        if (this->ApiKeyChange) {
            string key = AppConfig.GetApiKey();
            if (!key.empty()) {
                this->ApiKeyChange(key.c_str(), static_cast<int>(key.length()));
            }
            else {
                this->ApiKeyChange(nullptr, 0);
            }
        }

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
        return this->SupportsUpdating() && version.version != this->GetVersion();
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
            // Force updater to be captured, otherwise it goes out of scope and this lambda expression will never get called
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
        if (AppConfig.GetDiagnostics()) {
            this->metricOverall.Prepare();
            this->DrawFrameBeforeGui.GetMetric().Prepare();
            this->DrawFrameBeforePostProcessing.GetMetric().Prepare();
            this->DrawFrame.GetMetric().Prepare();
            this->AdvPreBeginScene.GetMetric().Prepare();
            this->AdvPostBeginScene.GetMetric().Prepare();
            this->AdvPreEndScene.GetMetric().Prepare();
            this->AdvPostEndScene.GetMetric().Prepare();
            this->AdvPreClear.GetMetric().Prepare();
            this->AdvPostClear.GetMetric().Prepare();
            this->AdvPreReset.GetMetric().Prepare();
            this->AdvPostReset.GetMetric().Prepare();
            this->AdvPrePresent.GetMetric().Prepare();
            this->AdvPostPresent.GetMetric().Prepare();
            this->AdvPreCreateTexture.GetMetric().Prepare();
            this->AdvPostCreateTexture.GetMetric().Prepare();
            this->AdvPreCreateVertexShader.GetMetric().Prepare();
            this->AdvPostCreateVertexShader.GetMetric().Prepare();
            this->AdvPreCreatePixelShader.GetMetric().Prepare();
            this->AdvPostCreatePixelShader.GetMetric().Prepare();
            this->AdvPreCreateRenderTarget.GetMetric().Prepare();
            this->AdvPostCreateRenderTarget.GetMetric().Prepare();
            this->AdvPreSetTexture.GetMetric().Prepare();
            this->AdvPostSetTexture.GetMetric().Prepare();
            this->AdvPreSetVertexShader.GetMetric().Prepare();
            this->AdvPostSetVertexShader.GetMetric().Prepare();
            this->AdvPreSetPixelShader.GetMetric().Prepare();
            this->AdvPostSetPixelShader.GetMetric().Prepare();
            this->AdvPreSetRenderTarget.GetMetric().Prepare();
            this->AdvPostSetRenderTarget.GetMetric().Prepare();
            this->AdvPreSetRenderState.GetMetric().Prepare();
            this->AdvPostSetRenderState.GetMetric().Prepare();
            this->AdvPreDrawIndexedPrimitive.GetMetric().Prepare();
            this->AdvPostDrawIndexedPrimitive.GetMetric().Prepare();
        }
    }

    void Addon::OnEndFrame(IDirect3DDevice9* device) {
        if (AppConfig.GetDiagnostics()) {
            this->metricOverall.Stop();
            this->DrawFrameBeforeGui.GetMetric().Stop();
            this->DrawFrameBeforePostProcessing.GetMetric().Stop();
            this->DrawFrame.GetMetric().Stop();
            this->AdvPreBeginScene.GetMetric().Stop();
            this->AdvPostBeginScene.GetMetric().Stop();
            this->AdvPreEndScene.GetMetric().Stop();
            this->AdvPostEndScene.GetMetric().Stop();
            this->AdvPreClear.GetMetric().Stop();
            this->AdvPostClear.GetMetric().Stop();
            this->AdvPreReset.GetMetric().Stop();
            this->AdvPostReset.GetMetric().Stop();
            this->AdvPrePresent.GetMetric().Stop();
            this->AdvPostPresent.GetMetric().Stop();
            this->AdvPreCreateTexture.GetMetric().Stop();
            this->AdvPostCreateTexture.GetMetric().Stop();
            this->AdvPreCreateVertexShader.GetMetric().Stop();
            this->AdvPostCreateVertexShader.GetMetric().Stop();
            this->AdvPreCreatePixelShader.GetMetric().Stop();
            this->AdvPostCreatePixelShader.GetMetric().Stop();
            this->AdvPreCreateRenderTarget.GetMetric().Stop();
            this->AdvPostCreateRenderTarget.GetMetric().Stop();
            this->AdvPreSetTexture.GetMetric().Stop();
            this->AdvPostSetTexture.GetMetric().Stop();
            this->AdvPreSetVertexShader.GetMetric().Stop();
            this->AdvPostSetVertexShader.GetMetric().Stop();
            this->AdvPreSetPixelShader.GetMetric().Stop();
            this->AdvPostSetPixelShader.GetMetric().Stop();
            this->AdvPreSetRenderTarget.GetMetric().Stop();
            this->AdvPostSetRenderTarget.GetMetric().Stop();
            this->AdvPreSetRenderState.GetMetric().Stop();
            this->AdvPostSetRenderState.GetMetric().Stop();
            this->AdvPreDrawIndexedPrimitive.GetMetric().Stop();
            this->AdvPostDrawIndexedPrimitive.GetMetric().Stop();
        }
    }


    void Addon::InitializeAddonFuncs() {
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


    void Addon::UpdateHasRenderingHooks() {
        this->hasRenderingHooks = this->DrawFrameBeforeGui ||
            this->DrawFrameBeforePostProcessing ||
            this->DrawFrame ||
            this->AdvPreBeginScene || this->AdvPostBeginScene ||
            this->AdvPreEndScene || this->AdvPostEndScene ||
            this->AdvPreClear || this->AdvPostClear ||
            this->AdvPreReset || this->AdvPostReset ||
            this->AdvPreCreateTexture || this->AdvPostCreateTexture ||
            this->AdvPreCreateVertexShader || this->AdvPostCreateVertexShader ||
            this->AdvPreCreatePixelShader || this->AdvPostCreatePixelShader ||
            this->AdvPreCreateRenderTarget || this->AdvPostCreateRenderTarget ||
            this->AdvPreSetTexture || this->AdvPostSetTexture ||
            this->AdvPreSetVertexShader || this->AdvPostSetVertexShader ||
            this->AdvPreSetPixelShader || this->AdvPostSetVertexShader ||
            this->AdvPreSetPixelShader || this->AdvPostSetPixelShader ||
            this->AdvPreSetRenderTarget || this->AdvPostSetRenderTarget ||
            this->AdvPreSetRenderState || this->AdvPostSetRenderState ||
            this->AdvPreDrawIndexedPrimitive || this->AdvPostDrawIndexedPrimitive;
    }

}
