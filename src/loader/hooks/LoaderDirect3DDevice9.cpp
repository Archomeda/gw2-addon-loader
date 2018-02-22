#include "LoaderDirect3DDevice9.h"
#include <iomanip>
#include <set>
#include <sstream>
#include <string>
#include "../log.h"
#include "../addons/addons_manager.h"

using namespace std;

namespace loader {
    namespace hooks {

        PreReset_t PreResetHook = nullptr;
        PostReset_t PostResetHook = nullptr;
        PrePresent_t PrePresentHook = nullptr;


        const DWORD GuiTextVertexShader[] = {
            0xfffe0300, 0x05000051, 0xa00f0004, 0x00000000, 0x00000000,
            0x00000000, 0x00000000, 0x0200001f, 0x80000000, 0x900f0000,
            0x0200001f, 0x80000003, 0x900f0003, 0x0200001f, 0x80000005,
            0x900f0007, 0x0200001f, 0x80000005, 0xe00f0000, 0x0200001f,
            0x80000000, 0xe00f0001, 0x02000001, 0x80070000, 0xa0000004,
            0x02000001, 0x80070000, 0xa0000004, 0x03000009, 0xe0010001,
            0x90e40000, 0xa0e40000, 0x03000009, 0xe0020001, 0x90e40000,
            0xa0e40001, 0x03000009, 0xe0040001, 0x90e40000, 0xa0e40002,
            0x03000009, 0xe0080001, 0x90e40000, 0xa0e40003, 0x02000001,
            0xe0030000, 0x90e40007
        };
        const int GuiTextVertexShaderLength = sizeof(GuiTextVertexShader) / sizeof(*GuiTextVertexShader);

        const DWORD GuiIconVertexShader[] = {
            0xfffe0300, 0x05000051, 0xa00f0006, 0x00000000, 0x3f800000,
            0x00000000, 0x00000000, 0x0200001f, 0x80000000, 0x900f0000,
            0x0200001f, 0x80000003, 0x900f0003, 0x0200001f, 0x80000005,
            0x900f0007, 0x0200001f, 0x80010005, 0x900f0008, 0x0200001f,
            0x80020005, 0x900f0009, 0x0200001f, 0x80030005, 0x900f000a,
            0x0200001f, 0x80000005, 0xe00f0000, 0x0200001f, 0x80000000,
            0xe00f0001, 0x02000001, 0x80070000, 0xa0000006, 0x02000001,
            0x80070000, 0xa0000006, 0x03000009, 0xe0010001, 0x90e40000,
            0xa0e40000, 0x03000009, 0xe0020001, 0x90e40000, 0xa0e40001,
            0x03000009, 0xe0040001, 0x90e40000, 0xa0e40002, 0x03000009,
            0xe0080001, 0x90e40000, 0xa0e40003, 0x02000001, 0x80030001,
            0x90e40007, 0x02000001, 0x800c0001, 0xa0550006, 0x03000009,
            0x80010002, 0x80e40001, 0xa0e40004, 0x03000009, 0x80020002,
            0x80e40001, 0xa0e40005, 0x02000001, 0xe0030000, 0x80440002
        };
        const int GuiIconVertexShaderLength = sizeof(GuiIconVertexShader) / sizeof(*GuiIconVertexShader);

        set<IDirect3DVertexShader9*> GuiVertexShaderPtrs;

        DWORD stateColorWriteEnable = 0;
        DWORD stateZEnable = 0;
        DWORD stateZFunc = 0;

        bool PrePresentGuiDone = false;
        int PrePostProcessingDone = 0;


        int GetShaderFunctionLength(const DWORD* pFunction) {
            int i = 0;
            while ((pFunction[i++] & D3DSI_OPCODE_MASK) != D3DSIO_END);
            return i - 1;
        }

        bool CheckShaderPattern(const DWORD* pFunction, int functionLength, const DWORD* pattern, int patternLength) {
            if (functionLength != patternLength) {
                return false;
            }
            for (int i = 0; i < patternLength; ++i) {
                if (pattern[i] != pFunction[i]) {
                    return false;
                }
            }
            return true;
        }


        HRESULT LoaderDirect3DDevice9::QueryInterface(REFIID riid, void** ppvObj) {
            return this->dev->QueryInterface(riid, ppvObj);
        }

        ULONG LoaderDirect3DDevice9::AddRef() {
            return this->dev->AddRef();
        }

        ULONG LoaderDirect3DDevice9::Release() {
            ULONG count = this->dev->Release();
            if (count == 0) {
                delete this;
            }
            return count;
        }

        HRESULT LoaderDirect3DDevice9::TestCooperativeLevel() {
            return this->dev->TestCooperativeLevel();
        }

        UINT LoaderDirect3DDevice9::GetAvailableTextureMem() {
            return this->dev->GetAvailableTextureMem();
        }

        HRESULT LoaderDirect3DDevice9::EvictManagedResources() {
            return this->dev->EvictManagedResources();
        }

        HRESULT LoaderDirect3DDevice9::GetDirect3D(IDirect3D9** ppD3D9) {
            return this->dev->GetDirect3D(ppD3D9);
        }

        HRESULT LoaderDirect3DDevice9::GetDeviceCaps(D3DCAPS9* pCaps) {
            return this->dev->GetDeviceCaps(pCaps);
        }

        HRESULT LoaderDirect3DDevice9::GetDisplayMode(UINT iSwapChain, D3DDISPLAYMODE* pMode) {
            return this->dev->GetDisplayMode(iSwapChain, pMode);
        }

        HRESULT LoaderDirect3DDevice9::GetCreationParameters(D3DDEVICE_CREATION_PARAMETERS *pParameters) {
            return this->dev->GetCreationParameters(pParameters);
        }

        HRESULT LoaderDirect3DDevice9::SetCursorProperties(UINT XHotSpot, UINT YHotSpot, IDirect3DSurface9* pCursorBitmap) {
            return this->dev->SetCursorProperties(XHotSpot, YHotSpot, pCursorBitmap);
        }

        void LoaderDirect3DDevice9::SetCursorPosition(int X, int Y, DWORD Flags) {
            return this->dev->SetCursorPosition(X, Y, Flags);
        }

        BOOL LoaderDirect3DDevice9::ShowCursor(BOOL bShow) {
            return this->dev->ShowCursor(bShow);
        }

        HRESULT LoaderDirect3DDevice9::CreateAdditionalSwapChain(D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DSwapChain9** pSwapChain) {
            return this->dev->CreateAdditionalSwapChain(pPresentationParameters, pSwapChain);
        }

        HRESULT LoaderDirect3DDevice9::GetSwapChain(UINT iSwapChain, IDirect3DSwapChain9** pSwapChain) {
            return this->dev->GetSwapChain(iSwapChain, pSwapChain);
        }

        UINT LoaderDirect3DDevice9::GetNumberOfSwapChains() {
            return this->dev->GetNumberOfSwapChains();
        }

        HRESULT LoaderDirect3DDevice9::Reset(D3DPRESENT_PARAMETERS* pPresentationParameters) {
            PreResetHook(this->dev, pPresentationParameters);
            addons::AdvPreReset(this->dev, pPresentationParameters);

            HRESULT hr = this->dev->Reset(pPresentationParameters);
            if (hr != D3D_OK) {
                // Fail
                return hr;
            }

            PostResetHook(this->dev, pPresentationParameters);
            addons::AdvPostReset(this->dev, pPresentationParameters);

            return hr;
        }

        HRESULT LoaderDirect3DDevice9::Present(CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion) {
            PrePresentHook(this->dev, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
            addons::AdvPrePresent(this->dev, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);

            HRESULT hr = this->dev->Present(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
            if (hr != D3D_OK) {
                // Fail
                return hr;
            }

            addons::AdvPostPresent(this->dev, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);

            // Reset this for a new frame
            PrePresentGuiDone = false;
            PrePostProcessingDone = 0;
            
            return hr;
        }

        HRESULT LoaderDirect3DDevice9::GetBackBuffer(UINT iSwapChain, UINT iBackBuffer, D3DBACKBUFFER_TYPE Type, IDirect3DSurface9** ppBackBuffer) {
            return this->dev->GetBackBuffer(iSwapChain, iBackBuffer, Type, ppBackBuffer);
        }

        HRESULT LoaderDirect3DDevice9::GetRasterStatus(UINT iSwapChain, D3DRASTER_STATUS* pRasterStatus) {
            return this->dev->GetRasterStatus(iSwapChain, pRasterStatus);
        }

        HRESULT LoaderDirect3DDevice9::SetDialogBoxMode(BOOL bEnableDialogs) {
            return this->dev->SetDialogBoxMode(bEnableDialogs);
        }

        void LoaderDirect3DDevice9::SetGammaRamp(UINT iSwapChain, DWORD Flags, CONST D3DGAMMARAMP* pRamp) {
            return this->dev->SetGammaRamp(iSwapChain, Flags, pRamp);
        }

        void LoaderDirect3DDevice9::GetGammaRamp(UINT iSwapChain, D3DGAMMARAMP* pRamp) {
            return this->dev->GetGammaRamp(iSwapChain, pRamp);
        }

        HRESULT LoaderDirect3DDevice9::CreateTexture(UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DTexture9** ppTexture, HANDLE* pSharedHandle) {
            IDirect3DTexture9* pOldTexture = *ppTexture;
            HRESULT hr = addons::AdvPreCreateTexture(this->dev, Width, Height, Levels, Usage, Format, Pool, ppTexture, pSharedHandle);
            if (hr != D3D_OK) {
                // Fail
                return hr;
            }

            if (*ppTexture == pOldTexture) {
                hr = this->dev->CreateTexture(Width, Height, Levels, Usage, Format, Pool, ppTexture, pSharedHandle);
                if (hr != D3D_OK) {
                    // Fail
                    return hr;
                }
            }

            addons::AdvPostCreateTexture(this->dev, *ppTexture, Width, Height, Levels, Usage, Format, Pool, pSharedHandle);

            return hr;
        }

        HRESULT LoaderDirect3DDevice9::CreateVolumeTexture(UINT Width, UINT Height, UINT Depth, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DVolumeTexture9** ppVolumeTexture, HANDLE* pSharedHandle) {
            return this->dev->CreateVolumeTexture(Width, Height, Depth, Levels, Usage, Format, Pool, ppVolumeTexture, pSharedHandle);
        }

        HRESULT LoaderDirect3DDevice9::CreateCubeTexture(UINT EdgeLength, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DCubeTexture9** ppCubeTexture, HANDLE* pSharedHandle) {
            return this->dev->CreateCubeTexture(EdgeLength, Levels, Usage, Format, Pool, ppCubeTexture, pSharedHandle);
        }

        HRESULT LoaderDirect3DDevice9::CreateVertexBuffer(UINT Length, DWORD Usage, DWORD FVF, D3DPOOL Pool, IDirect3DVertexBuffer9** ppVertexBuffer, HANDLE* pSharedHandle) {
            return this->dev->CreateVertexBuffer(Length, Usage, FVF, Pool, ppVertexBuffer, pSharedHandle);
        }

        HRESULT LoaderDirect3DDevice9::CreateIndexBuffer(UINT Length, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DIndexBuffer9** ppIndexBuffer, HANDLE* pSharedHandle) {
            return this->dev->CreateIndexBuffer(Length, Usage, Format, Pool, ppIndexBuffer, pSharedHandle);
        }

        HRESULT LoaderDirect3DDevice9::CreateRenderTarget(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Lockable, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle) {
            IDirect3DSurface9* pOldSurface = *ppSurface;
            HRESULT hr = addons::AdvPreCreateRenderTarget(this->dev, Width, Height, Format, MultiSample, MultisampleQuality, Lockable, ppSurface, pSharedHandle);
            if (hr != D3D_OK) {
                // Fail
                return hr;
            }

            if (*ppSurface == pOldSurface) {
                hr = this->dev->CreateRenderTarget(Width, Height, Format, MultiSample, MultisampleQuality, Lockable, ppSurface, pSharedHandle);
                if (hr != D3D_OK) {
                    // Fail
                    return hr;
                }
            }

            addons::AdvPostCreateRenderTarget(this->dev, *ppSurface, Width, Height, Format, MultiSample, MultisampleQuality, Lockable, pSharedHandle);

            return hr;
        }

        HRESULT LoaderDirect3DDevice9::CreateDepthStencilSurface(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Discard, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle) {
            return this->dev->CreateDepthStencilSurface(Width, Height, Format, MultiSample, MultisampleQuality, Discard, ppSurface, pSharedHandle);
        }

        HRESULT LoaderDirect3DDevice9::UpdateSurface(IDirect3DSurface9* pSourceSurface, CONST RECT* pSourceRect, IDirect3DSurface9* pDestinationSurface, CONST POINT* pDestPoint) {
            return this->dev->UpdateSurface(pSourceSurface, pSourceRect, pDestinationSurface, pDestPoint);
        }

        HRESULT LoaderDirect3DDevice9::UpdateTexture(IDirect3DBaseTexture9* pSourceTexture, IDirect3DBaseTexture9* pDestinationTexture) {
            return this->dev->UpdateTexture(pSourceTexture, pDestinationTexture);
        }

        HRESULT LoaderDirect3DDevice9::GetRenderTargetData(IDirect3DSurface9* pRenderTarget, IDirect3DSurface9* pDestSurface) {
            return this->dev->GetRenderTargetData(pRenderTarget, pDestSurface);
        }

        HRESULT LoaderDirect3DDevice9::GetFrontBufferData(UINT iSwapChain, IDirect3DSurface9* pDestSurface) {
            return this->dev->GetFrontBufferData(iSwapChain, pDestSurface);
        }

        HRESULT LoaderDirect3DDevice9::StretchRect(IDirect3DSurface9* pSourceSurface, CONST RECT* pSourceRect, IDirect3DSurface9* pDestSurface, CONST RECT* pDestRect, D3DTEXTUREFILTERTYPE Filter) {
            return this->dev->StretchRect(pSourceSurface, pSourceRect, pDestSurface, pDestRect, Filter);
        }

        HRESULT LoaderDirect3DDevice9::ColorFill(IDirect3DSurface9* pSurface, CONST RECT* pRect, D3DCOLOR color) {
            return this->dev->ColorFill(pSurface, pRect, color);
        }

        HRESULT LoaderDirect3DDevice9::CreateOffscreenPlainSurface(UINT Width, UINT Height, D3DFORMAT Format, D3DPOOL Pool, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle) {
            return this->dev->CreateOffscreenPlainSurface(Width, Height, Format, Pool, ppSurface, pSharedHandle);
        }

        HRESULT LoaderDirect3DDevice9::SetRenderTarget(DWORD RenderTargetIndex, IDirect3DSurface9* pRenderTarget) {
            addons::AdvPreSetRenderTarget(this->dev, RenderTargetIndex, pRenderTarget);

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

            HRESULT hr = this->dev->SetRenderTarget(RenderTargetIndex, pRenderTarget);
            if (hr != D3D_OK) {
                // Fail
                return hr;
            }

            addons::AdvPostSetRenderTarget(this->dev, RenderTargetIndex, pRenderTarget);

            return hr;
        }

        HRESULT LoaderDirect3DDevice9::GetRenderTarget(DWORD RenderTargetIndex, IDirect3DSurface9** ppRenderTarget) {
            return this->dev->GetRenderTarget(RenderTargetIndex, ppRenderTarget);
        }

        HRESULT LoaderDirect3DDevice9::SetDepthStencilSurface(IDirect3DSurface9* pNewZStencil) {
            return this->dev->SetDepthStencilSurface(pNewZStencil);
        }

        HRESULT LoaderDirect3DDevice9::GetDepthStencilSurface(IDirect3DSurface9** ppZStencilSurface) {
            return this->dev->GetDepthStencilSurface(ppZStencilSurface);
        }

        HRESULT LoaderDirect3DDevice9::BeginScene() {
            addons::AdvPreBeginScene(this->dev);

            HRESULT hr = this->dev->BeginScene();
            if (hr != D3D_OK) {
                // Fail
                return hr;
            }

            addons::AdvPostBeginScene(this->dev);

            return hr;
        }

        HRESULT LoaderDirect3DDevice9::EndScene() {
            addons::AdvPreEndScene(this->dev);

            HRESULT hr = this->dev->EndScene();
            if (hr != D3D_OK) {
                // Fail
                return hr;
            }

            addons::AdvPostEndScene(this->dev);

            return hr;
        }

        HRESULT LoaderDirect3DDevice9::Clear(DWORD Count, CONST D3DRECT* pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil) {
            addons::AdvPreClear(this->dev, Count, pRects, Flags, Color, Z, Stencil);

            HRESULT hr = this->dev->Clear(Count, pRects, Flags, Color, Z, Stencil);
            if (hr != D3D_OK) {
                // Fail
                return hr;
            }

            addons::AdvPostClear(this->dev, Count, pRects, Flags, Color, Z, Stencil);

            return hr;
        }

        HRESULT LoaderDirect3DDevice9::SetTransform(D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX* pMatrix) {
            return this->dev->SetTransform(State, pMatrix);
        }

        HRESULT LoaderDirect3DDevice9::GetTransform(D3DTRANSFORMSTATETYPE State, D3DMATRIX* pMatrix) {
            return this->dev->GetTransform(State, pMatrix);
        }

        HRESULT LoaderDirect3DDevice9::MultiplyTransform(D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX* pMatrix) {
            return this->dev->MultiplyTransform(State, pMatrix);
        }

        HRESULT LoaderDirect3DDevice9::SetViewport(CONST D3DVIEWPORT9* pViewport) {
            return this->dev->SetViewport(pViewport);
        }

        HRESULT LoaderDirect3DDevice9::GetViewport(D3DVIEWPORT9* pViewport) {
            return this->dev->GetViewport(pViewport);
        }

        HRESULT LoaderDirect3DDevice9::SetMaterial(CONST D3DMATERIAL9* pMaterial) {
            return this->dev->SetMaterial(pMaterial);
        }

        HRESULT LoaderDirect3DDevice9::GetMaterial(D3DMATERIAL9* pMaterial) {
            return this->dev->GetMaterial(pMaterial);
        }

        HRESULT LoaderDirect3DDevice9::SetLight(DWORD Index, CONST D3DLIGHT9* pLight) {
            return this->dev->SetLight(Index, pLight);
        }

        HRESULT LoaderDirect3DDevice9::GetLight(DWORD Index, D3DLIGHT9* pLight) {
            return this->dev->GetLight(Index, pLight);
        }

        HRESULT LoaderDirect3DDevice9::LightEnable(DWORD Index, BOOL Enable) {
            return this->dev->LightEnable(Index, Enable);
        }

        HRESULT LoaderDirect3DDevice9::GetLightEnable(DWORD Index, BOOL* pEnable) {
            return this->dev->GetLightEnable(Index, pEnable);
        }

        HRESULT LoaderDirect3DDevice9::SetClipPlane(DWORD Index, CONST float* pPlane) {
            return this->dev->SetClipPlane(Index, pPlane);
        }

        HRESULT LoaderDirect3DDevice9::GetClipPlane(DWORD Index, float* pPlane) {
            return this->dev->GetClipPlane(Index, pPlane);
        }

        HRESULT LoaderDirect3DDevice9::SetRenderState(D3DRENDERSTATETYPE State, DWORD Value) {
            addons::AdvPreSetRenderState(this->dev, State, Value);

            HRESULT hr = this->dev->SetRenderState(State, Value);
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

            addons::AdvPostSetRenderState(this->dev, State, Value);

            return hr;
        }

        HRESULT LoaderDirect3DDevice9::GetRenderState(D3DRENDERSTATETYPE State, DWORD* pValue) {
            return this->dev->GetRenderState(State, pValue);
        }

        HRESULT LoaderDirect3DDevice9::CreateStateBlock(D3DSTATEBLOCKTYPE Type, IDirect3DStateBlock9** ppSB) {
            return this->dev->CreateStateBlock(Type, ppSB);
        }

        HRESULT LoaderDirect3DDevice9::BeginStateBlock() {
            return this->dev->BeginStateBlock();
        }

        HRESULT LoaderDirect3DDevice9::EndStateBlock(IDirect3DStateBlock9** ppSB) {
            return this->dev->EndStateBlock(ppSB);
        }

        HRESULT LoaderDirect3DDevice9::SetClipStatus(CONST D3DCLIPSTATUS9* pClipStatus) {
            return this->dev->SetClipStatus(pClipStatus);
        }

        HRESULT LoaderDirect3DDevice9::GetClipStatus(D3DCLIPSTATUS9* pClipStatus) {
            return this->dev->GetClipStatus(pClipStatus);
        }

        HRESULT LoaderDirect3DDevice9::GetTexture(DWORD Stage, IDirect3DBaseTexture9** ppTexture) {
            return this->dev->GetTexture(Stage, ppTexture);
        }

        HRESULT LoaderDirect3DDevice9::SetTexture(DWORD Stage, IDirect3DBaseTexture9* pTexture) {
            addons::AdvPreSetTexture(this->dev, Stage, pTexture);

            HRESULT hr = this->dev->SetTexture(Stage, pTexture);
            if (hr != D3D_OK) {
                // Fail
                return hr;
            }

            addons::AdvPostSetTexture(this->dev, Stage, pTexture);

            return hr;
        }

        HRESULT LoaderDirect3DDevice9::GetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD* pValue) {
            return this->dev->GetTextureStageState(Stage, Type, pValue);
        }

        HRESULT LoaderDirect3DDevice9::SetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value) {
            return this->dev->SetTextureStageState(Stage, Type, Value);
        }

        HRESULT LoaderDirect3DDevice9::GetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD* pValue) {
            return this->dev->GetSamplerState(Sampler, Type, pValue);
        }

        HRESULT LoaderDirect3DDevice9::SetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD Value) {
            return this->dev->SetSamplerState(Sampler, Type, Value);
        }

        HRESULT LoaderDirect3DDevice9::ValidateDevice(DWORD* pNumPasses) {
            return this->dev->ValidateDevice(pNumPasses);
        }

        HRESULT LoaderDirect3DDevice9::SetPaletteEntries(UINT PaletteNumber, CONST PALETTEENTRY* pEntries) {
            return this->dev->SetPaletteEntries(PaletteNumber, pEntries);
        }

        HRESULT LoaderDirect3DDevice9::GetPaletteEntries(UINT PaletteNumber, PALETTEENTRY* pEntries) {
            return this->dev->GetPaletteEntries(PaletteNumber, pEntries);
        }

        HRESULT LoaderDirect3DDevice9::SetCurrentTexturePalette(UINT PaletteNumber) {
            return this->dev->SetCurrentTexturePalette(PaletteNumber);
        }

        HRESULT LoaderDirect3DDevice9::GetCurrentTexturePalette(UINT *PaletteNumber) {
            return this->dev->GetCurrentTexturePalette(PaletteNumber);
        }

        HRESULT LoaderDirect3DDevice9::SetScissorRect(CONST RECT* pRect) {
            return this->dev->SetScissorRect(pRect);
        }

        HRESULT LoaderDirect3DDevice9::GetScissorRect(RECT* pRect) {
            return this->dev->GetScissorRect(pRect);
        }

        HRESULT LoaderDirect3DDevice9::SetSoftwareVertexProcessing(BOOL bSoftware) {
            return this->dev->SetSoftwareVertexProcessing(bSoftware);
        }

        BOOL LoaderDirect3DDevice9::GetSoftwareVertexProcessing() {
            return this->dev->GetSoftwareVertexProcessing();
        }

        HRESULT LoaderDirect3DDevice9::SetNPatchMode(float nSegments) {
            return this->dev->SetNPatchMode(nSegments);
        }

        float LoaderDirect3DDevice9::GetNPatchMode() {
            return this->dev->GetNPatchMode();
        }

        HRESULT LoaderDirect3DDevice9::DrawPrimitive(D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount) {
            return this->dev->DrawPrimitive(PrimitiveType, StartVertex, PrimitiveCount);
        }

        HRESULT LoaderDirect3DDevice9::DrawIndexedPrimitive(D3DPRIMITIVETYPE PrimitiveType, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount) {
            addons::AdvPreDrawIndexedPrimitive(this->dev, PrimitiveType, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount);

            HRESULT hr = this->dev->DrawIndexedPrimitive(PrimitiveType, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount);
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
            
            addons::AdvPostDrawIndexedPrimitive(this->dev, PrimitiveType, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount);

            return hr;
        }

        HRESULT LoaderDirect3DDevice9::DrawPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT PrimitiveCount, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride) {
            return this->dev->DrawPrimitiveUP(PrimitiveType, PrimitiveCount, pVertexStreamZeroData, VertexStreamZeroStride);
        }

        HRESULT LoaderDirect3DDevice9::DrawIndexedPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT MinVertexIndex, UINT NumVertices, UINT PrimitiveCount, CONST void* pIndexData, D3DFORMAT IndexDataFormat, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride) {
            return this->dev->DrawIndexedPrimitiveUP(PrimitiveType, MinVertexIndex, NumVertices, PrimitiveCount, pIndexData, IndexDataFormat, pVertexStreamZeroData, VertexStreamZeroStride);
        }

        HRESULT LoaderDirect3DDevice9::ProcessVertices(UINT SrcStartIndex, UINT DestIndex, UINT VertexCount, IDirect3DVertexBuffer9* pDestBuffer, IDirect3DVertexDeclaration9* pVertexDecl, DWORD Flags) {
            return this->dev->ProcessVertices(SrcStartIndex, DestIndex, VertexCount, pDestBuffer, pVertexDecl, Flags);
        }

        HRESULT LoaderDirect3DDevice9::CreateVertexDeclaration(CONST D3DVERTEXELEMENT9* pVertexElements, IDirect3DVertexDeclaration9** ppDecl) {
            return this->dev->CreateVertexDeclaration(pVertexElements, ppDecl);
        }

        HRESULT LoaderDirect3DDevice9::SetVertexDeclaration(IDirect3DVertexDeclaration9* pDecl) {
            return this->dev->SetVertexDeclaration(pDecl);
        }

        HRESULT LoaderDirect3DDevice9::GetVertexDeclaration(IDirect3DVertexDeclaration9** ppDecl) {
            return this->dev->GetVertexDeclaration(ppDecl);
        }

        HRESULT LoaderDirect3DDevice9::SetFVF(DWORD FVF) {
            return this->dev->SetFVF(FVF);
        }

        HRESULT LoaderDirect3DDevice9::GetFVF(DWORD* pFVF) {
            return this->dev->GetFVF(pFVF);
        }

        HRESULT LoaderDirect3DDevice9::CreateVertexShader(CONST DWORD* pFunction, IDirect3DVertexShader9** ppShader) {
            IDirect3DVertexShader9* pOldShader = *ppShader;
            HRESULT hr = addons::AdvPreCreateVertexShader(this->dev, pFunction, ppShader);
            if (hr != D3D_OK) {
                // Fail
                return hr;
            }

            if (*ppShader == pOldShader) {
                hr = this->dev->CreateVertexShader(pFunction, ppShader);
                if (hr != D3D_OK) {
                    // Fail
                    return hr;
                }
            }

            // Check for vertex patterns
            int functionLength = GetShaderFunctionLength(pFunction);
            if (functionLength > 0) {
                if (CheckShaderPattern(pFunction, functionLength, GuiTextVertexShader, GuiTextVertexShaderLength)) {
                    // GUI text pattern
                    GuiVertexShaderPtrs.insert(*ppShader);
                    stringstream sstream;
                    sstream << hex << ppShader;
                    GetLog()->info("Found vertex shader for GUI text, initialized at 0x" + sstream.str());
                }
                else if (CheckShaderPattern(pFunction, functionLength, GuiIconVertexShader, GuiIconVertexShaderLength)) {
                    // GUI icon pattern
                    GuiVertexShaderPtrs.insert(*ppShader);
                    stringstream sstream;
                    sstream << hex << ppShader;
                    GetLog()->info("Found vertex shader for GUI icons, initialized at 0x" + sstream.str());
                }
            }

            addons::AdvPostCreateVertexShader(this->dev, *ppShader, pFunction);

            return hr;
        }

        HRESULT LoaderDirect3DDevice9::SetVertexShader(IDirect3DVertexShader9* pShader) {
            addons::AdvPreSetVertexShader(this->dev, pShader);

            if (pShader) {
                if (GuiVertexShaderPtrs.find(pShader) != GuiVertexShaderPtrs.end()) {
                    if (!PrePresentGuiDone) {
                        // The GUI is being rendered, HALT!

                        // Save our current state
                        IDirect3DStateBlock9* pStateBlock = NULL;
                        this->dev->CreateStateBlock(D3DSBT_ALL, &pStateBlock);

                        if (PrePostProcessingDone < 3) {
                            // Pre post processing has not been called yet, make sure it is called
                            PrePostProcessingDone = 3;
                            addons::DrawFrameBeforePostProcessing(this->dev);
                        }

                        // Call our hook
                        addons::DrawFrameBeforeGui(this->dev);

                        // Restore our state
                        pStateBlock->Apply();
                        pStateBlock->Release();

                        // Make sure we keep track that we've done this
                        PrePresentGuiDone = true;
                    }
                }
            }

            HRESULT hr = this->dev->SetVertexShader(pShader);
            if (hr != D3D_OK) {
                // Fail
                return hr;
            }

            addons::AdvPostSetVertexShader(this->dev, pShader);

            return hr;
        }

        HRESULT LoaderDirect3DDevice9::GetVertexShader(IDirect3DVertexShader9** ppShader) {
            return this->dev->GetVertexShader(ppShader);
        }

        HRESULT LoaderDirect3DDevice9::SetVertexShaderConstantF(UINT StartRegister, CONST float* pConstantData, UINT Vector4fCount) {
            return this->dev->SetVertexShaderConstantF(StartRegister, pConstantData, Vector4fCount);
        }

        HRESULT LoaderDirect3DDevice9::GetVertexShaderConstantF(UINT StartRegister, float* pConstantData, UINT Vector4fCount) {
            return this->dev->GetVertexShaderConstantF(StartRegister, pConstantData, Vector4fCount);
        }

        HRESULT LoaderDirect3DDevice9::SetVertexShaderConstantI(UINT StartRegister, CONST int* pConstantData, UINT Vector4iCount) {
            return this->dev->SetVertexShaderConstantI(StartRegister, pConstantData, Vector4iCount);
        }

        HRESULT LoaderDirect3DDevice9::GetVertexShaderConstantI(UINT StartRegister, int* pConstantData, UINT Vector4iCount) {
            return this->dev->GetVertexShaderConstantI(StartRegister, pConstantData, Vector4iCount);
        }

        HRESULT LoaderDirect3DDevice9::SetVertexShaderConstantB(UINT StartRegister, CONST BOOL* pConstantData, UINT BoolCount) {
            return this->dev->SetVertexShaderConstantB(StartRegister, pConstantData, BoolCount);
        }

        HRESULT LoaderDirect3DDevice9::GetVertexShaderConstantB(UINT StartRegister, BOOL* pConstantData, UINT BoolCount) {
            return this->dev->GetVertexShaderConstantB(StartRegister, pConstantData, BoolCount);
        }

        HRESULT LoaderDirect3DDevice9::SetStreamSource(UINT StreamNumber, IDirect3DVertexBuffer9* pStreamData, UINT OffsetInBytes, UINT Stride) {
            return this->dev->SetStreamSource(StreamNumber, pStreamData, OffsetInBytes, Stride);
        }

        HRESULT LoaderDirect3DDevice9::GetStreamSource(UINT StreamNumber, IDirect3DVertexBuffer9** ppStreamData, UINT* pOffsetInBytes, UINT* pStride) {
            return this->dev->GetStreamSource(StreamNumber, ppStreamData, pOffsetInBytes, pStride);
        }

        HRESULT LoaderDirect3DDevice9::SetStreamSourceFreq(UINT StreamNumber, UINT Setting) {
            return this->dev->SetStreamSourceFreq(StreamNumber, Setting);
        }

        HRESULT LoaderDirect3DDevice9::GetStreamSourceFreq(UINT StreamNumber, UINT* pSetting) {
            return this->dev->GetStreamSourceFreq(StreamNumber, pSetting);
        }

        HRESULT LoaderDirect3DDevice9::SetIndices(IDirect3DIndexBuffer9* pIndexData) {
            return this->dev->SetIndices(pIndexData);
        }

        HRESULT LoaderDirect3DDevice9::GetIndices(IDirect3DIndexBuffer9** ppIndexData) {
            return this->dev->GetIndices(ppIndexData);
        }

        HRESULT LoaderDirect3DDevice9::CreatePixelShader(CONST DWORD* pFunction, IDirect3DPixelShader9** ppShader) {
            IDirect3DPixelShader9* pOldShader = *ppShader;
            HRESULT hr = addons::AdvPreCreatePixelShader(this->dev, pFunction, ppShader);
            if (hr != D3D_OK) {
                // Fail
                return hr;
            }

            if (*ppShader == pOldShader) {
                hr = this->dev->CreatePixelShader(pFunction, ppShader);
                if (hr != D3D_OK) {
                    // Fail
                    return hr;
                }
            }

            addons::AdvPostCreatePixelShader(this->dev, *ppShader, pFunction);

            return hr;
        }

        HRESULT LoaderDirect3DDevice9::SetPixelShader(IDirect3DPixelShader9* pShader) {
            addons::AdvPreSetPixelShader(this->dev, pShader);

            HRESULT hr = this->dev->SetPixelShader(pShader);
            if (hr != D3D_OK) {
                // Fail
                return hr;
            }

            addons::AdvPostSetPixelShader(this->dev, pShader);

            return hr;
        }

        HRESULT LoaderDirect3DDevice9::GetPixelShader(IDirect3DPixelShader9** ppShader) {
            return this->dev->GetPixelShader(ppShader);
        }

        HRESULT LoaderDirect3DDevice9::SetPixelShaderConstantF(UINT StartRegister, CONST float* pConstantData, UINT Vector4fCount) {
            return this->dev->SetPixelShaderConstantF(StartRegister, pConstantData, Vector4fCount);
        }

        HRESULT LoaderDirect3DDevice9::GetPixelShaderConstantF(UINT StartRegister, float* pConstantData, UINT Vector4fCount) {
            return this->dev->GetPixelShaderConstantF(StartRegister, pConstantData, Vector4fCount);
        }

        HRESULT LoaderDirect3DDevice9::SetPixelShaderConstantI(UINT StartRegister, CONST int* pConstantData, UINT Vector4iCount) {
            return this->dev->SetPixelShaderConstantI(StartRegister, pConstantData, Vector4iCount);
        }

        HRESULT LoaderDirect3DDevice9::GetPixelShaderConstantI(UINT StartRegister, int* pConstantData, UINT Vector4iCount) {
            return this->dev->GetPixelShaderConstantI(StartRegister, pConstantData, Vector4iCount);
        }

        HRESULT LoaderDirect3DDevice9::SetPixelShaderConstantB(UINT StartRegister, CONST BOOL* pConstantData, UINT BoolCount) {
            return this->dev->SetPixelShaderConstantB(StartRegister, pConstantData, BoolCount);
        }

        HRESULT LoaderDirect3DDevice9::GetPixelShaderConstantB(UINT StartRegister, BOOL* pConstantData, UINT BoolCount) {
            return this->dev->GetPixelShaderConstantB(StartRegister, pConstantData, BoolCount);
        }

        HRESULT LoaderDirect3DDevice9::DrawRectPatch(UINT Handle, CONST float* pNumSegs, CONST D3DRECTPATCH_INFO* pRectPatchInfo) {
            return this->dev->DrawRectPatch(Handle, pNumSegs, pRectPatchInfo);
        }

        HRESULT LoaderDirect3DDevice9::DrawTriPatch(UINT Handle, CONST float* pNumSegs, CONST D3DTRIPATCH_INFO* pTriPatchInfo) {
            return this->dev->DrawTriPatch(Handle, pNumSegs, pTriPatchInfo);
        }

        HRESULT LoaderDirect3DDevice9::DeletePatch(UINT Handle) {
            return this->dev->DeletePatch(Handle);
        }

        HRESULT LoaderDirect3DDevice9::CreateQuery(D3DQUERYTYPE Type, IDirect3DQuery9** ppQuery) {
            return this->dev->CreateQuery(Type, ppQuery);
        }

    
        HRESULT LoaderDirect3DDevice9Ex::QueryInterface(REFIID riid, void** ppvObj) {
            return this->dev->QueryInterface(riid, ppvObj);
        }

        ULONG LoaderDirect3DDevice9Ex::AddRef() {
            return this->dev->AddRef();
        }

        ULONG LoaderDirect3DDevice9Ex::Release() {
            ULONG count = this->dev->Release();
            if (count == 0) {
                delete this;
            }
            return count;
        }

        HRESULT LoaderDirect3DDevice9Ex::TestCooperativeLevel() {
            return this->dev->TestCooperativeLevel();
        }

        UINT LoaderDirect3DDevice9Ex::GetAvailableTextureMem() {
            return this->dev->GetAvailableTextureMem();
        }

        HRESULT LoaderDirect3DDevice9Ex::EvictManagedResources() {
            return this->dev->EvictManagedResources();
        }

        HRESULT LoaderDirect3DDevice9Ex::GetDirect3D(IDirect3D9** ppD3D9) {
            return this->dev->GetDirect3D(ppD3D9);
        }

        HRESULT LoaderDirect3DDevice9Ex::GetDeviceCaps(D3DCAPS9* pCaps) {
            return this->dev->GetDeviceCaps(pCaps);
        }

        HRESULT LoaderDirect3DDevice9Ex::GetDisplayMode(UINT iSwapChain, D3DDISPLAYMODE* pMode) {
            return this->dev->GetDisplayMode(iSwapChain, pMode);
        }

        HRESULT LoaderDirect3DDevice9Ex::GetCreationParameters(D3DDEVICE_CREATION_PARAMETERS *pParameters) {
            return this->dev->GetCreationParameters(pParameters);
        }

        HRESULT LoaderDirect3DDevice9Ex::SetCursorProperties(UINT XHotSpot, UINT YHotSpot, IDirect3DSurface9* pCursorBitmap) {
            return this->dev->SetCursorProperties(XHotSpot, YHotSpot, pCursorBitmap);
        }

        void LoaderDirect3DDevice9Ex::SetCursorPosition(int X, int Y, DWORD Flags) {
            return this->dev->SetCursorPosition(X, Y, Flags);
        }

        BOOL LoaderDirect3DDevice9Ex::ShowCursor(BOOL bShow) {
            return this->dev->ShowCursor(bShow);
        }

        HRESULT LoaderDirect3DDevice9Ex::CreateAdditionalSwapChain(D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DSwapChain9** pSwapChain) {
            return this->dev->CreateAdditionalSwapChain(pPresentationParameters, pSwapChain);
        }

        HRESULT LoaderDirect3DDevice9Ex::GetSwapChain(UINT iSwapChain, IDirect3DSwapChain9** pSwapChain) {
            return this->dev->GetSwapChain(iSwapChain, pSwapChain);
        }

        UINT LoaderDirect3DDevice9Ex::GetNumberOfSwapChains() {
            return this->dev->GetNumberOfSwapChains();
        }

        HRESULT LoaderDirect3DDevice9Ex::Reset(D3DPRESENT_PARAMETERS* pPresentationParameters) {
            return this->dev->Reset(pPresentationParameters);
        }

        HRESULT LoaderDirect3DDevice9Ex::Present(CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion) {
            return this->dev->Present(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
        }

        HRESULT LoaderDirect3DDevice9Ex::GetBackBuffer(UINT iSwapChain, UINT iBackBuffer, D3DBACKBUFFER_TYPE Type, IDirect3DSurface9** ppBackBuffer) {
            return this->dev->GetBackBuffer(iSwapChain, iBackBuffer, Type, ppBackBuffer);
        }

        HRESULT LoaderDirect3DDevice9Ex::GetRasterStatus(UINT iSwapChain, D3DRASTER_STATUS* pRasterStatus) {
            return this->dev->GetRasterStatus(iSwapChain, pRasterStatus);
        }

        HRESULT LoaderDirect3DDevice9Ex::SetDialogBoxMode(BOOL bEnableDialogs) {
            return this->dev->SetDialogBoxMode(bEnableDialogs);
        }

        void LoaderDirect3DDevice9Ex::SetGammaRamp(UINT iSwapChain, DWORD Flags, CONST D3DGAMMARAMP* pRamp) {
            return this->dev->SetGammaRamp(iSwapChain, Flags, pRamp);
        }

        void LoaderDirect3DDevice9Ex::GetGammaRamp(UINT iSwapChain, D3DGAMMARAMP* pRamp) {
            return this->dev->GetGammaRamp(iSwapChain, pRamp);
        }

        HRESULT LoaderDirect3DDevice9Ex::CreateTexture(UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DTexture9** ppTexture, HANDLE* pSharedHandle) {
            // Not hooked, GW2 doesn't use Ex calls
            return this->dev->CreateTexture(Width, Height, Levels, Usage, Format, Pool, ppTexture, pSharedHandle);
        }

        HRESULT LoaderDirect3DDevice9Ex::CreateVolumeTexture(UINT Width, UINT Height, UINT Depth, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DVolumeTexture9** ppVolumeTexture, HANDLE* pSharedHandle) {
            return this->dev->CreateVolumeTexture(Width, Height, Depth, Levels, Usage, Format, Pool, ppVolumeTexture, pSharedHandle);
        }

        HRESULT LoaderDirect3DDevice9Ex::CreateCubeTexture(UINT EdgeLength, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DCubeTexture9** ppCubeTexture, HANDLE* pSharedHandle) {
            return this->dev->CreateCubeTexture(EdgeLength, Levels, Usage, Format, Pool, ppCubeTexture, pSharedHandle);
        }

        HRESULT LoaderDirect3DDevice9Ex::CreateVertexBuffer(UINT Length, DWORD Usage, DWORD FVF, D3DPOOL Pool, IDirect3DVertexBuffer9** ppVertexBuffer, HANDLE* pSharedHandle) {
            return this->dev->CreateVertexBuffer(Length, Usage, FVF, Pool, ppVertexBuffer, pSharedHandle);
        }

        HRESULT LoaderDirect3DDevice9Ex::CreateIndexBuffer(UINT Length, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DIndexBuffer9** ppIndexBuffer, HANDLE* pSharedHandle) {
            return this->dev->CreateIndexBuffer(Length, Usage, Format, Pool, ppIndexBuffer, pSharedHandle);
        }

        HRESULT LoaderDirect3DDevice9Ex::CreateRenderTarget(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Lockable, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle) {
            // Not hooked, GW2 doesn't use Ex calls
            return this->dev->CreateRenderTarget(Width, Height, Format, MultiSample, MultisampleQuality, Lockable, ppSurface, pSharedHandle);
        }

        HRESULT LoaderDirect3DDevice9Ex::CreateDepthStencilSurface(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Discard, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle) {
            return this->dev->CreateDepthStencilSurface(Width, Height, Format, MultiSample, MultisampleQuality, Discard, ppSurface, pSharedHandle);
        }

        HRESULT LoaderDirect3DDevice9Ex::UpdateSurface(IDirect3DSurface9* pSourceSurface, CONST RECT* pSourceRect, IDirect3DSurface9* pDestinationSurface, CONST POINT* pDestPoint) {
            return this->dev->UpdateSurface(pSourceSurface, pSourceRect, pDestinationSurface, pDestPoint);
        }

        HRESULT LoaderDirect3DDevice9Ex::UpdateTexture(IDirect3DBaseTexture9* pSourceTexture, IDirect3DBaseTexture9* pDestinationTexture) {
            return this->dev->UpdateTexture(pSourceTexture, pDestinationTexture);
        }

        HRESULT LoaderDirect3DDevice9Ex::GetRenderTargetData(IDirect3DSurface9* pRenderTarget, IDirect3DSurface9* pDestSurface) {
            return this->dev->GetRenderTargetData(pRenderTarget, pDestSurface);
        }

        HRESULT LoaderDirect3DDevice9Ex::GetFrontBufferData(UINT iSwapChain, IDirect3DSurface9* pDestSurface) {
            return this->dev->GetFrontBufferData(iSwapChain, pDestSurface);
        }

        HRESULT LoaderDirect3DDevice9Ex::StretchRect(IDirect3DSurface9* pSourceSurface, CONST RECT* pSourceRect, IDirect3DSurface9* pDestSurface, CONST RECT* pDestRect, D3DTEXTUREFILTERTYPE Filter) {
            return this->dev->StretchRect(pSourceSurface, pSourceRect, pDestSurface, pDestRect, Filter);
        }

        HRESULT LoaderDirect3DDevice9Ex::ColorFill(IDirect3DSurface9* pSurface, CONST RECT* pRect, D3DCOLOR color) {
            return this->dev->ColorFill(pSurface, pRect, color);
        }

        HRESULT LoaderDirect3DDevice9Ex::CreateOffscreenPlainSurface(UINT Width, UINT Height, D3DFORMAT Format, D3DPOOL Pool, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle) {
            return this->dev->CreateOffscreenPlainSurface(Width, Height, Format, Pool, ppSurface, pSharedHandle);
        }

        HRESULT LoaderDirect3DDevice9Ex::SetRenderTarget(DWORD RenderTargetIndex, IDirect3DSurface9* pRenderTarget) {
            // Not hooked, GW2 doesn't use Ex calls
            return this->dev->SetRenderTarget(RenderTargetIndex, pRenderTarget);
        }

        HRESULT LoaderDirect3DDevice9Ex::GetRenderTarget(DWORD RenderTargetIndex, IDirect3DSurface9** ppRenderTarget) {
            return this->dev->GetRenderTarget(RenderTargetIndex, ppRenderTarget);
        }

        HRESULT LoaderDirect3DDevice9Ex::SetDepthStencilSurface(IDirect3DSurface9* pNewZStencil) {
            return this->dev->SetDepthStencilSurface(pNewZStencil);
        }

        HRESULT LoaderDirect3DDevice9Ex::GetDepthStencilSurface(IDirect3DSurface9** ppZStencilSurface) {
            return this->dev->GetDepthStencilSurface(ppZStencilSurface);
        }

        HRESULT LoaderDirect3DDevice9Ex::BeginScene() {
            // Not hooked, GW2 doesn't use Ex calls
            return this->dev->BeginScene();
        }

        HRESULT LoaderDirect3DDevice9Ex::EndScene() {
            // Not hooked, GW2 doesn't use Ex calls
            return this->dev->EndScene();
        }

        HRESULT LoaderDirect3DDevice9Ex::Clear(DWORD Count, CONST D3DRECT* pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil) {
            // Not hooked, GW2 doesn't use Ex calls
            return this->dev->Clear(Count, pRects, Flags, Color, Z, Stencil);
        }

        HRESULT LoaderDirect3DDevice9Ex::SetTransform(D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX* pMatrix) {
            return this->dev->SetTransform(State, pMatrix);
        }

        HRESULT LoaderDirect3DDevice9Ex::GetTransform(D3DTRANSFORMSTATETYPE State, D3DMATRIX* pMatrix) {
            return this->dev->GetTransform(State, pMatrix);
        }

        HRESULT LoaderDirect3DDevice9Ex::MultiplyTransform(D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX* pMatrix) {
            return this->dev->MultiplyTransform(State, pMatrix);
        }

        HRESULT LoaderDirect3DDevice9Ex::SetViewport(CONST D3DVIEWPORT9* pViewport) {
            return this->dev->SetViewport(pViewport);
        }

        HRESULT LoaderDirect3DDevice9Ex::GetViewport(D3DVIEWPORT9* pViewport) {
            return this->dev->GetViewport(pViewport);
        }

        HRESULT LoaderDirect3DDevice9Ex::SetMaterial(CONST D3DMATERIAL9* pMaterial) {
            return this->dev->SetMaterial(pMaterial);
        }

        HRESULT LoaderDirect3DDevice9Ex::GetMaterial(D3DMATERIAL9* pMaterial) {
            return this->dev->GetMaterial(pMaterial);
        }

        HRESULT LoaderDirect3DDevice9Ex::SetLight(DWORD Index, CONST D3DLIGHT9* pLight) {
            return this->dev->SetLight(Index, pLight);
        }

        HRESULT LoaderDirect3DDevice9Ex::GetLight(DWORD Index, D3DLIGHT9* pLight) {
            return this->dev->GetLight(Index, pLight);
        }

        HRESULT LoaderDirect3DDevice9Ex::LightEnable(DWORD Index, BOOL Enable) {
            return this->dev->LightEnable(Index, Enable);
        }

        HRESULT LoaderDirect3DDevice9Ex::GetLightEnable(DWORD Index, BOOL* pEnable) {
            return this->dev->GetLightEnable(Index, pEnable);
        }

        HRESULT LoaderDirect3DDevice9Ex::SetClipPlane(DWORD Index, CONST float* pPlane) {
            return this->dev->SetClipPlane(Index, pPlane);
        }

        HRESULT LoaderDirect3DDevice9Ex::GetClipPlane(DWORD Index, float* pPlane) {
            return this->dev->GetClipPlane(Index, pPlane);
        }

        HRESULT LoaderDirect3DDevice9Ex::SetRenderState(D3DRENDERSTATETYPE State, DWORD Value) {
            // Not hooked, GW2 doesn't use Ex calls
            return this->dev->SetRenderState(State, Value);
        }

        HRESULT LoaderDirect3DDevice9Ex::GetRenderState(D3DRENDERSTATETYPE State, DWORD* pValue) {
            return this->dev->GetRenderState(State, pValue);
        }

        HRESULT LoaderDirect3DDevice9Ex::CreateStateBlock(D3DSTATEBLOCKTYPE Type, IDirect3DStateBlock9** ppSB) {
            return this->dev->CreateStateBlock(Type, ppSB);
        }

        HRESULT LoaderDirect3DDevice9Ex::BeginStateBlock() {
            return this->dev->BeginStateBlock();
        }

        HRESULT LoaderDirect3DDevice9Ex::EndStateBlock(IDirect3DStateBlock9** ppSB) {
            return this->dev->EndStateBlock(ppSB);
        }

        HRESULT LoaderDirect3DDevice9Ex::SetClipStatus(CONST D3DCLIPSTATUS9* pClipStatus) {
            return this->dev->SetClipStatus(pClipStatus);
        }

        HRESULT LoaderDirect3DDevice9Ex::GetClipStatus(D3DCLIPSTATUS9* pClipStatus) {
            return this->dev->GetClipStatus(pClipStatus);
        }

        HRESULT LoaderDirect3DDevice9Ex::GetTexture(DWORD Stage, IDirect3DBaseTexture9** ppTexture) {
            return this->dev->GetTexture(Stage, ppTexture);
        }

        HRESULT LoaderDirect3DDevice9Ex::SetTexture(DWORD Stage, IDirect3DBaseTexture9* pTexture) {
            // Not hooked, GW2 doesn't use Ex calls
            return this->dev->SetTexture(Stage, pTexture);
        }

        HRESULT LoaderDirect3DDevice9Ex::GetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD* pValue) {
            return this->dev->GetTextureStageState(Stage, Type, pValue);
        }

        HRESULT LoaderDirect3DDevice9Ex::SetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value) {
            return this->dev->SetTextureStageState(Stage, Type, Value);
        }

        HRESULT LoaderDirect3DDevice9Ex::GetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD* pValue) {
            return this->dev->GetSamplerState(Sampler, Type, pValue);
        }

        HRESULT LoaderDirect3DDevice9Ex::SetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD Value) {
            return this->dev->SetSamplerState(Sampler, Type, Value);
        }

        HRESULT LoaderDirect3DDevice9Ex::ValidateDevice(DWORD* pNumPasses) {
            return this->dev->ValidateDevice(pNumPasses);
        }

        HRESULT LoaderDirect3DDevice9Ex::SetPaletteEntries(UINT PaletteNumber, CONST PALETTEENTRY* pEntries) {
            return this->dev->SetPaletteEntries(PaletteNumber, pEntries);
        }

        HRESULT LoaderDirect3DDevice9Ex::GetPaletteEntries(UINT PaletteNumber, PALETTEENTRY* pEntries) {
            return this->dev->GetPaletteEntries(PaletteNumber, pEntries);
        }

        HRESULT LoaderDirect3DDevice9Ex::SetCurrentTexturePalette(UINT PaletteNumber) {
            return this->dev->SetCurrentTexturePalette(PaletteNumber);
        }

        HRESULT LoaderDirect3DDevice9Ex::GetCurrentTexturePalette(UINT *PaletteNumber) {
            return this->dev->GetCurrentTexturePalette(PaletteNumber);
        }

        HRESULT LoaderDirect3DDevice9Ex::SetScissorRect(CONST RECT* pRect) {
            return this->dev->SetScissorRect(pRect);
        }

        HRESULT LoaderDirect3DDevice9Ex::GetScissorRect(RECT* pRect) {
            return this->dev->GetScissorRect(pRect);
        }

        HRESULT LoaderDirect3DDevice9Ex::SetSoftwareVertexProcessing(BOOL bSoftware) {
            return this->dev->SetSoftwareVertexProcessing(bSoftware);
        }

        BOOL LoaderDirect3DDevice9Ex::GetSoftwareVertexProcessing() {
            return this->dev->GetSoftwareVertexProcessing();
        }

        HRESULT LoaderDirect3DDevice9Ex::SetNPatchMode(float nSegments) {
            return this->dev->SetNPatchMode(nSegments);
        }

        float LoaderDirect3DDevice9Ex::GetNPatchMode() {
            return this->dev->GetNPatchMode();
        }

        HRESULT LoaderDirect3DDevice9Ex::DrawPrimitive(D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount) {
            return this->dev->DrawPrimitive(PrimitiveType, StartVertex, PrimitiveCount);
        }

        HRESULT LoaderDirect3DDevice9Ex::DrawIndexedPrimitive(D3DPRIMITIVETYPE PrimitiveType, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount) {
            // Not hooked, GW2 doesn't use Ex calls
            return this->dev->DrawIndexedPrimitive(PrimitiveType, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount);
        }

        HRESULT LoaderDirect3DDevice9Ex::DrawPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT PrimitiveCount, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride) {
            return this->dev->DrawPrimitiveUP(PrimitiveType, PrimitiveCount, pVertexStreamZeroData, VertexStreamZeroStride);
        }

        HRESULT LoaderDirect3DDevice9Ex::DrawIndexedPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT MinVertexIndex, UINT NumVertices, UINT PrimitiveCount, CONST void* pIndexData, D3DFORMAT IndexDataFormat, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride) {
            return this->dev->DrawIndexedPrimitiveUP(PrimitiveType, MinVertexIndex, NumVertices, PrimitiveCount, pIndexData, IndexDataFormat, pVertexStreamZeroData, VertexStreamZeroStride);
        }

        HRESULT LoaderDirect3DDevice9Ex::ProcessVertices(UINT SrcStartIndex, UINT DestIndex, UINT VertexCount, IDirect3DVertexBuffer9* pDestBuffer, IDirect3DVertexDeclaration9* pVertexDecl, DWORD Flags) {
            return this->dev->ProcessVertices(SrcStartIndex, DestIndex, VertexCount, pDestBuffer, pVertexDecl, Flags);
        }

        HRESULT LoaderDirect3DDevice9Ex::CreateVertexDeclaration(CONST D3DVERTEXELEMENT9* pVertexElements, IDirect3DVertexDeclaration9** ppDecl) {
            return this->dev->CreateVertexDeclaration(pVertexElements, ppDecl);
        }

        HRESULT LoaderDirect3DDevice9Ex::SetVertexDeclaration(IDirect3DVertexDeclaration9* pDecl) {
            return this->dev->SetVertexDeclaration(pDecl);
        }

        HRESULT LoaderDirect3DDevice9Ex::GetVertexDeclaration(IDirect3DVertexDeclaration9** ppDecl) {
            return this->dev->GetVertexDeclaration(ppDecl);
        }

        HRESULT LoaderDirect3DDevice9Ex::SetFVF(DWORD FVF) {
            return this->dev->SetFVF(FVF);
        }

        HRESULT LoaderDirect3DDevice9Ex::GetFVF(DWORD* pFVF) {
            return this->dev->GetFVF(pFVF);
        }

        HRESULT LoaderDirect3DDevice9Ex::CreateVertexShader(CONST DWORD* pFunction, IDirect3DVertexShader9** ppShader) {
            // Not hooked, GW2 doesn't use Ex calls
            return this->dev->CreateVertexShader(pFunction, ppShader);
        }

        HRESULT LoaderDirect3DDevice9Ex::SetVertexShader(IDirect3DVertexShader9* pShader) {
            // Not hooked, GW2 doesn't use Ex calls
            return this->dev->SetVertexShader(pShader);
        }

        HRESULT LoaderDirect3DDevice9Ex::GetVertexShader(IDirect3DVertexShader9** ppShader) {
            return this->dev->GetVertexShader(ppShader);
        }

        HRESULT LoaderDirect3DDevice9Ex::SetVertexShaderConstantF(UINT StartRegister, CONST float* pConstantData, UINT Vector4fCount) {
            return this->dev->SetVertexShaderConstantF(StartRegister, pConstantData, Vector4fCount);
        }

        HRESULT LoaderDirect3DDevice9Ex::GetVertexShaderConstantF(UINT StartRegister, float* pConstantData, UINT Vector4fCount) {
            return this->dev->GetVertexShaderConstantF(StartRegister, pConstantData, Vector4fCount);
        }

        HRESULT LoaderDirect3DDevice9Ex::SetVertexShaderConstantI(UINT StartRegister, CONST int* pConstantData, UINT Vector4iCount) {
            return this->dev->SetVertexShaderConstantI(StartRegister, pConstantData, Vector4iCount);
        }

        HRESULT LoaderDirect3DDevice9Ex::GetVertexShaderConstantI(UINT StartRegister, int* pConstantData, UINT Vector4iCount) {
            return this->dev->GetVertexShaderConstantI(StartRegister, pConstantData, Vector4iCount);
        }

        HRESULT LoaderDirect3DDevice9Ex::SetVertexShaderConstantB(UINT StartRegister, CONST BOOL* pConstantData, UINT BoolCount) {
            return this->dev->SetVertexShaderConstantB(StartRegister, pConstantData, BoolCount);
        }

        HRESULT LoaderDirect3DDevice9Ex::GetVertexShaderConstantB(UINT StartRegister, BOOL* pConstantData, UINT BoolCount) {
            return this->dev->GetVertexShaderConstantB(StartRegister, pConstantData, BoolCount);
        }

        HRESULT LoaderDirect3DDevice9Ex::SetStreamSource(UINT StreamNumber, IDirect3DVertexBuffer9* pStreamData, UINT OffsetInBytes, UINT Stride) {
            return this->dev->SetStreamSource(StreamNumber, pStreamData, OffsetInBytes, Stride);
        }

        HRESULT LoaderDirect3DDevice9Ex::GetStreamSource(UINT StreamNumber, IDirect3DVertexBuffer9** ppStreamData, UINT* pOffsetInBytes, UINT* pStride) {
            return this->dev->GetStreamSource(StreamNumber, ppStreamData, pOffsetInBytes, pStride);
        }

        HRESULT LoaderDirect3DDevice9Ex::SetStreamSourceFreq(UINT StreamNumber, UINT Setting) {
            return this->dev->SetStreamSourceFreq(StreamNumber, Setting);
        }

        HRESULT LoaderDirect3DDevice9Ex::GetStreamSourceFreq(UINT StreamNumber, UINT* pSetting) {
            return this->dev->GetStreamSourceFreq(StreamNumber, pSetting);
        }

        HRESULT LoaderDirect3DDevice9Ex::SetIndices(IDirect3DIndexBuffer9* pIndexData) {
            return this->dev->SetIndices(pIndexData);
        }

        HRESULT LoaderDirect3DDevice9Ex::GetIndices(IDirect3DIndexBuffer9** ppIndexData) {
            return this->dev->GetIndices(ppIndexData);
        }

        HRESULT LoaderDirect3DDevice9Ex::CreatePixelShader(CONST DWORD* pFunction, IDirect3DPixelShader9** ppShader) {
            // Not hooked, GW2 doesn't use Ex calls
            return this->dev->CreatePixelShader(pFunction, ppShader);
        }

        HRESULT LoaderDirect3DDevice9Ex::SetPixelShader(IDirect3DPixelShader9* pShader) {
            // Not hooked, GW2 doesn't use Ex calls
            return this->dev->SetPixelShader(pShader);
        }

        HRESULT LoaderDirect3DDevice9Ex::GetPixelShader(IDirect3DPixelShader9** ppShader) {
            return this->dev->GetPixelShader(ppShader);
        }

        HRESULT LoaderDirect3DDevice9Ex::SetPixelShaderConstantF(UINT StartRegister, CONST float* pConstantData, UINT Vector4fCount) {
            return this->dev->SetPixelShaderConstantF(StartRegister, pConstantData, Vector4fCount);
        }

        HRESULT LoaderDirect3DDevice9Ex::GetPixelShaderConstantF(UINT StartRegister, float* pConstantData, UINT Vector4fCount) {
            return this->dev->GetPixelShaderConstantF(StartRegister, pConstantData, Vector4fCount);
        }

        HRESULT LoaderDirect3DDevice9Ex::SetPixelShaderConstantI(UINT StartRegister, CONST int* pConstantData, UINT Vector4iCount) {
            return this->dev->SetPixelShaderConstantI(StartRegister, pConstantData, Vector4iCount);
        }

        HRESULT LoaderDirect3DDevice9Ex::GetPixelShaderConstantI(UINT StartRegister, int* pConstantData, UINT Vector4iCount) {
            return this->dev->GetPixelShaderConstantI(StartRegister, pConstantData, Vector4iCount);
        }

        HRESULT LoaderDirect3DDevice9Ex::SetPixelShaderConstantB(UINT StartRegister, CONST BOOL* pConstantData, UINT BoolCount) {
            return this->dev->SetPixelShaderConstantB(StartRegister, pConstantData, BoolCount);
        }

        HRESULT LoaderDirect3DDevice9Ex::GetPixelShaderConstantB(UINT StartRegister, BOOL* pConstantData, UINT BoolCount) {
            return this->dev->GetPixelShaderConstantB(StartRegister, pConstantData, BoolCount);
        }

        HRESULT LoaderDirect3DDevice9Ex::DrawRectPatch(UINT Handle, CONST float* pNumSegs, CONST D3DRECTPATCH_INFO* pRectPatchInfo) {
            return this->dev->DrawRectPatch(Handle, pNumSegs, pRectPatchInfo);
        }

        HRESULT LoaderDirect3DDevice9Ex::DrawTriPatch(UINT Handle, CONST float* pNumSegs, CONST D3DTRIPATCH_INFO* pTriPatchInfo) {
            return this->dev->DrawTriPatch(Handle, pNumSegs, pTriPatchInfo);
        }

        HRESULT LoaderDirect3DDevice9Ex::DeletePatch(UINT Handle) {
            return this->dev->DeletePatch(Handle);
        }

        HRESULT LoaderDirect3DDevice9Ex::CreateQuery(D3DQUERYTYPE Type, IDirect3DQuery9** ppQuery) {
            return this->dev->CreateQuery(Type, ppQuery);
        }

        HRESULT LoaderDirect3DDevice9Ex::SetConvolutionMonoKernel(UINT width, UINT height, float* rows, float* columns) {
            return this->dev->SetConvolutionMonoKernel(width, height, rows, columns);
        }

        HRESULT LoaderDirect3DDevice9Ex::ComposeRects(IDirect3DSurface9* pSrc, IDirect3DSurface9* pDst, IDirect3DVertexBuffer9* pSrcRectDescs, UINT NumRects, IDirect3DVertexBuffer9* pDstRectDescs, D3DCOMPOSERECTSOP Operation, int Xoffset, int Yoffset) {
            return this->dev->ComposeRects(pSrc, pDst, pSrcRectDescs, NumRects, pDstRectDescs, Operation, Xoffset, Yoffset);
        }

        HRESULT LoaderDirect3DDevice9Ex::PresentEx(CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion, DWORD dwFlags) {
            // Not hooked, GW2 doesn't use Ex calls
            return this->dev->PresentEx(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion, dwFlags);
        }

        HRESULT LoaderDirect3DDevice9Ex::GetGPUThreadPriority(INT* pPriority) {
            return this->dev->GetGPUThreadPriority(pPriority);
        }

        HRESULT LoaderDirect3DDevice9Ex::SetGPUThreadPriority(INT Priority) {
            return this->dev->SetGPUThreadPriority(Priority);
        }

        HRESULT LoaderDirect3DDevice9Ex::WaitForVBlank(UINT iSwapChain) {
            return this->dev->WaitForVBlank(iSwapChain);
        }

        HRESULT LoaderDirect3DDevice9Ex::CheckResourceResidency(IDirect3DResource9** pResourceArray, UINT32 NumResources) {
            return this->dev->CheckResourceResidency(pResourceArray, NumResources);
        }

        HRESULT LoaderDirect3DDevice9Ex::SetMaximumFrameLatency(UINT MaxLatency) {
            return this->dev->SetMaximumFrameLatency(MaxLatency);
        }

        HRESULT LoaderDirect3DDevice9Ex::GetMaximumFrameLatency(UINT* pMaxLatency) {
            return this->dev->GetMaximumFrameLatency(pMaxLatency);
        }

        HRESULT LoaderDirect3DDevice9Ex::CheckDeviceState(HWND hDestinationWindow) {
            return this->dev->CheckDeviceState(hDestinationWindow);
        }

        HRESULT LoaderDirect3DDevice9Ex::CreateRenderTargetEx(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Lockable, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle, DWORD Usage) {
            // Not hooked, GW2 doesn't use Ex calls
            return this->dev->CreateRenderTargetEx(Width, Height, Format, MultiSample, MultisampleQuality, Lockable, ppSurface, pSharedHandle, Usage);
        }

        HRESULT LoaderDirect3DDevice9Ex::CreateOffscreenPlainSurfaceEx(UINT Width, UINT Height, D3DFORMAT Format, D3DPOOL Pool, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle, DWORD Usage) {
            return this->dev->CreateOffscreenPlainSurfaceEx(Width, Height, Format, Pool, ppSurface, pSharedHandle, Usage);
        }

        HRESULT LoaderDirect3DDevice9Ex::CreateDepthStencilSurfaceEx(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Discard, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle, DWORD Usage) {
            return this->dev->CreateDepthStencilSurfaceEx(Width, Height, Format, MultiSample, MultisampleQuality, Discard, ppSurface, pSharedHandle, Usage);
        }

        HRESULT LoaderDirect3DDevice9Ex::ResetEx(D3DPRESENT_PARAMETERS* pPresentationParameters, D3DDISPLAYMODEEX* pFullscreenDisplayMode) {
            // Not hooked, GW2 doesn't use Ex calls
            return this->dev->ResetEx(pPresentationParameters, pFullscreenDisplayMode);
        }

        HRESULT LoaderDirect3DDevice9Ex::GetDisplayModeEx(UINT iSwapChain, D3DDISPLAYMODEEX* pMode, D3DDISPLAYROTATION* pRotation) {
            return this->dev->GetDisplayModeEx(iSwapChain, pMode, pRotation);
        }

    }
}
