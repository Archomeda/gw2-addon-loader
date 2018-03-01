#pragma once
#include "../../windows.h"
#include <d3d9.h>
#include <chrono>
#include <memory>
#include <string>
#include <vector>
#include "../../TimeMeasure.h"

namespace loader {
    namespace addons {

        // Forward declare Addon, because we use weak_ptr
        class Addon;

        namespace types {

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

            class ITypeImpl {
            public:
                std::weak_ptr<Addon> GetAddon() const { return this->addon; }
                void SetAddon(std::weak_ptr<Addon> addon) { this->addon = addon; }

                virtual HMODULE GetHandle() const { return this->addonHandle; }

                virtual const std::string GetID() const = 0;
                virtual const std::string GetName() const = 0;
                virtual const std::string GetAuthor() const { return ""; }
                virtual const std::string GetDescription() const { return ""; }
                virtual const std::string GetVersion() const { return ""; }
                virtual const std::string GetHomepage() const { return ""; }
                virtual IDirect3DTexture9* GetIcon() const { return nullptr; }

                virtual const AddonState GetAddonState() const { return this->state; }
                virtual const std::string GetAddonStateString() const;

                virtual bool SupportsHotLoading() const = 0;
                virtual void Initialize() = 0;
                virtual void Uninitialize() = 0;
                virtual void Load() = 0;
                virtual void Unload() = 0;

                virtual void OnStartFrame(IDirect3DDevice9* device) { }
                virtual void OnEndFrame(IDirect3DDevice9* device) { }

                virtual bool HandleWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) { return false; }

                virtual void DrawFrameBeforeGui(IDirect3DDevice9* device) { }
                virtual void DrawFrameBeforePostProcessing(IDirect3DDevice9* device) { }
                virtual void DrawFrame(IDirect3DDevice9* device) { }

                virtual void AdvPreBeginScene(IDirect3DDevice9* device) { }
                virtual void AdvPostBeginScene(IDirect3DDevice9* device) { }
                virtual void AdvPreEndScene(IDirect3DDevice9* device) { }
                virtual void AdvPostEndScene(IDirect3DDevice9* device) { }
                virtual void AdvPreClear(IDirect3DDevice9* device, DWORD Count, CONST D3DRECT* pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil) { }
                virtual void AdvPostClear(IDirect3DDevice9* device, DWORD Count, CONST D3DRECT* pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil) { }
                virtual void AdvPreReset(IDirect3DDevice9* device, D3DPRESENT_PARAMETERS* pPresentationParameters) { }
                virtual void AdvPostReset(IDirect3DDevice9* device, D3DPRESENT_PARAMETERS* pPresentationParameters) { }
                virtual void AdvPrePresent(IDirect3DDevice9* device, CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion) { }
                virtual void AdvPostPresent(IDirect3DDevice9* device, CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion) { }
                virtual HRESULT AdvPreCreateTexture(IDirect3DDevice9* device, UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DTexture9** ppTexture, HANDLE* pSharedHandle) { return D3D_OK; }
                virtual void AdvPostCreateTexture(IDirect3DDevice9* device, IDirect3DTexture9* pTexture, UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, HANDLE* pSharedHandle) { }
                virtual HRESULT AdvPreCreateVertexShader(IDirect3DDevice9* device, CONST DWORD* pFunction, IDirect3DVertexShader9** ppShader) { return D3D_OK; }
                virtual void AdvPostCreateVertexShader(IDirect3DDevice9* device, IDirect3DVertexShader9* ppShader, CONST DWORD* pFunction) { }
                virtual HRESULT AdvPreCreatePixelShader(IDirect3DDevice9* device, CONST DWORD* pFunction, IDirect3DPixelShader9** ppShader) { return D3D_OK; }
                virtual void AdvPostCreatePixelShader(IDirect3DDevice9* device, IDirect3DPixelShader9* ppShader, CONST DWORD* pFunction) { }
                virtual HRESULT AdvPreCreateRenderTarget(IDirect3DDevice9* device, UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Lockable, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle) { return D3D_OK; }
                virtual void AdvPostCreateRenderTarget(IDirect3DDevice9* device, IDirect3DSurface9* ppSurface, UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Lockable, HANDLE* pSharedHandle) { }
                virtual void AdvPreSetTexture(IDirect3DDevice9* device, DWORD Stage, IDirect3DBaseTexture9* pTexture) { }
                virtual void AdvPostSetTexture(IDirect3DDevice9* device, DWORD Stage, IDirect3DBaseTexture9* pTexture) { }
                virtual void AdvPreSetVertexShader(IDirect3DDevice9* device, IDirect3DVertexShader9* pShader) { }
                virtual void AdvPostSetVertexShader(IDirect3DDevice9* device, IDirect3DVertexShader9* pShader) { }
                virtual void AdvPreSetPixelShader(IDirect3DDevice9* device, IDirect3DPixelShader9* pShader) { }
                virtual void AdvPostSetPixelShader(IDirect3DDevice9* device, IDirect3DPixelShader9* pShader) { }
                virtual void AdvPreSetRenderTarget(IDirect3DDevice9* device, DWORD RenderTargetIndex, IDirect3DSurface9* pRenderTarget) { }
                virtual void AdvPostSetRenderTarget(IDirect3DDevice9* device, DWORD RenderTargetIndex, IDirect3DSurface9* pRenderTarget) { }
                virtual void AdvPreSetRenderState(IDirect3DDevice9* device, D3DRENDERSTATETYPE State, DWORD Value) { }
                virtual void AdvPostSetRenderState(IDirect3DDevice9* device, D3DRENDERSTATETYPE State, DWORD Value) { }
                virtual void AdvPreDrawIndexedPrimitive(IDirect3DDevice9* device, D3DPRIMITIVETYPE PrimitiveType, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount) { }
                virtual void AdvPostDrawIndexedPrimitive(IDirect3DDevice9* device, D3DPRIMITIVETYPE PrimitiveType, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount) { }

                virtual const TimeMeasure GetTimeOverall() { return this->timeOverall; }
                virtual const TimeMeasure GetTimeDrawFrameBeforeGui() { return this->timeDrawFrameBeforeGui; }
                virtual const TimeMeasure GetTimeDrawFrameBeforePostProcessing() { return this->timeDrawFrameBeforePostProcessing; }
                virtual const TimeMeasure GetTimeDrawFrame() { return this->timeDrawFrame; }
                virtual const TimeMeasure GetTimeAdvPreBeginScene() { return this->timeAdvPreBeginScene; }
                virtual const TimeMeasure GetTimeAdvPostBeginScene() { return this->timeAdvPostBeginScene; }
                virtual const TimeMeasure GetTimeAdvPreEndScene() { return this->timeAdvPreEndScene; }
                virtual const TimeMeasure GetTimeAdvPostEndScene() { return this->timeAdvPostEndScene; }
                virtual const TimeMeasure GetTimeAdvPreClear() { return this->timeAdvPreClear; }
                virtual const TimeMeasure GetTimeAdvPostClear() { return this->timeAdvPostClear; }
                virtual const TimeMeasure GetTimeAdvPreReset() { return this->timeAdvPreReset; }
                virtual const TimeMeasure GetTimeAdvPostReset() { return this->timeAdvPostReset; }
                virtual const TimeMeasure GetTimeAdvPrePresent() { return this->timeAdvPrePresent; }
                virtual const TimeMeasure GetTimeAdvPostPresent() { return this->timeAdvPostPresent; }
                virtual const TimeMeasure GetTimeAdvPreCreateTexture() { return this->timeAdvPreCreateTexture; }
                virtual const TimeMeasure GetTimeAdvPostCreateTexture() { return this->timeAdvPostCreateTexture; }
                virtual const TimeMeasure GetTimeAdvPreCreateVertexShader() { return this->timeAdvPreCreateVertexShader; }
                virtual const TimeMeasure GetTimeAdvPostCreateVertexShader() { return this->timeAdvPostCreateVertexShader; }
                virtual const TimeMeasure GetTimeAdvPreCreatePixelShader() { return this->timeAdvPreCreatePixelShader; }
                virtual const TimeMeasure GetTimeAdvPostCreatePixelShader() { return this->timeAdvPostCreatePixelShader; }
                virtual const TimeMeasure GetTimeAdvPreCreateRenderTarget() { return this->timeAdvPreCreateRenderTarget; }
                virtual const TimeMeasure GetTimeAdvPostCreateRenderTarget() { return this->timeAdvPostCreateRenderTarget; }
                virtual const TimeMeasure GetTimeAdvPreSetTexture() { return this->timeAdvPreSetTexture; }
                virtual const TimeMeasure GetTimeAdvPostSetTexture() { return this->timeAdvPostSetTexture; }
                virtual const TimeMeasure GetTimeAdvPreSetVertexShader() { return this->timeAdvPreSetVertexShader; }
                virtual const TimeMeasure GetTimeAdvPostSetVertexShader() { return this->timeAdvPostSetVertexShader; }
                virtual const TimeMeasure GetTimeAdvPreSetPixelShader() { return this->timeAdvPreSetPixelShader; }
                virtual const TimeMeasure GetTimeAdvPostSetPixelShader() { return this->timeAdvPostSetPixelShader; }
                virtual const TimeMeasure GetTimeAdvPreSetRenderTarget() { return this->timeAdvPreSetRenderTarget; }
                virtual const TimeMeasure GetTimeAdvPostSetRenderTarget() { return this->timeAdvPostSetRenderTarget; }
                virtual const TimeMeasure GetTimeAdvPreSetRenderState() { return this->timeAdvPreSetRenderState; }
                virtual const TimeMeasure GetTimeAdvPostSetRenderState() { return this->timeAdvPostSetRenderState; }
                virtual const TimeMeasure GetTimeAdvPreDrawIndexedPrimitive() { return this->timeAdvPreDrawIndexedPrimitive; }
                virtual const TimeMeasure GetTimeAdvPostDrawIndexedPrimitive() { return this->timeAdvPostDrawIndexedPrimitive; }

                virtual const TimeMeasure GetTimeWndProc() { return this->timeWndProc; }

            protected:
                virtual void ChangeState(AddonState state) { this->state = state; }

                TimeMeasure timeOverall;
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

                TimeMeasure timeWndProc;

                HMODULE addonHandle = nullptr;

            private:
                AddonState state = AddonState::UnloadedState;
                std::weak_ptr<Addon> addon;
            };

        }
    }
}
