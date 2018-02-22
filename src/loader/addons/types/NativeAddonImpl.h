#pragma once
#include <functional>
#include <set>
#include <string>
#include <gw2addon-native.h>
#include "ITypeImpl.h"
#include "../../d3d9types.h"

namespace loader {
    namespace addons {
        namespace types {

            class NativeAddonImpl : public ITypeImpl {
            public:
                NativeAddonImpl(const std::wstring& filePath);

                virtual bool SupportsHotLoading() const override { return true; };
                virtual void Initialize() override;
                virtual void Uninitialize() override;
                virtual void Load() override;
                virtual void Unload() override;

                virtual bool HandleWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) override;

                virtual void DrawFrameBeforeGui(IDirect3DDevice9* device) override;
                virtual void DrawFrameBeforePostProcessing(IDirect3DDevice9* device) override;
                virtual void DrawFrame(IDirect3DDevice9* device) override;

                virtual void AdvPreBeginScene(IDirect3DDevice9* device) override;
                virtual void AdvPostBeginScene(IDirect3DDevice9* device) override;
                virtual void AdvPreEndScene(IDirect3DDevice9* device) override;
                virtual void AdvPostEndScene(IDirect3DDevice9* device) override;
                virtual void AdvPreClear(IDirect3DDevice9* device, DWORD Count, CONST D3DRECT* pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil) override;
                virtual void AdvPostClear(IDirect3DDevice9* device, DWORD Count, CONST D3DRECT* pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil) override;
                virtual void AdvPreReset(IDirect3DDevice9* device, D3DPRESENT_PARAMETERS* pPresentationParameters) override;
                virtual void AdvPostReset(IDirect3DDevice9* device, D3DPRESENT_PARAMETERS* pPresentationParameters) override;
                virtual void AdvPrePresent(IDirect3DDevice9* device, CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion) override;
                virtual void AdvPostPresent(IDirect3DDevice9* device, CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion) override;
                virtual HRESULT AdvPreCreateTexture(IDirect3DDevice9* device, UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DTexture9** ppTexture, HANDLE* pSharedHandle) override;
                virtual void AdvPostCreateTexture(IDirect3DDevice9* device, IDirect3DTexture9* pTexture, UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, HANDLE* pSharedHandle) override;
                virtual HRESULT AdvPreCreateVertexShader(IDirect3DDevice9* device, CONST DWORD* pFunction, IDirect3DVertexShader9** ppShader) override;
                virtual void AdvPostCreateVertexShader(IDirect3DDevice9* device, IDirect3DVertexShader9* pShader, CONST DWORD* pFunction) override;
                virtual HRESULT AdvPreCreatePixelShader(IDirect3DDevice9* device, CONST DWORD* pFunction, IDirect3DPixelShader9** ppShader) override;
                virtual void AdvPostCreatePixelShader(IDirect3DDevice9* device, IDirect3DPixelShader9* pShader, CONST DWORD* pFunction) override;
                virtual HRESULT AdvPreCreateRenderTarget(IDirect3DDevice9* device, UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Lockable, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle) override;
                virtual void AdvPostCreateRenderTarget(IDirect3DDevice9* device, IDirect3DSurface9* pSurface, UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Lockable, HANDLE* pSharedHandle) override;
                virtual void AdvPreSetTexture(IDirect3DDevice9* device, DWORD Stage, IDirect3DBaseTexture9* pTexture) override;
                virtual void AdvPostSetTexture(IDirect3DDevice9* device, DWORD Stage, IDirect3DBaseTexture9* pTexture) override;
                virtual void AdvPreSetVertexShader(IDirect3DDevice9* device, IDirect3DVertexShader9* pShader) override;
                virtual void AdvPostSetVertexShader(IDirect3DDevice9* device, IDirect3DVertexShader9* pShader) override;
                virtual void AdvPreSetPixelShader(IDirect3DDevice9* device, IDirect3DPixelShader9* pShader) override;
                virtual void AdvPostSetPixelShader(IDirect3DDevice9* device, IDirect3DPixelShader9* pShader) override;
                virtual void AdvPreSetRenderTarget(IDirect3DDevice9* device, DWORD RenderTargetIndex, IDirect3DSurface9* pRenderTarget) override;
                virtual void AdvPostSetRenderTarget(IDirect3DDevice9* device, DWORD RenderTargetIndex, IDirect3DSurface9* pRenderTarget) override;
                virtual void AdvPreSetRenderState(IDirect3DDevice9* device, D3DRENDERSTATETYPE State, DWORD Value) override;
                virtual void AdvPostSetRenderState(IDirect3DDevice9* device, D3DRENDERSTATETYPE State, DWORD Value) override;
                virtual void AdvPreDrawIndexedPrimitive(IDirect3DDevice9* device, D3DPRIMITIVETYPE PrimitiveType, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount) override;
                virtual void AdvPostDrawIndexedPrimitive(IDirect3DDevice9* device, D3DPRIMITIVETYPE PrimitiveType, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount) override;

            private:
                std::wstring filePath;
                std::wstring fileName;

                GW2Load GW2_Load = nullptr;
                GW2Unload GW2_Unload = nullptr;
                GW2DrawFrameBeforeGui GW2_DrawFrameBeforeGui = nullptr;
                GW2DrawFrameBeforePostProcessing GW2_DrawFrameBeforePostProcessing = nullptr;
                GW2DrawFrame GW2_DrawFrame = nullptr;
                GW2HandleWndProc GW2_HandleWndProc = nullptr;

                GW2AdvPreBeginScene GW2_AdvPreBeginScene = nullptr;
                GW2AdvPostBeginScene GW2_AdvPostBeginScene = nullptr;
                GW2AdvPreEndScene GW2_AdvPreEndScene = nullptr;
                GW2AdvPostEndScene GW2_AdvPostEndScene = nullptr;
                GW2AdvPreClear GW2_AdvPreClear = nullptr;
                GW2AdvPostClear GW2_AdvPostClear = nullptr;
                GW2AdvPreReset GW2_AdvPreReset = nullptr;
                GW2AdvPostReset GW2_AdvPostReset = nullptr;
                GW2AdvPrePresent GW2_AdvPrePresent = nullptr;
                GW2AdvPostPresent GW2_AdvPostPresent = nullptr;
                GW2AdvPreCreateTexture GW2_AdvPreCreateTexture = nullptr;
                GW2AdvPostCreateTexture GW2_AdvPostCreateTexture = nullptr;
                GW2AdvPreCreateVertexShader GW2_AdvPreCreateVertexShader = nullptr;
                GW2AdvPostCreateVertexShader GW2_AdvPostCreateVertexShader = nullptr;
                GW2AdvPreCreatePixelShader GW2_AdvPreCreatePixelShader = nullptr;
                GW2AdvPostCreatePixelShader GW2_AdvPostCreatePixelShader = nullptr;
                GW2AdvPreCreateRenderTarget GW2_AdvPreCreateRenderTarget = nullptr;
                GW2AdvPostCreateRenderTarget GW2_AdvPostCreateRenderTarget = nullptr;
                GW2AdvPreSetTexture GW2_AdvPreSetTexture = nullptr;
                GW2AdvPostSetTexture GW2_AdvPostSetTexture = nullptr;
                GW2AdvPreSetVertexShader GW2_AdvPreSetVertexShader = nullptr;
                GW2AdvPostSetVertexShader GW2_AdvPostSetVertexShader = nullptr;
                GW2AdvPreSetPixelShader GW2_AdvPreSetPixelShader = nullptr;
                GW2AdvPostSetPixelShader GW2_AdvPostSetPixelShader = nullptr;
                GW2AdvPreSetRenderTarget GW2_AdvPreSetRenderTarget = nullptr;
                GW2AdvPostSetRenderTarget GW2_AdvPostSetRenderTarget = nullptr;
                GW2AdvPreSetRenderState GW2_AdvPreSetRenderState = nullptr;
                GW2AdvPostSetRenderState GW2_AdvPostSetRenderState = nullptr;
                GW2AdvPreDrawIndexedPrimitive GW2_AdvPreDrawIndexedPrimitive = nullptr;
                GW2AdvPostDrawIndexedPrimitive GW2_AdvPostDrawIndexedPrimitive = nullptr;

                void callDrawFunc(const std::function<void(void)>& func);
                void callAdvFunc(const std::string& funcName, const std::function<void(void)>& func);
                HRESULT callAdvFuncWithResult(const std::string& funcName, const std::function<HRESULT(void)>& func);
            };

        }
    }
}
