#pragma once
#include "../stdafx.h"
#include "Addon.h"
#include "AddonsList.h"
#include "../hooks/LoaderDirect3DDevice9.h"

namespace loader::addons {

    extern AddonsList Addons;

    // These are pointers to the add-ons with enabled hooks.
    // With this we can improve the performance because these addon functions are called repeatedly every frame.
    // Especially in the debug build this can cause major performance issues if we would just iterate naively.
    struct AddonHooks {
        std::vector<Addon*> HandleWndProc;
        std::vector<Addon*> DrawFrameBeforePostProcessing;
        std::vector<Addon*> DrawFrameBeforeGui;
        std::vector<Addon*> DrawFrame;
        std::vector<Addon*> ApiKeyChange;
        std::vector<Addon*> AdvPreBeginScene;
        std::vector<Addon*> AdvPostBeginScene;
        std::vector<Addon*> AdvPreEndScene;
        std::vector<Addon*> AdvPostEndScene;
        std::vector<Addon*> AdvPreClear;
        std::vector<Addon*> AdvPostClear;
        std::vector<Addon*> AdvPreReset;
        std::vector<Addon*> AdvPostReset;
        std::vector<Addon*> AdvPrePresent;
        std::vector<Addon*> AdvPostPresent;
        std::vector<Addon*> AdvPreCreateTexture;
        std::vector<Addon*> AdvPostCreateTexture;
        std::vector<Addon*> AdvPreCreateVertexShader;
        std::vector<Addon*> AdvPostCreateVertexShader;
        std::vector<Addon*> AdvPreCreatePixelShader;
        std::vector<Addon*> AdvPostCreatePixelShader;
        std::vector<Addon*> AdvPreCreateRenderTarget;
        std::vector<Addon*> AdvPostCreateRenderTarget;
        std::vector<Addon*> AdvPreSetTexture;
        std::vector<Addon*> AdvPostSetTexture;
        std::vector<Addon*> AdvPreSetVertexShader;
        std::vector<Addon*> AdvPostSetVertexShader;
        std::vector<Addon*> AdvPreSetPixelShader;
        std::vector<Addon*> AdvPostSetPixelShader;
        std::vector<Addon*> AdvPreSetRenderTarget;
        std::vector<Addon*> AdvPostSetRenderTarget;
        std::vector<Addon*> AdvPreSetRenderState;
        std::vector<Addon*> AdvPostSetRenderState;
        std::vector<Addon*> AdvPreDrawIndexedPrimitive;
        std::vector<Addon*> AdvPostDrawIndexedPrimitive;
    };
    extern AddonHooks ActiveAddonHooks;

    void RefreshAddonList();

    void SwapAddonOrder(const Addon* const a, const Addon* const b);
    void SaveAddonOrder();
    void ReorderAddonHooks();
    void ResetLegacyAddonChain();

    void InitializeAddons(UINT sdkVersion, hooks::LoaderDirect3D9* d3d9, hooks::LoaderDirect3DDevice9* device);
    void UninitializeAddons();
    void LoadAddons(HWND hFocusWindow);
    void UnloadAddons();

    void OnStartFrame(IDirect3DDevice9* device);
    void OnEndFrame(IDirect3DDevice9* device);

    bool HandleWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    void DrawFrameBeforePostProcessing(IDirect3DDevice9* device);
    void DrawFrameBeforeGui(IDirect3DDevice9* device);
    void DrawFrame(IDirect3DDevice9* device);
    void ApiKeyChange(const char* key);
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
