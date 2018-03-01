#pragma once
#include <functional>
#include <gw2addon-native.h>
#include "ITypeImpl.h"

namespace loader {
    namespace addons {
        namespace types {

            class NativeAddonImpl : public ITypeImpl {
            public:
                NativeAddonImpl(const std::string& filePath);

                virtual const std::string GetID() const override { return this->id; }
                virtual const std::string GetName() const override { return this->name; }
                virtual const std::string GetAuthor() const override { return this->author; }
                virtual const std::string GetDescription() const override { return this->description; }
                virtual const std::string GetVersion() const override { return this->version; }
                virtual const std::string GetHomepage() const override { return this->homepage; }
                virtual IDirect3DTexture9* GetIcon() const override { return this->icon; }

                virtual bool SupportsHotLoading() const override { return true; };
                virtual bool SupportsSettings() const override { return this->AddonOpenSettings != nullptr; }

                virtual void Initialize() override;
                virtual void Uninitialize() override;
                virtual void Load() override;
                virtual void Unload() override;

                virtual void OnStartFrame(IDirect3DDevice9* device) override;
                virtual void OnEndFrame(IDirect3DDevice9* device) override;

                virtual void DrawFrameBeforeGui(IDirect3DDevice9* device) override;
                virtual void DrawFrameBeforePostProcessing(IDirect3DDevice9* device) override;
                virtual void DrawFrame(IDirect3DDevice9* device) override;

                virtual bool HandleWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) override;

                virtual void OpenSettings() override;

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
                std::string filePath = "";
                std::string fileName = "";
                std::string id = "";
                std::string name = "";
                std::string author = "";
                std::string description = "";
                std::string version = "";
                std::string homepage = "";
                IDirect3DTexture9* icon = nullptr;
                bool iconManaged = false;

                GW2AddonInitialize_t AddonInitialize = nullptr;
                GW2AddonRelease_t AddonRelease = nullptr;

                GW2AddonLoad_t AddonLoad = nullptr;
                GW2AddonDrawFrameBeforeGui_t AddonDrawFrameBeforeGui = nullptr;
                GW2AddonDrawFrameBeforePostProcessing_t AddonDrawFrameBeforePostProcessing = nullptr;
                GW2AddonDrawFrame_t AddonDrawFrame = nullptr;
                GW2AddonHandleWndProc_t AddonHandleWndProc = nullptr;
                GW2AddonOpenSettings_t AddonOpenSettings = nullptr;

                GW2AddonAdvPreBeginScene_t AddonAdvPreBeginScene = nullptr;
                GW2AddonAdvPostBeginScene_t AddonAdvPostBeginScene = nullptr;
                GW2AddonAdvPreEndScene_t AddonAdvPreEndScene = nullptr;
                GW2AddonAdvPostEndScene_t AddonAdvPostEndScene = nullptr;
                GW2AddonAdvPreClear_t AddonAdvPreClear = nullptr;
                GW2AddonAdvPostClear_t AddonAdvPostClear = nullptr;
                GW2AddonAdvPreReset_t AddonAdvPreReset = nullptr;
                GW2AddonAdvPostReset_t AddonAdvPostReset = nullptr;
                GW2AddonAdvPrePresent_t AddonAdvPrePresent = nullptr;
                GW2AddonAdvPostPresent_t AddonAdvPostPresent = nullptr;
                GW2AddonAdvPreCreateTexture_t AddonAdvPreCreateTexture = nullptr;
                GW2AddonAdvPostCreateTexture_t AddonAdvPostCreateTexture = nullptr;
                GW2AddonAdvPreCreateVertexShader_t AddonAdvPreCreateVertexShader = nullptr;
                GW2AddonAdvPostCreateVertexShader_t AddonAdvPostCreateVertexShader = nullptr;
                GW2AddonAdvPreCreatePixelShader_t AddonAdvPreCreatePixelShader = nullptr;
                GW2AddonAdvPostCreatePixelShader_t AddonAdvPostCreatePixelShader = nullptr;
                GW2AddonAdvPreCreateRenderTarget_t AddonAdvPreCreateRenderTarget = nullptr;
                GW2AddonAdvPostCreateRenderTarget_t AddonAdvPostCreateRenderTarget = nullptr;
                GW2AddonAdvPreSetTexture_t AddonAdvPreSetTexture = nullptr;
                GW2AddonAdvPostSetTexture_t AddonAdvPostSetTexture = nullptr;
                GW2AddonAdvPreSetVertexShader_t AddonAdvPreSetVertexShader = nullptr;
                GW2AddonAdvPostSetVertexShader_t AddonAdvPostSetVertexShader = nullptr;
                GW2AddonAdvPreSetPixelShader_t AddonAdvPreSetPixelShader = nullptr;
                GW2AddonAdvPostSetPixelShader_t AddonAdvPostSetPixelShader = nullptr;
                GW2AddonAdvPreSetRenderTarget_t AddonAdvPreSetRenderTarget = nullptr;
                GW2AddonAdvPostSetRenderTarget_t AddonAdvPostSetRenderTarget = nullptr;
                GW2AddonAdvPreSetRenderState_t AddonAdvPreSetRenderState = nullptr;
                GW2AddonAdvPostSetRenderState_t AddonAdvPostSetRenderState = nullptr;
                GW2AddonAdvPreDrawIndexedPrimitive_t AddonAdvPreDrawIndexedPrimitive = nullptr;
                GW2AddonAdvPostDrawIndexedPrimitive_t AddonAdvPostDrawIndexedPrimitive = nullptr;

                void callDrawFunc(const std::function<void(void)>& func);
                void callAdvFunc(const std::string& funcName, const std::function<void(void)>& func);
                HRESULT callAdvFuncWithResult(const std::string& funcName, const std::function<HRESULT(void)>& func);
            };

        }
    }
}
