#include "Addon.h"
#include <Strsafe.h>
#include <filesystem>
#include <string>
#include <gw2addon-native.h>
#include "addons_manager.h"
#include "exceptions.h"
#include "NativeAddon.h"
#include "UnknownAddon.h"
#include "../Config.h"
#include "../log.h"
#include "../utils/encoding.h"

using namespace std;
using namespace std::experimental::filesystem::v1;
using namespace loader::utils;

namespace loader {
    namespace addons {

        const string AddonTypeToString(AddonType type) {
            switch (type) {
            case AddonType::AddonTypeNative:
                return "Native";
            default:
                return "Unknown";
            }
        }

        const string AddonStateToString(AddonState state) {
            switch (state) {
            case AddonState::DeactivatedOnRestartState:
                return "Deactivates on restart";
            case AddonState::ActivatedOnRestartState:
                return "Activates on restart";
            case AddonState::ErroredState:
                return "Errored";
            case AddonState::UnloadingState:
                return "Unloading";
            case AddonState::UnloadedState:
                return "Not active";
            case AddonState::LoadingState:
                return "Loading";
            case AddonState::LoadedState:
                return "Active";
            default:
                return "Unknown";
            }
        }


        Addon::Addon(const std::string& fullFileName) {
            this->filePath = fullFileName;
            this->fileName = u8path(fullFileName).filename().u8string();
        }

        unique_ptr<Addon> Addon::GetAddon(const std::string& filePath) {
            unique_ptr<Addon> addon = make_unique<Addon>(filePath);
            wstring wFilePath = u16(filePath);
            HMODULE hAddon = LoadLibraryEx(wFilePath.c_str(), NULL, DONT_RESOLVE_DLL_REFERENCES);

            if (GetProcAddress(hAddon, GW2ADDON_DLL_Initialize) != nullptr) {
                // Our addon is native
                addon = make_unique<NativeAddon<Addon>>(*addon);
            }
            else {
                // Unknown addon
                addon = make_unique<UnknownAddon<Addon>>(*addon);
            }
            FreeLibrary(hAddon);

            return addon;
        }


        bool Addon::Initialize() {
            if (this->HasBaseAddon()) {
                return this->GetBaseAddon()->Initialize();
            }
            return true;
        }

        bool Addon::Uninitialize() {
            if (this->HasBaseAddon()) {
                return this->GetBaseAddon()->Uninitialize();
            }
            return true;
        }

        bool Addon::Load() {
            if (this->HasBaseAddon()) {
                return this->GetBaseAddon()->Load();
            }
            return true;
        }

        bool Addon::Unload() {
            if (this->HasBaseAddon()) {
                return this->GetBaseAddon()->Unload();
            }
            return true;
        }

        bool Addon::IsEnabledByConfig() const {
            return AppConfig.GetAddonEnabled(this);
        }


        const AddonState Addon::GetState() const {
            if (this->HasBaseAddon()) {
                return this->GetConstBaseAddon()->GetState();
            }
            return this->state;
        }

        void Addon::ChangeState(AddonState state) {
            if (this->HasBaseAddon()) {
                return this->GetBaseAddon()->ChangeState(state);
            }
            this->state = state;
        }


        bool Addon::SupportsLoading() const {
            if (this->HasBaseAddon()) {
                return this->GetConstBaseAddon()->SupportsLoading();
            }
            return true;
        }

        bool Addon::SupportsHotLoading() const {
            if (this->HasBaseAddon()) {
                return this->GetConstBaseAddon()->SupportsHotLoading();
            }
            return true;
        }

        bool Addon::SupportsSettings() const {
            if (this->HasBaseAddon()) {
                return this->GetConstBaseAddon()->SupportsSettings();
            }
            return true;
        }

        bool Addon::SupportsHomepage() const {
            if (this->HasBaseAddon()) {
                return this->GetConstBaseAddon()->SupportsHomepage();
            }
            return !this->GetHomepage().empty();
        }


        const AddonType Addon::GetType() const {
            if (this->HasBaseAddon()) {
                return this->GetConstBaseAddon()->GetType();
            }
            return AddonType::AddonTypeUnknown;
        }

        TimeDuration& Addon::GetLoadDuration() {
            if (this->HasBaseAddon()) {
                return this->GetBaseAddon()->GetLoadDuration();
            }
            return this->durationLoad;
        }


        UINT Addon::GetSdkVersion() const { 
            if (this->HasBaseAddon()) {
                return this->GetConstBaseAddon()->GetSdkVersion();
            }
            return this->sdkVersion;
        }

        void Addon::SetSdkVersion(UINT sdkVersion) { 
            if (this->HasBaseAddon()) {
                return this->GetBaseAddon()->SetSdkVersion(sdkVersion);
            }
            this->sdkVersion = sdkVersion;
        }

        IDirect3D9* Addon::GetD3D9() const { 
            if (this->HasBaseAddon()) {
                return this->GetConstBaseAddon()->GetD3D9();
            }
            return this->d3d9;
        }

        void Addon::SetD3D9(IDirect3D9* d3d9) {
            if (this->HasBaseAddon()) {
                return this->GetBaseAddon()->SetD3D9(d3d9);
            }
            this->d3d9 = d3d9;
        }

        IDirect3DDevice9* Addon::GetD3DDevice9() const { 
            if (this->HasBaseAddon()) {
                return this->GetConstBaseAddon()->GetD3DDevice9();
            }
            return this->d3ddevice9;
        }

        void Addon::SetD3DDevice9(IDirect3DDevice9* device) {
            if (this->HasBaseAddon()) {
                return this->GetBaseAddon()->SetD3DDevice9(device);
            }
            this->d3ddevice9 = device;
        }

        HWND Addon::GetFocusWindow() const {
            if (this->HasBaseAddon()) {
                return this->GetConstBaseAddon()->GetFocusWindow();
            }
            return this->focusWindow;
        }

        void Addon::SetFocusWindow(HWND focusWindow) {
            if (this->HasBaseAddon()) {
                return this->GetBaseAddon()->SetFocusWindow(focusWindow);
            }
            this->focusWindow = focusWindow;
        }


        const string Addon::GetFilePath() const {
            if (this->HasBaseAddon()) {
                return this->GetConstBaseAddon()->GetFilePath();
            }
            return this->filePath;
        }

        const string Addon::GetFileName() const {
            if (this->HasBaseAddon()) {
                return this->GetConstBaseAddon()->GetFileName();
            }
            return this->fileName;
        }

        const string Addon::GetID() const {
            if (this->HasBaseAddon()) {
                return this->GetConstBaseAddon()->GetID();
            }
            return this->GetFileName();
        }

        const string Addon::GetName() const {
            if (this->HasBaseAddon()) {
                return this->GetConstBaseAddon()->GetName();
            }
            return this->GetID();
        }

        const string Addon::GetAuthor() const {
            if (this->HasBaseAddon()) {
                return this->GetConstBaseAddon()->GetAuthor();
            }
            return "";
        }

        const string Addon::GetDescription() const {
            if (this->HasBaseAddon()) {
                return this->GetConstBaseAddon()->GetDescription();
            }
            return "";
        }

        const string Addon::GetVersion() const {
            if (this->HasBaseAddon()) {
                return this->GetConstBaseAddon()->GetVersion();
            }
            return "";
        }

        const string Addon::GetHomepage() const {
            if (this->HasBaseAddon()) {
                return this->GetConstBaseAddon()->GetHomepage();
            }
            return "";
        }

        IDirect3DTexture9* Addon::GetIcon() const {
            if (this->HasBaseAddon()) {
                return this->GetConstBaseAddon()->GetIcon();
            }
            return nullptr;
        }


        void Addon::OpenSettings() {
            if (this->HasBaseAddon()) {
                return this->GetBaseAddon()->OpenSettings();
            }
        }


        bool Addon::HandleWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
            if (this->HasBaseAddon()) {
                return this->GetBaseAddon()->HandleWndProc(hWnd, msg, wParam, lParam);
            }
            return false;
        }

        void Addon::DrawFrameBeforeGui(IDirect3DDevice9* device) {
            if (this->HasBaseAddon()) {
                return this->GetBaseAddon()->DrawFrameBeforeGui(device);
            }
        }

        void Addon::DrawFrameBeforePostProcessing(IDirect3DDevice9* device) {
            if (this->HasBaseAddon()) {
                return this->GetBaseAddon()->DrawFrameBeforePostProcessing(device);
            }
        }

        void Addon::DrawFrame(IDirect3DDevice9* device) {
            if (this->HasBaseAddon()) {
                return this->GetBaseAddon()->DrawFrame(device);
            }
        }


        void Addon::AdvPreBeginScene(IDirect3DDevice9* device) {
            if (this->HasBaseAddon()) {
                return this->GetBaseAddon()->AdvPreBeginScene(device);
            }
        }

        void Addon::AdvPostBeginScene(IDirect3DDevice9* device) {
            if (this->HasBaseAddon()) {
                return this->GetBaseAddon()->AdvPostBeginScene(device);
            }
        }

        void Addon::AdvPreEndScene(IDirect3DDevice9* device) {
            if (this->HasBaseAddon()) {
                return this->GetBaseAddon()->AdvPreEndScene(device);
            }
        }

        void Addon::AdvPostEndScene(IDirect3DDevice9* device) {
            if (this->HasBaseAddon()) {
                return this->GetBaseAddon()->AdvPostEndScene(device);
            }
        }

        void Addon::AdvPreClear(IDirect3DDevice9* device, DWORD Count, CONST D3DRECT* pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil) {
            if (this->HasBaseAddon()) {
                return this->GetBaseAddon()->AdvPreClear(device, Count, pRects, Flags, Color, Z, Stencil);
            }
        }

        void Addon::AdvPostClear(IDirect3DDevice9* device, DWORD Count, CONST D3DRECT* pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil) {
            if (this->HasBaseAddon()) {
                return this->GetBaseAddon()->AdvPostClear(device, Count, pRects, Flags, Color, Z, Stencil);
            }
        }

        void Addon::AdvPreReset(IDirect3DDevice9* device, D3DPRESENT_PARAMETERS* pPresentationParameters) {
            if (this->HasBaseAddon()) {
                return this->GetBaseAddon()->AdvPreReset(device, pPresentationParameters);
            }
        }

        void Addon::AdvPostReset(IDirect3DDevice9* device, D3DPRESENT_PARAMETERS* pPresentationParameters) {
            if (this->HasBaseAddon()) {
                return this->GetBaseAddon()->AdvPostReset(device, pPresentationParameters);
            }
        }

        void Addon::AdvPrePresent(IDirect3DDevice9* device, CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion) {
            if (this->HasBaseAddon()) {
                return this->GetBaseAddon()->AdvPrePresent(device, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
            }
        }

        void Addon::AdvPostPresent(IDirect3DDevice9* device, CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion) {
            if (this->HasBaseAddon()) {
                return this->GetBaseAddon()->AdvPostPresent(device, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
            }
        }

        HRESULT Addon::AdvPreCreateTexture(IDirect3DDevice9* device, UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DTexture9** ppTexture, HANDLE* pSharedHandle) {
            if (this->HasBaseAddon()) {
                return this->GetBaseAddon()->AdvPreCreateTexture(device, Width, Height, Levels, Usage, Format, Pool, ppTexture, pSharedHandle);
            }
            return D3D_OK;
        }

        void Addon::AdvPostCreateTexture(IDirect3DDevice9* device, IDirect3DTexture9* pTexture, UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, HANDLE* pSharedHandle) {
            if (this->HasBaseAddon()) {
                return this->GetBaseAddon()->AdvPostCreateTexture(device, pTexture, Width, Height, Levels, Usage, Format, Pool, pSharedHandle);
            }
        }

        HRESULT Addon::AdvPreCreateVertexShader(IDirect3DDevice9* device, CONST DWORD* pFunction, IDirect3DVertexShader9** ppShader) {
            if (this->HasBaseAddon()) {
                return this->GetBaseAddon()->AdvPreCreateVertexShader(device, pFunction, ppShader);
            }
            return D3D_OK;
        }

        void Addon::AdvPostCreateVertexShader(IDirect3DDevice9* device, IDirect3DVertexShader9* ppShader, CONST DWORD* pFunction) {
            if (this->HasBaseAddon()) {
                return this->GetBaseAddon()->AdvPostCreateVertexShader(device, ppShader, pFunction);
            }
        }

        HRESULT Addon::AdvPreCreatePixelShader(IDirect3DDevice9* device, CONST DWORD* pFunction, IDirect3DPixelShader9** ppShader) {
            if (this->HasBaseAddon()) {
                return this->GetBaseAddon()->AdvPreCreatePixelShader(device, pFunction, ppShader);
            }
            return D3D_OK;
        }

        void Addon::AdvPostCreatePixelShader(IDirect3DDevice9* device, IDirect3DPixelShader9* ppShader, CONST DWORD* pFunction) {
            if (this->HasBaseAddon()) {
                return this->GetBaseAddon()->AdvPostCreatePixelShader(device, ppShader, pFunction);
            }
        }

        HRESULT Addon::AdvPreCreateRenderTarget(IDirect3DDevice9* device, UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Lockable, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle) { 
            if (this->HasBaseAddon()) {
                return this->GetBaseAddon()->AdvPreCreateRenderTarget(device, Width, Height, Format, MultiSample, MultisampleQuality, Lockable, ppSurface, pSharedHandle);
            }
            return D3D_OK;
        }
        
        void Addon::AdvPostCreateRenderTarget(IDirect3DDevice9* device, IDirect3DSurface9* ppSurface, UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Lockable, HANDLE* pSharedHandle) {
            if (this->HasBaseAddon()) {
                return this->GetBaseAddon()->AdvPostCreateRenderTarget(device, ppSurface, Width, Height, Format, MultiSample, MultisampleQuality, Lockable, pSharedHandle);
            }
        }

        void Addon::AdvPreSetTexture(IDirect3DDevice9* device, DWORD Stage, IDirect3DBaseTexture9* pTexture) {
            if (this->HasBaseAddon()) {
                return this->GetBaseAddon()->AdvPreSetTexture(device, Stage, pTexture);
            }
        }

        void Addon::AdvPostSetTexture(IDirect3DDevice9* device, DWORD Stage, IDirect3DBaseTexture9* pTexture) {
            if (this->HasBaseAddon()) {
                return this->GetBaseAddon()->AdvPostSetTexture(device, Stage, pTexture);
            }
        }

        void Addon::AdvPreSetVertexShader(IDirect3DDevice9* device, IDirect3DVertexShader9* pShader) {
            if (this->HasBaseAddon()) {
                return this->GetBaseAddon()->AdvPreSetVertexShader(device, pShader);
            }
        }

        void Addon::AdvPostSetVertexShader(IDirect3DDevice9* device, IDirect3DVertexShader9* pShader) {
            if (this->HasBaseAddon()) {
                return this->GetBaseAddon()->AdvPostSetVertexShader(device, pShader);
            }
        }

        void Addon::AdvPreSetPixelShader(IDirect3DDevice9* device, IDirect3DPixelShader9* pShader) {
            if (this->HasBaseAddon()) {
                return this->GetBaseAddon()->AdvPreSetPixelShader(device, pShader);
            }
        }

        void Addon::AdvPostSetPixelShader(IDirect3DDevice9* device, IDirect3DPixelShader9* pShader) {
            if (this->HasBaseAddon()) {
                return this->GetBaseAddon()->AdvPostSetPixelShader(device, pShader);
            }
        }

        void Addon::AdvPreSetRenderTarget(IDirect3DDevice9* device, DWORD RenderTargetIndex, IDirect3DSurface9* pRenderTarget) {
            if (this->HasBaseAddon()) {
                return this->GetBaseAddon()->AdvPreSetRenderTarget(device, RenderTargetIndex, pRenderTarget);
            }
        }

        void Addon::AdvPostSetRenderTarget(IDirect3DDevice9* device, DWORD RenderTargetIndex, IDirect3DSurface9* pRenderTarget) {
            if (this->HasBaseAddon()) {
                return this->GetBaseAddon()->AdvPostSetRenderTarget(device, RenderTargetIndex, pRenderTarget);
            }
        }

        void Addon::AdvPreSetRenderState(IDirect3DDevice9* device, D3DRENDERSTATETYPE State, DWORD Value) {
            if (this->HasBaseAddon()) {
                return this->GetBaseAddon()->AdvPreSetRenderState(device, State, Value);
            }
        }

        void Addon::AdvPostSetRenderState(IDirect3DDevice9* device, D3DRENDERSTATETYPE State, DWORD Value) {
            if (this->HasBaseAddon()) {
                return this->GetBaseAddon()->AdvPostSetRenderState(device, State, Value);
            }
        }

        void Addon::AdvPreDrawIndexedPrimitive(IDirect3DDevice9* device, D3DPRIMITIVETYPE PrimitiveType, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount) {
            if (this->HasBaseAddon()) {
                return this->GetBaseAddon()->AdvPreDrawIndexedPrimitive(device, PrimitiveType, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount);
            }
        }

        void Addon::AdvPostDrawIndexedPrimitive(IDirect3DDevice9* device, D3DPRIMITIVETYPE PrimitiveType, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount) {
            if (this->HasBaseAddon()) {
                return this->GetBaseAddon()->AdvPostDrawIndexedPrimitive(device, PrimitiveType, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount);
            }
        }


        void Addon::OnStartFrame(IDirect3DDevice9* device) {
            if (this->HasBaseAddon()) {
                return this->GetBaseAddon()->OnStartFrame(device);
            }

            if (AppConfig.GetShowDebugFeatures()) {
                this->timeOverall.StartFrame();
                this->timeDrawFrameBeforeGui.StartFrame();
                this->timeDrawFrameBeforePostProcessing.StartFrame();
                this->timeDrawFrame.StartFrame();
                this->timeAdvPreBeginScene.StartFrame();
                this->timeAdvPostBeginScene.StartFrame();
                this->timeAdvPreEndScene.StartFrame();
                this->timeAdvPostEndScene.StartFrame();
                this->timeAdvPreClear.StartFrame();
                this->timeAdvPostClear.StartFrame();
                this->timeAdvPreReset.StartFrame();
                this->timeAdvPostReset.StartFrame();
                this->timeAdvPrePresent.StartFrame();
                this->timeAdvPostPresent.StartFrame();
                this->timeAdvPreCreateTexture.StartFrame();
                this->timeAdvPostCreateTexture.StartFrame();
                this->timeAdvPreCreateVertexShader.StartFrame();
                this->timeAdvPostCreateVertexShader.StartFrame();
                this->timeAdvPreCreatePixelShader.StartFrame();
                this->timeAdvPostCreatePixelShader.StartFrame();
                this->timeAdvPreCreateRenderTarget.StartFrame();
                this->timeAdvPostCreateRenderTarget.StartFrame();
                this->timeAdvPreSetTexture.StartFrame();
                this->timeAdvPostSetTexture.StartFrame();
                this->timeAdvPreSetVertexShader.StartFrame();
                this->timeAdvPostSetVertexShader.StartFrame();
                this->timeAdvPreSetPixelShader.StartFrame();
                this->timeAdvPostSetPixelShader.StartFrame();
                this->timeAdvPreSetRenderTarget.StartFrame();
                this->timeAdvPostSetRenderTarget.StartFrame();
                this->timeAdvPreSetRenderState.StartFrame();
                this->timeAdvPostSetRenderState.StartFrame();
                this->timeAdvPreDrawIndexedPrimitive.StartFrame();
                this->timeAdvPostDrawIndexedPrimitive.StartFrame();
            }
        }

        void Addon::OnEndFrame(IDirect3DDevice9* device) {
            if (this->HasBaseAddon()) {
                return this->GetBaseAddon()->OnEndFrame(device);
            }
      
            if (AppConfig.GetShowDebugFeatures()) {
                this->timeOverall.EndFrame();
                this->timeDrawFrameBeforeGui.EndFrame();
                this->timeDrawFrameBeforePostProcessing.EndFrame();
                this->timeDrawFrame.EndFrame();
                this->timeAdvPreBeginScene.EndFrame();
                this->timeAdvPostBeginScene.EndFrame();
                this->timeAdvPreEndScene.EndFrame();
                this->timeAdvPostEndScene.EndFrame();
                this->timeAdvPreClear.EndFrame();
                this->timeAdvPostClear.EndFrame();
                this->timeAdvPreReset.EndFrame();
                this->timeAdvPostReset.EndFrame();
                this->timeAdvPrePresent.EndFrame();
                this->timeAdvPostPresent.EndFrame();
                this->timeAdvPreCreateTexture.EndFrame();
                this->timeAdvPostCreateTexture.EndFrame();
                this->timeAdvPreCreateVertexShader.EndFrame();
                this->timeAdvPostCreateVertexShader.EndFrame();
                this->timeAdvPreCreatePixelShader.EndFrame();
                this->timeAdvPostCreatePixelShader.EndFrame();
                this->timeAdvPreCreateRenderTarget.EndFrame();
                this->timeAdvPostCreateRenderTarget.EndFrame();
                this->timeAdvPreSetTexture.EndFrame();
                this->timeAdvPostSetTexture.EndFrame();
                this->timeAdvPreSetVertexShader.EndFrame();
                this->timeAdvPostSetVertexShader.EndFrame();
                this->timeAdvPreSetPixelShader.EndFrame();
                this->timeAdvPostSetPixelShader.EndFrame();
                this->timeAdvPreSetRenderTarget.EndFrame();
                this->timeAdvPostSetRenderTarget.EndFrame();
                this->timeAdvPreSetRenderState.EndFrame();
                this->timeAdvPostSetRenderState.EndFrame();
                this->timeAdvPreDrawIndexedPrimitive.EndFrame();
                this->timeAdvPostDrawIndexedPrimitive.EndFrame();
            }
        }


        TimeMeasure& Addon::GetTimeOverall() {
            if (this->HasBaseAddon()) {
                return this->GetBaseAddon()->GetTimeOverall();
            }
            return this->timeOverall;
        }

        TimeMeasure& Addon::GetTimeWndProc() {
            if (this->HasBaseAddon()) {
                return this->GetBaseAddon()->GetTimeWndProc();
            }
            return this->timeWndProc;
        }

        TimeMeasure& Addon::GetTimeDrawFrameBeforeGui() {
            if (this->HasBaseAddon()) {
                return this->GetBaseAddon()->GetTimeDrawFrameBeforeGui();
            }
            return this->timeDrawFrameBeforeGui;
        }

        TimeMeasure& Addon::GetTimeDrawFrameBeforePostProcessing() {
            if (this->HasBaseAddon()) {
                return this->GetBaseAddon()->GetTimeDrawFrameBeforePostProcessing();
            }
            return this->timeDrawFrameBeforePostProcessing;
        }

        TimeMeasure& Addon::GetTimeDrawFrame() {
            if (this->HasBaseAddon()) {
                return this->GetBaseAddon()->GetTimeDrawFrame();
            }
            return this->timeDrawFrame;
        }

        TimeMeasure& Addon::GetTimeAdvPreBeginScene() {
            if (this->HasBaseAddon()) {
                return this->GetBaseAddon()->GetTimeAdvPreBeginScene();
            }
            return this->timeAdvPreBeginScene;
        }

        TimeMeasure& Addon::GetTimeAdvPostBeginScene() {
            if (this->HasBaseAddon()) {
                return this->GetBaseAddon()->GetTimeAdvPostBeginScene();
            }
            return this->timeAdvPostBeginScene;
        }

        TimeMeasure& Addon::GetTimeAdvPreEndScene() {
            if (this->HasBaseAddon()) {
                return this->GetBaseAddon()->GetTimeAdvPreEndScene();
            }
            return this->timeAdvPreEndScene;
        }

        TimeMeasure& Addon::GetTimeAdvPostEndScene() {
            if (this->HasBaseAddon()) {
                return this->GetBaseAddon()->GetTimeAdvPostEndScene();
            }
            return this->timeAdvPostEndScene;
        }

        TimeMeasure& Addon::GetTimeAdvPreClear() {
            if (this->HasBaseAddon()) {
                return this->GetBaseAddon()->GetTimeAdvPreClear();
            }
            return this->timeAdvPreClear;
        }

        TimeMeasure& Addon::GetTimeAdvPostClear() {
            if (this->HasBaseAddon()) {
                return this->GetBaseAddon()->GetTimeAdvPostClear();
            }
            return this->timeAdvPostClear;
        }

        TimeMeasure& Addon::GetTimeAdvPreReset() {
            if (this->HasBaseAddon()) {
                return this->GetBaseAddon()->GetTimeAdvPreReset();
            }
            return this->timeAdvPreReset;
        }

        TimeMeasure& Addon::GetTimeAdvPostReset() {
            if (this->HasBaseAddon()) {
                return this->GetBaseAddon()->GetTimeAdvPostReset();
            }
            return this->timeAdvPostReset;
        }

        TimeMeasure& Addon::GetTimeAdvPrePresent() {
            if (this->HasBaseAddon()) {
                return this->GetBaseAddon()->GetTimeAdvPrePresent();
            }
            return this->timeAdvPrePresent;
        }

        TimeMeasure& Addon::GetTimeAdvPostPresent() {
            if (this->HasBaseAddon()) {
                return this->GetBaseAddon()->GetTimeAdvPostPresent();
            }
            return this->timeAdvPostPresent;
        }

        TimeMeasure& Addon::GetTimeAdvPreCreateTexture() {
            if (this->HasBaseAddon()) {
                return this->GetBaseAddon()->GetTimeAdvPreCreateTexture();
            }
            return this->timeAdvPreCreateTexture;
        }

        TimeMeasure& Addon::GetTimeAdvPostCreateTexture() {
            if (this->HasBaseAddon()) {
                return this->GetBaseAddon()->GetTimeAdvPostCreateTexture();
            }
            return this->timeAdvPostCreateTexture;
        }

        TimeMeasure& Addon::GetTimeAdvPreCreateVertexShader() {
            if (this->HasBaseAddon()) {
                return this->GetBaseAddon()->GetTimeAdvPreCreateVertexShader();
            }
            return this->timeAdvPreCreateVertexShader;
        }

        TimeMeasure& Addon::GetTimeAdvPostCreateVertexShader() {
            if (this->HasBaseAddon()) {
                return this->GetBaseAddon()->GetTimeAdvPostCreateVertexShader();
            }
            return this->timeAdvPostCreateVertexShader;
        }

        TimeMeasure& Addon::GetTimeAdvPreCreatePixelShader() {
            if (this->HasBaseAddon()) {
                return this->GetBaseAddon()->GetTimeAdvPreCreatePixelShader();
            }
            return this->timeAdvPreCreatePixelShader;
        }

        TimeMeasure& Addon::GetTimeAdvPostCreatePixelShader() {
            if (this->HasBaseAddon()) {
                return this->GetBaseAddon()->GetTimeAdvPostCreatePixelShader();
            }
            return this->timeAdvPostCreatePixelShader;
        }

        TimeMeasure& Addon::GetTimeAdvPreCreateRenderTarget() {
            if (this->HasBaseAddon()) {
                return this->GetBaseAddon()->GetTimeAdvPreCreateRenderTarget();
            }
            return this->timeAdvPreCreateRenderTarget;
        }

        TimeMeasure& Addon::GetTimeAdvPostCreateRenderTarget() {
            if (this->HasBaseAddon()) {
                return this->GetBaseAddon()->GetTimeAdvPostCreateRenderTarget();
            }
            return this->timeAdvPostCreateRenderTarget;
        }

        TimeMeasure& Addon::GetTimeAdvPreSetTexture() {
            if (this->HasBaseAddon()) {
                return this->GetBaseAddon()->GetTimeAdvPreSetTexture();
            }
            return this->timeAdvPreSetTexture;
        }

        TimeMeasure& Addon::GetTimeAdvPostSetTexture() {
            if (this->HasBaseAddon()) {
                return this->GetBaseAddon()->GetTimeAdvPostSetTexture();
            }
            return this->timeAdvPostSetTexture;
        }

        TimeMeasure& Addon::GetTimeAdvPreSetVertexShader() {
            if (this->HasBaseAddon()) {
                return this->GetBaseAddon()->GetTimeAdvPreSetVertexShader();
            }
            return this->timeAdvPreSetVertexShader;
        }

        TimeMeasure& Addon::GetTimeAdvPostSetVertexShader() {
            if (this->HasBaseAddon()) {
                return this->GetBaseAddon()->GetTimeAdvPostSetVertexShader();
            }
            return this->timeAdvPostSetVertexShader;
        }

        TimeMeasure& Addon::GetTimeAdvPreSetPixelShader() {
            if (this->HasBaseAddon()) {
                return this->GetBaseAddon()->GetTimeAdvPreSetPixelShader();
            }
            return this->timeAdvPreSetPixelShader;
        }

        TimeMeasure& Addon::GetTimeAdvPostSetPixelShader() {
            if (this->HasBaseAddon()) {
                return this->GetBaseAddon()->GetTimeAdvPostSetPixelShader();
            }
            return this->timeAdvPostSetPixelShader;
        }

        TimeMeasure& Addon::GetTimeAdvPreSetRenderTarget() {
            if (this->HasBaseAddon()) {
                return this->GetBaseAddon()->GetTimeAdvPreSetRenderTarget();
            }
            return this->timeAdvPreSetRenderTarget;
        }

        TimeMeasure& Addon::GetTimeAdvPostSetRenderTarget() {
            if (this->HasBaseAddon()) {
                return this->GetBaseAddon()->GetTimeAdvPostSetRenderTarget();
            }
            return this->timeAdvPostSetRenderTarget;
        }

        TimeMeasure& Addon::GetTimeAdvPreSetRenderState() {
            if (this->HasBaseAddon()) {
                return this->GetBaseAddon()->GetTimeAdvPreSetRenderState();
            }
            return this->timeAdvPreSetRenderState;
        }

        TimeMeasure& Addon::GetTimeAdvPostSetRenderState() {
            if (this->HasBaseAddon()) {
                return this->GetBaseAddon()->GetTimeAdvPostSetRenderState();
            }
            return this->timeAdvPostSetRenderState;
        }

        TimeMeasure& Addon::GetTimeAdvPreDrawIndexedPrimitive() {
            if (this->HasBaseAddon()) {
                return this->GetBaseAddon()->GetTimeAdvPreDrawIndexedPrimitive();
            }
            return this->timeAdvPreDrawIndexedPrimitive;
        }

        TimeMeasure& Addon::GetTimeAdvPostDrawIndexedPrimitive() {
            if (this->HasBaseAddon()) {
                return this->GetBaseAddon()->GetTimeAdvPostDrawIndexedPrimitive();
            }
            return this->timeAdvPostDrawIndexedPrimitive;
        }

    }
}
