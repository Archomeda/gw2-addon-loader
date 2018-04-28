#include "ProxyDirect3DDevice9.h"

namespace loader {
    namespace addons {

        HRESULT ProxyDirect3DDevice9::QueryInterface(REFIID riid, void** ppvObj) {
            return this->FunctionAddresses.QueryInterface != nullptr ?
                this->FunctionAddresses.QueryInterface(riid, ppvObj) :
                this->dev->QueryInterface(riid, ppvObj);
        }

        ULONG ProxyDirect3DDevice9::AddRef() {
            return this->FunctionAddresses.AddRef != nullptr ?
                this->FunctionAddresses.AddRef() :
                this->dev->AddRef();
        }

        ULONG ProxyDirect3DDevice9::Release() {
            ULONG count = this->FunctionAddresses.Release != nullptr ?
                this->FunctionAddresses.Release() :
                this->dev->Release();
            if (count == 0) {
                delete this;
            }
            return count;
        }

        HRESULT ProxyDirect3DDevice9::TestCooperativeLevel() {
            return this->FunctionAddresses.TestCooperativeLevel != nullptr ?
                this->FunctionAddresses.TestCooperativeLevel() :
                this->dev->TestCooperativeLevel();
        }

        UINT ProxyDirect3DDevice9::GetAvailableTextureMem() {
            return this->FunctionAddresses.GetAvailableTextureMem != nullptr ?
                this->FunctionAddresses.GetAvailableTextureMem() :
                this->dev->GetAvailableTextureMem();
        }

        HRESULT ProxyDirect3DDevice9::EvictManagedResources() {
            return this->FunctionAddresses.EvictManagedResources != nullptr ?
                this->FunctionAddresses.EvictManagedResources() :
                this->dev->EvictManagedResources();
        }

        HRESULT ProxyDirect3DDevice9::GetDirect3D(IDirect3D9** ppD3D9) {
            return this->FunctionAddresses.GetDirect3D != nullptr ?
                this->FunctionAddresses.GetDirect3D(ppD3D9) :
                this->dev->GetDirect3D(ppD3D9);
        }

        HRESULT ProxyDirect3DDevice9::GetDeviceCaps(D3DCAPS9* pCaps) {
            return this->FunctionAddresses.GetDeviceCaps != nullptr ?
                this->FunctionAddresses.GetDeviceCaps(pCaps) :
                this->dev->GetDeviceCaps(pCaps);
        }

        HRESULT ProxyDirect3DDevice9::GetDisplayMode(UINT iSwapChain, D3DDISPLAYMODE* pMode) {
            return this->FunctionAddresses.GetDisplayMode != nullptr ?
                this->FunctionAddresses.GetDisplayMode(iSwapChain, pMode) :
                this->dev->GetDisplayMode(iSwapChain, pMode);
        }

        HRESULT ProxyDirect3DDevice9::GetCreationParameters(D3DDEVICE_CREATION_PARAMETERS *pParameters) {
            return this->FunctionAddresses.GetCreationParameters != nullptr ?
                this->FunctionAddresses.GetCreationParameters(pParameters) :
                this->dev->GetCreationParameters(pParameters);
        }

        HRESULT ProxyDirect3DDevice9::SetCursorProperties(UINT XHotSpot, UINT YHotSpot, IDirect3DSurface9* pCursorBitmap) {
            return this->FunctionAddresses.SetCursorProperties != nullptr ?
                this->FunctionAddresses.SetCursorProperties(XHotSpot, YHotSpot, pCursorBitmap) :
                this->dev->SetCursorProperties(XHotSpot, YHotSpot, pCursorBitmap);
        }

        void ProxyDirect3DDevice9::SetCursorPosition(int X, int Y, DWORD Flags) {
            return this->FunctionAddresses.SetCursorPosition != nullptr ?
                this->FunctionAddresses.SetCursorPosition(X, Y, Flags) :
                this->dev->SetCursorPosition(X, Y, Flags);
        }

        BOOL ProxyDirect3DDevice9::ShowCursor(BOOL bShow) {
            return this->FunctionAddresses.ShowCursor != nullptr ?
                this->FunctionAddresses.ShowCursor(bShow) :
                this->dev->ShowCursor(bShow);
        }

        HRESULT ProxyDirect3DDevice9::CreateAdditionalSwapChain(D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DSwapChain9** pSwapChain) {
            return this->FunctionAddresses.CreateAdditionalSwapChain != nullptr ?
                this->FunctionAddresses.CreateAdditionalSwapChain(pPresentationParameters, pSwapChain) :
                this->dev->CreateAdditionalSwapChain(pPresentationParameters, pSwapChain);
        }

        HRESULT ProxyDirect3DDevice9::GetSwapChain(UINT iSwapChain, IDirect3DSwapChain9** pSwapChain) {
            return this->FunctionAddresses.GetSwapChain != nullptr ?
                this->FunctionAddresses.GetSwapChain(iSwapChain, pSwapChain) :
                this->dev->GetSwapChain(iSwapChain, pSwapChain);
        }

        UINT ProxyDirect3DDevice9::GetNumberOfSwapChains() {
            return this->FunctionAddresses.GetNumberOfSwapChains != nullptr ?
                this->FunctionAddresses.GetNumberOfSwapChains() :
                this->dev->GetNumberOfSwapChains();
        }

        HRESULT ProxyDirect3DDevice9::Reset(D3DPRESENT_PARAMETERS* pPresentationParameters) {
            return this->FunctionAddresses.Reset != nullptr ?
                this->FunctionAddresses.Reset(pPresentationParameters) :
                this->dev->Reset(pPresentationParameters);
        }

        HRESULT ProxyDirect3DDevice9::Present(CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion) {
            return this->FunctionAddresses.Present != nullptr ?
                this->FunctionAddresses.Present(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion) :
                this->dev->Present(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
        }

        HRESULT ProxyDirect3DDevice9::GetBackBuffer(UINT iSwapChain, UINT iBackBuffer, D3DBACKBUFFER_TYPE Type, IDirect3DSurface9** ppBackBuffer) {
            return this->FunctionAddresses.GetBackBuffer != nullptr ?
                this->FunctionAddresses.GetBackBuffer(iSwapChain, iBackBuffer, Type, ppBackBuffer) :
                this->dev->GetBackBuffer(iSwapChain, iBackBuffer, Type, ppBackBuffer);
        }

        HRESULT ProxyDirect3DDevice9::GetRasterStatus(UINT iSwapChain, D3DRASTER_STATUS* pRasterStatus) {
            return this->FunctionAddresses.GetRasterStatus != nullptr ?
                this->FunctionAddresses.GetRasterStatus(iSwapChain, pRasterStatus) :
                this->dev->GetRasterStatus(iSwapChain, pRasterStatus);
        }

        HRESULT ProxyDirect3DDevice9::SetDialogBoxMode(BOOL bEnableDialogs) {
            return this->FunctionAddresses.SetDialogBoxMode != nullptr ?
                this->FunctionAddresses.SetDialogBoxMode(bEnableDialogs) :
                this->dev->SetDialogBoxMode(bEnableDialogs);
        }

        void ProxyDirect3DDevice9::SetGammaRamp(UINT iSwapChain, DWORD Flags, CONST D3DGAMMARAMP* pRamp) {
            return this->FunctionAddresses.SetGammaRamp != nullptr ?
                this->FunctionAddresses.SetGammaRamp(iSwapChain, Flags, pRamp) :
                this->dev->SetGammaRamp(iSwapChain, Flags, pRamp);
        }

        void ProxyDirect3DDevice9::GetGammaRamp(UINT iSwapChain, D3DGAMMARAMP* pRamp) {
            return this->FunctionAddresses.GetGammaRamp != nullptr ?
                this->FunctionAddresses.GetGammaRamp(iSwapChain, pRamp) :
                this->dev->GetGammaRamp(iSwapChain, pRamp);
        }

        HRESULT ProxyDirect3DDevice9::CreateTexture(UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DTexture9** ppTexture, HANDLE* pSharedHandle) {
            return this->FunctionAddresses.CreateTexture != nullptr ?
                this->FunctionAddresses.CreateTexture(Width, Height, Levels, Usage, Format, Pool, ppTexture, pSharedHandle) :
                this->dev->CreateTexture(Width, Height, Levels, Usage, Format, Pool, ppTexture, pSharedHandle);
        }

        HRESULT ProxyDirect3DDevice9::CreateVolumeTexture(UINT Width, UINT Height, UINT Depth, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DVolumeTexture9** ppVolumeTexture, HANDLE* pSharedHandle) {
            return this->FunctionAddresses.CreateVolumeTexture != nullptr ?
                this->FunctionAddresses.CreateVolumeTexture(Width, Height, Depth, Levels, Usage, Format, Pool, ppVolumeTexture, pSharedHandle) :
                this->dev->CreateVolumeTexture(Width, Height, Depth, Levels, Usage, Format, Pool, ppVolumeTexture, pSharedHandle);
        }

        HRESULT ProxyDirect3DDevice9::CreateCubeTexture(UINT EdgeLength, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DCubeTexture9** ppCubeTexture, HANDLE* pSharedHandle) {
            return this->FunctionAddresses.CreateCubeTexture != nullptr ?
                this->FunctionAddresses.CreateCubeTexture(EdgeLength, Levels, Usage, Format, Pool, ppCubeTexture, pSharedHandle) :
                this->dev->CreateCubeTexture(EdgeLength, Levels, Usage, Format, Pool, ppCubeTexture, pSharedHandle);
        }

        HRESULT ProxyDirect3DDevice9::CreateVertexBuffer(UINT Length, DWORD Usage, DWORD FVF, D3DPOOL Pool, IDirect3DVertexBuffer9** ppVertexBuffer, HANDLE* pSharedHandle) {
            return this->FunctionAddresses.CreateVertexBuffer != nullptr ?
                this->FunctionAddresses.CreateVertexBuffer(Length, Usage, FVF, Pool, ppVertexBuffer, pSharedHandle) :
                this->dev->CreateVertexBuffer(Length, Usage, FVF, Pool, ppVertexBuffer, pSharedHandle);
        }

        HRESULT ProxyDirect3DDevice9::CreateIndexBuffer(UINT Length, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DIndexBuffer9** ppIndexBuffer, HANDLE* pSharedHandle) {
            return this->FunctionAddresses.CreateIndexBuffer != nullptr ?
                this->FunctionAddresses.CreateIndexBuffer(Length, Usage, Format, Pool, ppIndexBuffer, pSharedHandle) :
                this->dev->CreateIndexBuffer(Length, Usage, Format, Pool, ppIndexBuffer, pSharedHandle);
        }

        HRESULT ProxyDirect3DDevice9::CreateRenderTarget(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Lockable, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle) {
            return this->FunctionAddresses.CreateRenderTarget != nullptr ?
                this->FunctionAddresses.CreateRenderTarget(Width, Height, Format, MultiSample, MultisampleQuality, Lockable, ppSurface, pSharedHandle) :
                this->dev->CreateRenderTarget(Width, Height, Format, MultiSample, MultisampleQuality, Lockable, ppSurface, pSharedHandle);
        }

        HRESULT ProxyDirect3DDevice9::CreateDepthStencilSurface(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Discard, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle) {
            return this->FunctionAddresses.CreateDepthStencilSurface != nullptr ?
                this->FunctionAddresses.CreateDepthStencilSurface(Width, Height, Format, MultiSample, MultisampleQuality, Discard, ppSurface, pSharedHandle) :
                this->dev->CreateDepthStencilSurface(Width, Height, Format, MultiSample, MultisampleQuality, Discard, ppSurface, pSharedHandle);
        }

        HRESULT ProxyDirect3DDevice9::UpdateSurface(IDirect3DSurface9* pSourceSurface, CONST RECT* pSourceRect, IDirect3DSurface9* pDestinationSurface, CONST POINT* pDestPoint) {
            return this->FunctionAddresses.UpdateSurface != nullptr ?
                this->FunctionAddresses.UpdateSurface(pSourceSurface, pSourceRect, pDestinationSurface, pDestPoint) :
                this->dev->UpdateSurface(pSourceSurface, pSourceRect, pDestinationSurface, pDestPoint);
        }

        HRESULT ProxyDirect3DDevice9::UpdateTexture(IDirect3DBaseTexture9* pSourceTexture, IDirect3DBaseTexture9* pDestinationTexture) {
            return this->FunctionAddresses.UpdateTexture != nullptr ?
                this->FunctionAddresses.UpdateTexture(pSourceTexture, pDestinationTexture) :
                this->dev->UpdateTexture(pSourceTexture, pDestinationTexture);
        }

        HRESULT ProxyDirect3DDevice9::GetRenderTargetData(IDirect3DSurface9* pRenderTarget, IDirect3DSurface9* pDestSurface) {
            return this->FunctionAddresses.GetRenderTargetData != nullptr ?
                this->FunctionAddresses.GetRenderTargetData(pRenderTarget, pDestSurface) :
                this->dev->GetRenderTargetData(pRenderTarget, pDestSurface);
        }

        HRESULT ProxyDirect3DDevice9::GetFrontBufferData(UINT iSwapChain, IDirect3DSurface9* pDestSurface) {
            return this->FunctionAddresses.GetFrontBufferData != nullptr ?
                this->FunctionAddresses.GetFrontBufferData(iSwapChain, pDestSurface) :
                this->dev->GetFrontBufferData(iSwapChain, pDestSurface);
        }

        HRESULT ProxyDirect3DDevice9::StretchRect(IDirect3DSurface9* pSourceSurface, CONST RECT* pSourceRect, IDirect3DSurface9* pDestSurface, CONST RECT* pDestRect, D3DTEXTUREFILTERTYPE Filter) {
            return this->FunctionAddresses.StretchRect != nullptr ?
                this->FunctionAddresses.StretchRect(pSourceSurface, pSourceRect, pDestSurface, pDestRect, Filter) :
                this->dev->StretchRect(pSourceSurface, pSourceRect, pDestSurface, pDestRect, Filter);
        }

        HRESULT ProxyDirect3DDevice9::ColorFill(IDirect3DSurface9* pSurface, CONST RECT* pRect, D3DCOLOR color) {
            return this->FunctionAddresses.ColorFill != nullptr ?
                this->FunctionAddresses.ColorFill(pSurface, pRect, color) :
                this->dev->ColorFill(pSurface, pRect, color);
        }

        HRESULT ProxyDirect3DDevice9::CreateOffscreenPlainSurface(UINT Width, UINT Height, D3DFORMAT Format, D3DPOOL Pool, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle) {
            return this->FunctionAddresses.CreateOffscreenPlainSurface != nullptr ?
                this->FunctionAddresses.CreateOffscreenPlainSurface(Width, Height, Format, Pool, ppSurface, pSharedHandle) :
                this->dev->CreateOffscreenPlainSurface(Width, Height, Format, Pool, ppSurface, pSharedHandle);
        }

        HRESULT ProxyDirect3DDevice9::SetRenderTarget(DWORD RenderTargetIndex, IDirect3DSurface9* pRenderTarget) {
            return this->FunctionAddresses.SetRenderTarget != nullptr ?
                this->FunctionAddresses.SetRenderTarget(RenderTargetIndex, pRenderTarget) :
                this->dev->SetRenderTarget(RenderTargetIndex, pRenderTarget);
        }

        HRESULT ProxyDirect3DDevice9::GetRenderTarget(DWORD RenderTargetIndex, IDirect3DSurface9** ppRenderTarget) {
            return this->FunctionAddresses.GetRenderTarget != nullptr ?
                this->FunctionAddresses.GetRenderTarget(RenderTargetIndex, ppRenderTarget) :
                this->dev->GetRenderTarget(RenderTargetIndex, ppRenderTarget);
        }

        HRESULT ProxyDirect3DDevice9::SetDepthStencilSurface(IDirect3DSurface9* pNewZStencil) {
            return this->FunctionAddresses.SetDepthStencilSurface != nullptr ?
                this->FunctionAddresses.SetDepthStencilSurface(pNewZStencil) :
                this->dev->SetDepthStencilSurface(pNewZStencil);
        }

        HRESULT ProxyDirect3DDevice9::GetDepthStencilSurface(IDirect3DSurface9** ppZStencilSurface) {
            return this->FunctionAddresses.GetDepthStencilSurface != nullptr ?
                this->FunctionAddresses.GetDepthStencilSurface(ppZStencilSurface) :
                this->dev->GetDepthStencilSurface(ppZStencilSurface);
        }

        HRESULT ProxyDirect3DDevice9::BeginScene() {
            return this->FunctionAddresses.BeginScene != nullptr ?
                this->FunctionAddresses.BeginScene() :
                this->dev->BeginScene();
        }

        HRESULT ProxyDirect3DDevice9::EndScene() {
            return this->FunctionAddresses.EndScene != nullptr ?
                this->FunctionAddresses.EndScene() :
                this->dev->EndScene();
        }

        HRESULT ProxyDirect3DDevice9::Clear(DWORD Count, CONST D3DRECT* pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil) {
            return this->FunctionAddresses.Clear != nullptr ?
                this->FunctionAddresses.Clear(Count, pRects, Flags, Color, Z, Stencil) :
                this->dev->Clear(Count, pRects, Flags, Color, Z, Stencil);
        }

        HRESULT ProxyDirect3DDevice9::SetTransform(D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX* pMatrix) {
            return this->FunctionAddresses.SetTransform != nullptr ?
                this->FunctionAddresses.SetTransform(State, pMatrix) :
                this->dev->SetTransform(State, pMatrix);
        }

        HRESULT ProxyDirect3DDevice9::GetTransform(D3DTRANSFORMSTATETYPE State, D3DMATRIX* pMatrix) {
            return this->FunctionAddresses.GetTransform != nullptr ?
                this->FunctionAddresses.GetTransform(State, pMatrix) :
                this->dev->GetTransform(State, pMatrix);
        }

        HRESULT ProxyDirect3DDevice9::MultiplyTransform(D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX* pMatrix) {
            return this->FunctionAddresses.MultiplyTransform != nullptr ?
                this->FunctionAddresses.MultiplyTransform(State, pMatrix) :
                this->dev->MultiplyTransform(State, pMatrix);
        }

        HRESULT ProxyDirect3DDevice9::SetViewport(CONST D3DVIEWPORT9* pViewport) {
            return this->FunctionAddresses.SetViewport != nullptr ?
                this->FunctionAddresses.SetViewport(pViewport) :
                this->dev->SetViewport(pViewport);
        }

        HRESULT ProxyDirect3DDevice9::GetViewport(D3DVIEWPORT9* pViewport) {
            return this->FunctionAddresses.GetViewport != nullptr ?
                this->FunctionAddresses.GetViewport(pViewport) :
                this->dev->GetViewport(pViewport);
        }

        HRESULT ProxyDirect3DDevice9::SetMaterial(CONST D3DMATERIAL9* pMaterial) {
            return this->FunctionAddresses.SetMaterial != nullptr ?
                this->FunctionAddresses.SetMaterial(pMaterial) :
                this->dev->SetMaterial(pMaterial);
        }

        HRESULT ProxyDirect3DDevice9::GetMaterial(D3DMATERIAL9* pMaterial) {
            return this->FunctionAddresses.GetMaterial != nullptr ?
                this->FunctionAddresses.GetMaterial(pMaterial) :
                this->dev->GetMaterial(pMaterial);
        }

        HRESULT ProxyDirect3DDevice9::SetLight(DWORD Index, CONST D3DLIGHT9* pLight) {
            return this->FunctionAddresses.SetLight != nullptr ?
                this->FunctionAddresses.SetLight(Index, pLight) :
                this->dev->SetLight(Index, pLight);
        }

        HRESULT ProxyDirect3DDevice9::GetLight(DWORD Index, D3DLIGHT9* pLight) {
            return this->FunctionAddresses.GetLight != nullptr ?
                this->FunctionAddresses.GetLight(Index, pLight) :
                this->dev->GetLight(Index, pLight);
        }

        HRESULT ProxyDirect3DDevice9::LightEnable(DWORD Index, BOOL Enable) {
            return this->FunctionAddresses.LightEnable != nullptr ?
                this->FunctionAddresses.LightEnable(Index, Enable) :
                this->dev->LightEnable(Index, Enable);
        }

        HRESULT ProxyDirect3DDevice9::GetLightEnable(DWORD Index, BOOL* pEnable) {
            return this->FunctionAddresses.GetLightEnable != nullptr ?
                this->FunctionAddresses.GetLightEnable(Index, pEnable) :
                this->dev->GetLightEnable(Index, pEnable);
        }

        HRESULT ProxyDirect3DDevice9::SetClipPlane(DWORD Index, CONST float* pPlane) {
            return this->FunctionAddresses.SetClipPlane != nullptr ?
                this->FunctionAddresses.SetClipPlane(Index, pPlane) :
                this->dev->SetClipPlane(Index, pPlane);
        }

        HRESULT ProxyDirect3DDevice9::GetClipPlane(DWORD Index, float* pPlane) {
            return this->FunctionAddresses.GetClipPlane != nullptr ?
                this->FunctionAddresses.GetClipPlane(Index, pPlane) :
                this->dev->GetClipPlane(Index, pPlane);
        }

        HRESULT ProxyDirect3DDevice9::SetRenderState(D3DRENDERSTATETYPE State, DWORD Value) {
            return this->FunctionAddresses.SetRenderState != nullptr ?
                this->FunctionAddresses.SetRenderState(State, Value) :
                this->dev->SetRenderState(State, Value);
        }

        HRESULT ProxyDirect3DDevice9::GetRenderState(D3DRENDERSTATETYPE State, DWORD* pValue) {
            return this->FunctionAddresses.GetRenderState != nullptr ?
                this->FunctionAddresses.GetRenderState(State, pValue) :
                this->dev->GetRenderState(State, pValue);
        }

        HRESULT ProxyDirect3DDevice9::CreateStateBlock(D3DSTATEBLOCKTYPE Type, IDirect3DStateBlock9** ppSB) {
            return this->FunctionAddresses.CreateStateBlock != nullptr ?
                this->FunctionAddresses.CreateStateBlock(Type, ppSB) :
                this->dev->CreateStateBlock(Type, ppSB);
        }

        HRESULT ProxyDirect3DDevice9::BeginStateBlock() {
            return this->FunctionAddresses.BeginStateBlock != nullptr ?
                this->FunctionAddresses.BeginStateBlock() :
                this->dev->BeginStateBlock();
        }

        HRESULT ProxyDirect3DDevice9::EndStateBlock(IDirect3DStateBlock9** ppSB) {
            return this->FunctionAddresses.EndStateBlock != nullptr ?
                this->FunctionAddresses.EndStateBlock(ppSB) :
                this->dev->EndStateBlock(ppSB);
        }

        HRESULT ProxyDirect3DDevice9::SetClipStatus(CONST D3DCLIPSTATUS9* pClipStatus) {
            return this->FunctionAddresses.SetClipStatus != nullptr ?
                this->FunctionAddresses.SetClipStatus(pClipStatus) :
                this->dev->SetClipStatus(pClipStatus);
        }

        HRESULT ProxyDirect3DDevice9::GetClipStatus(D3DCLIPSTATUS9* pClipStatus) {
            return this->FunctionAddresses.GetClipStatus != nullptr ?
                this->FunctionAddresses.GetClipStatus(pClipStatus) :
                this->dev->GetClipStatus(pClipStatus);
        }

        HRESULT ProxyDirect3DDevice9::GetTexture(DWORD Stage, IDirect3DBaseTexture9** ppTexture) {
            return this->FunctionAddresses.GetTexture != nullptr ?
                this->FunctionAddresses.GetTexture(Stage, ppTexture) :
                this->dev->GetTexture(Stage, ppTexture);
        }

        HRESULT ProxyDirect3DDevice9::SetTexture(DWORD Stage, IDirect3DBaseTexture9* pTexture) {
            return this->FunctionAddresses.SetTexture != nullptr ?
                this->FunctionAddresses.SetTexture(Stage, pTexture) :
                this->dev->SetTexture(Stage, pTexture);
        }

        HRESULT ProxyDirect3DDevice9::GetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD* pValue) {
            return this->FunctionAddresses.GetTextureStageState != nullptr ?
                this->FunctionAddresses.GetTextureStageState(Stage, Type, pValue) :
                this->dev->GetTextureStageState(Stage, Type, pValue);
        }

        HRESULT ProxyDirect3DDevice9::SetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value) {
            return this->FunctionAddresses.SetTextureStageState != nullptr ?
                this->FunctionAddresses.SetTextureStageState(Stage, Type, Value) :
                this->dev->SetTextureStageState(Stage, Type, Value);
        }

        HRESULT ProxyDirect3DDevice9::GetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD* pValue) {
            return this->FunctionAddresses.GetSamplerState != nullptr ?
                this->FunctionAddresses.GetSamplerState(Sampler, Type, pValue) :
                this->dev->GetSamplerState(Sampler, Type, pValue);
        }

        HRESULT ProxyDirect3DDevice9::SetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD Value) {
            return this->FunctionAddresses.SetSamplerState != nullptr ?
                this->FunctionAddresses.SetSamplerState(Sampler, Type, Value) :
                this->dev->SetSamplerState(Sampler, Type, Value);
        }

        HRESULT ProxyDirect3DDevice9::ValidateDevice(DWORD* pNumPasses) {
            return this->FunctionAddresses.ValidateDevice != nullptr ?
                this->FunctionAddresses.ValidateDevice(pNumPasses) :
                this->dev->ValidateDevice(pNumPasses);
        }

        HRESULT ProxyDirect3DDevice9::SetPaletteEntries(UINT PaletteNumber, CONST PALETTEENTRY* pEntries) {
            return this->FunctionAddresses.SetPaletteEntries != nullptr ?
                this->FunctionAddresses.SetPaletteEntries(PaletteNumber, pEntries) :
                this->dev->SetPaletteEntries(PaletteNumber, pEntries);
        }

        HRESULT ProxyDirect3DDevice9::GetPaletteEntries(UINT PaletteNumber, PALETTEENTRY* pEntries) {
            return this->FunctionAddresses.GetPaletteEntries != nullptr ?
                this->FunctionAddresses.GetPaletteEntries(PaletteNumber, pEntries) :
                this->dev->GetPaletteEntries(PaletteNumber, pEntries);
        }

        HRESULT ProxyDirect3DDevice9::SetCurrentTexturePalette(UINT PaletteNumber) {
            return this->FunctionAddresses.SetCurrentTexturePalette != nullptr ?
                this->FunctionAddresses.SetCurrentTexturePalette(PaletteNumber) :
                this->dev->SetCurrentTexturePalette(PaletteNumber);
        }

        HRESULT ProxyDirect3DDevice9::GetCurrentTexturePalette(UINT *PaletteNumber) {
            return this->FunctionAddresses.GetCurrentTexturePalette != nullptr ?
                this->FunctionAddresses.GetCurrentTexturePalette(PaletteNumber) :
                this->dev->GetCurrentTexturePalette(PaletteNumber);
        }

        HRESULT ProxyDirect3DDevice9::SetScissorRect(CONST RECT* pRect) {
            return this->FunctionAddresses.SetScissorRect != nullptr ?
                this->FunctionAddresses.SetScissorRect(pRect) :
                this->dev->SetScissorRect(pRect);
        }

        HRESULT ProxyDirect3DDevice9::GetScissorRect(RECT* pRect) {
            return this->FunctionAddresses.GetScissorRect != nullptr ?
                this->FunctionAddresses.GetScissorRect(pRect) :
                this->dev->GetScissorRect(pRect);
        }

        HRESULT ProxyDirect3DDevice9::SetSoftwareVertexProcessing(BOOL bSoftware) {
            return this->FunctionAddresses.SetSoftwareVertexProcessing != nullptr ?
                this->FunctionAddresses.SetSoftwareVertexProcessing(bSoftware) :
                this->dev->SetSoftwareVertexProcessing(bSoftware);
        }

        BOOL ProxyDirect3DDevice9::GetSoftwareVertexProcessing() {
            return this->FunctionAddresses.GetSoftwareVertexProcessing != nullptr ?
                this->FunctionAddresses.GetSoftwareVertexProcessing() :
                this->dev->GetSoftwareVertexProcessing();
        }

        HRESULT ProxyDirect3DDevice9::SetNPatchMode(float nSegments) {
            return this->FunctionAddresses.SetNPatchMode != nullptr ?
                this->FunctionAddresses.SetNPatchMode(nSegments) :
                this->dev->SetNPatchMode(nSegments);
        }

        float ProxyDirect3DDevice9::GetNPatchMode() {
            return this->FunctionAddresses.GetNPatchMode != nullptr ?
                this->FunctionAddresses.GetNPatchMode() :
                this->dev->GetNPatchMode();
        }

        HRESULT ProxyDirect3DDevice9::DrawPrimitive(D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount) {
            return this->FunctionAddresses.DrawPrimitive != nullptr ?
                this->FunctionAddresses.DrawPrimitive(PrimitiveType, StartVertex, PrimitiveCount) :
                this->dev->DrawPrimitive(PrimitiveType, StartVertex, PrimitiveCount);
        }

        HRESULT ProxyDirect3DDevice9::DrawIndexedPrimitive(D3DPRIMITIVETYPE PrimitiveType, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount) {
            return this->FunctionAddresses.DrawIndexedPrimitive != nullptr ?
                this->FunctionAddresses.DrawIndexedPrimitive(PrimitiveType, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount) :
                this->dev->DrawIndexedPrimitive(PrimitiveType, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount);
        }

        HRESULT ProxyDirect3DDevice9::DrawPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT PrimitiveCount, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride) {
            return this->FunctionAddresses.DrawPrimitiveUP != nullptr ?
                this->FunctionAddresses.DrawPrimitiveUP(PrimitiveType, PrimitiveCount, pVertexStreamZeroData, VertexStreamZeroStride) :
                this->dev->DrawPrimitiveUP(PrimitiveType, PrimitiveCount, pVertexStreamZeroData, VertexStreamZeroStride);
        }

        HRESULT ProxyDirect3DDevice9::DrawIndexedPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT MinVertexIndex, UINT NumVertices, UINT PrimitiveCount, CONST void* pIndexData, D3DFORMAT IndexDataFormat, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride) {
            return this->FunctionAddresses.DrawIndexedPrimitiveUP != nullptr ?
                this->FunctionAddresses.DrawIndexedPrimitiveUP(PrimitiveType, MinVertexIndex, NumVertices, PrimitiveCount, pIndexData, IndexDataFormat, pVertexStreamZeroData, VertexStreamZeroStride) :
                this->dev->DrawIndexedPrimitiveUP(PrimitiveType, MinVertexIndex, NumVertices, PrimitiveCount, pIndexData, IndexDataFormat, pVertexStreamZeroData, VertexStreamZeroStride);
        }

        HRESULT ProxyDirect3DDevice9::ProcessVertices(UINT SrcStartIndex, UINT DestIndex, UINT VertexCount, IDirect3DVertexBuffer9* pDestBuffer, IDirect3DVertexDeclaration9* pVertexDecl, DWORD Flags) {
            return this->FunctionAddresses.ProcessVertices != nullptr ?
                this->FunctionAddresses.ProcessVertices(SrcStartIndex, DestIndex, VertexCount, pDestBuffer, pVertexDecl, Flags) :
                this->dev->ProcessVertices(SrcStartIndex, DestIndex, VertexCount, pDestBuffer, pVertexDecl, Flags);
        }

        HRESULT ProxyDirect3DDevice9::CreateVertexDeclaration(CONST D3DVERTEXELEMENT9* pVertexElements, IDirect3DVertexDeclaration9** ppDecl) {
            return this->FunctionAddresses.CreateVertexDeclaration != nullptr ?
                this->FunctionAddresses.CreateVertexDeclaration(pVertexElements, ppDecl) :
                this->dev->CreateVertexDeclaration(pVertexElements, ppDecl);
        }

        HRESULT ProxyDirect3DDevice9::SetVertexDeclaration(IDirect3DVertexDeclaration9* pDecl) {
            return this->FunctionAddresses.SetVertexDeclaration != nullptr ?
                this->FunctionAddresses.SetVertexDeclaration(pDecl) :
                this->dev->SetVertexDeclaration(pDecl);
        }

        HRESULT ProxyDirect3DDevice9::GetVertexDeclaration(IDirect3DVertexDeclaration9** ppDecl) {
            return this->FunctionAddresses.GetVertexDeclaration != nullptr ?
                this->FunctionAddresses.GetVertexDeclaration(ppDecl) :
                this->dev->GetVertexDeclaration(ppDecl);
        }

        HRESULT ProxyDirect3DDevice9::SetFVF(DWORD FVF) {
            return this->FunctionAddresses.SetFVF != nullptr ?
                this->FunctionAddresses.SetFVF(FVF) :
                this->dev->SetFVF(FVF);
        }

        HRESULT ProxyDirect3DDevice9::GetFVF(DWORD* pFVF) {
            return this->FunctionAddresses.GetFVF != nullptr ?
                this->FunctionAddresses.GetFVF(pFVF) :
                this->dev->GetFVF(pFVF);
        }

        HRESULT ProxyDirect3DDevice9::CreateVertexShader(CONST DWORD* pFunction, IDirect3DVertexShader9** ppShader) {
            return this->FunctionAddresses.CreateVertexShader != nullptr ?
                this->FunctionAddresses.CreateVertexShader(pFunction, ppShader) :
                this->dev->CreateVertexShader(pFunction, ppShader);
        }

        HRESULT ProxyDirect3DDevice9::SetVertexShader(IDirect3DVertexShader9* pShader) {
            return this->FunctionAddresses.SetVertexShader != nullptr ?
                this->FunctionAddresses.SetVertexShader(pShader) :
                this->dev->SetVertexShader(pShader);
        }

        HRESULT ProxyDirect3DDevice9::GetVertexShader(IDirect3DVertexShader9** ppShader) {
            return this->FunctionAddresses.GetVertexShader != nullptr ?
                this->FunctionAddresses.GetVertexShader(ppShader) :
                this->dev->GetVertexShader(ppShader);
        }

        HRESULT ProxyDirect3DDevice9::SetVertexShaderConstantF(UINT StartRegister, CONST float* pConstantData, UINT Vector4fCount) {
            return this->FunctionAddresses.SetVertexShaderConstantF != nullptr ?
                this->FunctionAddresses.SetVertexShaderConstantF(StartRegister, pConstantData, Vector4fCount) :
                this->dev->SetVertexShaderConstantF(StartRegister, pConstantData, Vector4fCount);
        }

        HRESULT ProxyDirect3DDevice9::GetVertexShaderConstantF(UINT StartRegister, float* pConstantData, UINT Vector4fCount) {
            return this->FunctionAddresses.GetVertexShaderConstantF != nullptr ?
                this->FunctionAddresses.GetVertexShaderConstantF(StartRegister, pConstantData, Vector4fCount) :
                this->dev->GetVertexShaderConstantF(StartRegister, pConstantData, Vector4fCount);
        }

        HRESULT ProxyDirect3DDevice9::SetVertexShaderConstantI(UINT StartRegister, CONST int* pConstantData, UINT Vector4iCount) {
            return this->FunctionAddresses.SetVertexShaderConstantI != nullptr ?
                this->FunctionAddresses.SetVertexShaderConstantI(StartRegister, pConstantData, Vector4iCount) :
                this->dev->SetVertexShaderConstantI(StartRegister, pConstantData, Vector4iCount);
        }

        HRESULT ProxyDirect3DDevice9::GetVertexShaderConstantI(UINT StartRegister, int* pConstantData, UINT Vector4iCount) {
            return this->FunctionAddresses.GetVertexShaderConstantI != nullptr ?
                this->FunctionAddresses.GetVertexShaderConstantI(StartRegister, pConstantData, Vector4iCount) :
                this->dev->GetVertexShaderConstantI(StartRegister, pConstantData, Vector4iCount);
        }

        HRESULT ProxyDirect3DDevice9::SetVertexShaderConstantB(UINT StartRegister, CONST BOOL* pConstantData, UINT BoolCount) {
            return this->FunctionAddresses.SetVertexShaderConstantB != nullptr ?
                this->FunctionAddresses.SetVertexShaderConstantB(StartRegister, pConstantData, BoolCount) :
                this->dev->SetVertexShaderConstantB(StartRegister, pConstantData, BoolCount);
        }

        HRESULT ProxyDirect3DDevice9::GetVertexShaderConstantB(UINT StartRegister, BOOL* pConstantData, UINT BoolCount) {
            return this->FunctionAddresses.GetVertexShaderConstantB != nullptr ?
                this->FunctionAddresses.GetVertexShaderConstantB(StartRegister, pConstantData, BoolCount) :
                this->dev->GetVertexShaderConstantB(StartRegister, pConstantData, BoolCount);
        }

        HRESULT ProxyDirect3DDevice9::SetStreamSource(UINT StreamNumber, IDirect3DVertexBuffer9* pStreamData, UINT OffsetInBytes, UINT Stride) {
            return this->FunctionAddresses.SetStreamSource != nullptr ?
                this->FunctionAddresses.SetStreamSource(StreamNumber, pStreamData, OffsetInBytes, Stride) :
                this->dev->SetStreamSource(StreamNumber, pStreamData, OffsetInBytes, Stride);
        }

        HRESULT ProxyDirect3DDevice9::GetStreamSource(UINT StreamNumber, IDirect3DVertexBuffer9** ppStreamData, UINT* pOffsetInBytes, UINT* pStride) {
            return this->FunctionAddresses.GetStreamSource != nullptr ?
                this->FunctionAddresses.GetStreamSource(StreamNumber, ppStreamData, pOffsetInBytes, pStride) :
                this->dev->GetStreamSource(StreamNumber, ppStreamData, pOffsetInBytes, pStride);
        }

        HRESULT ProxyDirect3DDevice9::SetStreamSourceFreq(UINT StreamNumber, UINT Setting) {
            return this->FunctionAddresses.SetStreamSourceFreq != nullptr ?
                this->FunctionAddresses.SetStreamSourceFreq(StreamNumber, Setting) :
                this->dev->SetStreamSourceFreq(StreamNumber, Setting);
        }

        HRESULT ProxyDirect3DDevice9::GetStreamSourceFreq(UINT StreamNumber, UINT* pSetting) {
            return this->FunctionAddresses.GetStreamSourceFreq != nullptr ?
                this->FunctionAddresses.GetStreamSourceFreq(StreamNumber, pSetting) :
                this->dev->GetStreamSourceFreq(StreamNumber, pSetting);
        }

        HRESULT ProxyDirect3DDevice9::SetIndices(IDirect3DIndexBuffer9* pIndexData) {
            return this->FunctionAddresses.SetIndices != nullptr ?
                this->FunctionAddresses.SetIndices(pIndexData) :
                this->dev->SetIndices(pIndexData);
        }

        HRESULT ProxyDirect3DDevice9::GetIndices(IDirect3DIndexBuffer9** ppIndexData) {
            return this->FunctionAddresses.GetIndices != nullptr ?
                this->FunctionAddresses.GetIndices(ppIndexData) :
                this->dev->GetIndices(ppIndexData);
        }

        HRESULT ProxyDirect3DDevice9::CreatePixelShader(CONST DWORD* pFunction, IDirect3DPixelShader9** ppShader) {
            return this->FunctionAddresses.CreatePixelShader != nullptr ?
                this->FunctionAddresses.CreatePixelShader(pFunction, ppShader) :
                this->dev->CreatePixelShader(pFunction, ppShader);
        }

        HRESULT ProxyDirect3DDevice9::SetPixelShader(IDirect3DPixelShader9* pShader) {
            return this->FunctionAddresses.SetPixelShader != nullptr ?
                this->FunctionAddresses.SetPixelShader(pShader) :
                this->dev->SetPixelShader(pShader);
        }

        HRESULT ProxyDirect3DDevice9::GetPixelShader(IDirect3DPixelShader9** ppShader) {
            return this->FunctionAddresses.GetPixelShader != nullptr ?
                this->FunctionAddresses.GetPixelShader(ppShader) :
                this->dev->GetPixelShader(ppShader);
        }

        HRESULT ProxyDirect3DDevice9::SetPixelShaderConstantF(UINT StartRegister, CONST float* pConstantData, UINT Vector4fCount) {
            return this->FunctionAddresses.SetPixelShaderConstantF != nullptr ?
                this->FunctionAddresses.SetPixelShaderConstantF(StartRegister, pConstantData, Vector4fCount) :
                this->dev->SetPixelShaderConstantF(StartRegister, pConstantData, Vector4fCount);
        }

        HRESULT ProxyDirect3DDevice9::GetPixelShaderConstantF(UINT StartRegister, float* pConstantData, UINT Vector4fCount) {
            return this->FunctionAddresses.GetPixelShaderConstantF != nullptr ?
                this->FunctionAddresses.GetPixelShaderConstantF(StartRegister, pConstantData, Vector4fCount) :
                this->dev->GetPixelShaderConstantF(StartRegister, pConstantData, Vector4fCount);
        }

        HRESULT ProxyDirect3DDevice9::SetPixelShaderConstantI(UINT StartRegister, CONST int* pConstantData, UINT Vector4iCount) {
            return this->FunctionAddresses.SetPixelShaderConstantI != nullptr ?
                this->FunctionAddresses.SetPixelShaderConstantI(StartRegister, pConstantData, Vector4iCount) :
                this->dev->SetPixelShaderConstantI(StartRegister, pConstantData, Vector4iCount);
        }

        HRESULT ProxyDirect3DDevice9::GetPixelShaderConstantI(UINT StartRegister, int* pConstantData, UINT Vector4iCount) {
            return this->FunctionAddresses.GetPixelShaderConstantI != nullptr ?
                this->FunctionAddresses.GetPixelShaderConstantI(StartRegister, pConstantData, Vector4iCount) :
                this->dev->GetPixelShaderConstantI(StartRegister, pConstantData, Vector4iCount);
        }

        HRESULT ProxyDirect3DDevice9::SetPixelShaderConstantB(UINT StartRegister, CONST BOOL* pConstantData, UINT BoolCount) {
            return this->FunctionAddresses.SetPixelShaderConstantB != nullptr ?
                this->FunctionAddresses.SetPixelShaderConstantB(StartRegister, pConstantData, BoolCount) :
                this->dev->SetPixelShaderConstantB(StartRegister, pConstantData, BoolCount);
        }

        HRESULT ProxyDirect3DDevice9::GetPixelShaderConstantB(UINT StartRegister, BOOL* pConstantData, UINT BoolCount) {
            return this->FunctionAddresses.GetPixelShaderConstantB != nullptr ?
                this->FunctionAddresses.GetPixelShaderConstantB(StartRegister, pConstantData, BoolCount) :
                this->dev->GetPixelShaderConstantB(StartRegister, pConstantData, BoolCount);
        }

        HRESULT ProxyDirect3DDevice9::DrawRectPatch(UINT Handle, CONST float* pNumSegs, CONST D3DRECTPATCH_INFO* pRectPatchInfo) {
            return this->FunctionAddresses.DrawRectPatch != nullptr ?
                this->FunctionAddresses.DrawRectPatch(Handle, pNumSegs, pRectPatchInfo) :
                this->dev->DrawRectPatch(Handle, pNumSegs, pRectPatchInfo);
        }

        HRESULT ProxyDirect3DDevice9::DrawTriPatch(UINT Handle, CONST float* pNumSegs, CONST D3DTRIPATCH_INFO* pTriPatchInfo) {
            return this->FunctionAddresses.DrawTriPatch != nullptr ?
                this->FunctionAddresses.DrawTriPatch(Handle, pNumSegs, pTriPatchInfo) :
                this->dev->DrawTriPatch(Handle, pNumSegs, pTriPatchInfo);
        }

        HRESULT ProxyDirect3DDevice9::DeletePatch(UINT Handle) {
            return this->FunctionAddresses.DeletePatch != nullptr ?
                this->FunctionAddresses.DeletePatch(Handle) :
                this->dev->DeletePatch(Handle);
        }

        HRESULT ProxyDirect3DDevice9::CreateQuery(D3DQUERYTYPE Type, IDirect3DQuery9** ppQuery) {
            return this->FunctionAddresses.CreateQuery != nullptr ?
                this->FunctionAddresses.CreateQuery(Type, ppQuery) :
                this->dev->CreateQuery(Type, ppQuery);
        }


        HRESULT ProxyDirect3DDevice9Ex::QueryInterface(REFIID riid, void** ppvObj) {
            return this->FunctionAddresses.QueryInterface != nullptr ?
                this->FunctionAddresses.QueryInterface(riid, ppvObj) :
                this->dev->QueryInterface(riid, ppvObj);
        }

        ULONG ProxyDirect3DDevice9Ex::AddRef() {
            return this->FunctionAddresses.AddRef != nullptr ?
                this->FunctionAddresses.AddRef() :
                this->dev->AddRef();
        }

        ULONG ProxyDirect3DDevice9Ex::Release() {
            ULONG count = this->FunctionAddresses.Release != nullptr ?
                this->FunctionAddresses.Release() :
                this->dev->Release();
            if (count == 0) {
                delete this;
            }
            return count;
        }

        HRESULT ProxyDirect3DDevice9Ex::TestCooperativeLevel() {
            return this->FunctionAddresses.TestCooperativeLevel != nullptr ?
                this->FunctionAddresses.TestCooperativeLevel() :
                this->dev->TestCooperativeLevel();
        }

        UINT ProxyDirect3DDevice9Ex::GetAvailableTextureMem() {
            return this->FunctionAddresses.GetAvailableTextureMem != nullptr ?
                this->FunctionAddresses.GetAvailableTextureMem() :
                this->dev->GetAvailableTextureMem();
        }

        HRESULT ProxyDirect3DDevice9Ex::EvictManagedResources() {
            return this->FunctionAddresses.EvictManagedResources != nullptr ?
                this->FunctionAddresses.EvictManagedResources() :
                this->dev->EvictManagedResources();
        }

        HRESULT ProxyDirect3DDevice9Ex::GetDirect3D(IDirect3D9** ppD3D9) {
            return this->FunctionAddresses.GetDirect3D != nullptr ?
                this->FunctionAddresses.GetDirect3D(ppD3D9) :
                this->dev->GetDirect3D(ppD3D9);
        }

        HRESULT ProxyDirect3DDevice9Ex::GetDeviceCaps(D3DCAPS9* pCaps) {
            return this->FunctionAddresses.GetDeviceCaps != nullptr ?
                this->FunctionAddresses.GetDeviceCaps(pCaps) :
                this->dev->GetDeviceCaps(pCaps);
        }

        HRESULT ProxyDirect3DDevice9Ex::GetDisplayMode(UINT iSwapChain, D3DDISPLAYMODE* pMode) {
            return this->FunctionAddresses.GetDisplayMode != nullptr ?
                this->FunctionAddresses.GetDisplayMode(iSwapChain, pMode) :
                this->dev->GetDisplayMode(iSwapChain, pMode);
        }

        HRESULT ProxyDirect3DDevice9Ex::GetCreationParameters(D3DDEVICE_CREATION_PARAMETERS *pParameters) {
            return this->FunctionAddresses.GetCreationParameters != nullptr ?
                this->FunctionAddresses.GetCreationParameters(pParameters) :
                this->dev->GetCreationParameters(pParameters);
        }

        HRESULT ProxyDirect3DDevice9Ex::SetCursorProperties(UINT XHotSpot, UINT YHotSpot, IDirect3DSurface9* pCursorBitmap) {
            return this->FunctionAddresses.SetCursorProperties != nullptr ?
                this->FunctionAddresses.SetCursorProperties(XHotSpot, YHotSpot, pCursorBitmap) :
                this->dev->SetCursorProperties(XHotSpot, YHotSpot, pCursorBitmap);
        }

        void ProxyDirect3DDevice9Ex::SetCursorPosition(int X, int Y, DWORD Flags) {
            return this->FunctionAddresses.SetCursorPosition != nullptr ?
                this->FunctionAddresses.SetCursorPosition(X, Y, Flags) :
                this->dev->SetCursorPosition(X, Y, Flags);
        }

        BOOL ProxyDirect3DDevice9Ex::ShowCursor(BOOL bShow) {
            return this->FunctionAddresses.ShowCursor != nullptr ?
                this->FunctionAddresses.ShowCursor(bShow) :
                this->dev->ShowCursor(bShow);
        }

        HRESULT ProxyDirect3DDevice9Ex::CreateAdditionalSwapChain(D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DSwapChain9** pSwapChain) {
            return this->FunctionAddresses.CreateAdditionalSwapChain != nullptr ?
                this->FunctionAddresses.CreateAdditionalSwapChain(pPresentationParameters, pSwapChain) :
                this->dev->CreateAdditionalSwapChain(pPresentationParameters, pSwapChain);
        }

        HRESULT ProxyDirect3DDevice9Ex::GetSwapChain(UINT iSwapChain, IDirect3DSwapChain9** pSwapChain) {
            return this->FunctionAddresses.GetSwapChain != nullptr ?
                this->FunctionAddresses.GetSwapChain(iSwapChain, pSwapChain) :
                this->dev->GetSwapChain(iSwapChain, pSwapChain);
        }

        UINT ProxyDirect3DDevice9Ex::GetNumberOfSwapChains() {
            return this->FunctionAddresses.GetNumberOfSwapChains != nullptr ?
                this->FunctionAddresses.GetNumberOfSwapChains() :
                this->dev->GetNumberOfSwapChains();
        }

        HRESULT ProxyDirect3DDevice9Ex::Reset(D3DPRESENT_PARAMETERS* pPresentationParameters) {
            return this->Reset(pPresentationParameters);
        }

        HRESULT ProxyDirect3DDevice9Ex::Present(CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion) {
            return this->FunctionAddresses.Present != nullptr ?
                this->FunctionAddresses.Present(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion) :
                this->dev->Present(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
        }

        HRESULT ProxyDirect3DDevice9Ex::GetBackBuffer(UINT iSwapChain, UINT iBackBuffer, D3DBACKBUFFER_TYPE Type, IDirect3DSurface9** ppBackBuffer) {
            return this->FunctionAddresses.GetBackBuffer != nullptr ?
                this->FunctionAddresses.GetBackBuffer(iSwapChain, iBackBuffer, Type, ppBackBuffer) :
                this->dev->GetBackBuffer(iSwapChain, iBackBuffer, Type, ppBackBuffer);
        }

        HRESULT ProxyDirect3DDevice9Ex::GetRasterStatus(UINT iSwapChain, D3DRASTER_STATUS* pRasterStatus) {
            return this->FunctionAddresses.GetRasterStatus != nullptr ?
                this->FunctionAddresses.GetRasterStatus(iSwapChain, pRasterStatus) :
                this->dev->GetRasterStatus(iSwapChain, pRasterStatus);
        }

        HRESULT ProxyDirect3DDevice9Ex::SetDialogBoxMode(BOOL bEnableDialogs) {
            return this->FunctionAddresses.SetDialogBoxMode != nullptr ?
                this->FunctionAddresses.SetDialogBoxMode(bEnableDialogs) :
                this->dev->SetDialogBoxMode(bEnableDialogs);
        }

        void ProxyDirect3DDevice9Ex::SetGammaRamp(UINT iSwapChain, DWORD Flags, CONST D3DGAMMARAMP* pRamp) {
            return this->FunctionAddresses.SetGammaRamp != nullptr ?
                this->FunctionAddresses.SetGammaRamp(iSwapChain, Flags, pRamp) :
                this->dev->SetGammaRamp(iSwapChain, Flags, pRamp);
        }

        void ProxyDirect3DDevice9Ex::GetGammaRamp(UINT iSwapChain, D3DGAMMARAMP* pRamp) {
            return this->FunctionAddresses.GetGammaRamp != nullptr ?
                this->FunctionAddresses.GetGammaRamp(iSwapChain, pRamp) :
                this->dev->GetGammaRamp(iSwapChain, pRamp);
        }

        HRESULT ProxyDirect3DDevice9Ex::CreateTexture(UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DTexture9** ppTexture, HANDLE* pSharedHandle) {
            return this->FunctionAddresses.CreateTexture != nullptr ?
                this->FunctionAddresses.CreateTexture(Width, Height, Levels, Usage, Format, Pool, ppTexture, pSharedHandle) :
                this->dev->CreateTexture(Width, Height, Levels, Usage, Format, Pool, ppTexture, pSharedHandle);
        }

        HRESULT ProxyDirect3DDevice9Ex::CreateVolumeTexture(UINT Width, UINT Height, UINT Depth, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DVolumeTexture9** ppVolumeTexture, HANDLE* pSharedHandle) {
            return this->FunctionAddresses.CreateVolumeTexture != nullptr ?
                this->FunctionAddresses.CreateVolumeTexture(Width, Height, Depth, Levels, Usage, Format, Pool, ppVolumeTexture, pSharedHandle) :
                this->dev->CreateVolumeTexture(Width, Height, Depth, Levels, Usage, Format, Pool, ppVolumeTexture, pSharedHandle);
        }

        HRESULT ProxyDirect3DDevice9Ex::CreateCubeTexture(UINT EdgeLength, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DCubeTexture9** ppCubeTexture, HANDLE* pSharedHandle) {
            return this->FunctionAddresses.CreateCubeTexture != nullptr ?
                this->FunctionAddresses.CreateCubeTexture(EdgeLength, Levels, Usage, Format, Pool, ppCubeTexture, pSharedHandle) :
                this->dev->CreateCubeTexture(EdgeLength, Levels, Usage, Format, Pool, ppCubeTexture, pSharedHandle);
        }

        HRESULT ProxyDirect3DDevice9Ex::CreateVertexBuffer(UINT Length, DWORD Usage, DWORD FVF, D3DPOOL Pool, IDirect3DVertexBuffer9** ppVertexBuffer, HANDLE* pSharedHandle) {
            return this->FunctionAddresses.CreateVertexBuffer != nullptr ?
                this->FunctionAddresses.CreateVertexBuffer(Length, Usage, FVF, Pool, ppVertexBuffer, pSharedHandle) :
                this->dev->CreateVertexBuffer(Length, Usage, FVF, Pool, ppVertexBuffer, pSharedHandle);
        }

        HRESULT ProxyDirect3DDevice9Ex::CreateIndexBuffer(UINT Length, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DIndexBuffer9** ppIndexBuffer, HANDLE* pSharedHandle) {
            return this->FunctionAddresses.CreateIndexBuffer != nullptr ?
                this->FunctionAddresses.CreateIndexBuffer(Length, Usage, Format, Pool, ppIndexBuffer, pSharedHandle) :
                this->dev->CreateIndexBuffer(Length, Usage, Format, Pool, ppIndexBuffer, pSharedHandle);
        }

        HRESULT ProxyDirect3DDevice9Ex::CreateRenderTarget(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Lockable, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle) {
            return this->FunctionAddresses.CreateRenderTarget != nullptr ?
                this->FunctionAddresses.CreateRenderTarget(Width, Height, Format, MultiSample, MultisampleQuality, Lockable, ppSurface, pSharedHandle) :
                this->dev->CreateRenderTarget(Width, Height, Format, MultiSample, MultisampleQuality, Lockable, ppSurface, pSharedHandle);
        }

        HRESULT ProxyDirect3DDevice9Ex::CreateDepthStencilSurface(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Discard, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle) {
            return this->FunctionAddresses.CreateDepthStencilSurface != nullptr ?
                this->FunctionAddresses.CreateDepthStencilSurface(Width, Height, Format, MultiSample, MultisampleQuality, Discard, ppSurface, pSharedHandle) :
                this->dev->CreateDepthStencilSurface(Width, Height, Format, MultiSample, MultisampleQuality, Discard, ppSurface, pSharedHandle);
        }

        HRESULT ProxyDirect3DDevice9Ex::UpdateSurface(IDirect3DSurface9* pSourceSurface, CONST RECT* pSourceRect, IDirect3DSurface9* pDestinationSurface, CONST POINT* pDestPoint) {
            return this->FunctionAddresses.UpdateSurface != nullptr ?
                this->FunctionAddresses.UpdateSurface(pSourceSurface, pSourceRect, pDestinationSurface, pDestPoint) :
                this->dev->UpdateSurface(pSourceSurface, pSourceRect, pDestinationSurface, pDestPoint);
        }

        HRESULT ProxyDirect3DDevice9Ex::UpdateTexture(IDirect3DBaseTexture9* pSourceTexture, IDirect3DBaseTexture9* pDestinationTexture) {
            return this->FunctionAddresses.UpdateTexture != nullptr ?
                this->FunctionAddresses.UpdateTexture(pSourceTexture, pDestinationTexture) :
                this->dev->UpdateTexture(pSourceTexture, pDestinationTexture);
        }

        HRESULT ProxyDirect3DDevice9Ex::GetRenderTargetData(IDirect3DSurface9* pRenderTarget, IDirect3DSurface9* pDestSurface) {
            return this->FunctionAddresses.GetRenderTargetData != nullptr ?
                this->FunctionAddresses.GetRenderTargetData(pRenderTarget, pDestSurface) :
                this->dev->GetRenderTargetData(pRenderTarget, pDestSurface);
        }

        HRESULT ProxyDirect3DDevice9Ex::GetFrontBufferData(UINT iSwapChain, IDirect3DSurface9* pDestSurface) {
            return this->FunctionAddresses.GetFrontBufferData != nullptr ?
                this->FunctionAddresses.GetFrontBufferData(iSwapChain, pDestSurface) :
                this->dev->GetFrontBufferData(iSwapChain, pDestSurface);
        }

        HRESULT ProxyDirect3DDevice9Ex::StretchRect(IDirect3DSurface9* pSourceSurface, CONST RECT* pSourceRect, IDirect3DSurface9* pDestSurface, CONST RECT* pDestRect, D3DTEXTUREFILTERTYPE Filter) {
            return this->FunctionAddresses.StretchRect != nullptr ?
                this->FunctionAddresses.StretchRect(pSourceSurface, pSourceRect, pDestSurface, pDestRect, Filter) :
                this->dev->StretchRect(pSourceSurface, pSourceRect, pDestSurface, pDestRect, Filter);
        }

        HRESULT ProxyDirect3DDevice9Ex::ColorFill(IDirect3DSurface9* pSurface, CONST RECT* pRect, D3DCOLOR color) {
            return this->FunctionAddresses.ColorFill != nullptr ?
                this->FunctionAddresses.ColorFill(pSurface, pRect, color) :
                this->dev->ColorFill(pSurface, pRect, color);
        }

        HRESULT ProxyDirect3DDevice9Ex::CreateOffscreenPlainSurface(UINT Width, UINT Height, D3DFORMAT Format, D3DPOOL Pool, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle) {
            return this->FunctionAddresses.CreateOffscreenPlainSurface != nullptr ?
                this->FunctionAddresses.CreateOffscreenPlainSurface(Width, Height, Format, Pool, ppSurface, pSharedHandle) :
                this->dev->CreateOffscreenPlainSurface(Width, Height, Format, Pool, ppSurface, pSharedHandle);
        }

        HRESULT ProxyDirect3DDevice9Ex::SetRenderTarget(DWORD RenderTargetIndex, IDirect3DSurface9* pRenderTarget) {
            return this->FunctionAddresses.SetRenderTarget != nullptr ?
                this->FunctionAddresses.SetRenderTarget(RenderTargetIndex, pRenderTarget) :
                this->dev->SetRenderTarget(RenderTargetIndex, pRenderTarget);
        }

        HRESULT ProxyDirect3DDevice9Ex::GetRenderTarget(DWORD RenderTargetIndex, IDirect3DSurface9** ppRenderTarget) {
            return this->FunctionAddresses.GetRenderTarget != nullptr ?
                this->FunctionAddresses.GetRenderTarget(RenderTargetIndex, ppRenderTarget) :
                this->dev->GetRenderTarget(RenderTargetIndex, ppRenderTarget);
        }

        HRESULT ProxyDirect3DDevice9Ex::SetDepthStencilSurface(IDirect3DSurface9* pNewZStencil) {
            return this->FunctionAddresses.SetDepthStencilSurface != nullptr ?
                this->FunctionAddresses.SetDepthStencilSurface(pNewZStencil) :
                this->dev->SetDepthStencilSurface(pNewZStencil);
        }

        HRESULT ProxyDirect3DDevice9Ex::GetDepthStencilSurface(IDirect3DSurface9** ppZStencilSurface) {
            return this->FunctionAddresses.GetDepthStencilSurface != nullptr ?
                this->FunctionAddresses.GetDepthStencilSurface(ppZStencilSurface) :
                this->dev->GetDepthStencilSurface(ppZStencilSurface);
        }

        HRESULT ProxyDirect3DDevice9Ex::BeginScene() {
            return this->FunctionAddresses.BeginScene != nullptr ?
                this->FunctionAddresses.BeginScene() :
                this->dev->BeginScene();
        }

        HRESULT ProxyDirect3DDevice9Ex::EndScene() {
            return this->FunctionAddresses.EndScene != nullptr ?
                this->FunctionAddresses.EndScene() :
                this->dev->EndScene();
        }

        HRESULT ProxyDirect3DDevice9Ex::Clear(DWORD Count, CONST D3DRECT* pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil) {
            return this->FunctionAddresses.Clear != nullptr ?
                this->FunctionAddresses.Clear(Count, pRects, Flags, Color, Z, Stencil) :
                this->dev->Clear(Count, pRects, Flags, Color, Z, Stencil);
        }

        HRESULT ProxyDirect3DDevice9Ex::SetTransform(D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX* pMatrix) {
            return this->FunctionAddresses.SetTransform != nullptr ?
                this->FunctionAddresses.SetTransform(State, pMatrix) :
                this->dev->SetTransform(State, pMatrix);
        }

        HRESULT ProxyDirect3DDevice9Ex::GetTransform(D3DTRANSFORMSTATETYPE State, D3DMATRIX* pMatrix) {
            return this->FunctionAddresses.GetTransform != nullptr ?
                this->FunctionAddresses.GetTransform(State, pMatrix) :
                this->dev->GetTransform(State, pMatrix);
        }

        HRESULT ProxyDirect3DDevice9Ex::MultiplyTransform(D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX* pMatrix) {
            return this->FunctionAddresses.MultiplyTransform != nullptr ?
                this->FunctionAddresses.MultiplyTransform(State, pMatrix) :
                this->dev->MultiplyTransform(State, pMatrix);
        }

        HRESULT ProxyDirect3DDevice9Ex::SetViewport(CONST D3DVIEWPORT9* pViewport) {
            return this->FunctionAddresses.SetViewport != nullptr ?
                this->FunctionAddresses.SetViewport(pViewport) :
                this->dev->SetViewport(pViewport);
        }

        HRESULT ProxyDirect3DDevice9Ex::GetViewport(D3DVIEWPORT9* pViewport) {
            return this->FunctionAddresses.GetViewport != nullptr ?
                this->FunctionAddresses.GetViewport(pViewport) :
                this->dev->GetViewport(pViewport);
        }

        HRESULT ProxyDirect3DDevice9Ex::SetMaterial(CONST D3DMATERIAL9* pMaterial) {
            return this->FunctionAddresses.SetMaterial != nullptr ?
                this->FunctionAddresses.SetMaterial(pMaterial) :
                this->dev->SetMaterial(pMaterial);
        }

        HRESULT ProxyDirect3DDevice9Ex::GetMaterial(D3DMATERIAL9* pMaterial) {
            return this->FunctionAddresses.GetMaterial != nullptr ?
                this->FunctionAddresses.GetMaterial(pMaterial) :
                this->dev->GetMaterial(pMaterial);
        }

        HRESULT ProxyDirect3DDevice9Ex::SetLight(DWORD Index, CONST D3DLIGHT9* pLight) {
            return this->FunctionAddresses.SetLight != nullptr ?
                this->FunctionAddresses.SetLight(Index, pLight) :
                this->dev->SetLight(Index, pLight);
        }

        HRESULT ProxyDirect3DDevice9Ex::GetLight(DWORD Index, D3DLIGHT9* pLight) {
            return this->FunctionAddresses.GetLight != nullptr ?
                this->FunctionAddresses.GetLight(Index, pLight) :
                this->dev->GetLight(Index, pLight);
        }

        HRESULT ProxyDirect3DDevice9Ex::LightEnable(DWORD Index, BOOL Enable) {
            return this->FunctionAddresses.LightEnable != nullptr ?
                this->FunctionAddresses.LightEnable(Index, Enable) :
                this->dev->LightEnable(Index, Enable);
        }

        HRESULT ProxyDirect3DDevice9Ex::GetLightEnable(DWORD Index, BOOL* pEnable) {
            return this->FunctionAddresses.GetLightEnable != nullptr ?
                this->FunctionAddresses.GetLightEnable(Index, pEnable) :
                this->dev->GetLightEnable(Index, pEnable);
        }

        HRESULT ProxyDirect3DDevice9Ex::SetClipPlane(DWORD Index, CONST float* pPlane) {
            return this->FunctionAddresses.SetClipPlane != nullptr ?
                this->FunctionAddresses.SetClipPlane(Index, pPlane) :
                this->dev->SetClipPlane(Index, pPlane);
        }

        HRESULT ProxyDirect3DDevice9Ex::GetClipPlane(DWORD Index, float* pPlane) {
            return this->FunctionAddresses.GetClipPlane != nullptr ?
                this->FunctionAddresses.GetClipPlane(Index, pPlane) :
                this->dev->GetClipPlane(Index, pPlane);
        }

        HRESULT ProxyDirect3DDevice9Ex::SetRenderState(D3DRENDERSTATETYPE State, DWORD Value) {
            return this->FunctionAddresses.SetRenderState != nullptr ?
                this->FunctionAddresses.SetRenderState(State, Value) :
                this->dev->SetRenderState(State, Value);
        }

        HRESULT ProxyDirect3DDevice9Ex::GetRenderState(D3DRENDERSTATETYPE State, DWORD* pValue) {
            return this->FunctionAddresses.GetRenderState != nullptr ?
                this->FunctionAddresses.GetRenderState(State, pValue) :
                this->dev->GetRenderState(State, pValue);
        }

        HRESULT ProxyDirect3DDevice9Ex::CreateStateBlock(D3DSTATEBLOCKTYPE Type, IDirect3DStateBlock9** ppSB) {
            return this->FunctionAddresses.CreateStateBlock != nullptr ?
                this->FunctionAddresses.CreateStateBlock(Type, ppSB) :
                this->dev->CreateStateBlock(Type, ppSB);
        }

        HRESULT ProxyDirect3DDevice9Ex::BeginStateBlock() {
            return this->FunctionAddresses.BeginStateBlock != nullptr ?
                this->FunctionAddresses.BeginStateBlock() :
                this->dev->BeginStateBlock();
        }

        HRESULT ProxyDirect3DDevice9Ex::EndStateBlock(IDirect3DStateBlock9** ppSB) {
            return this->FunctionAddresses.EndStateBlock != nullptr ?
                this->FunctionAddresses.EndStateBlock(ppSB) :
                this->dev->EndStateBlock(ppSB);
        }

        HRESULT ProxyDirect3DDevice9Ex::SetClipStatus(CONST D3DCLIPSTATUS9* pClipStatus) {
            return this->FunctionAddresses.SetClipStatus != nullptr ?
                this->FunctionAddresses.SetClipStatus(pClipStatus) :
                this->dev->SetClipStatus(pClipStatus);
        }

        HRESULT ProxyDirect3DDevice9Ex::GetClipStatus(D3DCLIPSTATUS9* pClipStatus) {
            return this->FunctionAddresses.GetClipStatus != nullptr ?
                this->FunctionAddresses.GetClipStatus(pClipStatus) :
                this->dev->GetClipStatus(pClipStatus);
        }

        HRESULT ProxyDirect3DDevice9Ex::GetTexture(DWORD Stage, IDirect3DBaseTexture9** ppTexture) {
            return this->FunctionAddresses.GetTexture != nullptr ?
                this->FunctionAddresses.GetTexture(Stage, ppTexture) :
                this->dev->GetTexture(Stage, ppTexture);
        }

        HRESULT ProxyDirect3DDevice9Ex::SetTexture(DWORD Stage, IDirect3DBaseTexture9* pTexture) {
            return this->FunctionAddresses.SetTexture != nullptr ?
                this->FunctionAddresses.SetTexture(Stage, pTexture) :
                this->dev->SetTexture(Stage, pTexture);
        }

        HRESULT ProxyDirect3DDevice9Ex::GetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD* pValue) {
            return this->FunctionAddresses.GetTextureStageState != nullptr ?
                this->FunctionAddresses.GetTextureStageState(Stage, Type, pValue) :
                this->dev->GetTextureStageState(Stage, Type, pValue);
        }

        HRESULT ProxyDirect3DDevice9Ex::SetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value) {
            return this->FunctionAddresses.SetTextureStageState != nullptr ?
                this->FunctionAddresses.SetTextureStageState(Stage, Type, Value) :
                this->dev->SetTextureStageState(Stage, Type, Value);
        }

        HRESULT ProxyDirect3DDevice9Ex::GetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD* pValue) {
            return this->FunctionAddresses.GetSamplerState != nullptr ?
                this->FunctionAddresses.GetSamplerState(Sampler, Type, pValue) :
                this->dev->GetSamplerState(Sampler, Type, pValue);
        }

        HRESULT ProxyDirect3DDevice9Ex::SetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD Value) {
            return this->FunctionAddresses.SetSamplerState != nullptr ?
                this->FunctionAddresses.SetSamplerState(Sampler, Type, Value) :
                this->dev->SetSamplerState(Sampler, Type, Value);
        }

        HRESULT ProxyDirect3DDevice9Ex::ValidateDevice(DWORD* pNumPasses) {
            return this->FunctionAddresses.ValidateDevice != nullptr ?
                this->FunctionAddresses.ValidateDevice(pNumPasses) :
                this->dev->ValidateDevice(pNumPasses);
        }

        HRESULT ProxyDirect3DDevice9Ex::SetPaletteEntries(UINT PaletteNumber, CONST PALETTEENTRY* pEntries) {
            return this->FunctionAddresses.SetPaletteEntries != nullptr ?
                this->FunctionAddresses.SetPaletteEntries(PaletteNumber, pEntries) :
                this->dev->SetPaletteEntries(PaletteNumber, pEntries);
        }

        HRESULT ProxyDirect3DDevice9Ex::GetPaletteEntries(UINT PaletteNumber, PALETTEENTRY* pEntries) {
            return this->FunctionAddresses.GetPaletteEntries != nullptr ?
                this->FunctionAddresses.GetPaletteEntries(PaletteNumber, pEntries) :
                this->dev->GetPaletteEntries(PaletteNumber, pEntries);
        }

        HRESULT ProxyDirect3DDevice9Ex::SetCurrentTexturePalette(UINT PaletteNumber) {
            return this->FunctionAddresses.SetCurrentTexturePalette != nullptr ?
                this->FunctionAddresses.SetCurrentTexturePalette(PaletteNumber) :
                this->dev->SetCurrentTexturePalette(PaletteNumber);
        }

        HRESULT ProxyDirect3DDevice9Ex::GetCurrentTexturePalette(UINT *PaletteNumber) {
            return this->FunctionAddresses.GetCurrentTexturePalette != nullptr ?
                this->FunctionAddresses.GetCurrentTexturePalette(PaletteNumber) :
                this->dev->GetCurrentTexturePalette(PaletteNumber);
        }

        HRESULT ProxyDirect3DDevice9Ex::SetScissorRect(CONST RECT* pRect) {
            return this->FunctionAddresses.SetScissorRect != nullptr ?
                this->FunctionAddresses.SetScissorRect(pRect) :
                this->dev->SetScissorRect(pRect);
        }

        HRESULT ProxyDirect3DDevice9Ex::GetScissorRect(RECT* pRect) {
            return this->FunctionAddresses.GetScissorRect != nullptr ?
                this->FunctionAddresses.GetScissorRect(pRect) :
                this->dev->GetScissorRect(pRect);
        }

        HRESULT ProxyDirect3DDevice9Ex::SetSoftwareVertexProcessing(BOOL bSoftware) {
            return this->FunctionAddresses.SetSoftwareVertexProcessing != nullptr ?
                this->FunctionAddresses.SetSoftwareVertexProcessing(bSoftware) :
                this->dev->SetSoftwareVertexProcessing(bSoftware);
        }

        BOOL ProxyDirect3DDevice9Ex::GetSoftwareVertexProcessing() {
            return this->FunctionAddresses.GetSoftwareVertexProcessing != nullptr ?
                this->FunctionAddresses.GetSoftwareVertexProcessing() :
                this->dev->GetSoftwareVertexProcessing();
        }

        HRESULT ProxyDirect3DDevice9Ex::SetNPatchMode(float nSegments) {
            return this->FunctionAddresses.SetNPatchMode != nullptr ?
                this->FunctionAddresses.SetNPatchMode(nSegments) :
                this->dev->SetNPatchMode(nSegments);
        }

        float ProxyDirect3DDevice9Ex::GetNPatchMode() {
            return this->FunctionAddresses.GetNPatchMode != nullptr ?
                this->FunctionAddresses.GetNPatchMode() :
                this->dev->GetNPatchMode();
        }

        HRESULT ProxyDirect3DDevice9Ex::DrawPrimitive(D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount) {
            return this->FunctionAddresses.DrawPrimitive != nullptr ?
                this->FunctionAddresses.DrawPrimitive(PrimitiveType, StartVertex, PrimitiveCount) :
                this->dev->DrawPrimitive(PrimitiveType, StartVertex, PrimitiveCount);
        }

        HRESULT ProxyDirect3DDevice9Ex::DrawIndexedPrimitive(D3DPRIMITIVETYPE PrimitiveType, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount) {
            return this->FunctionAddresses.DrawIndexedPrimitive != nullptr ?
                this->FunctionAddresses.DrawIndexedPrimitive(PrimitiveType, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount) :
                this->dev->DrawIndexedPrimitive(PrimitiveType, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount);
        }

        HRESULT ProxyDirect3DDevice9Ex::DrawPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT PrimitiveCount, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride) {
            return this->FunctionAddresses.DrawPrimitiveUP != nullptr ?
                this->FunctionAddresses.DrawPrimitiveUP(PrimitiveType, PrimitiveCount, pVertexStreamZeroData, VertexStreamZeroStride) :
                this->dev->DrawPrimitiveUP(PrimitiveType, PrimitiveCount, pVertexStreamZeroData, VertexStreamZeroStride);
        }

        HRESULT ProxyDirect3DDevice9Ex::DrawIndexedPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT MinVertexIndex, UINT NumVertices, UINT PrimitiveCount, CONST void* pIndexData, D3DFORMAT IndexDataFormat, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride) {
            return this->FunctionAddresses.DrawIndexedPrimitiveUP != nullptr ?
                this->FunctionAddresses.DrawIndexedPrimitiveUP(PrimitiveType, MinVertexIndex, NumVertices, PrimitiveCount, pIndexData, IndexDataFormat, pVertexStreamZeroData, VertexStreamZeroStride) :
                this->dev->DrawIndexedPrimitiveUP(PrimitiveType, MinVertexIndex, NumVertices, PrimitiveCount, pIndexData, IndexDataFormat, pVertexStreamZeroData, VertexStreamZeroStride);
        }

        HRESULT ProxyDirect3DDevice9Ex::ProcessVertices(UINT SrcStartIndex, UINT DestIndex, UINT VertexCount, IDirect3DVertexBuffer9* pDestBuffer, IDirect3DVertexDeclaration9* pVertexDecl, DWORD Flags) {
            return this->FunctionAddresses.ProcessVertices != nullptr ?
                this->FunctionAddresses.ProcessVertices(SrcStartIndex, DestIndex, VertexCount, pDestBuffer, pVertexDecl, Flags) :
                this->dev->ProcessVertices(SrcStartIndex, DestIndex, VertexCount, pDestBuffer, pVertexDecl, Flags);
        }

        HRESULT ProxyDirect3DDevice9Ex::CreateVertexDeclaration(CONST D3DVERTEXELEMENT9* pVertexElements, IDirect3DVertexDeclaration9** ppDecl) {
            return this->FunctionAddresses.CreateVertexDeclaration != nullptr ?
                this->FunctionAddresses.CreateVertexDeclaration(pVertexElements, ppDecl) :
                this->dev->CreateVertexDeclaration(pVertexElements, ppDecl);
        }

        HRESULT ProxyDirect3DDevice9Ex::SetVertexDeclaration(IDirect3DVertexDeclaration9* pDecl) {
            return this->FunctionAddresses.SetVertexDeclaration != nullptr ?
                this->FunctionAddresses.SetVertexDeclaration(pDecl) :
                this->dev->SetVertexDeclaration(pDecl);
        }

        HRESULT ProxyDirect3DDevice9Ex::GetVertexDeclaration(IDirect3DVertexDeclaration9** ppDecl) {
            return this->FunctionAddresses.GetVertexDeclaration != nullptr ?
                this->FunctionAddresses.GetVertexDeclaration(ppDecl) :
                this->dev->GetVertexDeclaration(ppDecl);
        }

        HRESULT ProxyDirect3DDevice9Ex::SetFVF(DWORD FVF) {
            return this->FunctionAddresses.SetFVF != nullptr ?
                this->FunctionAddresses.SetFVF(FVF) :
                this->dev->SetFVF(FVF);
        }

        HRESULT ProxyDirect3DDevice9Ex::GetFVF(DWORD* pFVF) {
            return this->FunctionAddresses.GetFVF != nullptr ?
                this->FunctionAddresses.GetFVF(pFVF) :
                this->dev->GetFVF(pFVF);
        }

        HRESULT ProxyDirect3DDevice9Ex::CreateVertexShader(CONST DWORD* pFunction, IDirect3DVertexShader9** ppShader) {
            return this->FunctionAddresses.CreateVertexShader != nullptr ?
                this->FunctionAddresses.CreateVertexShader(pFunction, ppShader) :
                this->dev->CreateVertexShader(pFunction, ppShader);
        }

        HRESULT ProxyDirect3DDevice9Ex::SetVertexShader(IDirect3DVertexShader9* pShader) {
            return this->FunctionAddresses.SetVertexShader != nullptr ?
                this->FunctionAddresses.SetVertexShader(pShader) :
                this->dev->SetVertexShader(pShader);
        }

        HRESULT ProxyDirect3DDevice9Ex::GetVertexShader(IDirect3DVertexShader9** ppShader) {
            return this->FunctionAddresses.GetVertexShader != nullptr ?
                this->FunctionAddresses.GetVertexShader(ppShader) :
                this->dev->GetVertexShader(ppShader);
        }

        HRESULT ProxyDirect3DDevice9Ex::SetVertexShaderConstantF(UINT StartRegister, CONST float* pConstantData, UINT Vector4fCount) {
            return this->FunctionAddresses.SetVertexShaderConstantF != nullptr ?
                this->FunctionAddresses.SetVertexShaderConstantF(StartRegister, pConstantData, Vector4fCount) :
                this->dev->SetVertexShaderConstantF(StartRegister, pConstantData, Vector4fCount);
        }

        HRESULT ProxyDirect3DDevice9Ex::GetVertexShaderConstantF(UINT StartRegister, float* pConstantData, UINT Vector4fCount) {
            return this->FunctionAddresses.GetVertexShaderConstantF != nullptr ?
                this->FunctionAddresses.GetVertexShaderConstantF(StartRegister, pConstantData, Vector4fCount) :
                this->dev->GetVertexShaderConstantF(StartRegister, pConstantData, Vector4fCount);
        }

        HRESULT ProxyDirect3DDevice9Ex::SetVertexShaderConstantI(UINT StartRegister, CONST int* pConstantData, UINT Vector4iCount) {
            return this->FunctionAddresses.SetVertexShaderConstantI != nullptr ?
                this->FunctionAddresses.SetVertexShaderConstantI(StartRegister, pConstantData, Vector4iCount) :
                this->dev->SetVertexShaderConstantI(StartRegister, pConstantData, Vector4iCount);
        }

        HRESULT ProxyDirect3DDevice9Ex::GetVertexShaderConstantI(UINT StartRegister, int* pConstantData, UINT Vector4iCount) {
            return this->FunctionAddresses.GetVertexShaderConstantI != nullptr ?
                this->FunctionAddresses.GetVertexShaderConstantI(StartRegister, pConstantData, Vector4iCount) :
                this->dev->GetVertexShaderConstantI(StartRegister, pConstantData, Vector4iCount);
        }

        HRESULT ProxyDirect3DDevice9Ex::SetVertexShaderConstantB(UINT StartRegister, CONST BOOL* pConstantData, UINT BoolCount) {
            return this->FunctionAddresses.SetVertexShaderConstantB != nullptr ?
                this->FunctionAddresses.SetVertexShaderConstantB(StartRegister, pConstantData, BoolCount) :
                this->dev->SetVertexShaderConstantB(StartRegister, pConstantData, BoolCount);
        }

        HRESULT ProxyDirect3DDevice9Ex::GetVertexShaderConstantB(UINT StartRegister, BOOL* pConstantData, UINT BoolCount) {
            return this->FunctionAddresses.GetVertexShaderConstantB != nullptr ?
                this->FunctionAddresses.GetVertexShaderConstantB(StartRegister, pConstantData, BoolCount) :
                this->dev->GetVertexShaderConstantB(StartRegister, pConstantData, BoolCount);
        }

        HRESULT ProxyDirect3DDevice9Ex::SetStreamSource(UINT StreamNumber, IDirect3DVertexBuffer9* pStreamData, UINT OffsetInBytes, UINT Stride) {
            return this->FunctionAddresses.SetStreamSource != nullptr ?
                this->FunctionAddresses.SetStreamSource(StreamNumber, pStreamData, OffsetInBytes, Stride) :
                this->dev->SetStreamSource(StreamNumber, pStreamData, OffsetInBytes, Stride);
        }

        HRESULT ProxyDirect3DDevice9Ex::GetStreamSource(UINT StreamNumber, IDirect3DVertexBuffer9** ppStreamData, UINT* pOffsetInBytes, UINT* pStride) {
            return this->FunctionAddresses.GetStreamSource != nullptr ?
                this->FunctionAddresses.GetStreamSource(StreamNumber, ppStreamData, pOffsetInBytes, pStride) :
                this->dev->GetStreamSource(StreamNumber, ppStreamData, pOffsetInBytes, pStride);
        }

        HRESULT ProxyDirect3DDevice9Ex::SetStreamSourceFreq(UINT StreamNumber, UINT Setting) {
            return this->FunctionAddresses.SetStreamSourceFreq != nullptr ?
                this->FunctionAddresses.SetStreamSourceFreq(StreamNumber, Setting) :
                this->dev->SetStreamSourceFreq(StreamNumber, Setting);
        }

        HRESULT ProxyDirect3DDevice9Ex::GetStreamSourceFreq(UINT StreamNumber, UINT* pSetting) {
            return this->FunctionAddresses.GetStreamSourceFreq != nullptr ?
                this->FunctionAddresses.GetStreamSourceFreq(StreamNumber, pSetting) :
                this->dev->GetStreamSourceFreq(StreamNumber, pSetting);
        }

        HRESULT ProxyDirect3DDevice9Ex::SetIndices(IDirect3DIndexBuffer9* pIndexData) {
            return this->FunctionAddresses.SetIndices != nullptr ?
                this->FunctionAddresses.SetIndices(pIndexData) :
                this->dev->SetIndices(pIndexData);
        }

        HRESULT ProxyDirect3DDevice9Ex::GetIndices(IDirect3DIndexBuffer9** ppIndexData) {
            return this->FunctionAddresses.GetIndices != nullptr ?
                this->FunctionAddresses.GetIndices(ppIndexData) :
                this->dev->GetIndices(ppIndexData);
        }

        HRESULT ProxyDirect3DDevice9Ex::CreatePixelShader(CONST DWORD* pFunction, IDirect3DPixelShader9** ppShader) {
            return this->FunctionAddresses.CreatePixelShader != nullptr ?
                this->FunctionAddresses.CreatePixelShader(pFunction, ppShader) :
                this->dev->CreatePixelShader(pFunction, ppShader);
        }

        HRESULT ProxyDirect3DDevice9Ex::SetPixelShader(IDirect3DPixelShader9* pShader) {
            return this->FunctionAddresses.SetPixelShader != nullptr ?
                this->FunctionAddresses.SetPixelShader(pShader) :
                this->dev->SetPixelShader(pShader);
        }

        HRESULT ProxyDirect3DDevice9Ex::GetPixelShader(IDirect3DPixelShader9** ppShader) {
            return this->FunctionAddresses.GetPixelShader != nullptr ?
                this->FunctionAddresses.GetPixelShader(ppShader) :
                this->dev->GetPixelShader(ppShader);
        }

        HRESULT ProxyDirect3DDevice9Ex::SetPixelShaderConstantF(UINT StartRegister, CONST float* pConstantData, UINT Vector4fCount) {
            return this->FunctionAddresses.SetPixelShaderConstantF != nullptr ?
                this->FunctionAddresses.SetPixelShaderConstantF(StartRegister, pConstantData, Vector4fCount) :
                this->dev->SetPixelShaderConstantF(StartRegister, pConstantData, Vector4fCount);
        }

        HRESULT ProxyDirect3DDevice9Ex::GetPixelShaderConstantF(UINT StartRegister, float* pConstantData, UINT Vector4fCount) {
            return this->FunctionAddresses.GetPixelShaderConstantF != nullptr ?
                this->FunctionAddresses.GetPixelShaderConstantF(StartRegister, pConstantData, Vector4fCount) :
                this->dev->GetPixelShaderConstantF(StartRegister, pConstantData, Vector4fCount);
        }

        HRESULT ProxyDirect3DDevice9Ex::SetPixelShaderConstantI(UINT StartRegister, CONST int* pConstantData, UINT Vector4iCount) {
            return this->FunctionAddresses.SetPixelShaderConstantI != nullptr ?
                this->FunctionAddresses.SetPixelShaderConstantI(StartRegister, pConstantData, Vector4iCount) :
                this->dev->SetPixelShaderConstantI(StartRegister, pConstantData, Vector4iCount);
        }

        HRESULT ProxyDirect3DDevice9Ex::GetPixelShaderConstantI(UINT StartRegister, int* pConstantData, UINT Vector4iCount) {
            return this->FunctionAddresses.GetPixelShaderConstantI != nullptr ?
                this->FunctionAddresses.GetPixelShaderConstantI(StartRegister, pConstantData, Vector4iCount) :
                this->dev->GetPixelShaderConstantI(StartRegister, pConstantData, Vector4iCount);
        }

        HRESULT ProxyDirect3DDevice9Ex::SetPixelShaderConstantB(UINT StartRegister, CONST BOOL* pConstantData, UINT BoolCount) {
            return this->FunctionAddresses.SetPixelShaderConstantB != nullptr ?
                this->FunctionAddresses.SetPixelShaderConstantB(StartRegister, pConstantData, BoolCount) :
                this->dev->SetPixelShaderConstantB(StartRegister, pConstantData, BoolCount);
        }

        HRESULT ProxyDirect3DDevice9Ex::GetPixelShaderConstantB(UINT StartRegister, BOOL* pConstantData, UINT BoolCount) {
            return this->FunctionAddresses.GetPixelShaderConstantB != nullptr ?
                this->FunctionAddresses.GetPixelShaderConstantB(StartRegister, pConstantData, BoolCount) :
                this->dev->GetPixelShaderConstantB(StartRegister, pConstantData, BoolCount);
        }

        HRESULT ProxyDirect3DDevice9Ex::DrawRectPatch(UINT Handle, CONST float* pNumSegs, CONST D3DRECTPATCH_INFO* pRectPatchInfo) {
            return this->FunctionAddresses.DrawRectPatch != nullptr ?
                this->FunctionAddresses.DrawRectPatch(Handle, pNumSegs, pRectPatchInfo) :
                this->dev->DrawRectPatch(Handle, pNumSegs, pRectPatchInfo);
        }

        HRESULT ProxyDirect3DDevice9Ex::DrawTriPatch(UINT Handle, CONST float* pNumSegs, CONST D3DTRIPATCH_INFO* pTriPatchInfo) {
            return this->FunctionAddresses.DrawTriPatch != nullptr ?
                this->FunctionAddresses.DrawTriPatch(Handle, pNumSegs, pTriPatchInfo) :
                this->dev->DrawTriPatch(Handle, pNumSegs, pTriPatchInfo);
        }

        HRESULT ProxyDirect3DDevice9Ex::DeletePatch(UINT Handle) {
            return this->FunctionAddresses.DeletePatch != nullptr ?
                this->FunctionAddresses.DeletePatch(Handle) :
                this->dev->DeletePatch(Handle);
        }

        HRESULT ProxyDirect3DDevice9Ex::CreateQuery(D3DQUERYTYPE Type, IDirect3DQuery9** ppQuery) {
            return this->FunctionAddresses.CreateQuery != nullptr ?
                this->FunctionAddresses.CreateQuery(Type, ppQuery) :
                this->dev->CreateQuery(Type, ppQuery);
        }

        HRESULT ProxyDirect3DDevice9Ex::SetConvolutionMonoKernel(UINT width, UINT height, float* rows, float* columns) {
            return this->FunctionAddresses.SetConvolutionMonoKernel != nullptr ?
                this->FunctionAddresses.SetConvolutionMonoKernel(width, height, rows, columns) :
                this->dev->SetConvolutionMonoKernel(width, height, rows, columns);
        }

        HRESULT ProxyDirect3DDevice9Ex::ComposeRects(IDirect3DSurface9* pSrc, IDirect3DSurface9* pDst, IDirect3DVertexBuffer9* pSrcRectDescs, UINT NumRects, IDirect3DVertexBuffer9* pDstRectDescs, D3DCOMPOSERECTSOP Operation, int Xoffset, int Yoffset) {
            return this->FunctionAddresses.ComposeRects != nullptr ?
                this->FunctionAddresses.ComposeRects(pSrc, pDst, pSrcRectDescs, NumRects, pDstRectDescs, Operation, Xoffset, Yoffset) :
                this->dev->ComposeRects(pSrc, pDst, pSrcRectDescs, NumRects, pDstRectDescs, Operation, Xoffset, Yoffset);
        }

        HRESULT ProxyDirect3DDevice9Ex::PresentEx(CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion, DWORD dwFlags) {
            return this->FunctionAddresses.PresentEx != nullptr ?
                this->FunctionAddresses.PresentEx(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion, dwFlags) :
                this->dev->PresentEx(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion, dwFlags);
        }

        HRESULT ProxyDirect3DDevice9Ex::GetGPUThreadPriority(INT* pPriority) {
            return this->FunctionAddresses.GetGPUThreadPriority != nullptr ?
                this->FunctionAddresses.GetGPUThreadPriority(pPriority) :
                this->dev->GetGPUThreadPriority(pPriority);
        }

        HRESULT ProxyDirect3DDevice9Ex::SetGPUThreadPriority(INT Priority) {
            return this->FunctionAddresses.SetGPUThreadPriority != nullptr ?
                this->FunctionAddresses.SetGPUThreadPriority(Priority) :
                this->dev->SetGPUThreadPriority(Priority);
        }

        HRESULT ProxyDirect3DDevice9Ex::WaitForVBlank(UINT iSwapChain) {
            return this->FunctionAddresses.WaitForVBlank != nullptr ?
                this->FunctionAddresses.WaitForVBlank(iSwapChain) :
                this->dev->WaitForVBlank(iSwapChain);
        }

        HRESULT ProxyDirect3DDevice9Ex::CheckResourceResidency(IDirect3DResource9** pResourceArray, UINT32 NumResources) {
            return this->FunctionAddresses.CheckResourceResidency != nullptr ?
                this->FunctionAddresses.CheckResourceResidency(pResourceArray, NumResources) :
                this->dev->CheckResourceResidency(pResourceArray, NumResources);
        }

        HRESULT ProxyDirect3DDevice9Ex::SetMaximumFrameLatency(UINT MaxLatency) {
            return this->FunctionAddresses.SetMaximumFrameLatency != nullptr ?
                this->FunctionAddresses.SetMaximumFrameLatency(MaxLatency) :
                this->dev->SetMaximumFrameLatency(MaxLatency);
        }

        HRESULT ProxyDirect3DDevice9Ex::GetMaximumFrameLatency(UINT* pMaxLatency) {
            return this->FunctionAddresses.GetMaximumFrameLatency != nullptr ?
                this->FunctionAddresses.GetMaximumFrameLatency(pMaxLatency) :
                this->dev->GetMaximumFrameLatency(pMaxLatency);
        }

        HRESULT ProxyDirect3DDevice9Ex::CheckDeviceState(HWND hDestinationWindow) {
            return this->FunctionAddresses.CheckDeviceState != nullptr ?
                this->FunctionAddresses.CheckDeviceState(hDestinationWindow) :
                this->dev->CheckDeviceState(hDestinationWindow);
        }

        HRESULT ProxyDirect3DDevice9Ex::CreateRenderTargetEx(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Lockable, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle, DWORD Usage) {
            return this->FunctionAddresses.CreateRenderTargetEx != nullptr ?
                this->FunctionAddresses.CreateRenderTargetEx(Width, Height, Format, MultiSample, MultisampleQuality, Lockable, ppSurface, pSharedHandle, Usage) :
                this->dev->CreateRenderTargetEx(Width, Height, Format, MultiSample, MultisampleQuality, Lockable, ppSurface, pSharedHandle, Usage);
        }

        HRESULT ProxyDirect3DDevice9Ex::CreateOffscreenPlainSurfaceEx(UINT Width, UINT Height, D3DFORMAT Format, D3DPOOL Pool, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle, DWORD Usage) {
            return this->FunctionAddresses.CreateOffscreenPlainSurfaceEx != nullptr ?
                this->FunctionAddresses.CreateOffscreenPlainSurfaceEx(Width, Height, Format, Pool, ppSurface, pSharedHandle, Usage) :
                this->dev->CreateOffscreenPlainSurfaceEx(Width, Height, Format, Pool, ppSurface, pSharedHandle, Usage);
        }

        HRESULT ProxyDirect3DDevice9Ex::CreateDepthStencilSurfaceEx(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Discard, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle, DWORD Usage) {
            return this->FunctionAddresses.CreateDepthStencilSurfaceEx != nullptr ?
                this->FunctionAddresses.CreateDepthStencilSurfaceEx(Width, Height, Format, MultiSample, MultisampleQuality, Discard, ppSurface, pSharedHandle, Usage) :
                this->dev->CreateDepthStencilSurfaceEx(Width, Height, Format, MultiSample, MultisampleQuality, Discard, ppSurface, pSharedHandle, Usage);
        }

        HRESULT ProxyDirect3DDevice9Ex::ResetEx(D3DPRESENT_PARAMETERS* pPresentationParameters, D3DDISPLAYMODEEX* pFullscreenDisplayMode) {
            return this->FunctionAddresses.ResetEx != nullptr ?
                this->FunctionAddresses.ResetEx(pPresentationParameters, pFullscreenDisplayMode) :
                this->dev->ResetEx(pPresentationParameters, pFullscreenDisplayMode);
        }

        HRESULT ProxyDirect3DDevice9Ex::GetDisplayModeEx(UINT iSwapChain, D3DDISPLAYMODEEX* pMode, D3DDISPLAYROTATION* pRotation) {
            return this->FunctionAddresses.GetDisplayModeEx != nullptr ?
                this->FunctionAddresses.GetDisplayModeEx(iSwapChain, pMode, pRotation) :
                this->dev->GetDisplayModeEx(iSwapChain, pMode, pRotation);
        }

    }
}
