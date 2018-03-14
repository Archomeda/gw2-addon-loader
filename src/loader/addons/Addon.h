#pragma once
#include "../windows.h"
#include <d3d9.h>
#include <chrono>
#include <memory>
#include <string>
#include "../TimeDuration.h"
#include "../TimeMeasure.h"

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

        class Addon : public std::enable_shared_from_this<Addon> {
        public:
            Addon() = default;
            Addon(const std::string& filePath);
            static std::unique_ptr<Addon> GetAddon(const std::string& filePath);

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

            virtual const AddonType GetType() const;
            const std::string GetTypeString() const { return AddonTypeToString(this->GetType()); }


            UINT GetSdkVersion() const;
            void SetSdkVersion(UINT sdkVersion);
            IDirect3D9* GetD3D9() const;
            void SetD3D9(IDirect3D9* d3d9);
            IDirect3DDevice9* GetD3DDevice9() const;
            void SetD3DDevice9(IDirect3DDevice9* device);
            HWND GetFocusWindow() const;
            void SetFocusWindow(HWND focusWindow);

            virtual const std::string GetFilePath() const;
            virtual const std::string GetFileName() const;
            virtual const std::string GetID() const;
            virtual const std::string GetName() const;
            virtual const std::string GetAuthor() const;
            virtual const std::string GetDescription() const;
            virtual const std::string GetVersion() const;
            virtual const std::string GetHomepage() const;
            virtual IDirect3DTexture9* GetIcon() const;

            virtual TimeDuration& GetLoadDuration();

            virtual void OpenSettings();

            virtual bool HandleWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
            virtual void DrawFrameBeforeGui(IDirect3DDevice9* device);
            virtual void DrawFrameBeforePostProcessing(IDirect3DDevice9* device);
            virtual void DrawFrame(IDirect3DDevice9* device);

            virtual void AdvPreBeginScene(IDirect3DDevice9* device);
            virtual void AdvPostBeginScene(IDirect3DDevice9* device);
            virtual void AdvPreEndScene(IDirect3DDevice9* device);
            virtual void AdvPostEndScene(IDirect3DDevice9* device);
            virtual void AdvPreClear(IDirect3DDevice9* device, DWORD Count, CONST D3DRECT* pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil);
            virtual void AdvPostClear(IDirect3DDevice9* device, DWORD Count, CONST D3DRECT* pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil);
            virtual void AdvPreReset(IDirect3DDevice9* device, D3DPRESENT_PARAMETERS* pPresentationParameters);
            virtual void AdvPostReset(IDirect3DDevice9* device, D3DPRESENT_PARAMETERS* pPresentationParameters);
            virtual void AdvPrePresent(IDirect3DDevice9* device, CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion);
            virtual void AdvPostPresent(IDirect3DDevice9* device, CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion);
            virtual HRESULT AdvPreCreateTexture(IDirect3DDevice9* device, UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DTexture9** ppTexture, HANDLE* pSharedHandle);
            virtual void AdvPostCreateTexture(IDirect3DDevice9* device, IDirect3DTexture9* pTexture, UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, HANDLE* pSharedHandle);
            virtual HRESULT AdvPreCreateVertexShader(IDirect3DDevice9* device, CONST DWORD* pFunction, IDirect3DVertexShader9** ppShader);
            virtual void AdvPostCreateVertexShader(IDirect3DDevice9* device, IDirect3DVertexShader9* ppShader, CONST DWORD* pFunction);
            virtual HRESULT AdvPreCreatePixelShader(IDirect3DDevice9* device, CONST DWORD* pFunction, IDirect3DPixelShader9** ppShader);
            virtual void AdvPostCreatePixelShader(IDirect3DDevice9* device, IDirect3DPixelShader9* ppShader, CONST DWORD* pFunction);
            virtual HRESULT AdvPreCreateRenderTarget(IDirect3DDevice9* device, UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Lockable, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle);
            virtual void AdvPostCreateRenderTarget(IDirect3DDevice9* device, IDirect3DSurface9* ppSurface, UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Lockable, HANDLE* pSharedHandle);
            virtual void AdvPreSetTexture(IDirect3DDevice9* device, DWORD Stage, IDirect3DBaseTexture9* pTexture);
            virtual void AdvPostSetTexture(IDirect3DDevice9* device, DWORD Stage, IDirect3DBaseTexture9* pTexture);
            virtual void AdvPreSetVertexShader(IDirect3DDevice9* device, IDirect3DVertexShader9* pShader);
            virtual void AdvPostSetVertexShader(IDirect3DDevice9* device, IDirect3DVertexShader9* pShader);
            virtual void AdvPreSetPixelShader(IDirect3DDevice9* device, IDirect3DPixelShader9* pShader);
            virtual void AdvPostSetPixelShader(IDirect3DDevice9* device, IDirect3DPixelShader9* pShader);
            virtual void AdvPreSetRenderTarget(IDirect3DDevice9* device, DWORD RenderTargetIndex, IDirect3DSurface9* pRenderTarget);
            virtual void AdvPostSetRenderTarget(IDirect3DDevice9* device, DWORD RenderTargetIndex, IDirect3DSurface9* pRenderTarget);
            virtual void AdvPreSetRenderState(IDirect3DDevice9* device, D3DRENDERSTATETYPE State, DWORD Value);
            virtual void AdvPostSetRenderState(IDirect3DDevice9* device, D3DRENDERSTATETYPE State, DWORD Value);
            virtual void AdvPreDrawIndexedPrimitive(IDirect3DDevice9* device, D3DPRIMITIVETYPE PrimitiveType, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount);
            virtual void AdvPostDrawIndexedPrimitive(IDirect3DDevice9* device, D3DPRIMITIVETYPE PrimitiveType, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount);

            virtual void OnStartFrame(IDirect3DDevice9* device);
            virtual void OnEndFrame(IDirect3DDevice9* device);

            TimeMeasure& GetTimeOverall();
            TimeMeasure& GetTimeWndProc();
            TimeMeasure& GetTimeDrawFrameBeforeGui();
            TimeMeasure& GetTimeDrawFrameBeforePostProcessing();
            TimeMeasure& GetTimeDrawFrame();
            TimeMeasure& GetTimeAdvPreBeginScene();
            TimeMeasure& GetTimeAdvPostBeginScene();
            TimeMeasure& GetTimeAdvPreEndScene();
            TimeMeasure& GetTimeAdvPostEndScene();
            TimeMeasure& GetTimeAdvPreClear();
            TimeMeasure& GetTimeAdvPostClear();
            TimeMeasure& GetTimeAdvPreReset();
            TimeMeasure& GetTimeAdvPostReset();
            TimeMeasure& GetTimeAdvPrePresent();
            TimeMeasure& GetTimeAdvPostPresent();
            TimeMeasure& GetTimeAdvPreCreateTexture();
            TimeMeasure& GetTimeAdvPostCreateTexture();
            TimeMeasure& GetTimeAdvPreCreateVertexShader();
            TimeMeasure& GetTimeAdvPostCreateVertexShader();
            TimeMeasure& GetTimeAdvPreCreatePixelShader();
            TimeMeasure& GetTimeAdvPostCreatePixelShader();
            TimeMeasure& GetTimeAdvPreCreateRenderTarget();
            TimeMeasure& GetTimeAdvPostCreateRenderTarget();
            TimeMeasure& GetTimeAdvPreSetTexture();
            TimeMeasure& GetTimeAdvPostSetTexture();
            TimeMeasure& GetTimeAdvPreSetVertexShader();
            TimeMeasure& GetTimeAdvPostSetVertexShader();
            TimeMeasure& GetTimeAdvPreSetPixelShader();
            TimeMeasure& GetTimeAdvPostSetPixelShader();
            TimeMeasure& GetTimeAdvPreSetRenderTarget();
            TimeMeasure& GetTimeAdvPostSetRenderTarget();
            TimeMeasure& GetTimeAdvPreSetRenderState();
            TimeMeasure& GetTimeAdvPostSetRenderState();
            TimeMeasure& GetTimeAdvPreDrawIndexedPrimitive();
            TimeMeasure& GetTimeAdvPostDrawIndexedPrimitive();

        protected:
            virtual const Addon* GetConstBaseAddon() const { return this; }
            virtual Addon* GetBaseAddon() { return this; }
            bool HasBaseAddon() const { return this != this->GetConstBaseAddon(); }

            void ChangeState(AddonState state);

        private:
            AddonState state = AddonState::UnloadedState;
            std::string filePath;
            std::string fileName;

            TimeDuration durationLoad;

            UINT sdkVersion;
            IDirect3D9* d3d9;
            IDirect3D9Ex* d3d9Ex;
            IDirect3DDevice9* d3ddevice9;
            HWND focusWindow;

            TimeMeasure timeOverall;
            TimeMeasure timeWndProc;
            TimeMeasure timeDrawFrameBeforeGui;
            TimeMeasure timeDrawFrameBeforePostProcessing;
            TimeMeasure timeDrawFrame;
            TimeMeasure timeAdvPreBeginScene;
            TimeMeasure timeAdvPostBeginScene;
            TimeMeasure timeAdvPreEndScene;
            TimeMeasure timeAdvPostEndScene;
            TimeMeasure timeAdvPreClear;
            TimeMeasure timeAdvPostClear;
            TimeMeasure timeAdvPreReset;
            TimeMeasure timeAdvPostReset;
            TimeMeasure timeAdvPrePresent;
            TimeMeasure timeAdvPostPresent;
            TimeMeasure timeAdvPreCreateTexture;
            TimeMeasure timeAdvPostCreateTexture;
            TimeMeasure timeAdvPreCreateVertexShader;
            TimeMeasure timeAdvPostCreateVertexShader;
            TimeMeasure timeAdvPreCreatePixelShader;
            TimeMeasure timeAdvPostCreatePixelShader;
            TimeMeasure timeAdvPreCreateRenderTarget;
            TimeMeasure timeAdvPostCreateRenderTarget;
            TimeMeasure timeAdvPreSetTexture;
            TimeMeasure timeAdvPostSetTexture;
            TimeMeasure timeAdvPreSetVertexShader;
            TimeMeasure timeAdvPostSetVertexShader;
            TimeMeasure timeAdvPreSetPixelShader;
            TimeMeasure timeAdvPostSetPixelShader;
            TimeMeasure timeAdvPreSetRenderTarget;
            TimeMeasure timeAdvPostSetRenderTarget;
            TimeMeasure timeAdvPreSetRenderState;
            TimeMeasure timeAdvPostSetRenderState;
            TimeMeasure timeAdvPreDrawIndexedPrimitive;
            TimeMeasure timeAdvPostDrawIndexedPrimitive;
        };

    }
}
