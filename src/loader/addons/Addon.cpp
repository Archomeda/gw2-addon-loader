#include "Addon.h"
#include <Strsafe.h>
#include <filesystem>
#include <string>
#include <gw2addon-native.h>
#include "addons_manager.h"
#include "exceptions.h"
#include "NativeAddon.h"
#include "UnknownAddon.h"
#include "../Config.h"
#include "../log.h"
#include "../utils/encoding.h"

using namespace std;
using namespace std::experimental::filesystem;
using namespace loader::utils;

namespace loader {
    namespace addons {

        const string AddonTypeToString(AddonType type) {
            switch (type) {
            case AddonType::AddonTypeNative:
                return "Native";
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

            if (GetProcAddress(hAddon, GW2ADDON_DLL_Initialize) != nullptr) {
                // Our addon is native
                addon = make_unique<NativeAddon<Addon>>(*addon);
            }
            else {
                // Unknown addon
                addon = make_unique<UnknownAddon<Addon>>(*addon);
            }
            FreeLibrary(hAddon);

            return addon;
        }


        bool Addon::Initialize() {
            if (this->HasBaseAddon()) {
                return this->GetBaseAddon()->Initialize();
            }
            return true;
        }

        bool Addon::Uninitialize() {
            if (this->HasBaseAddon()) {
                return this->GetBaseAddon()->Uninitialize();
            }
            return true;
        }

        bool Addon::Load() {
            if (this->HasBaseAddon()) {
                return this->GetBaseAddon()->Load();
            }
            return true;
        }

        bool Addon::Unload() {
            if (this->HasBaseAddon()) {
                return this->GetBaseAddon()->Unload();
            }
            return true;
        }

        bool Addon::IsEnabledByConfig() const {
            return AppConfig.GetAddonEnabled(this);
        }


        const AddonState Addon::GetState() const {
            if (this->HasBaseAddon()) {
                return this->GetConstBaseAddon()->GetState();
            }
            return this->state;
        }

        void Addon::ChangeState(AddonState state) {
            if (this->HasBaseAddon()) {
                return this->GetBaseAddon()->ChangeState(state);
            }
            this->state = state;
        }


        bool Addon::SupportsLoading() const {
            if (this->HasBaseAddon()) {
                return this->GetConstBaseAddon()->SupportsLoading();
            }
            return true;
        }

        bool Addon::SupportsHotLoading() const {
            if (this->HasBaseAddon()) {
                return this->GetConstBaseAddon()->SupportsHotLoading();
            }
            return true;
        }

        bool Addon::SupportsSettings() const {
            if (this->HasBaseAddon()) {
                return this->GetConstBaseAddon()->SupportsSettings();
            }
            return true;
        }

        bool Addon::SupportsHomepage() const {
            if (this->HasBaseAddon()) {
                return this->GetConstBaseAddon()->SupportsHomepage();
            }
            return !this->GetHomepage().empty();
        }


        AddonType Addon::GetType() const {
            if (this->HasBaseAddon()) {
                return this->GetConstBaseAddon()->GetType();
            }
            return AddonType::AddonTypeUnknown;
        }


        UINT Addon::GetSdkVersion() const {
            if (this->HasBaseAddon()) {
                return this->GetConstBaseAddon()->GetSdkVersion();
            }
            return this->sdkVersion;
        }

        void Addon::SetSdkVersion(UINT sdkVersion) {
            if (this->HasBaseAddon()) {
                return this->GetBaseAddon()->SetSdkVersion(sdkVersion);
            }
            this->sdkVersion = sdkVersion;
        }

        IDirect3D9* Addon::GetD3D9() const {
            if (this->HasBaseAddon()) {
                return this->GetConstBaseAddon()->GetD3D9();
            }
            return this->d3d9;
        }

        void Addon::SetD3D9(IDirect3D9* d3d9) {
            if (this->HasBaseAddon()) {
                return this->GetBaseAddon()->SetD3D9(d3d9);
            }
            this->d3d9 = d3d9;
        }

        IDirect3DDevice9* Addon::GetD3DDevice9() const {
            if (this->HasBaseAddon()) {
                return this->GetConstBaseAddon()->GetD3DDevice9();
            }
            return this->d3ddevice9;
        }

        void Addon::SetD3DDevice9(IDirect3DDevice9* device) {
            if (this->HasBaseAddon()) {
                return this->GetBaseAddon()->SetD3DDevice9(device);
            }
            this->d3ddevice9 = device;
        }

        HWND Addon::GetFocusWindow() const {
            if (this->HasBaseAddon()) {
                return this->GetConstBaseAddon()->GetFocusWindow();
            }
            return this->focusWindow;
        }

        void Addon::SetFocusWindow(HWND focusWindow) {
            if (this->HasBaseAddon()) {
                return this->GetBaseAddon()->SetFocusWindow(focusWindow);
            }
            this->focusWindow = focusWindow;
        }


        const path Addon::GetFilePath() const {
            if (this->HasBaseAddon()) {
                return this->GetConstBaseAddon()->GetFilePath();
            }
            return this->filePath;
        }

        const string Addon::GetID() const {
            if (this->HasBaseAddon()) {
                return this->GetConstBaseAddon()->GetID();
            }
            return this->GetFileName();
        }

        const string Addon::GetName() const {
            if (this->HasBaseAddon()) {
                return this->GetConstBaseAddon()->GetName();
            }
            return this->GetID();
        }

        const string Addon::GetAuthor() const {
            if (this->HasBaseAddon()) {
                return this->GetConstBaseAddon()->GetAuthor();
            }
            return "";
        }

        const string Addon::GetDescription() const {
            if (this->HasBaseAddon()) {
                return this->GetConstBaseAddon()->GetDescription();
            }
            return "";
        }

        const string Addon::GetVersion() const {
            if (this->HasBaseAddon()) {
                return this->GetConstBaseAddon()->GetVersion();
            }
            return "";
        }

        const string Addon::GetHomepage() const {
            if (this->HasBaseAddon()) {
                return this->GetConstBaseAddon()->GetHomepage();
            }
            return "";
        }

        IDirect3DTexture9* Addon::GetIcon() const {
            if (this->HasBaseAddon()) {
                return this->GetConstBaseAddon()->GetIcon();
            }
            return nullptr;
        }


        void Addon::OpenSettings() {
            if (this->HasBaseAddon()) {
                return this->GetBaseAddon()->OpenSettings();
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


        AddonMetric& Addon::GetMetricLoad() {
            return this->metricLoad;
        }

        AddonMetric& Addon::GetMetricOverall() {
            return this->metricOverall;
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

    }
}
