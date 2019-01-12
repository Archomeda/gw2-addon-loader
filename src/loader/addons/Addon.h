#pragma once
#include "../stdafx.h"
#include "AddonCallFunc.h"
#include "AddonFrameFunc.h"
#include "../diagnostics/HistoricTimeMetric.h"
#include "../diagnostics/SingleTimeMetric.h"
#include "../hooks/LoaderDirect3D9.h"
#include "../hooks/LoaderDirect3DDevice9.h"
#include "../updaters/Downloader.h"
#include "../updaters/Updater.h"

namespace loader::addons {

    enum AddonType {
        AddonTypeUnknown,
        AddonTypeNative,
        AddonTypeLegacy,
        AddonTypeLoaderProxy
    };

    enum AddonState {
        UnknownState,
        DeactivatedOnRestartState,
        ActivatedOnRestartState,
        ErroredState,
        UnloadingState,
        UnloadedState,
        LoadingState,
        LoadedState
    };

    const std::string AddonTypeToString(AddonType type);
    const std::string AddonStateToString(AddonState state);

    class Addon : public std::enable_shared_from_this<Addon> {
    public:
        Addon() {
            this->InitializeAddonFuncs();
        }
        Addon(const std::string& filePath) :
            filePath(std::filesystem::u8path(filePath)),
            fileName(std::filesystem::u8path(filePath).filename().u8string()) {
            this->InitializeAddonFuncs();
        }
        Addon(const std::filesystem::path& filePath) :
            filePath(filePath),
            fileName(filePath.filename().u8string()) {
            this->InitializeAddonFuncs();
        }

        static std::unique_ptr<Addon> GetAddon(const std::string& filePath);
        static std::unique_ptr<Addon> GetAddon(const std::filesystem::path& filePath);

        virtual bool Initialize() { return false; }
        virtual bool Uninitialize() { return false; }
        virtual bool Load();
        virtual bool Unload();
        virtual bool LoadNextRestart();
        virtual bool UnloadNextRestart();
        bool IsEnabledByConfig() { return AppConfig.GetAddonEnabled(this); }

        const AddonState GetState() const { return this->state; }
        const std::string GetStateString() const { return AddonStateToString(this->GetState()); }
        bool IsLoaded() const { return this->GetState() == AddonState::LoadedState; }
        bool HasUpdate() const;

        virtual bool IsForced() const { return false; }
        virtual bool IsHidden() const { return this->IsForced() || !this->SupportsLoading(); }
        virtual bool SupportsLoading() const { return false; }
        virtual bool SupportsHotLoading() const { return false; }
        virtual bool SupportsSettings() const { return false; }
        virtual bool SupportsHomepage() const { return !this->GetHomepage().empty(); }
        virtual bool SupportsUpdating() const { return false; }
        virtual AddonUpdateMethod GetUpdateMethod() const { return AddonUpdateMethod::NoUpdateMethod; }

        virtual AddonType GetType() const { return AddonType::AddonTypeUnknown; }
        const std::string GetTypeString() const { return AddonTypeToString(this->GetType()); }

        virtual const std::filesystem::path GetFilePath() const { return this->filePath; }
        virtual const std::string GetFileName() const { return this->GetFilePath().filename().u8string(); }
        virtual const std::string GetID() const { return this->GetFileName(); }
        virtual const std::string GetName() const { return this->GetID(); }
        virtual const std::string GetAuthor() const { return ""; }
        virtual const std::string GetDescription() const { return ""; }
        virtual const std::string GetVersion() const { return ""; }
        virtual const std::string GetHomepage() const { return ""; }
        virtual IDirect3DTexture9* GetIcon() const { return nullptr; }
        updaters::VersionInfo GetLatestVersion() const;

        virtual std::unique_ptr<updaters::Updater> GetUpdater() { return nullptr; }
        virtual std::unique_ptr<updaters::Downloader> GetDownloader() { return nullptr; }

        virtual void OpenSettings() { }
        virtual void CloseSettings() { }
        void CheckUpdate(const std::function<updaters::UpdateCheckCallback_t>& callback);

        virtual void OnStartFrame(IDirect3DDevice9* device);
        virtual void OnEndFrame(IDirect3DDevice9* device);

        diagnostics::SingleTimeMetric& GetMetricLoad() { return this->metricLoad; }
        diagnostics::HistoricTimeMetric<1000000, 2>& GetMetricOverall() { return this->metricOverall; }

        UINT D3D9SdkVersion = 0;
        hooks::LoaderDirect3D9* D3D9 = nullptr;
        hooks::LoaderDirect3DDevice9* D3DDevice9 = nullptr;
        HWND FocusWindow = NULL;

        AddonCallFunc<void, const char*, int> ApiKeyChange;

        AddonCallFunc<bool, HWND, UINT, WPARAM, LPARAM> HandleWndProc;

        bool HasRenderingHooks() const { return this->hasRenderingHooks; }
        AddonFrameFunc<void, IDirect3DDevice9*> DrawFrameBeforeGui;
        AddonFrameFunc<void, IDirect3DDevice9*> DrawFrameBeforePostProcessing;
        AddonFrameFunc<void, IDirect3DDevice9*> DrawFrame;
        AddonFrameFunc<void, IDirect3DDevice9*> AdvPreBeginScene;
        AddonFrameFunc<void, IDirect3DDevice9*> AdvPostBeginScene;
        AddonFrameFunc<void, IDirect3DDevice9*> AdvPreEndScene;
        AddonFrameFunc<void, IDirect3DDevice9*> AdvPostEndScene;
        AddonFrameFunc<void, IDirect3DDevice9*, DWORD, CONST D3DRECT*, DWORD, D3DCOLOR, float, DWORD> AdvPreClear;
        AddonFrameFunc<void, IDirect3DDevice9*, DWORD, CONST D3DRECT*, DWORD, D3DCOLOR, float, DWORD> AdvPostClear;
        AddonFrameFunc<void, IDirect3DDevice9*, D3DPRESENT_PARAMETERS*> AdvPreReset;
        AddonFrameFunc<void, IDirect3DDevice9*, D3DPRESENT_PARAMETERS*> AdvPostReset;
        AddonFrameFunc<void, IDirect3DDevice9*, CONST RECT*, CONST RECT*, HWND, CONST RGNDATA*> AdvPrePresent;
        AddonFrameFunc<void, IDirect3DDevice9*, CONST RECT*, CONST RECT*, HWND, CONST RGNDATA*> AdvPostPresent;
        AddonFrameFunc<HRESULT, IDirect3DDevice9*, UINT, UINT, UINT, DWORD, D3DFORMAT, D3DPOOL, IDirect3DTexture9**, HANDLE*> AdvPreCreateTexture;
        AddonFrameFunc<void, IDirect3DDevice9*, IDirect3DTexture9*, UINT, UINT, UINT, DWORD, D3DFORMAT, D3DPOOL, HANDLE*> AdvPostCreateTexture;
        AddonFrameFunc<HRESULT, IDirect3DDevice9*, CONST DWORD*, IDirect3DVertexShader9**> AdvPreCreateVertexShader;
        AddonFrameFunc<void, IDirect3DDevice9*, IDirect3DVertexShader9*, CONST DWORD*> AdvPostCreateVertexShader;
        AddonFrameFunc<HRESULT, IDirect3DDevice9*, CONST DWORD*, IDirect3DPixelShader9**> AdvPreCreatePixelShader;
        AddonFrameFunc<void, IDirect3DDevice9*, IDirect3DPixelShader9*, CONST DWORD*> AdvPostCreatePixelShader;
        AddonFrameFunc<HRESULT, IDirect3DDevice9*, UINT, UINT, D3DFORMAT, D3DMULTISAMPLE_TYPE, DWORD, BOOL, IDirect3DSurface9**, HANDLE*> AdvPreCreateRenderTarget;
        AddonFrameFunc<void, IDirect3DDevice9*, IDirect3DSurface9*, UINT, UINT, D3DFORMAT, D3DMULTISAMPLE_TYPE, DWORD, BOOL, HANDLE*> AdvPostCreateRenderTarget;
        AddonFrameFunc<void, IDirect3DDevice9*, DWORD, IDirect3DBaseTexture9*> AdvPreSetTexture;
        AddonFrameFunc<void, IDirect3DDevice9*, DWORD, IDirect3DBaseTexture9*> AdvPostSetTexture;
        AddonFrameFunc<void, IDirect3DDevice9*, IDirect3DVertexShader9*> AdvPreSetVertexShader;
        AddonFrameFunc<void, IDirect3DDevice9*, IDirect3DVertexShader9*> AdvPostSetVertexShader;
        AddonFrameFunc<void, IDirect3DDevice9*, IDirect3DPixelShader9*> AdvPreSetPixelShader;
        AddonFrameFunc<void, IDirect3DDevice9*, IDirect3DPixelShader9*> AdvPostSetPixelShader;
        AddonFrameFunc<void, IDirect3DDevice9*, DWORD, IDirect3DSurface9*> AdvPreSetRenderTarget;
        AddonFrameFunc<void, IDirect3DDevice9*, DWORD, IDirect3DSurface9*> AdvPostSetRenderTarget;
        AddonFrameFunc<void, IDirect3DDevice9*, D3DRENDERSTATETYPE, DWORD> AdvPreSetRenderState;
        AddonFrameFunc<void, IDirect3DDevice9*, D3DRENDERSTATETYPE, DWORD> AdvPostSetRenderState;
        AddonFrameFunc<void, IDirect3DDevice9*, D3DPRIMITIVETYPE, INT, UINT, UINT, UINT, UINT> AdvPreDrawIndexedPrimitive;
        AddonFrameFunc<void, IDirect3DDevice9*, D3DPRIMITIVETYPE, INT, UINT, UINT, UINT, UINT> AdvPostDrawIndexedPrimitive;

    protected:
        void ChangeState(AddonState state) { this->state = state; }

        void UpdateHasRenderingHooks();

    private:
        void InitializeAddonFuncs();

        void AddHook(std::vector<Addon*>& addons);
        void RemoveHook(std::vector<Addon*>& addons);

        AddonState state = AddonState::UnloadedState;
        std::filesystem::path filePath;
        std::string fileName;

        bool hasRenderingHooks = false;
        diagnostics::SingleTimeMetric metricLoad;
        diagnostics::HistoricTimeMetric<1000000, 2> metricOverall;
    };

}
