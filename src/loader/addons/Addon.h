#pragma once
#include "../windows.h"
#include <d3d9.h>
#include <chrono>
#include <filesystem>
#include <memory>
#include <string>
#include "AddonFunc.h"
#include "AddonMetric.h"

namespace loader {
    namespace addons {

        enum AddonType {
            AddonTypeUnknown,
            AddonTypeNative
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

            virtual bool Initialize();
            virtual bool Uninitialize();
            virtual bool Load();
            virtual bool Unload();
            bool IsEnabledByConfig() const;

            const AddonState GetState() const;
            const std::string GetStateString() const { return AddonStateToString(this->GetState()); }
            bool IsLoaded() const { return this->GetState() == AddonState::LoadedState; }

            virtual bool SupportsLoading() const;
            virtual bool SupportsHotLoading() const;
            virtual bool SupportsSettings() const;
            virtual bool SupportsHomepage() const;

            virtual AddonType GetType() const;
            const std::string GetTypeString() const { return AddonTypeToString(this->GetType()); }

            UINT GetSdkVersion() const;
            void SetSdkVersion(UINT sdkVersion);
            IDirect3D9* GetD3D9() const;
            void SetD3D9(IDirect3D9* d3d9);
            IDirect3DDevice9* GetD3DDevice9() const;
            void SetD3DDevice9(IDirect3DDevice9* device);
            HWND GetFocusWindow() const;
            void SetFocusWindow(HWND focusWindow);

            virtual const std::experimental::filesystem::path GetFilePath() const;
            virtual const std::string GetFileName() const { return this->GetFilePath().filename().u8string(); }
            virtual const std::string GetID() const;
            virtual const std::string GetName() const;
            virtual const std::string GetAuthor() const;
            virtual const std::string GetDescription() const;
            virtual const std::string GetVersion() const;
            virtual const std::string GetHomepage() const;
            virtual IDirect3DTexture9* GetIcon() const;

            virtual void OpenSettings();

            virtual void OnStartFrame(IDirect3DDevice9* device);
            virtual void OnEndFrame(IDirect3DDevice9* device);

            AddonMetric& GetMetricLoad();
            AddonMetric& GetMetricOverall();

            AddonFunc<bool, HWND, UINT, WPARAM, LPARAM> HandleWndProc;
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
            virtual const Addon* GetConstBaseAddon() const { return this; }
            virtual Addon* GetBaseAddon() { return this; }
            bool HasBaseAddon() const { return this != this->GetConstBaseAddon(); }

            void ChangeState(AddonState state);

        private:
            void InitializeAddonFuncs();

            AddonState state = AddonState::UnloadedState;
            std::experimental::filesystem::path filePath;
            std::string fileName;

            UINT sdkVersion = 0;
            IDirect3D9* d3d9 = nullptr;
            IDirect3D9Ex* d3d9Ex = nullptr;
            IDirect3DDevice9* d3ddevice9 = nullptr;
            HWND focusWindow = NULL;

            AddonMetric metricLoad = AddonMetric(AddonMetricType::SingleMetric);
            AddonMetric metricOverall;
        };

    }
}
