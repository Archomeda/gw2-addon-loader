#include "LoaderDirect3DDevice9.h"
#include "ChainHook.h"
#include "MumbleLink.h"
#include "../Config.h"
#include "../log.h"
#include "../addons/addons_manager.h"
#include "../disasm/d3d9Vtbl.h"

using namespace std;
using namespace std::experimental::filesystem;

namespace loader::hooks {

    PreReset_t* PreResetHook = nullptr;
    PostReset_t* PostResetHook = nullptr;
    PrePresent_t* PrePresentHook = nullptr;


    D3DFORMAT stateFormat = D3DFMT_UNKNOWN;
    DWORD stateColorWriteEnable = 0;
    DWORD stateZEnable = 0;
    DWORD stateZFunc = 0;

    int PrePresentGuiDone = 0;
    int PrePostProcessingDone = 0;

    chrono::steady_clock::time_point d3d9ProcessingStart;
    vector<float> DurationHistoryD3D9Processing;
    vector<float> DurationHistoryLoaderDrawFrame;

    IDirect3DDevice9* LegacyAddonChainDevice = nullptr;

    ChainHook currentChainHook;


    HRESULT LoaderDirect3DDevice9::QueryInterface(REFIID riid, void** ppvObj) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->QueryInterface(riid, ppvObj) :
            this->dev->QueryInterface(riid, ppvObj);
    }

    ULONG LoaderDirect3DDevice9::AddRef() {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->AddRef() :
            this->dev->AddRef();
    }

    ULONG LoaderDirect3DDevice9::Release() {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->Release() :
            this->dev->Release();
    }

    HRESULT LoaderDirect3DDevice9::TestCooperativeLevel() {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->TestCooperativeLevel() :
            this->dev->TestCooperativeLevel();
    }

    UINT LoaderDirect3DDevice9::GetAvailableTextureMem() {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->GetAvailableTextureMem() :
            this->dev->GetAvailableTextureMem();
    }

    HRESULT LoaderDirect3DDevice9::EvictManagedResources() {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->EvictManagedResources() :
            this->dev->EvictManagedResources();
    }

    HRESULT LoaderDirect3DDevice9::GetDirect3D(IDirect3D9** ppD3D9) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->GetDirect3D(ppD3D9) :
            this->dev->GetDirect3D(ppD3D9);
    }

    HRESULT LoaderDirect3DDevice9::GetDeviceCaps(D3DCAPS9* pCaps) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->GetDeviceCaps(pCaps) :
            this->dev->GetDeviceCaps(pCaps);
    }

    HRESULT LoaderDirect3DDevice9::GetDisplayMode(UINT iSwapChain, D3DDISPLAYMODE* pMode) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->GetDisplayMode(iSwapChain, pMode) :
            this->dev->GetDisplayMode(iSwapChain, pMode);
    }

    HRESULT LoaderDirect3DDevice9::GetCreationParameters(D3DDEVICE_CREATION_PARAMETERS *pParameters) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->GetCreationParameters(pParameters) :
            this->dev->GetCreationParameters(pParameters);
    }

    HRESULT LoaderDirect3DDevice9::SetCursorProperties(UINT XHotSpot, UINT YHotSpot, IDirect3DSurface9* pCursorBitmap) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->SetCursorProperties(XHotSpot, YHotSpot, pCursorBitmap) :
            this->dev->SetCursorProperties(XHotSpot, YHotSpot, pCursorBitmap);
    }

    void LoaderDirect3DDevice9::SetCursorPosition(int X, int Y, DWORD Flags) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->SetCursorPosition(X, Y, Flags) :
            this->dev->SetCursorPosition(X, Y, Flags);
    }

    BOOL LoaderDirect3DDevice9::ShowCursor(BOOL bShow) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->ShowCursor(bShow) :
            this->dev->ShowCursor(bShow);
    }

    HRESULT LoaderDirect3DDevice9::CreateAdditionalSwapChain(D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DSwapChain9** pSwapChain) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->CreateAdditionalSwapChain(pPresentationParameters, pSwapChain) :
            this->dev->CreateAdditionalSwapChain(pPresentationParameters, pSwapChain);
    }

    HRESULT LoaderDirect3DDevice9::GetSwapChain(UINT iSwapChain, IDirect3DSwapChain9** pSwapChain) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->GetSwapChain(iSwapChain, pSwapChain) :
            this->dev->GetSwapChain(iSwapChain, pSwapChain);
    }

    UINT LoaderDirect3DDevice9::GetNumberOfSwapChains() {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->GetNumberOfSwapChains() :
            this->dev->GetNumberOfSwapChains();
    }

    HRESULT LoaderDirect3DDevice9::Reset(D3DPRESENT_PARAMETERS* pPresentationParameters) {
        PreResetHook(this->dev, pPresentationParameters);
        addons::AdvPreReset(this->dev, pPresentationParameters);

        HRESULT hr = LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->Reset(pPresentationParameters) :
            this->dev->Reset(pPresentationParameters);
        if (hr != D3D_OK) {
            // Fail
            return hr;
        }

        PostResetHook(this->dev, pPresentationParameters);
        addons::AdvPostReset(this->dev, pPresentationParameters);

        return hr;
    }

    bool isPresentingAddonLoader = false;
    HRESULT WINAPI PresentAddonLoader(IDirect3DDevice9* dev, IDirect3DDevice9* chainDev, CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion) {
        if (isPresentingAddonLoader) {
            return dev->Present(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
        }
        isPresentingAddonLoader = true;

        bool trackStats = AppConfig.GetShowDebugFeatures();

        dev->BeginScene();
        addons::DrawFrame(dev);

        if (trackStats) {
            auto presentStart = chrono::steady_clock::now();
            PrePresentHook(dev, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
            if (DurationHistoryLoaderDrawFrame.size() < 4 * 60) {
                DurationHistoryLoaderDrawFrame.resize(4 * 60);
                DurationHistoryLoaderDrawFrame.reserve(8 * 60);
            }
            if (DurationHistoryLoaderDrawFrame.size() == 4 * 60) {
                DurationHistoryLoaderDrawFrame.erase(DurationHistoryLoaderDrawFrame.begin());
            }
            DurationHistoryLoaderDrawFrame.push_back(((chrono::steady_clock::now() - presentStart).count() / 10000) / 100.0f);
        }
        else {
            PrePresentHook(dev, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
        }

        dev->EndScene();

        addons::AdvPrePresent(dev, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);

        HRESULT hr = chainDev != nullptr ?
            chainDev->Present(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion) :
            dev->Present(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
        if (hr != D3D_OK) {
            // Fail
            return hr;
        }

        addons::AdvPostPresent(dev, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);

        isPresentingAddonLoader = false;
        return hr;
    }

    HRESULT WINAPI PresentAddonLoader(IDirect3DDevice9* dev, CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion) {
        return PresentAddonLoader(dev, nullptr, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
    }

    HRESULT LoaderDirect3DDevice9::Present(CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion) {
        bool trackStats = AppConfig.GetShowDebugFeatures();
        bool obsCompatibilityMode = AppConfig.GetOBSCompatibilityMode();

        if (trackStats) {
            if (DurationHistoryD3D9Processing.size() < 4 * 60) {
                DurationHistoryD3D9Processing.resize(4 * 60);
                DurationHistoryD3D9Processing.reserve(8 * 60);
            }
            if (DurationHistoryD3D9Processing.size() == 4 * 60) {
                DurationHistoryD3D9Processing.erase(DurationHistoryD3D9Processing.begin());
            }
            DurationHistoryD3D9Processing.push_back(((chrono::steady_clock::now() - d3d9ProcessingStart).count() / 100000) / 10.0f);
        }

        // Before we start, determine the hook chain and see if we need to adapt
        D3DDevice9Vtbl vtbl = GetD3DDevice9Vtbl(this->dev);
        ChainHook newHook = ChainHook::FindCurrentChainHook(ChainHookFunctionType::PresentFunction, vtbl.Present);
        if (newHook.GetType() != currentChainHook.GetType()) {
            // New chain hook type
            HOOKS_LOG()->info("New chain hook type detected: {0}", newHook.GetTypeString());
            HOOKS_LOG()->info("Originating function at 0x{0:X}", reinterpret_cast<size_t>(newHook.GetOriginatingFunction()));
            HOOKS_LOG()->info("Chain function at 0x{0:X}", reinterpret_cast<size_t>(newHook.GetChainFunction()));
            currentChainHook = newHook;
        }

        ChainHookType chainHookType = currentChainHook.GetType();

        // Hook the hook chain
        if (chainHookType != ChainHookType::NoHookType) {
            currentChainHook.HookCallback(static_cast<HRESULT(WINAPI*)(IDirect3DDevice9*, CONST RECT*, CONST RECT*, HWND, CONST RGNDATA*)>(&PresentAddonLoader));
        }

        HRESULT hr;
        if (obsCompatibilityMode && chainHookType == ChainHookType::OBSHook) {
            // We have an OBS chain hook, call Present directly, as we have hooked PresentAddonLoader deeper into the chain
            hr = LegacyAddonChainDevice != nullptr ?
                LegacyAddonChainDevice->Present(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion) :
                this->dev->Present(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
        }
        else {
            // No chain hook, proceed as normal
            hr = PresentAddonLoader(this->dev, LegacyAddonChainDevice, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
        }

        // Unhook the hook chain again, to prevent issues if we suddenly crash
        if (chainHookType != ChainHookType::NoHookType) {
            currentChainHook.UnhookCallback();
        }

        if (hr != D3D_OK) {
            // Fail
            return hr;
        }

        // Reset this for a new frame
        PrePresentGuiDone = 0;
        PrePostProcessingDone = 0;
        d3d9ProcessingStart = {};
        addons::OnEndFrame(this->dev);

        // Signal MumbleLink
        MumbleLink::GetInstance().SignalFrame();

        return hr;
    }

    HRESULT LoaderDirect3DDevice9::GetBackBuffer(UINT iSwapChain, UINT iBackBuffer, D3DBACKBUFFER_TYPE Type, IDirect3DSurface9** ppBackBuffer) {
        // This is the very first call Guild Wars 2 does upon a new frame
        if (AppConfig.GetShowDebugFeatures()) {
            d3d9ProcessingStart = chrono::steady_clock::now();
        }
        addons::OnStartFrame(this->dev);

        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->GetBackBuffer(iSwapChain, iBackBuffer, Type, ppBackBuffer) :
            this->dev->GetBackBuffer(iSwapChain, iBackBuffer, Type, ppBackBuffer);
    }

    HRESULT LoaderDirect3DDevice9::GetRasterStatus(UINT iSwapChain, D3DRASTER_STATUS* pRasterStatus) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->GetRasterStatus(iSwapChain, pRasterStatus) :
            this->dev->GetRasterStatus(iSwapChain, pRasterStatus);
    }

    HRESULT LoaderDirect3DDevice9::SetDialogBoxMode(BOOL bEnableDialogs) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->SetDialogBoxMode(bEnableDialogs) :
            this->dev->SetDialogBoxMode(bEnableDialogs);
    }

    void LoaderDirect3DDevice9::SetGammaRamp(UINT iSwapChain, DWORD Flags, CONST D3DGAMMARAMP* pRamp) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->SetGammaRamp(iSwapChain, Flags, pRamp) :
            this->dev->SetGammaRamp(iSwapChain, Flags, pRamp);
    }

    void LoaderDirect3DDevice9::GetGammaRamp(UINT iSwapChain, D3DGAMMARAMP* pRamp) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->GetGammaRamp(iSwapChain, pRamp) :
            this->dev->GetGammaRamp(iSwapChain, pRamp);
    }

    HRESULT LoaderDirect3DDevice9::CreateTexture(UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DTexture9** ppTexture, HANDLE* pSharedHandle) {
        IDirect3DTexture9* pOldTexture = *ppTexture;
        HRESULT hr;
        if (!MumbleLink::GetInstance().IsTypeCompetitive()) {
            hr = addons::AdvPreCreateTexture(this->dev, Width, Height, Levels, Usage, Format, Pool, ppTexture, pSharedHandle);
            if (hr != D3D_OK) {
                // Fail
                return hr;
            }
        }

        if (*ppTexture == pOldTexture) {
            hr = LegacyAddonChainDevice != nullptr ?
                LegacyAddonChainDevice->CreateTexture(Width, Height, Levels, Usage, Format, Pool, ppTexture, pSharedHandle) :
                this->dev->CreateTexture(Width, Height, Levels, Usage, Format, Pool, ppTexture, pSharedHandle);
            if (hr != D3D_OK) {
                // Fail
                return hr;
            }
        }

        if (!MumbleLink::GetInstance().IsTypeCompetitive()) {
            addons::AdvPostCreateTexture(this->dev, *ppTexture, Width, Height, Levels, Usage, Format, Pool, pSharedHandle);
        }

        return hr;
    }

    HRESULT LoaderDirect3DDevice9::CreateVolumeTexture(UINT Width, UINT Height, UINT Depth, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DVolumeTexture9** ppVolumeTexture, HANDLE* pSharedHandle) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->CreateVolumeTexture(Width, Height, Depth, Levels, Usage, Format, Pool, ppVolumeTexture, pSharedHandle) :
            this->dev->CreateVolumeTexture(Width, Height, Depth, Levels, Usage, Format, Pool, ppVolumeTexture, pSharedHandle);
    }

    HRESULT LoaderDirect3DDevice9::CreateCubeTexture(UINT EdgeLength, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DCubeTexture9** ppCubeTexture, HANDLE* pSharedHandle) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->CreateCubeTexture(EdgeLength, Levels, Usage, Format, Pool, ppCubeTexture, pSharedHandle) :
            this->dev->CreateCubeTexture(EdgeLength, Levels, Usage, Format, Pool, ppCubeTexture, pSharedHandle);
    }

    HRESULT LoaderDirect3DDevice9::CreateVertexBuffer(UINT Length, DWORD Usage, DWORD FVF, D3DPOOL Pool, IDirect3DVertexBuffer9** ppVertexBuffer, HANDLE* pSharedHandle) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->CreateVertexBuffer(Length, Usage, FVF, Pool, ppVertexBuffer, pSharedHandle) :
            this->dev->CreateVertexBuffer(Length, Usage, FVF, Pool, ppVertexBuffer, pSharedHandle);
    }

    HRESULT LoaderDirect3DDevice9::CreateIndexBuffer(UINT Length, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DIndexBuffer9** ppIndexBuffer, HANDLE* pSharedHandle) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->CreateIndexBuffer(Length, Usage, Format, Pool, ppIndexBuffer, pSharedHandle) :
            this->dev->CreateIndexBuffer(Length, Usage, Format, Pool, ppIndexBuffer, pSharedHandle);
    }

    HRESULT LoaderDirect3DDevice9::CreateRenderTarget(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Lockable, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle) {
        IDirect3DSurface9* pOldSurface = *ppSurface;
        HRESULT hr;
        if (!MumbleLink::GetInstance().IsTypeCompetitive()) {
            hr = addons::AdvPreCreateRenderTarget(this->dev, Width, Height, Format, MultiSample, MultisampleQuality, Lockable, ppSurface, pSharedHandle);
            if (hr != D3D_OK) {
                // Fail
                return hr;
            }
        }

        if (*ppSurface == pOldSurface) {
            hr = LegacyAddonChainDevice != nullptr ?
                LegacyAddonChainDevice->CreateRenderTarget(Width, Height, Format, MultiSample, MultisampleQuality, Lockable, ppSurface, pSharedHandle) :
                this->dev->CreateRenderTarget(Width, Height, Format, MultiSample, MultisampleQuality, Lockable, ppSurface, pSharedHandle);
            if (hr != D3D_OK) {
                // Fail
                return hr;
            }
        }

        if (!MumbleLink::GetInstance().IsTypeCompetitive()) {
            addons::AdvPostCreateRenderTarget(this->dev, *ppSurface, Width, Height, Format, MultiSample, MultisampleQuality, Lockable, pSharedHandle);
        }

        return hr;
    }

    HRESULT LoaderDirect3DDevice9::CreateDepthStencilSurface(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Discard, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->CreateDepthStencilSurface(Width, Height, Format, MultiSample, MultisampleQuality, Discard, ppSurface, pSharedHandle) :
            this->dev->CreateDepthStencilSurface(Width, Height, Format, MultiSample, MultisampleQuality, Discard, ppSurface, pSharedHandle);
    }

    HRESULT LoaderDirect3DDevice9::UpdateSurface(IDirect3DSurface9* pSourceSurface, CONST RECT* pSourceRect, IDirect3DSurface9* pDestinationSurface, CONST POINT* pDestPoint) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->UpdateSurface(pSourceSurface, pSourceRect, pDestinationSurface, pDestPoint) :
            this->dev->UpdateSurface(pSourceSurface, pSourceRect, pDestinationSurface, pDestPoint);
    }

    HRESULT LoaderDirect3DDevice9::UpdateTexture(IDirect3DBaseTexture9* pSourceTexture, IDirect3DBaseTexture9* pDestinationTexture) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->UpdateTexture(pSourceTexture, pDestinationTexture) :
            this->dev->UpdateTexture(pSourceTexture, pDestinationTexture);
    }

    HRESULT LoaderDirect3DDevice9::GetRenderTargetData(IDirect3DSurface9* pRenderTarget, IDirect3DSurface9* pDestSurface) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->GetRenderTargetData(pRenderTarget, pDestSurface) :
            this->dev->GetRenderTargetData(pRenderTarget, pDestSurface);
    }

    HRESULT LoaderDirect3DDevice9::GetFrontBufferData(UINT iSwapChain, IDirect3DSurface9* pDestSurface) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->GetFrontBufferData(iSwapChain, pDestSurface) :
            this->dev->GetFrontBufferData(iSwapChain, pDestSurface);
    }

    HRESULT LoaderDirect3DDevice9::StretchRect(IDirect3DSurface9* pSourceSurface, CONST RECT* pSourceRect, IDirect3DSurface9* pDestSurface, CONST RECT* pDestRect, D3DTEXTUREFILTERTYPE Filter) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->StretchRect(pSourceSurface, pSourceRect, pDestSurface, pDestRect, Filter) :
            this->dev->StretchRect(pSourceSurface, pSourceRect, pDestSurface, pDestRect, Filter);
    }

    HRESULT LoaderDirect3DDevice9::ColorFill(IDirect3DSurface9* pSurface, CONST RECT* pRect, D3DCOLOR color) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->ColorFill(pSurface, pRect, color) :
            this->dev->ColorFill(pSurface, pRect, color);
    }

    HRESULT LoaderDirect3DDevice9::CreateOffscreenPlainSurface(UINT Width, UINT Height, D3DFORMAT Format, D3DPOOL Pool, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->CreateOffscreenPlainSurface(Width, Height, Format, Pool, ppSurface, pSharedHandle) :
            this->dev->CreateOffscreenPlainSurface(Width, Height, Format, Pool, ppSurface, pSharedHandle);
    }

    HRESULT LoaderDirect3DDevice9::SetRenderTarget(DWORD RenderTargetIndex, IDirect3DSurface9* pRenderTarget) {
        if (!MumbleLink::GetInstance().IsTypeCompetitive()) {
            addons::AdvPreSetRenderTarget(this->dev, RenderTargetIndex, pRenderTarget);
        }

        D3DSURFACE_DESC desc;
        if (pRenderTarget->GetDesc(&desc) == D3D_OK) {
            stateFormat = desc.Format;
        }

        if (PrePostProcessingDone == 1) {
            // Guild Wars 2 will now be rendering the world view.
            // We need wait until this is done.
            ++PrePostProcessingDone;
        }
        else if (PrePostProcessingDone == 2) {
            // Guild Wars 2 is done rendering the world view.
            // We can now do our thing.

            // Make sure we keep track that we are doing this before actually doing it, in order to prevent stack overflows
            ++PrePostProcessingDone;

            // Save our current state
            IDirect3DStateBlock9* pStateBlock = NULL;
            this->dev->CreateStateBlock(D3DSBT_ALL, &pStateBlock);

            // Call our addons
            addons::DrawFrameBeforePostProcessing(this->dev);

            // Restore our state
            pStateBlock->Apply();
            pStateBlock->Release();
        }

        HRESULT hr = LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->SetRenderTarget(RenderTargetIndex, pRenderTarget) :
            this->dev->SetRenderTarget(RenderTargetIndex, pRenderTarget);
        if (hr != D3D_OK) {
            // Fail
            return hr;
        }

        if (!MumbleLink::GetInstance().IsTypeCompetitive()) {
            addons::AdvPostSetRenderTarget(this->dev, RenderTargetIndex, pRenderTarget);
        }

        return hr;
    }

    HRESULT LoaderDirect3DDevice9::GetRenderTarget(DWORD RenderTargetIndex, IDirect3DSurface9** ppRenderTarget) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->GetRenderTarget(RenderTargetIndex, ppRenderTarget) :
            this->dev->GetRenderTarget(RenderTargetIndex, ppRenderTarget);
    }

    HRESULT LoaderDirect3DDevice9::SetDepthStencilSurface(IDirect3DSurface9* pNewZStencil) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->SetDepthStencilSurface(pNewZStencil) :
            this->dev->SetDepthStencilSurface(pNewZStencil);
    }

    HRESULT LoaderDirect3DDevice9::GetDepthStencilSurface(IDirect3DSurface9** ppZStencilSurface) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->GetDepthStencilSurface(ppZStencilSurface) :
            this->dev->GetDepthStencilSurface(ppZStencilSurface);
    }

    HRESULT LoaderDirect3DDevice9::BeginScene() {
        addons::AdvPreBeginScene(this->dev);

        HRESULT hr = LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->BeginScene() :
            this->dev->BeginScene();
        if (hr != D3D_OK) {
            // Fail
            return hr;
        }

        addons::AdvPostBeginScene(this->dev);

        return hr;
    }

    HRESULT LoaderDirect3DDevice9::EndScene() {
        addons::AdvPreEndScene(this->dev);

        HRESULT hr = LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->EndScene() :
            this->dev->EndScene();
        if (hr != D3D_OK) {
            // Fail
            return hr;
        }

        addons::AdvPostEndScene(this->dev);

        return hr;
    }

    HRESULT LoaderDirect3DDevice9::Clear(DWORD Count, CONST D3DRECT* pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil) {
        addons::AdvPreClear(this->dev, Count, pRects, Flags, Color, Z, Stencil);

        HRESULT hr = LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->Clear(Count, pRects, Flags, Color, Z, Stencil) :
            this->dev->Clear(Count, pRects, Flags, Color, Z, Stencil);
        if (hr != D3D_OK) {
            // Fail
            return hr;
        }

        addons::AdvPostClear(this->dev, Count, pRects, Flags, Color, Z, Stencil);

        return hr;
    }

    HRESULT LoaderDirect3DDevice9::SetTransform(D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX* pMatrix) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->SetTransform(State, pMatrix) :
            this->dev->SetTransform(State, pMatrix);
    }

    HRESULT LoaderDirect3DDevice9::GetTransform(D3DTRANSFORMSTATETYPE State, D3DMATRIX* pMatrix) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->GetTransform(State, pMatrix) :
            this->dev->GetTransform(State, pMatrix);
    }

    HRESULT LoaderDirect3DDevice9::MultiplyTransform(D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX* pMatrix) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->MultiplyTransform(State, pMatrix) :
            this->dev->MultiplyTransform(State, pMatrix);
    }

    HRESULT LoaderDirect3DDevice9::SetViewport(CONST D3DVIEWPORT9* pViewport) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->SetViewport(pViewport) :
            this->dev->SetViewport(pViewport);
    }

    HRESULT LoaderDirect3DDevice9::GetViewport(D3DVIEWPORT9* pViewport) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->GetViewport(pViewport) :
            this->dev->GetViewport(pViewport);
    }

    HRESULT LoaderDirect3DDevice9::SetMaterial(CONST D3DMATERIAL9* pMaterial) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->SetMaterial(pMaterial) :
            this->dev->SetMaterial(pMaterial);
    }

    HRESULT LoaderDirect3DDevice9::GetMaterial(D3DMATERIAL9* pMaterial) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->GetMaterial(pMaterial) :
            this->dev->GetMaterial(pMaterial);
    }

    HRESULT LoaderDirect3DDevice9::SetLight(DWORD Index, CONST D3DLIGHT9* pLight) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->SetLight(Index, pLight) :
            this->dev->SetLight(Index, pLight);
    }

    HRESULT LoaderDirect3DDevice9::GetLight(DWORD Index, D3DLIGHT9* pLight) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->GetLight(Index, pLight) :
            this->dev->GetLight(Index, pLight);
    }

    HRESULT LoaderDirect3DDevice9::LightEnable(DWORD Index, BOOL Enable) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->LightEnable(Index, Enable) :
            this->dev->LightEnable(Index, Enable);
    }

    HRESULT LoaderDirect3DDevice9::GetLightEnable(DWORD Index, BOOL* pEnable) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->GetLightEnable(Index, pEnable) :
            this->dev->GetLightEnable(Index, pEnable);
    }

    HRESULT LoaderDirect3DDevice9::SetClipPlane(DWORD Index, CONST float* pPlane) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->SetClipPlane(Index, pPlane) :
            this->dev->SetClipPlane(Index, pPlane);
    }

    HRESULT LoaderDirect3DDevice9::GetClipPlane(DWORD Index, float* pPlane) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->GetClipPlane(Index, pPlane) :
            this->dev->GetClipPlane(Index, pPlane);
    }

    HRESULT LoaderDirect3DDevice9::SetRenderState(D3DRENDERSTATETYPE State, DWORD Value) {
        if (!MumbleLink::GetInstance().IsTypeCompetitive()) {
            addons::AdvPreSetRenderState(this->dev, State, Value);
        }

        HRESULT hr = LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->SetRenderState(State, Value) :
            this->dev->SetRenderState(State, Value);
        if (hr != D3D_OK) {
            // Fail
            return hr;
        }

        switch (State) {
        case D3DRS_COLORWRITEENABLE:
            stateColorWriteEnable = Value;
            break;
        case D3DRS_ZENABLE:
            stateZEnable = Value;
            break;
        case D3DRS_ZFUNC:
            stateZFunc = Value;
            break;
        }

        if (!MumbleLink::GetInstance().IsTypeCompetitive()) {
            addons::AdvPostSetRenderState(this->dev, State, Value);
        }

        return hr;
    }

    HRESULT LoaderDirect3DDevice9::GetRenderState(D3DRENDERSTATETYPE State, DWORD* pValue) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->GetRenderState(State, pValue) :
            this->dev->GetRenderState(State, pValue);
    }

    HRESULT LoaderDirect3DDevice9::CreateStateBlock(D3DSTATEBLOCKTYPE Type, IDirect3DStateBlock9** ppSB) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->CreateStateBlock(Type, ppSB) :
            this->dev->CreateStateBlock(Type, ppSB);
    }

    HRESULT LoaderDirect3DDevice9::BeginStateBlock() {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->BeginStateBlock() :
            this->dev->BeginStateBlock();
    }

    HRESULT LoaderDirect3DDevice9::EndStateBlock(IDirect3DStateBlock9** ppSB) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->EndStateBlock(ppSB) :
            this->dev->EndStateBlock(ppSB);
    }

    HRESULT LoaderDirect3DDevice9::SetClipStatus(CONST D3DCLIPSTATUS9* pClipStatus) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->SetClipStatus(pClipStatus) :
            this->dev->SetClipStatus(pClipStatus);
    }

    HRESULT LoaderDirect3DDevice9::GetClipStatus(D3DCLIPSTATUS9* pClipStatus) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->GetClipStatus(pClipStatus) :
            this->dev->GetClipStatus(pClipStatus);
    }

    HRESULT LoaderDirect3DDevice9::GetTexture(DWORD Stage, IDirect3DBaseTexture9** ppTexture) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->GetTexture(Stage, ppTexture) :
            this->dev->GetTexture(Stage, ppTexture);
    }

    HRESULT LoaderDirect3DDevice9::SetTexture(DWORD Stage, IDirect3DBaseTexture9* pTexture) {
        if (!MumbleLink::GetInstance().IsTypeCompetitive()) {
            addons::AdvPreSetTexture(this->dev, Stage, pTexture);
        }

        HRESULT hr = LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->SetTexture(Stage, pTexture) :
            this->dev->SetTexture(Stage, pTexture);
        if (hr != D3D_OK) {
            // Fail
            return hr;
        }

        if (!MumbleLink::GetInstance().IsTypeCompetitive()) {
            addons::AdvPostSetTexture(this->dev, Stage, pTexture);
        }

        return hr;
    }

    HRESULT LoaderDirect3DDevice9::GetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD* pValue) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->GetTextureStageState(Stage, Type, pValue) :
            this->dev->GetTextureStageState(Stage, Type, pValue);
    }

    HRESULT LoaderDirect3DDevice9::SetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->SetTextureStageState(Stage, Type, Value) :
            this->dev->SetTextureStageState(Stage, Type, Value);
    }

    HRESULT LoaderDirect3DDevice9::GetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD* pValue) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->GetSamplerState(Sampler, Type, pValue) :
            this->dev->GetSamplerState(Sampler, Type, pValue);
    }

    HRESULT LoaderDirect3DDevice9::SetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD Value) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->SetSamplerState(Sampler, Type, Value) :
            this->dev->SetSamplerState(Sampler, Type, Value);
    }

    HRESULT LoaderDirect3DDevice9::ValidateDevice(DWORD* pNumPasses) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->ValidateDevice(pNumPasses) :
            this->dev->ValidateDevice(pNumPasses);
    }

    HRESULT LoaderDirect3DDevice9::SetPaletteEntries(UINT PaletteNumber, CONST PALETTEENTRY* pEntries) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->SetPaletteEntries(PaletteNumber, pEntries) :
            this->dev->SetPaletteEntries(PaletteNumber, pEntries);
    }

    HRESULT LoaderDirect3DDevice9::GetPaletteEntries(UINT PaletteNumber, PALETTEENTRY* pEntries) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->GetPaletteEntries(PaletteNumber, pEntries) :
            this->dev->GetPaletteEntries(PaletteNumber, pEntries);
    }

    HRESULT LoaderDirect3DDevice9::SetCurrentTexturePalette(UINT PaletteNumber) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->SetCurrentTexturePalette(PaletteNumber) :
            this->dev->SetCurrentTexturePalette(PaletteNumber);
    }

    HRESULT LoaderDirect3DDevice9::GetCurrentTexturePalette(UINT *PaletteNumber) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->GetCurrentTexturePalette(PaletteNumber) :
            this->dev->GetCurrentTexturePalette(PaletteNumber);
    }

    HRESULT LoaderDirect3DDevice9::SetScissorRect(CONST RECT* pRect) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->SetScissorRect(pRect) :
            this->dev->SetScissorRect(pRect);
    }

    HRESULT LoaderDirect3DDevice9::GetScissorRect(RECT* pRect) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->GetScissorRect(pRect) :
            this->dev->GetScissorRect(pRect);
    }

    HRESULT LoaderDirect3DDevice9::SetSoftwareVertexProcessing(BOOL bSoftware) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->SetSoftwareVertexProcessing(bSoftware) :
            this->dev->SetSoftwareVertexProcessing(bSoftware);
    }

    BOOL LoaderDirect3DDevice9::GetSoftwareVertexProcessing() {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->GetSoftwareVertexProcessing() :
            this->dev->GetSoftwareVertexProcessing();
    }

    HRESULT LoaderDirect3DDevice9::SetNPatchMode(float nSegments) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->SetNPatchMode(nSegments) :
            this->dev->SetNPatchMode(nSegments);
    }

    float LoaderDirect3DDevice9::GetNPatchMode() {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->GetNPatchMode() :
            this->dev->GetNPatchMode();
    }

    HRESULT LoaderDirect3DDevice9::DrawPrimitive(D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->DrawPrimitive(PrimitiveType, StartVertex, PrimitiveCount) :
            this->dev->DrawPrimitive(PrimitiveType, StartVertex, PrimitiveCount);
    }

    HRESULT LoaderDirect3DDevice9::DrawIndexedPrimitive(D3DPRIMITIVETYPE PrimitiveType, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount) {
        if (!MumbleLink::GetInstance().IsTypeCompetitive()) {
            addons::AdvPreDrawIndexedPrimitive(this->dev, PrimitiveType, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount);
        }

        HRESULT hr = LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->DrawIndexedPrimitive(PrimitiveType, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount) :
            this->dev->DrawIndexedPrimitive(PrimitiveType, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount);
        if (hr != D3D_OK) {
            // Fail
            return hr;
        }

        if (PrePostProcessingDone == 0) {
            // Check if our render state has these values.
            // If that's the case, the current render target has the environment color
            // and the next render target is where the world view gets rendered.
            if (stateColorWriteEnable == D3DCOLORWRITEENABLE_ALPHA && stateZEnable == D3DZB_FALSE && stateZFunc == D3DCMP_ALWAYS) {
                ++PrePostProcessingDone;
            }
        }
        if (PrePresentGuiDone == 0) {
            // Check if our render state has these values.
            // If that's the case, the current render target is our last one before the GUI gets drawn.
            if (stateColorWriteEnable == (D3DCOLORWRITEENABLE_ALPHA | D3DCOLORWRITEENABLE_BLUE | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_RED) &&
                stateZEnable == D3DZB_FALSE && stateZFunc == D3DCMP_LESSEQUAL && stateFormat == D3DFMT_X8R8G8B8) {
                ++PrePresentGuiDone;
            }
        }

        if (!MumbleLink::GetInstance().IsTypeCompetitive()) {
            addons::AdvPostDrawIndexedPrimitive(this->dev, PrimitiveType, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount);
        }

        return hr;
    }

    HRESULT LoaderDirect3DDevice9::DrawPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT PrimitiveCount, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->DrawPrimitiveUP(PrimitiveType, PrimitiveCount, pVertexStreamZeroData, VertexStreamZeroStride) :
            this->dev->DrawPrimitiveUP(PrimitiveType, PrimitiveCount, pVertexStreamZeroData, VertexStreamZeroStride);
    }

    HRESULT LoaderDirect3DDevice9::DrawIndexedPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT MinVertexIndex, UINT NumVertices, UINT PrimitiveCount, CONST void* pIndexData, D3DFORMAT IndexDataFormat, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->DrawIndexedPrimitiveUP(PrimitiveType, MinVertexIndex, NumVertices, PrimitiveCount, pIndexData, IndexDataFormat, pVertexStreamZeroData, VertexStreamZeroStride) :
            this->dev->DrawIndexedPrimitiveUP(PrimitiveType, MinVertexIndex, NumVertices, PrimitiveCount, pIndexData, IndexDataFormat, pVertexStreamZeroData, VertexStreamZeroStride);
    }

    HRESULT LoaderDirect3DDevice9::ProcessVertices(UINT SrcStartIndex, UINT DestIndex, UINT VertexCount, IDirect3DVertexBuffer9* pDestBuffer, IDirect3DVertexDeclaration9* pVertexDecl, DWORD Flags) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->ProcessVertices(SrcStartIndex, DestIndex, VertexCount, pDestBuffer, pVertexDecl, Flags) :
            this->dev->ProcessVertices(SrcStartIndex, DestIndex, VertexCount, pDestBuffer, pVertexDecl, Flags);
    }

    HRESULT LoaderDirect3DDevice9::CreateVertexDeclaration(CONST D3DVERTEXELEMENT9* pVertexElements, IDirect3DVertexDeclaration9** ppDecl) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->CreateVertexDeclaration(pVertexElements, ppDecl) :
            this->dev->CreateVertexDeclaration(pVertexElements, ppDecl);
    }

    HRESULT LoaderDirect3DDevice9::SetVertexDeclaration(IDirect3DVertexDeclaration9* pDecl) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->SetVertexDeclaration(pDecl) :
            this->dev->SetVertexDeclaration(pDecl);
    }

    HRESULT LoaderDirect3DDevice9::GetVertexDeclaration(IDirect3DVertexDeclaration9** ppDecl) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->GetVertexDeclaration(ppDecl) :
            this->dev->GetVertexDeclaration(ppDecl);
    }

    HRESULT LoaderDirect3DDevice9::SetFVF(DWORD FVF) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->SetFVF(FVF) :
            this->dev->SetFVF(FVF);
    }

    HRESULT LoaderDirect3DDevice9::GetFVF(DWORD* pFVF) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->GetFVF(pFVF) :
            this->dev->GetFVF(pFVF);
    }

    HRESULT LoaderDirect3DDevice9::CreateVertexShader(CONST DWORD* pFunction, IDirect3DVertexShader9** ppShader) {
        IDirect3DVertexShader9* pOldShader = *ppShader;
        HRESULT hr;
        if (!MumbleLink::GetInstance().IsTypeCompetitive()) {
            hr = addons::AdvPreCreateVertexShader(this->dev, pFunction, ppShader);
            if (hr != D3D_OK) {
                // Fail
                return hr;
            }
        }

        if (*ppShader == pOldShader) {
            hr = LegacyAddonChainDevice != nullptr ?
                LegacyAddonChainDevice->CreateVertexShader(pFunction, ppShader) :
                this->dev->CreateVertexShader(pFunction, ppShader);
            if (hr != D3D_OK) {
                // Fail
                return hr;
            }
        }

        if (!MumbleLink::GetInstance().IsTypeCompetitive()) {
            addons::AdvPostCreateVertexShader(this->dev, *ppShader, pFunction);
        }

        return hr;
    }

    HRESULT LoaderDirect3DDevice9::SetVertexShader(IDirect3DVertexShader9* pShader) {
        if (!MumbleLink::GetInstance().IsTypeCompetitive()) {
            addons::AdvPreSetVertexShader(this->dev, pShader);
        }

        HRESULT hr = LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->SetVertexShader(pShader) :
            this->dev->SetVertexShader(pShader);
        if (hr != D3D_OK) {
            // Fail
            return hr;
        }

        if (!pShader && PrePresentGuiDone == 1) {
            // Guild Wars 2 will now be rendering the GUI

            // Make sure we keep track that we are doing this before actually doing it, in order to prevent stack overflows
            ++PrePresentGuiDone;

            // Save our current state
            IDirect3DStateBlock9* pStateBlock = NULL;
            this->dev->CreateStateBlock(D3DSBT_ALL, &pStateBlock);

            // Call our addons
            addons::DrawFrameBeforeGui(this->dev);

            // Restore our state
            pStateBlock->Apply();
            pStateBlock->Release();
        }

        if (!MumbleLink::GetInstance().IsTypeCompetitive()) {
            addons::AdvPostSetVertexShader(this->dev, pShader);
        }

        return hr;
    }

    HRESULT LoaderDirect3DDevice9::GetVertexShader(IDirect3DVertexShader9** ppShader) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->GetVertexShader(ppShader) :
            this->dev->GetVertexShader(ppShader);
    }

    HRESULT LoaderDirect3DDevice9::SetVertexShaderConstantF(UINT StartRegister, CONST float* pConstantData, UINT Vector4fCount) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->SetVertexShaderConstantF(StartRegister, pConstantData, Vector4fCount) :
            this->dev->SetVertexShaderConstantF(StartRegister, pConstantData, Vector4fCount);
    }

    HRESULT LoaderDirect3DDevice9::GetVertexShaderConstantF(UINT StartRegister, float* pConstantData, UINT Vector4fCount) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->GetVertexShaderConstantF(StartRegister, pConstantData, Vector4fCount) :
            this->dev->GetVertexShaderConstantF(StartRegister, pConstantData, Vector4fCount);
    }

    HRESULT LoaderDirect3DDevice9::SetVertexShaderConstantI(UINT StartRegister, CONST int* pConstantData, UINT Vector4iCount) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->SetVertexShaderConstantI(StartRegister, pConstantData, Vector4iCount) :
            this->dev->SetVertexShaderConstantI(StartRegister, pConstantData, Vector4iCount);
    }

    HRESULT LoaderDirect3DDevice9::GetVertexShaderConstantI(UINT StartRegister, int* pConstantData, UINT Vector4iCount) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->GetVertexShaderConstantI(StartRegister, pConstantData, Vector4iCount) :
            this->dev->GetVertexShaderConstantI(StartRegister, pConstantData, Vector4iCount);
    }

    HRESULT LoaderDirect3DDevice9::SetVertexShaderConstantB(UINT StartRegister, CONST BOOL* pConstantData, UINT BoolCount) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->SetVertexShaderConstantB(StartRegister, pConstantData, BoolCount) :
            this->dev->SetVertexShaderConstantB(StartRegister, pConstantData, BoolCount);
    }

    HRESULT LoaderDirect3DDevice9::GetVertexShaderConstantB(UINT StartRegister, BOOL* pConstantData, UINT BoolCount) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->GetVertexShaderConstantB(StartRegister, pConstantData, BoolCount) :
            this->dev->GetVertexShaderConstantB(StartRegister, pConstantData, BoolCount);
    }

    HRESULT LoaderDirect3DDevice9::SetStreamSource(UINT StreamNumber, IDirect3DVertexBuffer9* pStreamData, UINT OffsetInBytes, UINT Stride) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->SetStreamSource(StreamNumber, pStreamData, OffsetInBytes, Stride) :
            this->dev->SetStreamSource(StreamNumber, pStreamData, OffsetInBytes, Stride);
    }

    HRESULT LoaderDirect3DDevice9::GetStreamSource(UINT StreamNumber, IDirect3DVertexBuffer9** ppStreamData, UINT* pOffsetInBytes, UINT* pStride) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->GetStreamSource(StreamNumber, ppStreamData, pOffsetInBytes, pStride) :
            this->dev->GetStreamSource(StreamNumber, ppStreamData, pOffsetInBytes, pStride);
    }

    HRESULT LoaderDirect3DDevice9::SetStreamSourceFreq(UINT StreamNumber, UINT Setting) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->SetStreamSourceFreq(StreamNumber, Setting) :
            this->dev->SetStreamSourceFreq(StreamNumber, Setting);
    }

    HRESULT LoaderDirect3DDevice9::GetStreamSourceFreq(UINT StreamNumber, UINT* pSetting) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->GetStreamSourceFreq(StreamNumber, pSetting) :
            this->dev->GetStreamSourceFreq(StreamNumber, pSetting);
    }

    HRESULT LoaderDirect3DDevice9::SetIndices(IDirect3DIndexBuffer9* pIndexData) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->SetIndices(pIndexData) :
            this->dev->SetIndices(pIndexData);
    }

    HRESULT LoaderDirect3DDevice9::GetIndices(IDirect3DIndexBuffer9** ppIndexData) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->GetIndices(ppIndexData) :
            this->dev->GetIndices(ppIndexData);
    }

    HRESULT LoaderDirect3DDevice9::CreatePixelShader(CONST DWORD* pFunction, IDirect3DPixelShader9** ppShader) {
        IDirect3DPixelShader9* pOldShader = *ppShader;
        HRESULT hr;
        if (!MumbleLink::GetInstance().IsTypeCompetitive()) {
            hr = addons::AdvPreCreatePixelShader(this->dev, pFunction, ppShader);
            if (hr != D3D_OK) {
                // Fail
                return hr;
            }
        }

        if (*ppShader == pOldShader) {
            hr = LegacyAddonChainDevice != nullptr ?
                LegacyAddonChainDevice->CreatePixelShader(pFunction, ppShader) :
                this->dev->CreatePixelShader(pFunction, ppShader);
            if (hr != D3D_OK) {
                // Fail
                return hr;
            }
        }

        if (!MumbleLink::GetInstance().IsTypeCompetitive()) {
            addons::AdvPostCreatePixelShader(this->dev, *ppShader, pFunction);
        }

        return hr;
    }

    HRESULT LoaderDirect3DDevice9::SetPixelShader(IDirect3DPixelShader9* pShader) {
        if (!MumbleLink::GetInstance().IsTypeCompetitive()) {
            addons::AdvPreSetPixelShader(this->dev, pShader);
        }

        HRESULT hr = LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->SetPixelShader(pShader) :
            this->dev->SetPixelShader(pShader);
        if (hr != D3D_OK) {
            // Fail
            return hr;
        }

        if (!MumbleLink::GetInstance().IsTypeCompetitive()) {
            addons::AdvPostSetPixelShader(this->dev, pShader);
        }

        return hr;
    }

    HRESULT LoaderDirect3DDevice9::GetPixelShader(IDirect3DPixelShader9** ppShader) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->GetPixelShader(ppShader) :
            this->dev->GetPixelShader(ppShader);
    }

    HRESULT LoaderDirect3DDevice9::SetPixelShaderConstantF(UINT StartRegister, CONST float* pConstantData, UINT Vector4fCount) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->SetPixelShaderConstantF(StartRegister, pConstantData, Vector4fCount) :
            this->dev->SetPixelShaderConstantF(StartRegister, pConstantData, Vector4fCount);
    }

    HRESULT LoaderDirect3DDevice9::GetPixelShaderConstantF(UINT StartRegister, float* pConstantData, UINT Vector4fCount) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->GetPixelShaderConstantF(StartRegister, pConstantData, Vector4fCount) :
            this->dev->GetPixelShaderConstantF(StartRegister, pConstantData, Vector4fCount);
    }

    HRESULT LoaderDirect3DDevice9::SetPixelShaderConstantI(UINT StartRegister, CONST int* pConstantData, UINT Vector4iCount) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->SetPixelShaderConstantI(StartRegister, pConstantData, Vector4iCount) :
            this->dev->SetPixelShaderConstantI(StartRegister, pConstantData, Vector4iCount);
    }

    HRESULT LoaderDirect3DDevice9::GetPixelShaderConstantI(UINT StartRegister, int* pConstantData, UINT Vector4iCount) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->GetPixelShaderConstantI(StartRegister, pConstantData, Vector4iCount) :
            this->dev->GetPixelShaderConstantI(StartRegister, pConstantData, Vector4iCount);
    }

    HRESULT LoaderDirect3DDevice9::SetPixelShaderConstantB(UINT StartRegister, CONST BOOL* pConstantData, UINT BoolCount) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->SetPixelShaderConstantB(StartRegister, pConstantData, BoolCount) :
            this->dev->SetPixelShaderConstantB(StartRegister, pConstantData, BoolCount);
    }

    HRESULT LoaderDirect3DDevice9::GetPixelShaderConstantB(UINT StartRegister, BOOL* pConstantData, UINT BoolCount) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->GetPixelShaderConstantB(StartRegister, pConstantData, BoolCount) :
            this->dev->GetPixelShaderConstantB(StartRegister, pConstantData, BoolCount);
    }

    HRESULT LoaderDirect3DDevice9::DrawRectPatch(UINT Handle, CONST float* pNumSegs, CONST D3DRECTPATCH_INFO* pRectPatchInfo) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->DrawRectPatch(Handle, pNumSegs, pRectPatchInfo) :
            this->dev->DrawRectPatch(Handle, pNumSegs, pRectPatchInfo);
    }

    HRESULT LoaderDirect3DDevice9::DrawTriPatch(UINT Handle, CONST float* pNumSegs, CONST D3DTRIPATCH_INFO* pTriPatchInfo) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->DrawTriPatch(Handle, pNumSegs, pTriPatchInfo) :
            this->dev->DrawTriPatch(Handle, pNumSegs, pTriPatchInfo);
    }

    HRESULT LoaderDirect3DDevice9::DeletePatch(UINT Handle) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->DeletePatch(Handle) :
            this->dev->DeletePatch(Handle);
    }

    HRESULT LoaderDirect3DDevice9::CreateQuery(D3DQUERYTYPE Type, IDirect3DQuery9** ppQuery) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->CreateQuery(Type, ppQuery) :
            this->dev->CreateQuery(Type, ppQuery);
    }


    HRESULT LoaderDirect3DDevice9Ex::QueryInterface(REFIID riid, void** ppvObj) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->QueryInterface(riid, ppvObj) :
            this->dev->QueryInterface(riid, ppvObj);
    }

    ULONG LoaderDirect3DDevice9Ex::AddRef() {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->AddRef() :
            this->dev->AddRef();
    }

    ULONG LoaderDirect3DDevice9Ex::Release() {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->Release() :
            this->dev->Release();
    }

    HRESULT LoaderDirect3DDevice9Ex::TestCooperativeLevel() {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->TestCooperativeLevel() :
            this->dev->TestCooperativeLevel();
    }

    UINT LoaderDirect3DDevice9Ex::GetAvailableTextureMem() {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->GetAvailableTextureMem() :
            this->dev->GetAvailableTextureMem();
    }

    HRESULT LoaderDirect3DDevice9Ex::EvictManagedResources() {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->EvictManagedResources() :
            this->dev->EvictManagedResources();
    }

    HRESULT LoaderDirect3DDevice9Ex::GetDirect3D(IDirect3D9** ppD3D9) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->GetDirect3D(ppD3D9) :
            this->dev->GetDirect3D(ppD3D9);
    }

    HRESULT LoaderDirect3DDevice9Ex::GetDeviceCaps(D3DCAPS9* pCaps) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->GetDeviceCaps(pCaps) :
            this->dev->GetDeviceCaps(pCaps);
    }

    HRESULT LoaderDirect3DDevice9Ex::GetDisplayMode(UINT iSwapChain, D3DDISPLAYMODE* pMode) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->GetDisplayMode(iSwapChain, pMode) :
            this->dev->GetDisplayMode(iSwapChain, pMode);
    }

    HRESULT LoaderDirect3DDevice9Ex::GetCreationParameters(D3DDEVICE_CREATION_PARAMETERS *pParameters) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->GetCreationParameters(pParameters) :
            this->dev->GetCreationParameters(pParameters);
    }

    HRESULT LoaderDirect3DDevice9Ex::SetCursorProperties(UINT XHotSpot, UINT YHotSpot, IDirect3DSurface9* pCursorBitmap) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->SetCursorProperties(XHotSpot, YHotSpot, pCursorBitmap) :
            this->dev->SetCursorProperties(XHotSpot, YHotSpot, pCursorBitmap);
    }

    void LoaderDirect3DDevice9Ex::SetCursorPosition(int X, int Y, DWORD Flags) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->SetCursorPosition(X, Y, Flags) :
            this->dev->SetCursorPosition(X, Y, Flags);
    }

    BOOL LoaderDirect3DDevice9Ex::ShowCursor(BOOL bShow) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->ShowCursor(bShow) :
            this->dev->ShowCursor(bShow);
    }

    HRESULT LoaderDirect3DDevice9Ex::CreateAdditionalSwapChain(D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DSwapChain9** pSwapChain) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->CreateAdditionalSwapChain(pPresentationParameters, pSwapChain) :
            this->dev->CreateAdditionalSwapChain(pPresentationParameters, pSwapChain);
    }

    HRESULT LoaderDirect3DDevice9Ex::GetSwapChain(UINT iSwapChain, IDirect3DSwapChain9** pSwapChain) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->GetSwapChain(iSwapChain, pSwapChain) :
            this->dev->GetSwapChain(iSwapChain, pSwapChain);
    }

    UINT LoaderDirect3DDevice9Ex::GetNumberOfSwapChains() {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->GetNumberOfSwapChains() :
            this->dev->GetNumberOfSwapChains();
    }

    HRESULT LoaderDirect3DDevice9Ex::Reset(D3DPRESENT_PARAMETERS* pPresentationParameters) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->Reset(pPresentationParameters) :
            this->dev->Reset(pPresentationParameters);
    }

    HRESULT LoaderDirect3DDevice9Ex::Present(CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->Present(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion) :
            this->dev->Present(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
    }

    HRESULT LoaderDirect3DDevice9Ex::GetBackBuffer(UINT iSwapChain, UINT iBackBuffer, D3DBACKBUFFER_TYPE Type, IDirect3DSurface9** ppBackBuffer) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->GetBackBuffer(iSwapChain, iBackBuffer, Type, ppBackBuffer) :
            this->dev->GetBackBuffer(iSwapChain, iBackBuffer, Type, ppBackBuffer);
    }

    HRESULT LoaderDirect3DDevice9Ex::GetRasterStatus(UINT iSwapChain, D3DRASTER_STATUS* pRasterStatus) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->GetRasterStatus(iSwapChain, pRasterStatus) :
            this->dev->GetRasterStatus(iSwapChain, pRasterStatus);
    }

    HRESULT LoaderDirect3DDevice9Ex::SetDialogBoxMode(BOOL bEnableDialogs) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->SetDialogBoxMode(bEnableDialogs) :
            this->dev->SetDialogBoxMode(bEnableDialogs);
    }

    void LoaderDirect3DDevice9Ex::SetGammaRamp(UINT iSwapChain, DWORD Flags, CONST D3DGAMMARAMP* pRamp) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->SetGammaRamp(iSwapChain, Flags, pRamp) :
            this->dev->SetGammaRamp(iSwapChain, Flags, pRamp);
    }

    void LoaderDirect3DDevice9Ex::GetGammaRamp(UINT iSwapChain, D3DGAMMARAMP* pRamp) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->GetGammaRamp(iSwapChain, pRamp) :
            this->dev->GetGammaRamp(iSwapChain, pRamp);
    }

    HRESULT LoaderDirect3DDevice9Ex::CreateTexture(UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DTexture9** ppTexture, HANDLE* pSharedHandle) {
        // Not hooked, GW2 doesn't use Ex calls
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->CreateTexture(Width, Height, Levels, Usage, Format, Pool, ppTexture, pSharedHandle) :
            this->dev->CreateTexture(Width, Height, Levels, Usage, Format, Pool, ppTexture, pSharedHandle);
    }

    HRESULT LoaderDirect3DDevice9Ex::CreateVolumeTexture(UINT Width, UINT Height, UINT Depth, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DVolumeTexture9** ppVolumeTexture, HANDLE* pSharedHandle) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->CreateVolumeTexture(Width, Height, Depth, Levels, Usage, Format, Pool, ppVolumeTexture, pSharedHandle) :
            this->dev->CreateVolumeTexture(Width, Height, Depth, Levels, Usage, Format, Pool, ppVolumeTexture, pSharedHandle);
    }

    HRESULT LoaderDirect3DDevice9Ex::CreateCubeTexture(UINT EdgeLength, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DCubeTexture9** ppCubeTexture, HANDLE* pSharedHandle) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->CreateCubeTexture(EdgeLength, Levels, Usage, Format, Pool, ppCubeTexture, pSharedHandle) :
            this->dev->CreateCubeTexture(EdgeLength, Levels, Usage, Format, Pool, ppCubeTexture, pSharedHandle);
    }

    HRESULT LoaderDirect3DDevice9Ex::CreateVertexBuffer(UINT Length, DWORD Usage, DWORD FVF, D3DPOOL Pool, IDirect3DVertexBuffer9** ppVertexBuffer, HANDLE* pSharedHandle) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->CreateVertexBuffer(Length, Usage, FVF, Pool, ppVertexBuffer, pSharedHandle) :
            this->dev->CreateVertexBuffer(Length, Usage, FVF, Pool, ppVertexBuffer, pSharedHandle);
    }

    HRESULT LoaderDirect3DDevice9Ex::CreateIndexBuffer(UINT Length, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DIndexBuffer9** ppIndexBuffer, HANDLE* pSharedHandle) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->CreateIndexBuffer(Length, Usage, Format, Pool, ppIndexBuffer, pSharedHandle) :
            this->dev->CreateIndexBuffer(Length, Usage, Format, Pool, ppIndexBuffer, pSharedHandle);
    }

    HRESULT LoaderDirect3DDevice9Ex::CreateRenderTarget(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Lockable, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle) {
        // Not hooked, GW2 doesn't use Ex calls
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->CreateRenderTarget(Width, Height, Format, MultiSample, MultisampleQuality, Lockable, ppSurface, pSharedHandle) :
            this->dev->CreateRenderTarget(Width, Height, Format, MultiSample, MultisampleQuality, Lockable, ppSurface, pSharedHandle);
    }

    HRESULT LoaderDirect3DDevice9Ex::CreateDepthStencilSurface(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Discard, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->CreateDepthStencilSurface(Width, Height, Format, MultiSample, MultisampleQuality, Discard, ppSurface, pSharedHandle) :
            this->dev->CreateDepthStencilSurface(Width, Height, Format, MultiSample, MultisampleQuality, Discard, ppSurface, pSharedHandle);
    }

    HRESULT LoaderDirect3DDevice9Ex::UpdateSurface(IDirect3DSurface9* pSourceSurface, CONST RECT* pSourceRect, IDirect3DSurface9* pDestinationSurface, CONST POINT* pDestPoint) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->UpdateSurface(pSourceSurface, pSourceRect, pDestinationSurface, pDestPoint) :
            this->dev->UpdateSurface(pSourceSurface, pSourceRect, pDestinationSurface, pDestPoint);
    }

    HRESULT LoaderDirect3DDevice9Ex::UpdateTexture(IDirect3DBaseTexture9* pSourceTexture, IDirect3DBaseTexture9* pDestinationTexture) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->UpdateTexture(pSourceTexture, pDestinationTexture) :
            this->dev->UpdateTexture(pSourceTexture, pDestinationTexture);
    }

    HRESULT LoaderDirect3DDevice9Ex::GetRenderTargetData(IDirect3DSurface9* pRenderTarget, IDirect3DSurface9* pDestSurface) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->GetRenderTargetData(pRenderTarget, pDestSurface) :
            this->dev->GetRenderTargetData(pRenderTarget, pDestSurface);
    }

    HRESULT LoaderDirect3DDevice9Ex::GetFrontBufferData(UINT iSwapChain, IDirect3DSurface9* pDestSurface) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->GetFrontBufferData(iSwapChain, pDestSurface) :
            this->dev->GetFrontBufferData(iSwapChain, pDestSurface);
    }

    HRESULT LoaderDirect3DDevice9Ex::StretchRect(IDirect3DSurface9* pSourceSurface, CONST RECT* pSourceRect, IDirect3DSurface9* pDestSurface, CONST RECT* pDestRect, D3DTEXTUREFILTERTYPE Filter) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->StretchRect(pSourceSurface, pSourceRect, pDestSurface, pDestRect, Filter) :
            this->dev->StretchRect(pSourceSurface, pSourceRect, pDestSurface, pDestRect, Filter);
    }

    HRESULT LoaderDirect3DDevice9Ex::ColorFill(IDirect3DSurface9* pSurface, CONST RECT* pRect, D3DCOLOR color) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->ColorFill(pSurface, pRect, color) :
            this->dev->ColorFill(pSurface, pRect, color);
    }

    HRESULT LoaderDirect3DDevice9Ex::CreateOffscreenPlainSurface(UINT Width, UINT Height, D3DFORMAT Format, D3DPOOL Pool, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->CreateOffscreenPlainSurface(Width, Height, Format, Pool, ppSurface, pSharedHandle) :
            this->dev->CreateOffscreenPlainSurface(Width, Height, Format, Pool, ppSurface, pSharedHandle);
    }

    HRESULT LoaderDirect3DDevice9Ex::SetRenderTarget(DWORD RenderTargetIndex, IDirect3DSurface9* pRenderTarget) {
        // Not hooked, GW2 doesn't use Ex calls
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->SetRenderTarget(RenderTargetIndex, pRenderTarget) :
            this->dev->SetRenderTarget(RenderTargetIndex, pRenderTarget);
    }

    HRESULT LoaderDirect3DDevice9Ex::GetRenderTarget(DWORD RenderTargetIndex, IDirect3DSurface9** ppRenderTarget) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->GetRenderTarget(RenderTargetIndex, ppRenderTarget) :
            this->dev->GetRenderTarget(RenderTargetIndex, ppRenderTarget);
    }

    HRESULT LoaderDirect3DDevice9Ex::SetDepthStencilSurface(IDirect3DSurface9* pNewZStencil) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->SetDepthStencilSurface(pNewZStencil) :
            this->dev->SetDepthStencilSurface(pNewZStencil);
    }

    HRESULT LoaderDirect3DDevice9Ex::GetDepthStencilSurface(IDirect3DSurface9** ppZStencilSurface) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->GetDepthStencilSurface(ppZStencilSurface) :
            this->dev->GetDepthStencilSurface(ppZStencilSurface);
    }

    HRESULT LoaderDirect3DDevice9Ex::BeginScene() {
        // Not hooked, GW2 doesn't use Ex calls
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->BeginScene() :
            this->dev->BeginScene();
    }

    HRESULT LoaderDirect3DDevice9Ex::EndScene() {
        // Not hooked, GW2 doesn't use Ex calls
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->EndScene() :
            this->dev->EndScene();
    }

    HRESULT LoaderDirect3DDevice9Ex::Clear(DWORD Count, CONST D3DRECT* pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil) {
        // Not hooked, GW2 doesn't use Ex calls
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->Clear(Count, pRects, Flags, Color, Z, Stencil) :
            this->dev->Clear(Count, pRects, Flags, Color, Z, Stencil);
    }

    HRESULT LoaderDirect3DDevice9Ex::SetTransform(D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX* pMatrix) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->SetTransform(State, pMatrix) :
            this->dev->SetTransform(State, pMatrix);
    }

    HRESULT LoaderDirect3DDevice9Ex::GetTransform(D3DTRANSFORMSTATETYPE State, D3DMATRIX* pMatrix) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->GetTransform(State, pMatrix) :
            this->dev->GetTransform(State, pMatrix);
    }

    HRESULT LoaderDirect3DDevice9Ex::MultiplyTransform(D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX* pMatrix) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->MultiplyTransform(State, pMatrix) :
            this->dev->MultiplyTransform(State, pMatrix);
    }

    HRESULT LoaderDirect3DDevice9Ex::SetViewport(CONST D3DVIEWPORT9* pViewport) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->SetViewport(pViewport) :
            this->dev->SetViewport(pViewport);
    }

    HRESULT LoaderDirect3DDevice9Ex::GetViewport(D3DVIEWPORT9* pViewport) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->GetViewport(pViewport) :
            this->dev->GetViewport(pViewport);
    }

    HRESULT LoaderDirect3DDevice9Ex::SetMaterial(CONST D3DMATERIAL9* pMaterial) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->SetMaterial(pMaterial) :
            this->dev->SetMaterial(pMaterial);
    }

    HRESULT LoaderDirect3DDevice9Ex::GetMaterial(D3DMATERIAL9* pMaterial) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->GetMaterial(pMaterial) :
            this->dev->GetMaterial(pMaterial);
    }

    HRESULT LoaderDirect3DDevice9Ex::SetLight(DWORD Index, CONST D3DLIGHT9* pLight) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->SetLight(Index, pLight) :
            this->dev->SetLight(Index, pLight);
    }

    HRESULT LoaderDirect3DDevice9Ex::GetLight(DWORD Index, D3DLIGHT9* pLight) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->GetLight(Index, pLight) :
            this->dev->GetLight(Index, pLight);
    }

    HRESULT LoaderDirect3DDevice9Ex::LightEnable(DWORD Index, BOOL Enable) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->LightEnable(Index, Enable) :
            this->dev->LightEnable(Index, Enable);
    }

    HRESULT LoaderDirect3DDevice9Ex::GetLightEnable(DWORD Index, BOOL* pEnable) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->GetLightEnable(Index, pEnable) :
            this->dev->GetLightEnable(Index, pEnable);
    }

    HRESULT LoaderDirect3DDevice9Ex::SetClipPlane(DWORD Index, CONST float* pPlane) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->SetClipPlane(Index, pPlane) :
            this->dev->SetClipPlane(Index, pPlane);
    }

    HRESULT LoaderDirect3DDevice9Ex::GetClipPlane(DWORD Index, float* pPlane) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->GetClipPlane(Index, pPlane) :
            this->dev->GetClipPlane(Index, pPlane);
    }

    HRESULT LoaderDirect3DDevice9Ex::SetRenderState(D3DRENDERSTATETYPE State, DWORD Value) {
        // Not hooked, GW2 doesn't use Ex calls
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->SetRenderState(State, Value) :
            this->dev->SetRenderState(State, Value);
    }

    HRESULT LoaderDirect3DDevice9Ex::GetRenderState(D3DRENDERSTATETYPE State, DWORD* pValue) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->GetRenderState(State, pValue) :
            this->dev->GetRenderState(State, pValue);
    }

    HRESULT LoaderDirect3DDevice9Ex::CreateStateBlock(D3DSTATEBLOCKTYPE Type, IDirect3DStateBlock9** ppSB) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->CreateStateBlock(Type, ppSB) :
            this->dev->CreateStateBlock(Type, ppSB);
    }

    HRESULT LoaderDirect3DDevice9Ex::BeginStateBlock() {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->BeginStateBlock() :
            this->dev->BeginStateBlock();
    }

    HRESULT LoaderDirect3DDevice9Ex::EndStateBlock(IDirect3DStateBlock9** ppSB) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->EndStateBlock(ppSB) :
            this->dev->EndStateBlock(ppSB);
    }

    HRESULT LoaderDirect3DDevice9Ex::SetClipStatus(CONST D3DCLIPSTATUS9* pClipStatus) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->SetClipStatus(pClipStatus) :
            this->dev->SetClipStatus(pClipStatus);
    }

    HRESULT LoaderDirect3DDevice9Ex::GetClipStatus(D3DCLIPSTATUS9* pClipStatus) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->GetClipStatus(pClipStatus) :
            this->dev->GetClipStatus(pClipStatus);
    }

    HRESULT LoaderDirect3DDevice9Ex::GetTexture(DWORD Stage, IDirect3DBaseTexture9** ppTexture) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->GetTexture(Stage, ppTexture) :
            this->dev->GetTexture(Stage, ppTexture);
    }

    HRESULT LoaderDirect3DDevice9Ex::SetTexture(DWORD Stage, IDirect3DBaseTexture9* pTexture) {
        // Not hooked, GW2 doesn't use Ex calls
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->SetTexture(Stage, pTexture) :
            this->dev->SetTexture(Stage, pTexture);
    }

    HRESULT LoaderDirect3DDevice9Ex::GetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD* pValue) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->GetTextureStageState(Stage, Type, pValue) :
            this->dev->GetTextureStageState(Stage, Type, pValue);
    }

    HRESULT LoaderDirect3DDevice9Ex::SetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->SetTextureStageState(Stage, Type, Value) :
            this->dev->SetTextureStageState(Stage, Type, Value);
    }

    HRESULT LoaderDirect3DDevice9Ex::GetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD* pValue) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->GetSamplerState(Sampler, Type, pValue) :
            this->dev->GetSamplerState(Sampler, Type, pValue);
    }

    HRESULT LoaderDirect3DDevice9Ex::SetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD Value) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->SetSamplerState(Sampler, Type, Value) :
            this->dev->SetSamplerState(Sampler, Type, Value);
    }

    HRESULT LoaderDirect3DDevice9Ex::ValidateDevice(DWORD* pNumPasses) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->ValidateDevice(pNumPasses) :
            this->dev->ValidateDevice(pNumPasses);
    }

    HRESULT LoaderDirect3DDevice9Ex::SetPaletteEntries(UINT PaletteNumber, CONST PALETTEENTRY* pEntries) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->SetPaletteEntries(PaletteNumber, pEntries) :
            this->dev->SetPaletteEntries(PaletteNumber, pEntries);
    }

    HRESULT LoaderDirect3DDevice9Ex::GetPaletteEntries(UINT PaletteNumber, PALETTEENTRY* pEntries) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->GetPaletteEntries(PaletteNumber, pEntries) :
            this->dev->GetPaletteEntries(PaletteNumber, pEntries);
    }

    HRESULT LoaderDirect3DDevice9Ex::SetCurrentTexturePalette(UINT PaletteNumber) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->SetCurrentTexturePalette(PaletteNumber) :
            this->dev->SetCurrentTexturePalette(PaletteNumber);
    }

    HRESULT LoaderDirect3DDevice9Ex::GetCurrentTexturePalette(UINT *PaletteNumber) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->GetCurrentTexturePalette(PaletteNumber) :
            this->dev->GetCurrentTexturePalette(PaletteNumber);
    }

    HRESULT LoaderDirect3DDevice9Ex::SetScissorRect(CONST RECT* pRect) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->SetScissorRect(pRect) :
            this->dev->SetScissorRect(pRect);
    }

    HRESULT LoaderDirect3DDevice9Ex::GetScissorRect(RECT* pRect) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->GetScissorRect(pRect) :
            this->dev->GetScissorRect(pRect);
    }

    HRESULT LoaderDirect3DDevice9Ex::SetSoftwareVertexProcessing(BOOL bSoftware) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->SetSoftwareVertexProcessing(bSoftware) :
            this->dev->SetSoftwareVertexProcessing(bSoftware);
    }

    BOOL LoaderDirect3DDevice9Ex::GetSoftwareVertexProcessing() {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->GetSoftwareVertexProcessing() :
            this->dev->GetSoftwareVertexProcessing();
    }

    HRESULT LoaderDirect3DDevice9Ex::SetNPatchMode(float nSegments) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->SetNPatchMode(nSegments) :
            this->dev->SetNPatchMode(nSegments);
    }

    float LoaderDirect3DDevice9Ex::GetNPatchMode() {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->GetNPatchMode() :
            this->dev->GetNPatchMode();
    }

    HRESULT LoaderDirect3DDevice9Ex::DrawPrimitive(D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->DrawPrimitive(PrimitiveType, StartVertex, PrimitiveCount) :
            this->dev->DrawPrimitive(PrimitiveType, StartVertex, PrimitiveCount);
    }

    HRESULT LoaderDirect3DDevice9Ex::DrawIndexedPrimitive(D3DPRIMITIVETYPE PrimitiveType, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount) {
        // Not hooked, GW2 doesn't use Ex calls
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->DrawIndexedPrimitive(PrimitiveType, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount) :
            this->dev->DrawIndexedPrimitive(PrimitiveType, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount);
    }

    HRESULT LoaderDirect3DDevice9Ex::DrawPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT PrimitiveCount, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->DrawPrimitiveUP(PrimitiveType, PrimitiveCount, pVertexStreamZeroData, VertexStreamZeroStride) :
            this->dev->DrawPrimitiveUP(PrimitiveType, PrimitiveCount, pVertexStreamZeroData, VertexStreamZeroStride);
    }

    HRESULT LoaderDirect3DDevice9Ex::DrawIndexedPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT MinVertexIndex, UINT NumVertices, UINT PrimitiveCount, CONST void* pIndexData, D3DFORMAT IndexDataFormat, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->DrawIndexedPrimitiveUP(PrimitiveType, MinVertexIndex, NumVertices, PrimitiveCount, pIndexData, IndexDataFormat, pVertexStreamZeroData, VertexStreamZeroStride) :
            this->dev->DrawIndexedPrimitiveUP(PrimitiveType, MinVertexIndex, NumVertices, PrimitiveCount, pIndexData, IndexDataFormat, pVertexStreamZeroData, VertexStreamZeroStride);
    }

    HRESULT LoaderDirect3DDevice9Ex::ProcessVertices(UINT SrcStartIndex, UINT DestIndex, UINT VertexCount, IDirect3DVertexBuffer9* pDestBuffer, IDirect3DVertexDeclaration9* pVertexDecl, DWORD Flags) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->ProcessVertices(SrcStartIndex, DestIndex, VertexCount, pDestBuffer, pVertexDecl, Flags) :
            this->dev->ProcessVertices(SrcStartIndex, DestIndex, VertexCount, pDestBuffer, pVertexDecl, Flags);
    }

    HRESULT LoaderDirect3DDevice9Ex::CreateVertexDeclaration(CONST D3DVERTEXELEMENT9* pVertexElements, IDirect3DVertexDeclaration9** ppDecl) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->CreateVertexDeclaration(pVertexElements, ppDecl) :
            this->dev->CreateVertexDeclaration(pVertexElements, ppDecl);
    }

    HRESULT LoaderDirect3DDevice9Ex::SetVertexDeclaration(IDirect3DVertexDeclaration9* pDecl) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->SetVertexDeclaration(pDecl) :
            this->dev->SetVertexDeclaration(pDecl);
    }

    HRESULT LoaderDirect3DDevice9Ex::GetVertexDeclaration(IDirect3DVertexDeclaration9** ppDecl) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->GetVertexDeclaration(ppDecl) :
            this->dev->GetVertexDeclaration(ppDecl);
    }

    HRESULT LoaderDirect3DDevice9Ex::SetFVF(DWORD FVF) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->SetFVF(FVF) :
            this->dev->SetFVF(FVF);
    }

    HRESULT LoaderDirect3DDevice9Ex::GetFVF(DWORD* pFVF) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->GetFVF(pFVF) :
            this->dev->GetFVF(pFVF);
    }

    HRESULT LoaderDirect3DDevice9Ex::CreateVertexShader(CONST DWORD* pFunction, IDirect3DVertexShader9** ppShader) {
        // Not hooked, GW2 doesn't use Ex calls
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->CreateVertexShader(pFunction, ppShader) :
            this->dev->CreateVertexShader(pFunction, ppShader);
    }

    HRESULT LoaderDirect3DDevice9Ex::SetVertexShader(IDirect3DVertexShader9* pShader) {
        // Not hooked, GW2 doesn't use Ex calls
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->SetVertexShader(pShader) :
            this->dev->SetVertexShader(pShader);
    }

    HRESULT LoaderDirect3DDevice9Ex::GetVertexShader(IDirect3DVertexShader9** ppShader) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->GetVertexShader(ppShader) :
            this->dev->GetVertexShader(ppShader);
    }

    HRESULT LoaderDirect3DDevice9Ex::SetVertexShaderConstantF(UINT StartRegister, CONST float* pConstantData, UINT Vector4fCount) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->SetVertexShaderConstantF(StartRegister, pConstantData, Vector4fCount) :
            this->dev->SetVertexShaderConstantF(StartRegister, pConstantData, Vector4fCount);
    }

    HRESULT LoaderDirect3DDevice9Ex::GetVertexShaderConstantF(UINT StartRegister, float* pConstantData, UINT Vector4fCount) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->GetVertexShaderConstantF(StartRegister, pConstantData, Vector4fCount) :
            this->dev->GetVertexShaderConstantF(StartRegister, pConstantData, Vector4fCount);
    }

    HRESULT LoaderDirect3DDevice9Ex::SetVertexShaderConstantI(UINT StartRegister, CONST int* pConstantData, UINT Vector4iCount) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->SetVertexShaderConstantI(StartRegister, pConstantData, Vector4iCount) :
            this->dev->SetVertexShaderConstantI(StartRegister, pConstantData, Vector4iCount);
    }

    HRESULT LoaderDirect3DDevice9Ex::GetVertexShaderConstantI(UINT StartRegister, int* pConstantData, UINT Vector4iCount) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->GetVertexShaderConstantI(StartRegister, pConstantData, Vector4iCount) :
            this->dev->GetVertexShaderConstantI(StartRegister, pConstantData, Vector4iCount);
    }

    HRESULT LoaderDirect3DDevice9Ex::SetVertexShaderConstantB(UINT StartRegister, CONST BOOL* pConstantData, UINT BoolCount) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->SetVertexShaderConstantB(StartRegister, pConstantData, BoolCount) :
            this->dev->SetVertexShaderConstantB(StartRegister, pConstantData, BoolCount);
    }

    HRESULT LoaderDirect3DDevice9Ex::GetVertexShaderConstantB(UINT StartRegister, BOOL* pConstantData, UINT BoolCount) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->GetVertexShaderConstantB(StartRegister, pConstantData, BoolCount) :
            this->dev->GetVertexShaderConstantB(StartRegister, pConstantData, BoolCount);
    }

    HRESULT LoaderDirect3DDevice9Ex::SetStreamSource(UINT StreamNumber, IDirect3DVertexBuffer9* pStreamData, UINT OffsetInBytes, UINT Stride) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->SetStreamSource(StreamNumber, pStreamData, OffsetInBytes, Stride) :
            this->dev->SetStreamSource(StreamNumber, pStreamData, OffsetInBytes, Stride);
    }

    HRESULT LoaderDirect3DDevice9Ex::GetStreamSource(UINT StreamNumber, IDirect3DVertexBuffer9** ppStreamData, UINT* pOffsetInBytes, UINT* pStride) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->GetStreamSource(StreamNumber, ppStreamData, pOffsetInBytes, pStride) :
            this->dev->GetStreamSource(StreamNumber, ppStreamData, pOffsetInBytes, pStride);
    }

    HRESULT LoaderDirect3DDevice9Ex::SetStreamSourceFreq(UINT StreamNumber, UINT Setting) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->SetStreamSourceFreq(StreamNumber, Setting) :
            this->dev->SetStreamSourceFreq(StreamNumber, Setting);
    }

    HRESULT LoaderDirect3DDevice9Ex::GetStreamSourceFreq(UINT StreamNumber, UINT* pSetting) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->GetStreamSourceFreq(StreamNumber, pSetting) :
            this->dev->GetStreamSourceFreq(StreamNumber, pSetting);
    }

    HRESULT LoaderDirect3DDevice9Ex::SetIndices(IDirect3DIndexBuffer9* pIndexData) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->SetIndices(pIndexData) :
            this->dev->SetIndices(pIndexData);
    }

    HRESULT LoaderDirect3DDevice9Ex::GetIndices(IDirect3DIndexBuffer9** ppIndexData) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->GetIndices(ppIndexData) :
            this->dev->GetIndices(ppIndexData);
    }

    HRESULT LoaderDirect3DDevice9Ex::CreatePixelShader(CONST DWORD* pFunction, IDirect3DPixelShader9** ppShader) {
        // Not hooked, GW2 doesn't use Ex calls
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->CreatePixelShader(pFunction, ppShader) :
            this->dev->CreatePixelShader(pFunction, ppShader);
    }

    HRESULT LoaderDirect3DDevice9Ex::SetPixelShader(IDirect3DPixelShader9* pShader) {
        // Not hooked, GW2 doesn't use Ex calls
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->SetPixelShader(pShader) :
            this->dev->SetPixelShader(pShader);
    }

    HRESULT LoaderDirect3DDevice9Ex::GetPixelShader(IDirect3DPixelShader9** ppShader) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->GetPixelShader(ppShader) :
            this->dev->GetPixelShader(ppShader);
    }

    HRESULT LoaderDirect3DDevice9Ex::SetPixelShaderConstantF(UINT StartRegister, CONST float* pConstantData, UINT Vector4fCount) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->SetPixelShaderConstantF(StartRegister, pConstantData, Vector4fCount) :
            this->dev->SetPixelShaderConstantF(StartRegister, pConstantData, Vector4fCount);
    }

    HRESULT LoaderDirect3DDevice9Ex::GetPixelShaderConstantF(UINT StartRegister, float* pConstantData, UINT Vector4fCount) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->GetPixelShaderConstantF(StartRegister, pConstantData, Vector4fCount) :
            this->dev->GetPixelShaderConstantF(StartRegister, pConstantData, Vector4fCount);
    }

    HRESULT LoaderDirect3DDevice9Ex::SetPixelShaderConstantI(UINT StartRegister, CONST int* pConstantData, UINT Vector4iCount) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->SetPixelShaderConstantI(StartRegister, pConstantData, Vector4iCount) :
            this->dev->SetPixelShaderConstantI(StartRegister, pConstantData, Vector4iCount);
    }

    HRESULT LoaderDirect3DDevice9Ex::GetPixelShaderConstantI(UINT StartRegister, int* pConstantData, UINT Vector4iCount) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->GetPixelShaderConstantI(StartRegister, pConstantData, Vector4iCount) :
            this->dev->GetPixelShaderConstantI(StartRegister, pConstantData, Vector4iCount);
    }

    HRESULT LoaderDirect3DDevice9Ex::SetPixelShaderConstantB(UINT StartRegister, CONST BOOL* pConstantData, UINT BoolCount) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->SetPixelShaderConstantB(StartRegister, pConstantData, BoolCount) :
            this->dev->SetPixelShaderConstantB(StartRegister, pConstantData, BoolCount);
    }

    HRESULT LoaderDirect3DDevice9Ex::GetPixelShaderConstantB(UINT StartRegister, BOOL* pConstantData, UINT BoolCount) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->GetPixelShaderConstantB(StartRegister, pConstantData, BoolCount) :
            this->dev->GetPixelShaderConstantB(StartRegister, pConstantData, BoolCount);
    }

    HRESULT LoaderDirect3DDevice9Ex::DrawRectPatch(UINT Handle, CONST float* pNumSegs, CONST D3DRECTPATCH_INFO* pRectPatchInfo) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->DrawRectPatch(Handle, pNumSegs, pRectPatchInfo) :
            this->dev->DrawRectPatch(Handle, pNumSegs, pRectPatchInfo);
    }

    HRESULT LoaderDirect3DDevice9Ex::DrawTriPatch(UINT Handle, CONST float* pNumSegs, CONST D3DTRIPATCH_INFO* pTriPatchInfo) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->DrawTriPatch(Handle, pNumSegs, pTriPatchInfo) :
            this->dev->DrawTriPatch(Handle, pNumSegs, pTriPatchInfo);
    }

    HRESULT LoaderDirect3DDevice9Ex::DeletePatch(UINT Handle) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->DeletePatch(Handle) :
            this->dev->DeletePatch(Handle);
    }

    HRESULT LoaderDirect3DDevice9Ex::CreateQuery(D3DQUERYTYPE Type, IDirect3DQuery9** ppQuery) {
        return LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->CreateQuery(Type, ppQuery) :
            this->dev->CreateQuery(Type, ppQuery);
    }

    HRESULT LoaderDirect3DDevice9Ex::SetConvolutionMonoKernel(UINT width, UINT height, float* rows, float* columns) {
        return /*LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->SetConvolutionMonoKernel(width, height, rows, columns) :
            */this->dev->SetConvolutionMonoKernel(width, height, rows, columns);
    }

    HRESULT LoaderDirect3DDevice9Ex::ComposeRects(IDirect3DSurface9* pSrc, IDirect3DSurface9* pDst, IDirect3DVertexBuffer9* pSrcRectDescs, UINT NumRects, IDirect3DVertexBuffer9* pDstRectDescs, D3DCOMPOSERECTSOP Operation, int Xoffset, int Yoffset) {
        return /*LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->ComposeRects(pSrc, pDst, pSrcRectDescs, NumRects, pDstRectDescs, Operation, Xoffset, Yoffset) :
            */this->dev->ComposeRects(pSrc, pDst, pSrcRectDescs, NumRects, pDstRectDescs, Operation, Xoffset, Yoffset);
    }

    HRESULT LoaderDirect3DDevice9Ex::PresentEx(CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion, DWORD dwFlags) {
        // Not hooked, GW2 doesn't use Ex calls
        return /*LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->PresentEx(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion, dwFlags) :
            */this->dev->PresentEx(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion, dwFlags);
    }

    HRESULT LoaderDirect3DDevice9Ex::GetGPUThreadPriority(INT* pPriority) {
        return /*LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->GetGPUThreadPriority(pPriority) :
            */this->dev->GetGPUThreadPriority(pPriority);
    }

    HRESULT LoaderDirect3DDevice9Ex::SetGPUThreadPriority(INT Priority) {
        return /*LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->SetGPUThreadPriority(Priority) :
            */this->dev->SetGPUThreadPriority(Priority);
    }

    HRESULT LoaderDirect3DDevice9Ex::WaitForVBlank(UINT iSwapChain) {
        return /*LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->WaitForVBlank(iSwapChain) :
            */this->dev->WaitForVBlank(iSwapChain);
    }

    HRESULT LoaderDirect3DDevice9Ex::CheckResourceResidency(IDirect3DResource9** pResourceArray, UINT32 NumResources) {
        return /*LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->CheckResourceResidency(pResourceArray, NumResources) :
            */this->dev->CheckResourceResidency(pResourceArray, NumResources);
    }

    HRESULT LoaderDirect3DDevice9Ex::SetMaximumFrameLatency(UINT MaxLatency) {
        return /*LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->SetMaximumFrameLatency(MaxLatency) :
            */this->dev->SetMaximumFrameLatency(MaxLatency);
    }

    HRESULT LoaderDirect3DDevice9Ex::GetMaximumFrameLatency(UINT* pMaxLatency) {
        return /*LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->GetMaximumFrameLatency(pMaxLatency) :
            */this->dev->GetMaximumFrameLatency(pMaxLatency);
    }

    HRESULT LoaderDirect3DDevice9Ex::CheckDeviceState(HWND hDestinationWindow) {
        return /*LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->CheckDeviceState(hDestinationWindow) :
            */this->dev->CheckDeviceState(hDestinationWindow);
    }

    HRESULT LoaderDirect3DDevice9Ex::CreateRenderTargetEx(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Lockable, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle, DWORD Usage) {
        // Not hooked, GW2 doesn't use Ex calls
        return /*LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->CreateRenderTargetEx(Width, Height, Format, MultiSample, MultisampleQuality, Lockable, ppSurface, pSharedHandle, Usage) :
            */this->dev->CreateRenderTargetEx(Width, Height, Format, MultiSample, MultisampleQuality, Lockable, ppSurface, pSharedHandle, Usage);
    }

    HRESULT LoaderDirect3DDevice9Ex::CreateOffscreenPlainSurfaceEx(UINT Width, UINT Height, D3DFORMAT Format, D3DPOOL Pool, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle, DWORD Usage) {
        return /*LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->CreateOffscreenPlainSurfaceEx(Width, Height, Format, Pool, ppSurface, pSharedHandle, Usage) :
            */this->dev->CreateOffscreenPlainSurfaceEx(Width, Height, Format, Pool, ppSurface, pSharedHandle, Usage);
    }

    HRESULT LoaderDirect3DDevice9Ex::CreateDepthStencilSurfaceEx(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Discard, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle, DWORD Usage) {
        return /*LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->CreateDepthStencilSurfaceEx(Width, Height, Format, MultiSample, MultisampleQuality, Discard, ppSurface, pSharedHandle, Usage) :
            */this->dev->CreateDepthStencilSurfaceEx(Width, Height, Format, MultiSample, MultisampleQuality, Discard, ppSurface, pSharedHandle, Usage);
    }

    HRESULT LoaderDirect3DDevice9Ex::ResetEx(D3DPRESENT_PARAMETERS* pPresentationParameters, D3DDISPLAYMODEEX* pFullscreenDisplayMode) {
        // Not hooked, GW2 doesn't use Ex calls
        return /*LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->ResetEx(pPresentationParameters, pFullscreenDisplayMode) :
            */this->dev->ResetEx(pPresentationParameters, pFullscreenDisplayMode);
    }

    HRESULT LoaderDirect3DDevice9Ex::GetDisplayModeEx(UINT iSwapChain, D3DDISPLAYMODEEX* pMode, D3DDISPLAYROTATION* pRotation) {
        return /*LegacyAddonChainDevice != nullptr ?
            LegacyAddonChainDevice->GetDisplayModeEx(iSwapChain, pMode, pRotation) :
            */this->dev->GetDisplayModeEx(iSwapChain, pMode, pRotation);
    }

}
