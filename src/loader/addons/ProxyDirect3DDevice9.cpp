#include "ProxyDirect3DDevice9.h"

namespace loader {
    namespace addons {

        __declspec(noinline) HRESULT ProxyDirect3DDevice9::QueryInterface(REFIID riid, void** ppvObj) {
            return this->dev->QueryInterface(riid, ppvObj);
        }

        __declspec(noinline) ULONG ProxyDirect3DDevice9::AddRef() {
            return this->dev->AddRef();
        }

        __declspec(noinline) ULONG ProxyDirect3DDevice9::Release() {
            return this->dev->Release();
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9::TestCooperativeLevel() {
            return this->dev->TestCooperativeLevel();
        }

        __declspec(noinline) UINT ProxyDirect3DDevice9::GetAvailableTextureMem() {
            return this->dev->GetAvailableTextureMem();
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9::EvictManagedResources() {
            return this->dev->EvictManagedResources();
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9::GetDirect3D(IDirect3D9** ppD3D9) {
            return this->dev->GetDirect3D(ppD3D9);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9::GetDeviceCaps(D3DCAPS9* pCaps) {
            return this->dev->GetDeviceCaps(pCaps);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9::GetDisplayMode(UINT iSwapChain, D3DDISPLAYMODE* pMode) {
            return this->dev->GetDisplayMode(iSwapChain, pMode);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9::GetCreationParameters(D3DDEVICE_CREATION_PARAMETERS *pParameters) {
            return this->dev->GetCreationParameters(pParameters);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9::SetCursorProperties(UINT XHotSpot, UINT YHotSpot, IDirect3DSurface9* pCursorBitmap) {
            return this->dev->SetCursorProperties(XHotSpot, YHotSpot, pCursorBitmap);
        }

        __declspec(noinline) void ProxyDirect3DDevice9::SetCursorPosition(int X, int Y, DWORD Flags) {
            return this->dev->SetCursorPosition(X, Y, Flags);
        }

        __declspec(noinline) BOOL ProxyDirect3DDevice9::ShowCursor(BOOL bShow) {
            return this->dev->ShowCursor(bShow);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9::CreateAdditionalSwapChain(D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DSwapChain9** pSwapChain) {
            return this->dev->CreateAdditionalSwapChain(pPresentationParameters, pSwapChain);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9::GetSwapChain(UINT iSwapChain, IDirect3DSwapChain9** pSwapChain) {
            return this->dev->GetSwapChain(iSwapChain, pSwapChain);
        }

        __declspec(noinline) UINT ProxyDirect3DDevice9::GetNumberOfSwapChains() {
            return this->dev->GetNumberOfSwapChains();
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9::Reset(D3DPRESENT_PARAMETERS* pPresentationParameters) {
            return this->dev->Reset(pPresentationParameters);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9::Present(CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion) {
            return this->dev->Present(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9::GetBackBuffer(UINT iSwapChain, UINT iBackBuffer, D3DBACKBUFFER_TYPE Type, IDirect3DSurface9** ppBackBuffer) {
            return this->dev->GetBackBuffer(iSwapChain, iBackBuffer, Type, ppBackBuffer);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9::GetRasterStatus(UINT iSwapChain, D3DRASTER_STATUS* pRasterStatus) {
            return this->dev->GetRasterStatus(iSwapChain, pRasterStatus);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9::SetDialogBoxMode(BOOL bEnableDialogs) {
            return this->dev->SetDialogBoxMode(bEnableDialogs);
        }

        __declspec(noinline) void ProxyDirect3DDevice9::SetGammaRamp(UINT iSwapChain, DWORD Flags, CONST D3DGAMMARAMP* pRamp) {
            return this->dev->SetGammaRamp(iSwapChain, Flags, pRamp);
        }

        __declspec(noinline) void ProxyDirect3DDevice9::GetGammaRamp(UINT iSwapChain, D3DGAMMARAMP* pRamp) {
            return this->dev->GetGammaRamp(iSwapChain, pRamp);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9::CreateTexture(UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DTexture9** ppTexture, HANDLE* pSharedHandle) {
            return this->dev->CreateTexture(Width, Height, Levels, Usage, Format, Pool, ppTexture, pSharedHandle);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9::CreateVolumeTexture(UINT Width, UINT Height, UINT Depth, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DVolumeTexture9** ppVolumeTexture, HANDLE* pSharedHandle) {
            return this->dev->CreateVolumeTexture(Width, Height, Depth, Levels, Usage, Format, Pool, ppVolumeTexture, pSharedHandle);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9::CreateCubeTexture(UINT EdgeLength, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DCubeTexture9** ppCubeTexture, HANDLE* pSharedHandle) {
            return this->dev->CreateCubeTexture(EdgeLength, Levels, Usage, Format, Pool, ppCubeTexture, pSharedHandle);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9::CreateVertexBuffer(UINT Length, DWORD Usage, DWORD FVF, D3DPOOL Pool, IDirect3DVertexBuffer9** ppVertexBuffer, HANDLE* pSharedHandle) {
            return this->dev->CreateVertexBuffer(Length, Usage, FVF, Pool, ppVertexBuffer, pSharedHandle);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9::CreateIndexBuffer(UINT Length, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DIndexBuffer9** ppIndexBuffer, HANDLE* pSharedHandle) {
            return this->dev->CreateIndexBuffer(Length, Usage, Format, Pool, ppIndexBuffer, pSharedHandle);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9::CreateRenderTarget(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Lockable, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle) {
            return this->dev->CreateRenderTarget(Width, Height, Format, MultiSample, MultisampleQuality, Lockable, ppSurface, pSharedHandle);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9::CreateDepthStencilSurface(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Discard, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle) {
            return this->dev->CreateDepthStencilSurface(Width, Height, Format, MultiSample, MultisampleQuality, Discard, ppSurface, pSharedHandle);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9::UpdateSurface(IDirect3DSurface9* pSourceSurface, CONST RECT* pSourceRect, IDirect3DSurface9* pDestinationSurface, CONST POINT* pDestPoint) {
            return this->dev->UpdateSurface(pSourceSurface, pSourceRect, pDestinationSurface, pDestPoint);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9::UpdateTexture(IDirect3DBaseTexture9* pSourceTexture, IDirect3DBaseTexture9* pDestinationTexture) {
            return this->dev->UpdateTexture(pSourceTexture, pDestinationTexture);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9::GetRenderTargetData(IDirect3DSurface9* pRenderTarget, IDirect3DSurface9* pDestSurface) {
            return this->dev->GetRenderTargetData(pRenderTarget, pDestSurface);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9::GetFrontBufferData(UINT iSwapChain, IDirect3DSurface9* pDestSurface) {
            return this->dev->GetFrontBufferData(iSwapChain, pDestSurface);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9::StretchRect(IDirect3DSurface9* pSourceSurface, CONST RECT* pSourceRect, IDirect3DSurface9* pDestSurface, CONST RECT* pDestRect, D3DTEXTUREFILTERTYPE Filter) {
            return this->dev->StretchRect(pSourceSurface, pSourceRect, pDestSurface, pDestRect, Filter);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9::ColorFill(IDirect3DSurface9* pSurface, CONST RECT* pRect, D3DCOLOR color) {
            return this->dev->ColorFill(pSurface, pRect, color);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9::CreateOffscreenPlainSurface(UINT Width, UINT Height, D3DFORMAT Format, D3DPOOL Pool, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle) {
            return this->dev->CreateOffscreenPlainSurface(Width, Height, Format, Pool, ppSurface, pSharedHandle);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9::SetRenderTarget(DWORD RenderTargetIndex, IDirect3DSurface9* pRenderTarget) {
            return this->dev->SetRenderTarget(RenderTargetIndex, pRenderTarget);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9::GetRenderTarget(DWORD RenderTargetIndex, IDirect3DSurface9** ppRenderTarget) {
            return this->dev->GetRenderTarget(RenderTargetIndex, ppRenderTarget);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9::SetDepthStencilSurface(IDirect3DSurface9* pNewZStencil) {
            return this->dev->SetDepthStencilSurface(pNewZStencil);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9::GetDepthStencilSurface(IDirect3DSurface9** ppZStencilSurface) {
            return this->dev->GetDepthStencilSurface(ppZStencilSurface);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9::BeginScene() {
            return this->dev->BeginScene();
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9::EndScene() {
            return this->dev->EndScene();
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9::Clear(DWORD Count, CONST D3DRECT* pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil) {
            return this->dev->Clear(Count, pRects, Flags, Color, Z, Stencil);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9::SetTransform(D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX* pMatrix) {
            return this->dev->SetTransform(State, pMatrix);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9::GetTransform(D3DTRANSFORMSTATETYPE State, D3DMATRIX* pMatrix) {
            return this->dev->GetTransform(State, pMatrix);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9::MultiplyTransform(D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX* pMatrix) {
            return this->dev->MultiplyTransform(State, pMatrix);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9::SetViewport(CONST D3DVIEWPORT9* pViewport) {
            return this->dev->SetViewport(pViewport);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9::GetViewport(D3DVIEWPORT9* pViewport) {
            return this->dev->GetViewport(pViewport);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9::SetMaterial(CONST D3DMATERIAL9* pMaterial) {
            return this->dev->SetMaterial(pMaterial);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9::GetMaterial(D3DMATERIAL9* pMaterial) {
            return this->dev->GetMaterial(pMaterial);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9::SetLight(DWORD Index, CONST D3DLIGHT9* pLight) {
            return this->dev->SetLight(Index, pLight);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9::GetLight(DWORD Index, D3DLIGHT9* pLight) {
            return this->dev->GetLight(Index, pLight);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9::LightEnable(DWORD Index, BOOL Enable) {
            return this->dev->LightEnable(Index, Enable);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9::GetLightEnable(DWORD Index, BOOL* pEnable) {
            return this->dev->GetLightEnable(Index, pEnable);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9::SetClipPlane(DWORD Index, CONST float* pPlane) {
            return this->dev->SetClipPlane(Index, pPlane);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9::GetClipPlane(DWORD Index, float* pPlane) {
            return this->dev->GetClipPlane(Index, pPlane);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9::SetRenderState(D3DRENDERSTATETYPE State, DWORD Value) {
            return this->dev->SetRenderState(State, Value);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9::GetRenderState(D3DRENDERSTATETYPE State, DWORD* pValue) {
            return this->dev->GetRenderState(State, pValue);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9::CreateStateBlock(D3DSTATEBLOCKTYPE Type, IDirect3DStateBlock9** ppSB) {
            return this->dev->CreateStateBlock(Type, ppSB);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9::BeginStateBlock() {
            return this->dev->BeginStateBlock();
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9::EndStateBlock(IDirect3DStateBlock9** ppSB) {
            return this->dev->EndStateBlock(ppSB);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9::SetClipStatus(CONST D3DCLIPSTATUS9* pClipStatus) {
            return this->dev->SetClipStatus(pClipStatus);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9::GetClipStatus(D3DCLIPSTATUS9* pClipStatus) {
            return this->dev->GetClipStatus(pClipStatus);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9::GetTexture(DWORD Stage, IDirect3DBaseTexture9** ppTexture) {
            return this->dev->GetTexture(Stage, ppTexture);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9::SetTexture(DWORD Stage, IDirect3DBaseTexture9* pTexture) {
            return this->dev->SetTexture(Stage, pTexture);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9::GetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD* pValue) {
            return this->dev->GetTextureStageState(Stage, Type, pValue);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9::SetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value) {
            return this->dev->SetTextureStageState(Stage, Type, Value);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9::GetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD* pValue) {
            return this->dev->GetSamplerState(Sampler, Type, pValue);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9::SetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD Value) {
            return this->dev->SetSamplerState(Sampler, Type, Value);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9::ValidateDevice(DWORD* pNumPasses) {
            return this->dev->ValidateDevice(pNumPasses);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9::SetPaletteEntries(UINT PaletteNumber, CONST PALETTEENTRY* pEntries) {
            return this->dev->SetPaletteEntries(PaletteNumber, pEntries);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9::GetPaletteEntries(UINT PaletteNumber, PALETTEENTRY* pEntries) {
            return this->dev->GetPaletteEntries(PaletteNumber, pEntries);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9::SetCurrentTexturePalette(UINT PaletteNumber) {
            return this->dev->SetCurrentTexturePalette(PaletteNumber);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9::GetCurrentTexturePalette(UINT *PaletteNumber) {
            return this->dev->GetCurrentTexturePalette(PaletteNumber);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9::SetScissorRect(CONST RECT* pRect) {
            return this->dev->SetScissorRect(pRect);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9::GetScissorRect(RECT* pRect) {
            return this->dev->GetScissorRect(pRect);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9::SetSoftwareVertexProcessing(BOOL bSoftware) {
            return this->dev->SetSoftwareVertexProcessing(bSoftware);
        }

        __declspec(noinline) BOOL ProxyDirect3DDevice9::GetSoftwareVertexProcessing() {
            return this->dev->GetSoftwareVertexProcessing();
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9::SetNPatchMode(float nSegments) {
            return this->dev->SetNPatchMode(nSegments);
        }

        __declspec(noinline) float ProxyDirect3DDevice9::GetNPatchMode() {
            return this->dev->GetNPatchMode();
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9::DrawPrimitive(D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount) {
            return this->dev->DrawPrimitive(PrimitiveType, StartVertex, PrimitiveCount);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9::DrawIndexedPrimitive(D3DPRIMITIVETYPE PrimitiveType, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount) {
            return this->dev->DrawIndexedPrimitive(PrimitiveType, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9::DrawPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT PrimitiveCount, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride) {
            return this->dev->DrawPrimitiveUP(PrimitiveType, PrimitiveCount, pVertexStreamZeroData, VertexStreamZeroStride);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9::DrawIndexedPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT MinVertexIndex, UINT NumVertices, UINT PrimitiveCount, CONST void* pIndexData, D3DFORMAT IndexDataFormat, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride) {
            return this->dev->DrawIndexedPrimitiveUP(PrimitiveType, MinVertexIndex, NumVertices, PrimitiveCount, pIndexData, IndexDataFormat, pVertexStreamZeroData, VertexStreamZeroStride);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9::ProcessVertices(UINT SrcStartIndex, UINT DestIndex, UINT VertexCount, IDirect3DVertexBuffer9* pDestBuffer, IDirect3DVertexDeclaration9* pVertexDecl, DWORD Flags) {
            return this->dev->ProcessVertices(SrcStartIndex, DestIndex, VertexCount, pDestBuffer, pVertexDecl, Flags);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9::CreateVertexDeclaration(CONST D3DVERTEXELEMENT9* pVertexElements, IDirect3DVertexDeclaration9** ppDecl) {
            return this->dev->CreateVertexDeclaration(pVertexElements, ppDecl);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9::SetVertexDeclaration(IDirect3DVertexDeclaration9* pDecl) {
            return this->dev->SetVertexDeclaration(pDecl);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9::GetVertexDeclaration(IDirect3DVertexDeclaration9** ppDecl) {
            return this->dev->GetVertexDeclaration(ppDecl);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9::SetFVF(DWORD FVF) {
            return this->dev->SetFVF(FVF);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9::GetFVF(DWORD* pFVF) {
            return this->dev->GetFVF(pFVF);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9::CreateVertexShader(CONST DWORD* pFunction, IDirect3DVertexShader9** ppShader) {
            return this->dev->CreateVertexShader(pFunction, ppShader);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9::SetVertexShader(IDirect3DVertexShader9* pShader) {
            return this->dev->SetVertexShader(pShader);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9::GetVertexShader(IDirect3DVertexShader9** ppShader) {
            return this->dev->GetVertexShader(ppShader);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9::SetVertexShaderConstantF(UINT StartRegister, CONST float* pConstantData, UINT Vector4fCount) {
            return this->dev->SetVertexShaderConstantF(StartRegister, pConstantData, Vector4fCount);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9::GetVertexShaderConstantF(UINT StartRegister, float* pConstantData, UINT Vector4fCount) {
            return this->dev->GetVertexShaderConstantF(StartRegister, pConstantData, Vector4fCount);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9::SetVertexShaderConstantI(UINT StartRegister, CONST int* pConstantData, UINT Vector4iCount) {
            return this->dev->SetVertexShaderConstantI(StartRegister, pConstantData, Vector4iCount);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9::GetVertexShaderConstantI(UINT StartRegister, int* pConstantData, UINT Vector4iCount) {
            return this->dev->GetVertexShaderConstantI(StartRegister, pConstantData, Vector4iCount);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9::SetVertexShaderConstantB(UINT StartRegister, CONST BOOL* pConstantData, UINT BoolCount) {
            return this->dev->SetVertexShaderConstantB(StartRegister, pConstantData, BoolCount);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9::GetVertexShaderConstantB(UINT StartRegister, BOOL* pConstantData, UINT BoolCount) {
            return this->dev->GetVertexShaderConstantB(StartRegister, pConstantData, BoolCount);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9::SetStreamSource(UINT StreamNumber, IDirect3DVertexBuffer9* pStreamData, UINT OffsetInBytes, UINT Stride) {
            return this->dev->SetStreamSource(StreamNumber, pStreamData, OffsetInBytes, Stride);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9::GetStreamSource(UINT StreamNumber, IDirect3DVertexBuffer9** ppStreamData, UINT* pOffsetInBytes, UINT* pStride) {
            return this->dev->GetStreamSource(StreamNumber, ppStreamData, pOffsetInBytes, pStride);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9::SetStreamSourceFreq(UINT StreamNumber, UINT Setting) {
            return this->dev->SetStreamSourceFreq(StreamNumber, Setting);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9::GetStreamSourceFreq(UINT StreamNumber, UINT* pSetting) {
            return this->dev->GetStreamSourceFreq(StreamNumber, pSetting);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9::SetIndices(IDirect3DIndexBuffer9* pIndexData) {
            return this->dev->SetIndices(pIndexData);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9::GetIndices(IDirect3DIndexBuffer9** ppIndexData) {
            return this->dev->GetIndices(ppIndexData);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9::CreatePixelShader(CONST DWORD* pFunction, IDirect3DPixelShader9** ppShader) {
            return this->dev->CreatePixelShader(pFunction, ppShader);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9::SetPixelShader(IDirect3DPixelShader9* pShader) {
            return this->dev->SetPixelShader(pShader);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9::GetPixelShader(IDirect3DPixelShader9** ppShader) {
            return this->dev->GetPixelShader(ppShader);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9::SetPixelShaderConstantF(UINT StartRegister, CONST float* pConstantData, UINT Vector4fCount) {
            return this->dev->SetPixelShaderConstantF(StartRegister, pConstantData, Vector4fCount);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9::GetPixelShaderConstantF(UINT StartRegister, float* pConstantData, UINT Vector4fCount) {
            return this->dev->GetPixelShaderConstantF(StartRegister, pConstantData, Vector4fCount);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9::SetPixelShaderConstantI(UINT StartRegister, CONST int* pConstantData, UINT Vector4iCount) {
            return this->dev->SetPixelShaderConstantI(StartRegister, pConstantData, Vector4iCount);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9::GetPixelShaderConstantI(UINT StartRegister, int* pConstantData, UINT Vector4iCount) {
            return this->dev->GetPixelShaderConstantI(StartRegister, pConstantData, Vector4iCount);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9::SetPixelShaderConstantB(UINT StartRegister, CONST BOOL* pConstantData, UINT BoolCount) {
            return this->dev->SetPixelShaderConstantB(StartRegister, pConstantData, BoolCount);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9::GetPixelShaderConstantB(UINT StartRegister, BOOL* pConstantData, UINT BoolCount) {
            return this->dev->GetPixelShaderConstantB(StartRegister, pConstantData, BoolCount);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9::DrawRectPatch(UINT Handle, CONST float* pNumSegs, CONST D3DRECTPATCH_INFO* pRectPatchInfo) {
            return this->dev->DrawRectPatch(Handle, pNumSegs, pRectPatchInfo);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9::DrawTriPatch(UINT Handle, CONST float* pNumSegs, CONST D3DTRIPATCH_INFO* pTriPatchInfo) {
            return this->dev->DrawTriPatch(Handle, pNumSegs, pTriPatchInfo);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9::DeletePatch(UINT Handle) {
            return this->dev->DeletePatch(Handle);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9::CreateQuery(D3DQUERYTYPE Type, IDirect3DQuery9** ppQuery) {
            return this->dev->CreateQuery(Type, ppQuery);
        }

    
        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::QueryInterface(REFIID riid, void** ppvObj) {
            return this->dev->QueryInterface(riid, ppvObj);
        }

        __declspec(noinline) ULONG ProxyDirect3DDevice9Ex::AddRef() {
            return this->dev->AddRef();
        }

        __declspec(noinline) ULONG ProxyDirect3DDevice9Ex::Release() {
            return this->dev->Release();
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::TestCooperativeLevel() {
            return this->dev->TestCooperativeLevel();
        }

        __declspec(noinline) UINT ProxyDirect3DDevice9Ex::GetAvailableTextureMem() {
            return this->dev->GetAvailableTextureMem();
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::EvictManagedResources() {
            return this->dev->EvictManagedResources();
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::GetDirect3D(IDirect3D9** ppD3D9) {
            return this->dev->GetDirect3D(ppD3D9);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::GetDeviceCaps(D3DCAPS9* pCaps) {
            return this->dev->GetDeviceCaps(pCaps);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::GetDisplayMode(UINT iSwapChain, D3DDISPLAYMODE* pMode) {
            return this->dev->GetDisplayMode(iSwapChain, pMode);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::GetCreationParameters(D3DDEVICE_CREATION_PARAMETERS *pParameters) {
            return this->dev->GetCreationParameters(pParameters);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::SetCursorProperties(UINT XHotSpot, UINT YHotSpot, IDirect3DSurface9* pCursorBitmap) {
            return this->dev->SetCursorProperties(XHotSpot, YHotSpot, pCursorBitmap);
        }

        __declspec(noinline) void ProxyDirect3DDevice9Ex::SetCursorPosition(int X, int Y, DWORD Flags) {
            return this->dev->SetCursorPosition(X, Y, Flags);
        }

        __declspec(noinline) BOOL ProxyDirect3DDevice9Ex::ShowCursor(BOOL bShow) {
            return this->dev->ShowCursor(bShow);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::CreateAdditionalSwapChain(D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DSwapChain9** pSwapChain) {
            return this->dev->CreateAdditionalSwapChain(pPresentationParameters, pSwapChain);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::GetSwapChain(UINT iSwapChain, IDirect3DSwapChain9** pSwapChain) {
            return this->dev->GetSwapChain(iSwapChain, pSwapChain);
        }

        __declspec(noinline) UINT ProxyDirect3DDevice9Ex::GetNumberOfSwapChains() {
            return this->dev->GetNumberOfSwapChains();
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::Reset(D3DPRESENT_PARAMETERS* pPresentationParameters) {
            return this->dev->Reset(pPresentationParameters);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::Present(CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion) {
            return this->dev->Present(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::GetBackBuffer(UINT iSwapChain, UINT iBackBuffer, D3DBACKBUFFER_TYPE Type, IDirect3DSurface9** ppBackBuffer) {
            return this->dev->GetBackBuffer(iSwapChain, iBackBuffer, Type, ppBackBuffer);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::GetRasterStatus(UINT iSwapChain, D3DRASTER_STATUS* pRasterStatus) {
            return this->dev->GetRasterStatus(iSwapChain, pRasterStatus);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::SetDialogBoxMode(BOOL bEnableDialogs) {
            return this->dev->SetDialogBoxMode(bEnableDialogs);
        }

        __declspec(noinline) void ProxyDirect3DDevice9Ex::SetGammaRamp(UINT iSwapChain, DWORD Flags, CONST D3DGAMMARAMP* pRamp) {
            return this->dev->SetGammaRamp(iSwapChain, Flags, pRamp);
        }

        __declspec(noinline) void ProxyDirect3DDevice9Ex::GetGammaRamp(UINT iSwapChain, D3DGAMMARAMP* pRamp) {
            return this->dev->GetGammaRamp(iSwapChain, pRamp);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::CreateTexture(UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DTexture9** ppTexture, HANDLE* pSharedHandle) {
            return this->dev->CreateTexture(Width, Height, Levels, Usage, Format, Pool, ppTexture, pSharedHandle);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::CreateVolumeTexture(UINT Width, UINT Height, UINT Depth, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DVolumeTexture9** ppVolumeTexture, HANDLE* pSharedHandle) {
            return this->dev->CreateVolumeTexture(Width, Height, Depth, Levels, Usage, Format, Pool, ppVolumeTexture, pSharedHandle);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::CreateCubeTexture(UINT EdgeLength, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DCubeTexture9** ppCubeTexture, HANDLE* pSharedHandle) {
            return this->dev->CreateCubeTexture(EdgeLength, Levels, Usage, Format, Pool, ppCubeTexture, pSharedHandle);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::CreateVertexBuffer(UINT Length, DWORD Usage, DWORD FVF, D3DPOOL Pool, IDirect3DVertexBuffer9** ppVertexBuffer, HANDLE* pSharedHandle) {
            return this->dev->CreateVertexBuffer(Length, Usage, FVF, Pool, ppVertexBuffer, pSharedHandle);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::CreateIndexBuffer(UINT Length, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DIndexBuffer9** ppIndexBuffer, HANDLE* pSharedHandle) {
            return this->dev->CreateIndexBuffer(Length, Usage, Format, Pool, ppIndexBuffer, pSharedHandle);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::CreateRenderTarget(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Lockable, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle) {
            return this->dev->CreateRenderTarget(Width, Height, Format, MultiSample, MultisampleQuality, Lockable, ppSurface, pSharedHandle);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::CreateDepthStencilSurface(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Discard, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle) {
            return this->dev->CreateDepthStencilSurface(Width, Height, Format, MultiSample, MultisampleQuality, Discard, ppSurface, pSharedHandle);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::UpdateSurface(IDirect3DSurface9* pSourceSurface, CONST RECT* pSourceRect, IDirect3DSurface9* pDestinationSurface, CONST POINT* pDestPoint) {
            return this->dev->UpdateSurface(pSourceSurface, pSourceRect, pDestinationSurface, pDestPoint);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::UpdateTexture(IDirect3DBaseTexture9* pSourceTexture, IDirect3DBaseTexture9* pDestinationTexture) {
            return this->dev->UpdateTexture(pSourceTexture, pDestinationTexture);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::GetRenderTargetData(IDirect3DSurface9* pRenderTarget, IDirect3DSurface9* pDestSurface) {
            return this->dev->GetRenderTargetData(pRenderTarget, pDestSurface);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::GetFrontBufferData(UINT iSwapChain, IDirect3DSurface9* pDestSurface) {
            return this->dev->GetFrontBufferData(iSwapChain, pDestSurface);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::StretchRect(IDirect3DSurface9* pSourceSurface, CONST RECT* pSourceRect, IDirect3DSurface9* pDestSurface, CONST RECT* pDestRect, D3DTEXTUREFILTERTYPE Filter) {
            return this->dev->StretchRect(pSourceSurface, pSourceRect, pDestSurface, pDestRect, Filter);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::ColorFill(IDirect3DSurface9* pSurface, CONST RECT* pRect, D3DCOLOR color) {
            return this->dev->ColorFill(pSurface, pRect, color);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::CreateOffscreenPlainSurface(UINT Width, UINT Height, D3DFORMAT Format, D3DPOOL Pool, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle) {
            return this->dev->CreateOffscreenPlainSurface(Width, Height, Format, Pool, ppSurface, pSharedHandle);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::SetRenderTarget(DWORD RenderTargetIndex, IDirect3DSurface9* pRenderTarget) {
            return this->dev->SetRenderTarget(RenderTargetIndex, pRenderTarget);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::GetRenderTarget(DWORD RenderTargetIndex, IDirect3DSurface9** ppRenderTarget) {
            return this->dev->GetRenderTarget(RenderTargetIndex, ppRenderTarget);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::SetDepthStencilSurface(IDirect3DSurface9* pNewZStencil) {
            return this->dev->SetDepthStencilSurface(pNewZStencil);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::GetDepthStencilSurface(IDirect3DSurface9** ppZStencilSurface) {
            return this->dev->GetDepthStencilSurface(ppZStencilSurface);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::BeginScene() {
            return this->dev->BeginScene();
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::EndScene() {
            return this->dev->EndScene();
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::Clear(DWORD Count, CONST D3DRECT* pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil) {
            return this->dev->Clear(Count, pRects, Flags, Color, Z, Stencil);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::SetTransform(D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX* pMatrix) {
            return this->dev->SetTransform(State, pMatrix);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::GetTransform(D3DTRANSFORMSTATETYPE State, D3DMATRIX* pMatrix) {
            return this->dev->GetTransform(State, pMatrix);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::MultiplyTransform(D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX* pMatrix) {
            return this->dev->MultiplyTransform(State, pMatrix);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::SetViewport(CONST D3DVIEWPORT9* pViewport) {
            return this->dev->SetViewport(pViewport);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::GetViewport(D3DVIEWPORT9* pViewport) {
            return this->dev->GetViewport(pViewport);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::SetMaterial(CONST D3DMATERIAL9* pMaterial) {
            return this->dev->SetMaterial(pMaterial);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::GetMaterial(D3DMATERIAL9* pMaterial) {
            return this->dev->GetMaterial(pMaterial);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::SetLight(DWORD Index, CONST D3DLIGHT9* pLight) {
            return this->dev->SetLight(Index, pLight);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::GetLight(DWORD Index, D3DLIGHT9* pLight) {
            return this->dev->GetLight(Index, pLight);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::LightEnable(DWORD Index, BOOL Enable) {
            return this->dev->LightEnable(Index, Enable);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::GetLightEnable(DWORD Index, BOOL* pEnable) {
            return this->dev->GetLightEnable(Index, pEnable);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::SetClipPlane(DWORD Index, CONST float* pPlane) {
            return this->dev->SetClipPlane(Index, pPlane);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::GetClipPlane(DWORD Index, float* pPlane) {
            return this->dev->GetClipPlane(Index, pPlane);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::SetRenderState(D3DRENDERSTATETYPE State, DWORD Value) {
            return this->dev->SetRenderState(State, Value);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::GetRenderState(D3DRENDERSTATETYPE State, DWORD* pValue) {
            return this->dev->GetRenderState(State, pValue);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::CreateStateBlock(D3DSTATEBLOCKTYPE Type, IDirect3DStateBlock9** ppSB) {
            return this->dev->CreateStateBlock(Type, ppSB);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::BeginStateBlock() {
            return this->dev->BeginStateBlock();
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::EndStateBlock(IDirect3DStateBlock9** ppSB) {
            return this->dev->EndStateBlock(ppSB);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::SetClipStatus(CONST D3DCLIPSTATUS9* pClipStatus) {
            return this->dev->SetClipStatus(pClipStatus);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::GetClipStatus(D3DCLIPSTATUS9* pClipStatus) {
            return this->dev->GetClipStatus(pClipStatus);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::GetTexture(DWORD Stage, IDirect3DBaseTexture9** ppTexture) {
            return this->dev->GetTexture(Stage, ppTexture);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::SetTexture(DWORD Stage, IDirect3DBaseTexture9* pTexture) {
            return this->dev->SetTexture(Stage, pTexture);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::GetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD* pValue) {
            return this->dev->GetTextureStageState(Stage, Type, pValue);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::SetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value) {
            return this->dev->SetTextureStageState(Stage, Type, Value);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::GetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD* pValue) {
            return this->dev->GetSamplerState(Sampler, Type, pValue);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::SetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD Value) {
            return this->dev->SetSamplerState(Sampler, Type, Value);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::ValidateDevice(DWORD* pNumPasses) {
            return this->dev->ValidateDevice(pNumPasses);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::SetPaletteEntries(UINT PaletteNumber, CONST PALETTEENTRY* pEntries) {
            return this->dev->SetPaletteEntries(PaletteNumber, pEntries);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::GetPaletteEntries(UINT PaletteNumber, PALETTEENTRY* pEntries) {
            return this->dev->GetPaletteEntries(PaletteNumber, pEntries);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::SetCurrentTexturePalette(UINT PaletteNumber) {
            return this->dev->SetCurrentTexturePalette(PaletteNumber);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::GetCurrentTexturePalette(UINT *PaletteNumber) {
            return this->dev->GetCurrentTexturePalette(PaletteNumber);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::SetScissorRect(CONST RECT* pRect) {
            return this->dev->SetScissorRect(pRect);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::GetScissorRect(RECT* pRect) {
            return this->dev->GetScissorRect(pRect);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::SetSoftwareVertexProcessing(BOOL bSoftware) {
            return this->dev->SetSoftwareVertexProcessing(bSoftware);
        }

        __declspec(noinline) BOOL ProxyDirect3DDevice9Ex::GetSoftwareVertexProcessing() {
            return this->dev->GetSoftwareVertexProcessing();
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::SetNPatchMode(float nSegments) {
            return this->dev->SetNPatchMode(nSegments);
        }

        __declspec(noinline) float ProxyDirect3DDevice9Ex::GetNPatchMode() {
            return this->dev->GetNPatchMode();
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::DrawPrimitive(D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount) {
            return this->dev->DrawPrimitive(PrimitiveType, StartVertex, PrimitiveCount);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::DrawIndexedPrimitive(D3DPRIMITIVETYPE PrimitiveType, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount) {
            return this->dev->DrawIndexedPrimitive(PrimitiveType, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::DrawPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT PrimitiveCount, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride) {
            return this->dev->DrawPrimitiveUP(PrimitiveType, PrimitiveCount, pVertexStreamZeroData, VertexStreamZeroStride);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::DrawIndexedPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT MinVertexIndex, UINT NumVertices, UINT PrimitiveCount, CONST void* pIndexData, D3DFORMAT IndexDataFormat, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride) {
            return this->dev->DrawIndexedPrimitiveUP(PrimitiveType, MinVertexIndex, NumVertices, PrimitiveCount, pIndexData, IndexDataFormat, pVertexStreamZeroData, VertexStreamZeroStride);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::ProcessVertices(UINT SrcStartIndex, UINT DestIndex, UINT VertexCount, IDirect3DVertexBuffer9* pDestBuffer, IDirect3DVertexDeclaration9* pVertexDecl, DWORD Flags) {
            return this->dev->ProcessVertices(SrcStartIndex, DestIndex, VertexCount, pDestBuffer, pVertexDecl, Flags);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::CreateVertexDeclaration(CONST D3DVERTEXELEMENT9* pVertexElements, IDirect3DVertexDeclaration9** ppDecl) {
            return this->dev->CreateVertexDeclaration(pVertexElements, ppDecl);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::SetVertexDeclaration(IDirect3DVertexDeclaration9* pDecl) {
            return this->dev->SetVertexDeclaration(pDecl);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::GetVertexDeclaration(IDirect3DVertexDeclaration9** ppDecl) {
            return this->dev->GetVertexDeclaration(ppDecl);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::SetFVF(DWORD FVF) {
            return this->dev->SetFVF(FVF);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::GetFVF(DWORD* pFVF) {
            return this->dev->GetFVF(pFVF);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::CreateVertexShader(CONST DWORD* pFunction, IDirect3DVertexShader9** ppShader) {
            return this->dev->CreateVertexShader(pFunction, ppShader);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::SetVertexShader(IDirect3DVertexShader9* pShader) {
            return this->dev->SetVertexShader(pShader);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::GetVertexShader(IDirect3DVertexShader9** ppShader) {
            return this->dev->GetVertexShader(ppShader);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::SetVertexShaderConstantF(UINT StartRegister, CONST float* pConstantData, UINT Vector4fCount) {
            return this->dev->SetVertexShaderConstantF(StartRegister, pConstantData, Vector4fCount);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::GetVertexShaderConstantF(UINT StartRegister, float* pConstantData, UINT Vector4fCount) {
            return this->dev->GetVertexShaderConstantF(StartRegister, pConstantData, Vector4fCount);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::SetVertexShaderConstantI(UINT StartRegister, CONST int* pConstantData, UINT Vector4iCount) {
            return this->dev->SetVertexShaderConstantI(StartRegister, pConstantData, Vector4iCount);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::GetVertexShaderConstantI(UINT StartRegister, int* pConstantData, UINT Vector4iCount) {
            return this->dev->GetVertexShaderConstantI(StartRegister, pConstantData, Vector4iCount);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::SetVertexShaderConstantB(UINT StartRegister, CONST BOOL* pConstantData, UINT BoolCount) {
            return this->dev->SetVertexShaderConstantB(StartRegister, pConstantData, BoolCount);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::GetVertexShaderConstantB(UINT StartRegister, BOOL* pConstantData, UINT BoolCount) {
            return this->dev->GetVertexShaderConstantB(StartRegister, pConstantData, BoolCount);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::SetStreamSource(UINT StreamNumber, IDirect3DVertexBuffer9* pStreamData, UINT OffsetInBytes, UINT Stride) {
            return this->dev->SetStreamSource(StreamNumber, pStreamData, OffsetInBytes, Stride);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::GetStreamSource(UINT StreamNumber, IDirect3DVertexBuffer9** ppStreamData, UINT* pOffsetInBytes, UINT* pStride) {
            return this->dev->GetStreamSource(StreamNumber, ppStreamData, pOffsetInBytes, pStride);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::SetStreamSourceFreq(UINT StreamNumber, UINT Setting) {
            return this->dev->SetStreamSourceFreq(StreamNumber, Setting);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::GetStreamSourceFreq(UINT StreamNumber, UINT* pSetting) {
            return this->dev->GetStreamSourceFreq(StreamNumber, pSetting);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::SetIndices(IDirect3DIndexBuffer9* pIndexData) {
            return this->dev->SetIndices(pIndexData);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::GetIndices(IDirect3DIndexBuffer9** ppIndexData) {
            return this->dev->GetIndices(ppIndexData);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::CreatePixelShader(CONST DWORD* pFunction, IDirect3DPixelShader9** ppShader) {
            return this->dev->CreatePixelShader(pFunction, ppShader);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::SetPixelShader(IDirect3DPixelShader9* pShader) {
            return this->dev->SetPixelShader(pShader);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::GetPixelShader(IDirect3DPixelShader9** ppShader) {
            return this->dev->GetPixelShader(ppShader);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::SetPixelShaderConstantF(UINT StartRegister, CONST float* pConstantData, UINT Vector4fCount) {
            return this->dev->SetPixelShaderConstantF(StartRegister, pConstantData, Vector4fCount);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::GetPixelShaderConstantF(UINT StartRegister, float* pConstantData, UINT Vector4fCount) {
            return this->dev->GetPixelShaderConstantF(StartRegister, pConstantData, Vector4fCount);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::SetPixelShaderConstantI(UINT StartRegister, CONST int* pConstantData, UINT Vector4iCount) {
            return this->dev->SetPixelShaderConstantI(StartRegister, pConstantData, Vector4iCount);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::GetPixelShaderConstantI(UINT StartRegister, int* pConstantData, UINT Vector4iCount) {
            return this->dev->GetPixelShaderConstantI(StartRegister, pConstantData, Vector4iCount);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::SetPixelShaderConstantB(UINT StartRegister, CONST BOOL* pConstantData, UINT BoolCount) {
            return this->dev->SetPixelShaderConstantB(StartRegister, pConstantData, BoolCount);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::GetPixelShaderConstantB(UINT StartRegister, BOOL* pConstantData, UINT BoolCount) {
            return this->dev->GetPixelShaderConstantB(StartRegister, pConstantData, BoolCount);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::DrawRectPatch(UINT Handle, CONST float* pNumSegs, CONST D3DRECTPATCH_INFO* pRectPatchInfo) {
            return this->dev->DrawRectPatch(Handle, pNumSegs, pRectPatchInfo);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::DrawTriPatch(UINT Handle, CONST float* pNumSegs, CONST D3DTRIPATCH_INFO* pTriPatchInfo) {
            return this->dev->DrawTriPatch(Handle, pNumSegs, pTriPatchInfo);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::DeletePatch(UINT Handle) {
            return this->dev->DeletePatch(Handle);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::CreateQuery(D3DQUERYTYPE Type, IDirect3DQuery9** ppQuery) {
            return this->dev->CreateQuery(Type, ppQuery);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::SetConvolutionMonoKernel(UINT width, UINT height, float* rows, float* columns) {
            return this->dev->SetConvolutionMonoKernel(width, height, rows, columns);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::ComposeRects(IDirect3DSurface9* pSrc, IDirect3DSurface9* pDst, IDirect3DVertexBuffer9* pSrcRectDescs, UINT NumRects, IDirect3DVertexBuffer9* pDstRectDescs, D3DCOMPOSERECTSOP Operation, int Xoffset, int Yoffset) {
            return this->dev->ComposeRects(pSrc, pDst, pSrcRectDescs, NumRects, pDstRectDescs, Operation, Xoffset, Yoffset);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::PresentEx(CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion, DWORD dwFlags) {
            return this->dev->PresentEx(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion, dwFlags);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::GetGPUThreadPriority(INT* pPriority) {
            return this->dev->GetGPUThreadPriority(pPriority);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::SetGPUThreadPriority(INT Priority) {
            return this->dev->SetGPUThreadPriority(Priority);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::WaitForVBlank(UINT iSwapChain) {
            return this->dev->WaitForVBlank(iSwapChain);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::CheckResourceResidency(IDirect3DResource9** pResourceArray, UINT32 NumResources) {
            return this->dev->CheckResourceResidency(pResourceArray, NumResources);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::SetMaximumFrameLatency(UINT MaxLatency) {
            return this->dev->SetMaximumFrameLatency(MaxLatency);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::GetMaximumFrameLatency(UINT* pMaxLatency) {
            return this->dev->GetMaximumFrameLatency(pMaxLatency);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::CheckDeviceState(HWND hDestinationWindow) {
            return this->dev->CheckDeviceState(hDestinationWindow);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::CreateRenderTargetEx(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Lockable, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle, DWORD Usage) {
            return this->dev->CreateRenderTargetEx(Width, Height, Format, MultiSample, MultisampleQuality, Lockable, ppSurface, pSharedHandle, Usage);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::CreateOffscreenPlainSurfaceEx(UINT Width, UINT Height, D3DFORMAT Format, D3DPOOL Pool, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle, DWORD Usage) {
            return this->dev->CreateOffscreenPlainSurfaceEx(Width, Height, Format, Pool, ppSurface, pSharedHandle, Usage);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::CreateDepthStencilSurfaceEx(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Discard, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle, DWORD Usage) {
            return this->dev->CreateDepthStencilSurfaceEx(Width, Height, Format, MultiSample, MultisampleQuality, Discard, ppSurface, pSharedHandle, Usage);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::ResetEx(D3DPRESENT_PARAMETERS* pPresentationParameters, D3DDISPLAYMODEEX* pFullscreenDisplayMode) {
            return this->dev->ResetEx(pPresentationParameters, pFullscreenDisplayMode);
        }

        __declspec(noinline) HRESULT ProxyDirect3DDevice9Ex::GetDisplayModeEx(UINT iSwapChain, D3DDISPLAYMODEEX* pMode, D3DDISPLAYROTATION* pRotation) {
            return this->dev->GetDisplayModeEx(iSwapChain, pMode, pRotation);
        }


    }
}