#pragma once
#include "../windows.h"
#include <memory>
#include <vector>
#include "Addon.h"

namespace loader {
    namespace addons {

        extern std::vector<std::shared_ptr<Addon>> AddonsList;

        // These counters are mostly here for increasing debug performance.
        // Every hook is called many, many times. If all the addons were iterated for every hook call,
        // there's a lot of unnecessary jumps that decreases performance significantly. Especially in the debug build.
        struct AddonHookCounts {
            int DrawFrameBeforePostProcessing = 0;
            int DrawFrameBeforeGui = 0;
            int DrawFrame = 0;

            int AdvPreBeginScene = 0;
            int AdvPostBeginScene = 0;
            int AdvPreEndScene = 0;
            int AdvPostEndScene = 0;
            int AdvPreClear = 0;
            int AdvPostClear = 0;
            int AdvPreReset = 0;
            int AdvPostReset = 0;
            int AdvPrePresent = 0;
            int AdvPostPresent = 0;
            int AdvPreCreateTexture = 0;
            int AdvPostCreateTexture = 0;
            int AdvPreCreateVertexShader = 0;
            int AdvPostCreateVertexShader = 0;
            int AdvPreCreatePixelShader = 0;
            int AdvPostCreatePixelShader = 0;
            int AdvPreCreateRenderTarget = 0;
            int AdvPostCreateRenderTarget = 0;
            int AdvPreSetTexture = 0;
            int AdvPostSetTexture = 0;
            int AdvPreSetVertexShader = 0;
            int AdvPostSetVertexShader = 0;
            int AdvPreSetPixelShader = 0;
            int AdvPostSetPixelShader = 0;
            int AdvPreSetRenderTarget = 0;
            int AdvPostSetRenderTarget = 0;
            int AdvPreSetRenderState = 0;
            int AdvPostSetRenderState = 0;
            int AdvPreDrawIndexedPrimitive = 0;
            int AdvPostDrawIndexedPrimitive = 0;
        };
        extern AddonHookCounts ActiveAddonHookCounts;

        void RefreshAddonList();

        void InitializeAddons(UINT sdkVersion, IDirect3D9* d3d9);
        void UninitializeAddons();
        void LoadAddons(HWND hFocusWindow);
        void UnloadAddons();

        void OnStartFrame(IDirect3DDevice9* device);
        void OnEndFrame(IDirect3DDevice9* device);

        void DrawFrameBeforePostProcessing(IDirect3DDevice9* device);
        void DrawFrameBeforeGui(IDirect3DDevice9* device);
        void DrawFrame(IDirect3DDevice9* device);

        void AdvPreBeginScene(IDirect3DDevice9* device);
        void AdvPostBeginScene(IDirect3DDevice9* device);
        void AdvPreEndScene(IDirect3DDevice9* device);
        void AdvPostEndScene(IDirect3DDevice9* device);
        void AdvPreClear(IDirect3DDevice9* device, DWORD Count, CONST D3DRECT* pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil);
        void AdvPostClear(IDirect3DDevice9* device, DWORD Count, CONST D3DRECT* pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil);
        void AdvPreReset(IDirect3DDevice9* device, D3DPRESENT_PARAMETERS* pPresentationParameters);
        void AdvPostReset(IDirect3DDevice9* device, D3DPRESENT_PARAMETERS* pPresentationParameters);
        void AdvPrePresent(IDirect3DDevice9* device, CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion);
        void AdvPostPresent(IDirect3DDevice9* device, CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion);
        HRESULT AdvPreCreateTexture(IDirect3DDevice9* device, UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DTexture9** ppTexture, HANDLE* pSharedHandle);
        void AdvPostCreateTexture(IDirect3DDevice9* device, IDirect3DTexture9* pTexture, UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, HANDLE* pSharedHandle);
        HRESULT AdvPreCreateVertexShader(IDirect3DDevice9* device, CONST DWORD* pFunction, IDirect3DVertexShader9** ppShader);
        void AdvPostCreateVertexShader(IDirect3DDevice9* device, IDirect3DVertexShader9* ppShader, CONST DWORD* pFunction);
        HRESULT AdvPreCreatePixelShader(IDirect3DDevice9* device, CONST DWORD* pFunction, IDirect3DPixelShader9** ppShader);
        void AdvPostCreatePixelShader(IDirect3DDevice9* device, IDirect3DPixelShader9* ppShader, CONST DWORD* pFunction);
        HRESULT AdvPreCreateRenderTarget(IDirect3DDevice9* device, UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Lockable, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle);
        void AdvPostCreateRenderTarget(IDirect3DDevice9* device, IDirect3DSurface9* ppSurface, UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Lockable, HANDLE* pSharedHandle);
        void AdvPreSetTexture(IDirect3DDevice9* device, DWORD Stage, IDirect3DBaseTexture9* pTexture);
        void AdvPostSetTexture(IDirect3DDevice9* device, DWORD Stage, IDirect3DBaseTexture9* pTexture);
        void AdvPreSetVertexShader(IDirect3DDevice9* device, IDirect3DVertexShader9* pShader);
        void AdvPostSetVertexShader(IDirect3DDevice9* device, IDirect3DVertexShader9* pShader);
        void AdvPreSetPixelShader(IDirect3DDevice9* device, IDirect3DPixelShader9* pShader);
        void AdvPostSetPixelShader(IDirect3DDevice9* device, IDirect3DPixelShader9* pShader);
        void AdvPreSetRenderTarget(IDirect3DDevice9* device, DWORD RenderTargetIndex, IDirect3DSurface9* pRenderTarget);
        void AdvPostSetRenderTarget(IDirect3DDevice9* device, DWORD RenderTargetIndex, IDirect3DSurface9* pRenderTarget);
        void AdvPreSetRenderState(IDirect3DDevice9* device, D3DRENDERSTATETYPE State, DWORD Value);
        void AdvPostSetRenderState(IDirect3DDevice9* device, D3DRENDERSTATETYPE State, DWORD Value);
        void AdvPreDrawIndexedPrimitive(IDirect3DDevice9* device, D3DPRIMITIVETYPE PrimitiveType, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount);
        void AdvPostDrawIndexedPrimitive(IDirect3DDevice9* device, D3DPRIMITIVETYPE PrimitiveType, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount);
        
    }
}
