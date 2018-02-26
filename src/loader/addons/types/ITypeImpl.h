#pragma once
#include "../../windows.h"
#include <d3d9.h>
#include <chrono>
#include <memory>
#include <string>
#include <vector>

namespace loader {
    namespace addons {

        // Forward declare Addon, because we use weak_ptr
        class Addon;

        namespace types {

            enum AddonSubType {
                NoSubType,
                GraphicsAddon,
                NonGraphicsAddon,
                PointerReplacingAddon,
                VirtualTableReplacingAddon,
                WrapperAddon
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

            class ITypeImpl {
            public:
                ITypeImpl();

                std::weak_ptr<Addon> GetAddon() const { return this->addon; }
                void SetAddon(std::weak_ptr<Addon> addon) { this->addon = addon; }

                virtual HMODULE GetHandle() const { return this->handle; }

                virtual const AddonSubType GetAddonSubType() const { return this->subType; }
                virtual const std::wstring GetAddonSubTypeString() const;

                virtual const AddonState GetAddonState() const { return this->state; }
                virtual const std::wstring GetAddonStateString() const;

                virtual bool SupportsHotLoading() const = 0;
                virtual void Initialize() = 0;
                virtual void Uninitialize() = 0;
                virtual void Load() = 0;
                virtual void Unload() = 0;

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

                virtual const std::vector<float> GetDurationHistoryDrawFrameBeforeGui() { return this->durationHistoryDrawFrameBeforeGui; }
                virtual const std::vector<float> GetDurationHistoryDrawFrameBeforePostProcessing() { return this->durationHistoryDrawFrameBeforePostProcessing; }
                virtual const std::vector<float> GetDurationHistoryDrawFrame() { return this->durationHistoryDrawFrame; }

            protected:
                virtual void ChangeSubType(AddonSubType type) { this->subType = type; }
                virtual void ChangeState(AddonState state) { this->state = state; }

                std::vector<float> durationHistoryDrawFrameBeforeGui;
                std::vector<float> durationHistoryDrawFrameBeforePostProcessing;
                std::vector<float> durationHistoryDrawFrame;

                void AddDurationHistory(std::vector<float>* durationHistory, float value);

                std::chrono::steady_clock::time_point timeMeasureStart;
                void StartTimeMeasure();
                float EndTimeMeasure();

                HMODULE handle = nullptr;

            private:
                AddonSubType subType;
                AddonState state = AddonState::UnloadedState;
                std::weak_ptr<Addon> addon;
            };

        }
    }
}
