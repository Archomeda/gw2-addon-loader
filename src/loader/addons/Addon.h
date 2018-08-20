#pragma once
#include "../stdafx.h"
#include "AddonFunc.h"
#include "AddonMetric.h"
#include "../hooks/LoaderDirect3D9.h"
#include "../hooks/LoaderDirect3DDevice9.h"
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

    class Addon {
    public:
        Addon() {
            this->InitializeAddonFuncs();
        }
        Addon(const std::string& filePath) :
            filePath(std::experimental::filesystem::u8path(filePath)),
            fileName(std::experimental::filesystem::u8path(filePath).filename().u8string()) {
            this->InitializeAddonFuncs();
        }
        Addon(const std::experimental::filesystem::path& filePath) :
            filePath(filePath),
            fileName(filePath.filename().u8string()) {
            this->InitializeAddonFuncs();
        }

        static std::unique_ptr<Addon> GetAddon(const std::string& filePath);
        static std::unique_ptr<Addon> GetAddon(const std::experimental::filesystem::path& filePath);

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

        virtual AddonType GetType() const { return AddonType::AddonTypeUnknown; }
        const std::string GetTypeString() const { return AddonTypeToString(this->GetType()); }

        virtual const std::experimental::filesystem::path GetFilePath() const { return this->filePath; }
        virtual const std::string GetFileName() const { return this->GetFilePath().filename().u8string(); }
        virtual const std::string GetID() const { return this->GetFileName(); }
        virtual const std::string GetName() const { return this->GetID(); }
        virtual const std::string GetAuthor() const { return ""; }
        virtual const std::string GetDescription() const { return ""; }
        virtual const std::string GetVersion() const { return ""; }
        virtual const std::string GetHomepage() const { return ""; }
        virtual IDirect3DTexture9* GetIcon() const { return nullptr; }
        updaters::VersionInfo GetLatestVersion() const;

        virtual void OpenSettings() { }
        void CheckUpdate(const std::function<updaters::UpdateCheckCallback_t>& callback);

        virtual void OnStartFrame(IDirect3DDevice9* device);
        virtual void OnEndFrame(IDirect3DDevice9* device);

        AddonMetric& GetMetricLoad() { return this->metricLoad; }
        AddonMetric& GetMetricOverall() { return this->metricOverall; }

        UINT D3D9SdkVersion = 0;
        hooks::LoaderDirect3D9* D3D9 = nullptr;
        hooks::LoaderDirect3DDevice9* D3DDevice9 = nullptr;
        HWND FocusWindow = NULL;

        AddonFunc<void, const char*> ApiKeyChange;

        AddonFunc<bool, HWND, UINT, WPARAM, LPARAM> HandleWndProc;

        bool HasRenderingHooks() const { return this->hasRenderingHooks; }
        AddonFunc<void, IDirect3DDevice9*> DrawFrameBeforeGui;
        AddonFunc<void, IDirect3DDevice9*> DrawFrameBeforePostProcessing;
        AddonFunc<void, IDirect3DDevice9*> DrawFrame;
        AddonFunc<void, IDirect3DDevice9*> AdvPreBeginScene;
        AddonFunc<void, IDirect3DDevice9*> AdvPostBeginScene;
        AddonFunc<void, IDirect3DDevice9*> AdvPreEndScene;
        AddonFunc<void, IDirect3DDevice9*> AdvPostEndScene;
        AddonFunc<void, IDirect3DDevice9*, DWORD, CONST D3DRECT*, DWORD, D3DCOLOR, float, DWORD> AdvPreClear;
        AddonFunc<void, IDirect3DDevice9*, DWORD, CONST D3DRECT*, DWORD, D3DCOLOR, float, DWORD> AdvPostClear;
        AddonFunc<void, IDirect3DDevice9*, D3DPRESENT_PARAMETERS*> AdvPreReset;
        AddonFunc<void, IDirect3DDevice9*, D3DPRESENT_PARAMETERS*> AdvPostReset;
        AddonFunc<void, IDirect3DDevice9*, CONST RECT*, CONST RECT*, HWND, CONST RGNDATA*> AdvPrePresent;
        AddonFunc<void, IDirect3DDevice9*, CONST RECT*, CONST RECT*, HWND, CONST RGNDATA*> AdvPostPresent;
        AddonFunc<HRESULT, IDirect3DDevice9*, UINT, UINT, UINT, DWORD, D3DFORMAT, D3DPOOL, IDirect3DTexture9**, HANDLE*> AdvPreCreateTexture;
        AddonFunc<void, IDirect3DDevice9*, IDirect3DTexture9*, UINT, UINT, UINT, DWORD, D3DFORMAT, D3DPOOL, HANDLE*> AdvPostCreateTexture;
        AddonFunc<HRESULT, IDirect3DDevice9*, CONST DWORD*, IDirect3DVertexShader9**> AdvPreCreateVertexShader;
        AddonFunc<void, IDirect3DDevice9*, IDirect3DVertexShader9*, CONST DWORD*> AdvPostCreateVertexShader;
        AddonFunc<HRESULT, IDirect3DDevice9*, CONST DWORD*, IDirect3DPixelShader9**> AdvPreCreatePixelShader;
        AddonFunc<void, IDirect3DDevice9*, IDirect3DPixelShader9*, CONST DWORD*> AdvPostCreatePixelShader;
        AddonFunc<HRESULT, IDirect3DDevice9*, UINT, UINT, D3DFORMAT, D3DMULTISAMPLE_TYPE, DWORD, BOOL, IDirect3DSurface9**, HANDLE*> AdvPreCreateRenderTarget;
        AddonFunc<void, IDirect3DDevice9*, IDirect3DSurface9*, UINT, UINT, D3DFORMAT, D3DMULTISAMPLE_TYPE, DWORD, BOOL, HANDLE*> AdvPostCreateRenderTarget;
        AddonFunc<void, IDirect3DDevice9*, DWORD, IDirect3DBaseTexture9*> AdvPreSetTexture;
        AddonFunc<void, IDirect3DDevice9*, DWORD, IDirect3DBaseTexture9*> AdvPostSetTexture;
        AddonFunc<void, IDirect3DDevice9*, IDirect3DVertexShader9*> AdvPreSetVertexShader;
        AddonFunc<void, IDirect3DDevice9*, IDirect3DVertexShader9*> AdvPostSetVertexShader;
        AddonFunc<void, IDirect3DDevice9*, IDirect3DPixelShader9*> AdvPreSetPixelShader;
        AddonFunc<void, IDirect3DDevice9*, IDirect3DPixelShader9*> AdvPostSetPixelShader;
        AddonFunc<void, IDirect3DDevice9*, DWORD, IDirect3DSurface9*> AdvPreSetRenderTarget;
        AddonFunc<void, IDirect3DDevice9*, DWORD, IDirect3DSurface9*> AdvPostSetRenderTarget;
        AddonFunc<void, IDirect3DDevice9*, D3DRENDERSTATETYPE, DWORD> AdvPreSetRenderState;
        AddonFunc<void, IDirect3DDevice9*, D3DRENDERSTATETYPE, DWORD> AdvPostSetRenderState;
        AddonFunc<void, IDirect3DDevice9*, D3DPRIMITIVETYPE, INT, UINT, UINT, UINT, UINT> AdvPreDrawIndexedPrimitive;
        AddonFunc<void, IDirect3DDevice9*, D3DPRIMITIVETYPE, INT, UINT, UINT, UINT, UINT> AdvPostDrawIndexedPrimitive;

    protected:
        virtual std::unique_ptr<updaters::Updater> GetUpdater() { return nullptr; }

        void ChangeState(AddonState state) { this->state = state; }

        void UpdateHasRenderingHooks();

    private:
        void InitializeAddonFuncs();

        void AddHook(std::vector<Addon*>& addons);
        void RemoveHook(std::vector<Addon*>& addons);

        AddonState state = AddonState::UnloadedState;
        std::experimental::filesystem::path filePath;
        std::string fileName;

        bool hasRenderingHooks = false;
        AddonMetric metricLoad = AddonMetric(AddonMetricType::SingleMetric);
        AddonMetric metricOverall;
    };

}
