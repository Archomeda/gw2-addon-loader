#include "NativeAddonImpl.h"
#include <exception>
#include <filesystem>
#include "../Addon.h"
#include "../addons_manager.h"
#include "../exceptions.h"
#include "../../utils/encoding.h"

using namespace std;
using namespace std::experimental::filesystem::v1;
using namespace loader::utils;

namespace loader {
    namespace addons {
        namespace types {

            NativeAddonImpl::NativeAddonImpl(const string& filePath) : ITypeImpl() {
                this->filePath = filePath;
                this->fileName = u8path(filePath).filename().u8string();
            }

            void NativeAddonImpl::Initialize() {
                wstring wFilePath = u16(this->filePath);
                HMODULE h = LoadLibrary(wFilePath.c_str());

                if (!h) {
                    this->ChangeState(AddonState::ErroredState);
                    throw exceptions::AddonInitializationException("Library handle is empty");
                }
                this->addonHandle = h;

                this->AddonInitialize = reinterpret_cast<GW2AddonInitialize_t>(GetProcAddress(h, GW2ADDON_DLL_Initialize));
                this->AddonRelease = reinterpret_cast<GW2AddonRelease_t>(GetProcAddress(h, GW2ADDON_DLL_Release));

                if (this->AddonInitialize == nullptr) {
                    this->ChangeState(AddonState::ErroredState);
                    throw exceptions::AddonInitializationException("Addon didn't contain a valid " GW2ADDON_DLL_Initialize " export");
                }
                if (this->AddonRelease == nullptr) {
                    this->ChangeState(AddonState::ErroredState);
                    throw exceptions::AddonInitializationException("Addon didn't contain a valid " GW2ADDON_DLL_Release " export");
                }

                GW2AddonAPIBase* addonBase = this->AddonInitialize(GW2ADDON_VER);
                if (addonBase == nullptr) {
                    this->ChangeState(AddonState::ErroredState);
                    throw exceptions::AddonInitializationException("Addon didn't return a valid GW2AddonAPIBase when calling " GW2ADDON_DLL_Initialize);
                }

                if (addonBase->ver == 1) {
                    GW2AddonAPIV1* v1 = reinterpret_cast<GW2AddonAPIV1*>(addonBase);
                    this->id = v1->id;
                    this->name = v1->name;
                    this->author = v1->author;
                    this->description = v1->description;
                    this->version = v1->version;
                    this->homepage = v1->homepage;

                    this->AddonLoad = v1->Load;
                    this->AddonDrawFrameBeforePostProcessing = v1->DrawFrameBeforePostProcessing;
                    this->AddonDrawFrameBeforeGui = v1->DrawFrameBeforeGui;
                    this->AddonDrawFrame = v1->DrawFrame;
                    this->AddonHandleWndProc = v1->HandleWndProc;
                    this->AddonAdvPreBeginScene = v1->AdvPreBeginScene;
                    this->AddonAdvPostBeginScene = v1->AdvPostBeginScene;
                    this->AddonAdvPreEndScene = v1->AdvPreEndScene;
                    this->AddonAdvPostEndScene = v1->AdvPostEndScene;
                    this->AddonAdvPreClear = v1->AdvPreClear;
                    this->AddonAdvPostClear = v1->AdvPostClear;
                    this->AddonAdvPreReset = v1->AdvPreReset;
                    this->AddonAdvPostReset = v1->AdvPostReset;
                    this->AddonAdvPrePresent = v1->AdvPrePresent;
                    this->AddonAdvPostPresent = v1->AdvPostPresent;
                    this->AddonAdvPreCreateTexture = v1->AdvPreCreateTexture;
                    this->AddonAdvPostCreateTexture = v1->AdvPostCreateTexture;
                    this->AddonAdvPreCreateVertexShader = v1->AdvPreCreateVertexShader;
                    this->AddonAdvPostCreateVertexShader = v1->AdvPostCreateVertexShader;
                    this->AddonAdvPreCreatePixelShader = v1->AdvPreCreatePixelShader;
                    this->AddonAdvPostCreatePixelShader = v1->AdvPostCreatePixelShader;
                    this->AddonAdvPreCreateRenderTarget = v1->AdvPreCreateRenderTarget;
                    this->AddonAdvPostCreateRenderTarget = v1->AdvPostCreateRenderTarget;
                    this->AddonAdvPreSetTexture = v1->AdvPreSetTexture;
                    this->AddonAdvPostSetTexture = v1->AdvPostSetTexture;
                    this->AddonAdvPreSetVertexShader = v1->AdvPreSetVertexShader;
                    this->AddonAdvPostSetVertexShader = v1->AdvPostSetVertexShader;
                    this->AddonAdvPreSetPixelShader = v1->AdvPreSetPixelShader;
                    this->AddonAdvPostSetPixelShader = v1->AdvPostSetPixelShader;
                    this->AddonAdvPreSetRenderTarget = v1->AdvPreSetRenderTarget;
                    this->AddonAdvPostSetRenderTarget = v1->AdvPostSetRenderTarget;
                    this->AddonAdvPreSetRenderState = v1->AdvPreSetRenderState;
                    this->AddonAdvPostSetRenderState = v1->AdvPostSetRenderState;
                    this->AddonAdvPreDrawIndexedPrimitive = v1->AdvPreDrawIndexedPrimitive;
                    this->AddonAdvPostDrawIndexedPrimitive = v1->AdvPostDrawIndexedPrimitive;
                }
                else {
                    this->ChangeState(AddonState::ErroredState);
                    throw exceptions::AddonLoadingException("Addon uses a version (" + to_string(addonBase->ver) + ") that is not supported, are both the addon and your addon loader updated?");
                }
            }

            void NativeAddonImpl::Uninitialize() {
                this->Unload();

                this->AddonLoad = nullptr;
                this->AddonDrawFrame = nullptr;
                this->AddonDrawFrameBeforeGui = nullptr;
                this->AddonDrawFrameBeforePostProcessing = nullptr;
                this->AddonHandleWndProc = nullptr;
                this->AddonAdvPreBeginScene = nullptr;
                this->AddonAdvPostBeginScene = nullptr;
                this->AddonAdvPreEndScene = nullptr;
                this->AddonAdvPostEndScene = nullptr;
                this->AddonAdvPreClear = nullptr;
                this->AddonAdvPostClear = nullptr;
                this->AddonAdvPreReset = nullptr;
                this->AddonAdvPostReset = nullptr;
                this->AddonAdvPrePresent = nullptr;
                this->AddonAdvPostPresent = nullptr;
                this->AddonAdvPreCreateTexture = nullptr;
                this->AddonAdvPostCreateTexture = nullptr;
                this->AddonAdvPreCreateVertexShader = nullptr;
                this->AddonAdvPostCreateVertexShader = nullptr;
                this->AddonAdvPreCreatePixelShader = nullptr;
                this->AddonAdvPostCreatePixelShader = nullptr;
                this->AddonAdvPreCreateRenderTarget = nullptr;
                this->AddonAdvPostCreateRenderTarget = nullptr;
                this->AddonAdvPreSetTexture = nullptr;
                this->AddonAdvPostSetTexture = nullptr;
                this->AddonAdvPreSetVertexShader = nullptr;
                this->AddonAdvPostSetVertexShader = nullptr;
                this->AddonAdvPreSetPixelShader = nullptr;
                this->AddonAdvPostSetPixelShader = nullptr;
                this->AddonAdvPreSetRenderTarget = nullptr;
                this->AddonAdvPostSetRenderTarget = nullptr;
                this->AddonAdvPreSetRenderState = nullptr;
                this->AddonAdvPostSetRenderState = nullptr;
                this->AddonAdvPreDrawIndexedPrimitive = nullptr;
                this->AddonAdvPostDrawIndexedPrimitive = nullptr;
                
                if (this->addonHandle) {
                    FreeLibrary(this->addonHandle);
                    this->addonHandle = NULL;
                }
            }

            void NativeAddonImpl::Load() {
                auto state = this->GetAddonState();
                if (state != AddonState::UnloadedState) {
                    return;
                }

                if (this->AddonLoad) {
                    auto addon = this->GetAddon().lock();
                    GW2ADDON_RESULT result = this->AddonLoad(addon->GetFocusWindow(), addon->GetD3DDevice9());
                    if (result) {
                        this->ChangeState(AddonState::ErroredState);
                        throw exceptions::AddonLoadingException("Addon returned " + to_string(result));
                    }
                }

                if (this->AddonDrawFrameBeforeGui) ++ActiveAddonHookCounts.DrawFrameBeforeGui;
                if (this->AddonDrawFrameBeforePostProcessing) ++ActiveAddonHookCounts.DrawFrameBeforePostProcessing;
                if (this->AddonDrawFrame) ++ActiveAddonHookCounts.DrawFrame;

                if (this->AddonAdvPreBeginScene) ++ActiveAddonHookCounts.AdvPreBeginScene;
                if (this->AddonAdvPostBeginScene) ++ActiveAddonHookCounts.AdvPostBeginScene;
                if (this->AddonAdvPreEndScene) ++ActiveAddonHookCounts.AdvPreEndScene;
                if (this->AddonAdvPostEndScene) ++ActiveAddonHookCounts.AdvPostEndScene;
                if (this->AddonAdvPreClear) ++ActiveAddonHookCounts.AdvPreClear;
                if (this->AddonAdvPostClear) ++ActiveAddonHookCounts.AdvPostClear;
                if (this->AddonAdvPreReset) ++ActiveAddonHookCounts.AdvPreReset;
                if (this->AddonAdvPostReset) ++ActiveAddonHookCounts.AdvPostReset;
                if (this->AddonAdvPrePresent) ++ActiveAddonHookCounts.AdvPrePresent;
                if (this->AddonAdvPostPresent) ++ActiveAddonHookCounts.AdvPostPresent;
                if (this->AddonAdvPreCreateTexture) ++ActiveAddonHookCounts.AdvPreCreateTexture;
                if (this->AddonAdvPostCreateTexture) ++ActiveAddonHookCounts.AdvPostCreateTexture;
                if (this->AddonAdvPreCreateVertexShader) ++ActiveAddonHookCounts.AdvPreCreateVertexShader;
                if (this->AddonAdvPostCreateVertexShader) ++ActiveAddonHookCounts.AdvPostCreateVertexShader;
                if (this->AddonAdvPreCreatePixelShader) ++ActiveAddonHookCounts.AdvPreCreatePixelShader;
                if (this->AddonAdvPostCreatePixelShader) ++ActiveAddonHookCounts.AdvPostCreatePixelShader;
                if (this->AddonAdvPreCreateRenderTarget) ++ActiveAddonHookCounts.AdvPreCreateRenderTarget;
                if (this->AddonAdvPostCreateRenderTarget) ++ActiveAddonHookCounts.AdvPostCreateRenderTarget;
                if (this->AddonAdvPreSetTexture) ++ActiveAddonHookCounts.AdvPreSetTexture;
                if (this->AddonAdvPostSetTexture) ++ActiveAddonHookCounts.AdvPostSetTexture;
                if (this->AddonAdvPreSetVertexShader) ++ActiveAddonHookCounts.AdvPreSetVertexShader;
                if (this->AddonAdvPostSetVertexShader) ++ActiveAddonHookCounts.AdvPostSetVertexShader;
                if (this->AddonAdvPreSetPixelShader) ++ActiveAddonHookCounts.AdvPreSetPixelShader;
                if (this->AddonAdvPostSetPixelShader) ++ActiveAddonHookCounts.AdvPostSetPixelShader;
                if (this->AddonAdvPreSetRenderTarget) ++ActiveAddonHookCounts.AdvPreSetRenderTarget;
                if (this->AddonAdvPostSetRenderTarget) ++ActiveAddonHookCounts.AdvPostSetRenderTarget;
                if (this->AddonAdvPreSetRenderState) ++ActiveAddonHookCounts.AdvPreSetRenderState;
                if (this->AddonAdvPostSetRenderState) ++ActiveAddonHookCounts.AdvPostSetRenderState;
                if (this->AddonAdvPreDrawIndexedPrimitive) ++ActiveAddonHookCounts.AdvPreDrawIndexedPrimitive;
                if (this->AddonAdvPostDrawIndexedPrimitive) ++ActiveAddonHookCounts.AdvPostDrawIndexedPrimitive;
                
                // Done
                this->ChangeState(AddonState::LoadedState);
            }

            void NativeAddonImpl::Unload() {
                auto state = this->GetAddonState();
                if (state != AddonState::LoadedState) {
                    return;
                }

                // Unload the addon
                this->ChangeState(AddonState::UnloadingState);

                if (this->AddonDrawFrameBeforeGui) --ActiveAddonHookCounts.DrawFrameBeforeGui;
                if (this->AddonDrawFrameBeforePostProcessing) --ActiveAddonHookCounts.DrawFrameBeforePostProcessing;
                if (this->AddonDrawFrame) --ActiveAddonHookCounts.DrawFrame;

                if (this->AddonAdvPreBeginScene) --ActiveAddonHookCounts.AdvPreBeginScene;
                if (this->AddonAdvPostBeginScene) --ActiveAddonHookCounts.AdvPostBeginScene;
                if (this->AddonAdvPreEndScene) --ActiveAddonHookCounts.AdvPreEndScene;
                if (this->AddonAdvPostEndScene) --ActiveAddonHookCounts.AdvPostEndScene;
                if (this->AddonAdvPreClear) --ActiveAddonHookCounts.AdvPreClear;
                if (this->AddonAdvPostClear) --ActiveAddonHookCounts.AdvPostClear;
                if (this->AddonAdvPreReset) --ActiveAddonHookCounts.AdvPreReset;
                if (this->AddonAdvPostReset) --ActiveAddonHookCounts.AdvPostReset;
                if (this->AddonAdvPrePresent) --ActiveAddonHookCounts.AdvPrePresent;
                if (this->AddonAdvPostPresent) --ActiveAddonHookCounts.AdvPostPresent;
                if (this->AddonAdvPreCreateTexture) --ActiveAddonHookCounts.AdvPreCreateTexture;
                if (this->AddonAdvPostCreateTexture) --ActiveAddonHookCounts.AdvPostCreateTexture;
                if (this->AddonAdvPreCreateVertexShader) --ActiveAddonHookCounts.AdvPreCreateVertexShader;
                if (this->AddonAdvPostCreateVertexShader) --ActiveAddonHookCounts.AdvPostCreateVertexShader;
                if (this->AddonAdvPreCreatePixelShader) --ActiveAddonHookCounts.AdvPreCreatePixelShader;
                if (this->AddonAdvPostCreatePixelShader) --ActiveAddonHookCounts.AdvPostCreatePixelShader;
                if (this->AddonAdvPreCreateRenderTarget) --ActiveAddonHookCounts.AdvPreCreateRenderTarget;
                if (this->AddonAdvPostCreateRenderTarget) --ActiveAddonHookCounts.AdvPostCreateRenderTarget;
                if (this->AddonAdvPreSetTexture) --ActiveAddonHookCounts.AdvPreSetTexture;
                if (this->AddonAdvPostSetTexture) --ActiveAddonHookCounts.AdvPostSetTexture;
                if (this->AddonAdvPreSetVertexShader) --ActiveAddonHookCounts.AdvPreSetVertexShader;
                if (this->AddonAdvPostSetVertexShader) --ActiveAddonHookCounts.AdvPostSetVertexShader;
                if (this->AddonAdvPreSetPixelShader) --ActiveAddonHookCounts.AdvPreSetPixelShader;
                if (this->AddonAdvPostSetPixelShader) --ActiveAddonHookCounts.AdvPostSetPixelShader;
                if (this->AddonAdvPreSetRenderTarget) --ActiveAddonHookCounts.AdvPreSetRenderTarget;
                if (this->AddonAdvPostSetRenderTarget) --ActiveAddonHookCounts.AdvPostSetRenderTarget;
                if (this->AddonAdvPreSetRenderState) --ActiveAddonHookCounts.AdvPreSetRenderState;
                if (this->AddonAdvPostSetRenderState) --ActiveAddonHookCounts.AdvPostSetRenderState;
                if (this->AddonAdvPreDrawIndexedPrimitive) --ActiveAddonHookCounts.AdvPreDrawIndexedPrimitive;
                if (this->AddonAdvPostDrawIndexedPrimitive) --ActiveAddonHookCounts.AdvPostDrawIndexedPrimitive;

                this->AddonRelease();

                // Done
                this->ChangeState(AddonState::UnloadedState);
            }


            bool NativeAddonImpl::HandleWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
                if (!this->AddonHandleWndProc) {
                    return false;
                }
                try {
                    return this->AddonHandleWndProc(hWnd, msg, wParam, lParam);
                }
                catch (const exception& ex) {
                    this->ChangeState(AddonState::ErroredState);
                    throw exceptions::AddonWndProcException(ex.what());
                }
                catch (const char* err) {
                    this->ChangeState(AddonState::ErroredState);
                    throw exceptions::AddonWndProcException(err);
                }
                catch (const wchar_t* err) {
                    this->ChangeState(AddonState::ErroredState);
                    throw exceptions::AddonWndProcException(err);
                }
                catch (...) {
                    this->ChangeState(AddonState::ErroredState);
                    throw exceptions::AddonWndProcException("Unknown error");
                }
            }


            void NativeAddonImpl::DrawFrameBeforeGui(IDirect3DDevice9* device) {
                if (!this->AddonDrawFrameBeforeGui) {
                    return;
                }
                this->callDrawFunc([this, device]() {
                    this->StartTimeMeasure();
                    this->AddonDrawFrameBeforeGui(device);
                    this->AddDurationHistory(&this->durationHistoryDrawFrameBeforeGui, this->EndTimeMeasure());
                });
            }

            void NativeAddonImpl::DrawFrameBeforePostProcessing(IDirect3DDevice9* device) {
                if (!this->AddonDrawFrameBeforePostProcessing) {
                    return;
                }
                this->callDrawFunc([this, device]() {
                    this->StartTimeMeasure();
                    this->AddonDrawFrameBeforePostProcessing(device);
                    this->AddDurationHistory(&this->durationHistoryDrawFrameBeforePostProcessing, this->EndTimeMeasure());
                });
            }

            void NativeAddonImpl::DrawFrame(IDirect3DDevice9* device) {
                if (!this->AddonDrawFrame) {
                    return;
                }
                this->callDrawFunc([this, device]() {
                    this->StartTimeMeasure();
                    this->AddonDrawFrame(device);
                    this->AddDurationHistory(&this->durationHistoryDrawFrame, this->EndTimeMeasure());
                });
            }


            void NativeAddonImpl::AdvPreBeginScene(IDirect3DDevice9* device) {
                if (!this->AddonAdvPreBeginScene) {
                    return;
                }
                this->callAdvFunc("PreBeginScene", [=]() {
                    this->AddonAdvPreBeginScene(device);
                });
            }

            void NativeAddonImpl::AdvPostBeginScene(IDirect3DDevice9* device) {
                if (!this->AddonAdvPostBeginScene) {
                    return;
                }
                this->callAdvFunc("PostBeginScene", [=]() {
                    this->AddonAdvPostBeginScene(device);
                });
            }

            void NativeAddonImpl::AdvPreEndScene(IDirect3DDevice9* device) {
                if (!this->AddonAdvPreEndScene) {
                    return;
                }
                this->callAdvFunc("PreEndScene", [=]() {
                    this->AddonAdvPreEndScene(device);
                });
            }

            void NativeAddonImpl::AdvPostEndScene(IDirect3DDevice9* device) {
                if (!this->AddonAdvPostEndScene) {
                    return;
                }
                this->callAdvFunc("PostEndScene", [=]() {
                    this->AddonAdvPostEndScene(device);
                });
            }

            void NativeAddonImpl::AdvPreClear(IDirect3DDevice9* device, DWORD Count, CONST D3DRECT* pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil) {
                if (!this->AddonAdvPreClear) {
                    return;
                }
                this->callAdvFunc("PreClear", [=]() {
                    this->AddonAdvPreClear(device, Count, pRects, Flags, Color, Z, Stencil);
                });
            }

            void NativeAddonImpl::AdvPostClear(IDirect3DDevice9* device, DWORD Count, CONST D3DRECT* pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil) {
                if (!this->AddonAdvPostClear) {
                    return;
                }
                this->callAdvFunc("PostClear", [=]() {
                    this->AddonAdvPostClear(device, Count, pRects, Flags, Color, Z, Stencil);
                });
            }

            void NativeAddonImpl::AdvPreReset(IDirect3DDevice9* device, D3DPRESENT_PARAMETERS* pPresentationParameters) {
                if (!this->AddonAdvPreReset) {
                    return;
                }
                this->callAdvFunc("PreReset", [=]() {
                    this->AddonAdvPreReset(device, pPresentationParameters);
                });
            }

            void NativeAddonImpl::AdvPostReset(IDirect3DDevice9* device, D3DPRESENT_PARAMETERS* pPresentationParameters) {
                if (!this->AddonAdvPostReset) {
                    return;
                }
                this->callAdvFunc("PostReset", [=]() {
                    this->AddonAdvPostReset(device, pPresentationParameters);
                });
            }

            void NativeAddonImpl::AdvPrePresent(IDirect3DDevice9* device, CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion) {
                if (!this->AddonAdvPrePresent) {
                    return;
                }
                this->callAdvFunc("PrePresent", [=]() {
                    this->AddonAdvPrePresent(device, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
                });
            }

            void NativeAddonImpl::AdvPostPresent(IDirect3DDevice9* device, CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion) {
                if (!this->AddonAdvPostPresent) {
                    return;
                }
                this->callAdvFunc("PostPresent", [=]() {
                    this->AddonAdvPostPresent(device, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
                });
            }

            HRESULT NativeAddonImpl::AdvPreCreateTexture(IDirect3DDevice9* device, UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DTexture9** ppTexture, HANDLE* pSharedHandle) {
                if (!this->AddonAdvPreCreateTexture) {
                    return D3D_OK;
                }
                return this->callAdvFuncWithResult("PreCreateTexture", [=]() {
                    return this->AddonAdvPreCreateTexture(device, Width, Height, Levels, Usage, Format, Pool, ppTexture, pSharedHandle);
                });
            }

            void NativeAddonImpl::AdvPostCreateTexture(IDirect3DDevice9* device, IDirect3DTexture9* pTexture, UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, HANDLE* pSharedHandle) {
                if (!this->AddonAdvPostCreateTexture) {
                    return;
                }
                this->callAdvFunc("PostCreateTexture", [=]() {
                    this->AddonAdvPostCreateTexture(device, pTexture, Width, Height, Levels, Usage, Format, Pool, pSharedHandle);
                });
            }

            HRESULT NativeAddonImpl::AdvPreCreateVertexShader(IDirect3DDevice9* device, CONST DWORD* pFunction, IDirect3DVertexShader9** ppShader) {
                if (!this->AddonAdvPreCreateVertexShader) {
                    return D3D_OK;
                }
                return this->callAdvFuncWithResult("PreCreateVertexShader", [=]() {
                    return this->AddonAdvPreCreateVertexShader(device, pFunction, ppShader);
                });
            }

            void NativeAddonImpl::AdvPostCreateVertexShader(IDirect3DDevice9* device, IDirect3DVertexShader9* pShader, CONST DWORD* pFunction) {
                if (!this->AddonAdvPostCreateVertexShader) {
                    return;
                }
                this->callAdvFunc("PostCreateVertexShader", [=]() {
                    this->AddonAdvPostCreateVertexShader(device, pShader, pFunction);
                });
            }

            HRESULT NativeAddonImpl::AdvPreCreatePixelShader(IDirect3DDevice9* device, CONST DWORD* pFunction, IDirect3DPixelShader9** ppShader) {
                if (!this->AddonAdvPreCreatePixelShader) {
                    return D3D_OK;
                }
                return this->callAdvFuncWithResult("PreCraetePixelShader", [=]() {
                    return this->AddonAdvPreCreatePixelShader(device, pFunction, ppShader);
                });
            }

            void NativeAddonImpl::AdvPostCreatePixelShader(IDirect3DDevice9* device, IDirect3DPixelShader9* pShader, CONST DWORD* pFunction) {
                if (!this->AddonAdvPostCreatePixelShader) {
                    return;
                }
                this->callAdvFunc("PostCraetePixelShader", [=]() {
                    this->AddonAdvPostCreatePixelShader(device, pShader, pFunction);
                });
            }

            HRESULT NativeAddonImpl::AdvPreCreateRenderTarget(IDirect3DDevice9* device, UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Lockable, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle) {
                if (!this->AddonAdvPreCreateRenderTarget) {
                    return D3D_OK;
                }
                return this->callAdvFuncWithResult("PreCreateRenderTarget", [=]() {
                    return this->AddonAdvPreCreateRenderTarget(device, Width, Height, Format, MultiSample, MultisampleQuality, Lockable, ppSurface, pSharedHandle);
                });
            }

            void NativeAddonImpl::AdvPostCreateRenderTarget(IDirect3DDevice9* device, IDirect3DSurface9* pSurface, UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Lockable, HANDLE* pSharedHandle) {
                if (!this->AddonAdvPostCreateRenderTarget) {
                    return;
                }
                this->callAdvFunc("PostCreateRenderTarget", [=]() {
                    this->AddonAdvPostCreateRenderTarget(device, pSurface, Width, Height, Format, MultiSample, MultisampleQuality, Lockable, pSharedHandle);
                });
            }

            void NativeAddonImpl::AdvPreSetTexture(IDirect3DDevice9* device, DWORD Stage, IDirect3DBaseTexture9* pTexture) {
                if (!this->AddonAdvPreSetTexture) {
                    return;
                }
                this->callAdvFunc("PreSetTexture", [=]() {
                    this->AddonAdvPreSetTexture(device, Stage, pTexture);
                });
            }

            void NativeAddonImpl::AdvPostSetTexture(IDirect3DDevice9* device, DWORD Stage, IDirect3DBaseTexture9* pTexture) {
                if (!this->AddonAdvPostSetTexture) {
                    return;
                }
                this->callAdvFunc("PostSetTexture", [=]() {
                    this->AddonAdvPostSetTexture(device, Stage, pTexture);
                });
            }

            void NativeAddonImpl::AdvPreSetVertexShader(IDirect3DDevice9* device, IDirect3DVertexShader9* pShader) {
                if (!this->AddonAdvPreSetVertexShader) {
                    return;
                }
                this->callAdvFunc("PreSetVertexShader", [=]() {
                    this->AddonAdvPreSetVertexShader(device, pShader);
                });
            }

            void NativeAddonImpl::AdvPostSetVertexShader(IDirect3DDevice9* device, IDirect3DVertexShader9* pShader) {
                if (!this->AddonAdvPostSetVertexShader) {
                    return;
                }
                this->callAdvFunc("PostSetVertexShader", [=]() {
                    this->AddonAdvPostSetVertexShader(device, pShader);
                });
            }

            void NativeAddonImpl::AdvPreSetPixelShader(IDirect3DDevice9* device, IDirect3DPixelShader9* pShader) {
                if (!this->AddonAdvPreSetPixelShader) {
                    return;
                }
                this->callAdvFunc("PreSetPixelShader", [=]() {
                    this->AddonAdvPreSetPixelShader(device, pShader);
                });
            }

            void NativeAddonImpl::AdvPostSetPixelShader(IDirect3DDevice9* device, IDirect3DPixelShader9* pShader) {
                if (!this->AddonAdvPostSetPixelShader) {
                    return;
                }
                this->callAdvFunc("PostSetPixelShader", [=]() {
                    this->AddonAdvPostSetPixelShader(device, pShader);
                });
            }

            void NativeAddonImpl::AdvPreSetRenderTarget(IDirect3DDevice9* device, DWORD RenderTargetIndex, IDirect3DSurface9* pRenderTarget) {
                if (!this->AddonAdvPreSetRenderTarget) {
                    return;
                }
                this->callAdvFunc("PreSetRenderTarget", [=]() {
                    this->AddonAdvPreSetRenderTarget(device, RenderTargetIndex, pRenderTarget);
                });
            }

            void NativeAddonImpl::AdvPostSetRenderTarget(IDirect3DDevice9* device, DWORD RenderTargetIndex, IDirect3DSurface9* pRenderTarget) {
                if (!this->AddonAdvPostSetRenderTarget) {
                    return;
                }
                this->callAdvFunc("PostSetRenderTarget", [=]() {
                    this->AddonAdvPostSetRenderTarget(device, RenderTargetIndex, pRenderTarget);
                });
            }

            void NativeAddonImpl::AdvPreSetRenderState(IDirect3DDevice9* device, D3DRENDERSTATETYPE State, DWORD Value) {
                if (!this->AddonAdvPreSetRenderState) {
                    return;
                }
                this->callAdvFunc("PreSetRenderState", [=]() {
                    this->AddonAdvPreSetRenderState(device, State, Value);
                });
            }

            void NativeAddonImpl::AdvPostSetRenderState(IDirect3DDevice9* device, D3DRENDERSTATETYPE State, DWORD Value) {
                if (!this->AddonAdvPostSetRenderState) {
                    return;
                }
                this->callAdvFunc("PostSetRenderState", [=]() {
                    this->AddonAdvPostSetRenderState(device, State, Value);
                });
            }

            void NativeAddonImpl::AdvPreDrawIndexedPrimitive(IDirect3DDevice9* device, D3DPRIMITIVETYPE PrimitiveType, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount) {
                if (!this->AddonAdvPreDrawIndexedPrimitive) {
                    return;
                }
                this->callAdvFunc("PreDrawIndexedPrimitive", [=]() {
                    this->AddonAdvPreDrawIndexedPrimitive(device, PrimitiveType, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount);
                });
            }

            void NativeAddonImpl::AdvPostDrawIndexedPrimitive(IDirect3DDevice9* device, D3DPRIMITIVETYPE PrimitiveType, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount) {
                if (!this->AddonAdvPostDrawIndexedPrimitive) {
                    return;
                }
                this->callAdvFunc("PostDrawIndexedPrimitive", [=]() {
                    this->AddonAdvPostDrawIndexedPrimitive(device, PrimitiveType, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount);
                });
            }


            void NativeAddonImpl::callDrawFunc(const function<void(void)>& func) {
                try {
                    func();
                }
                catch (const exception& ex) {
                    this->ChangeState(AddonState::ErroredState);
                    throw exceptions::AddonDrawException(ex.what());
                }
                catch (const char* err) {
                    this->ChangeState(AddonState::ErroredState);
                    throw exceptions::AddonDrawException(err);
                }
                catch (const wchar_t* err) {
                    this->ChangeState(AddonState::ErroredState);
                    throw exceptions::AddonDrawException(err);
                }
                catch (...) {
                    this->ChangeState(AddonState::ErroredState);
                    throw exceptions::AddonDrawException("Unknown error");
                }
            }

            void NativeAddonImpl::callAdvFunc(const string& funcName, const function<void(void)>& func) {
                try {
                    func();
                }
                catch (const exception& ex) {
                    this->ChangeState(AddonState::ErroredState);
                    throw exceptions::AddonAdvFuncException(funcName, ex.what());
                }
                catch (const char* err) {
                    this->ChangeState(AddonState::ErroredState);
                    throw exceptions::AddonAdvFuncException(funcName, err);
                }
                catch (const wchar_t* err) {
                    this->ChangeState(AddonState::ErroredState);
                    throw exceptions::AddonAdvFuncException(funcName, err);
                }
                catch (...) {
                    this->ChangeState(AddonState::ErroredState);
                    throw exceptions::AddonAdvFuncException(funcName, "Unknown error");
                }
            }

            HRESULT NativeAddonImpl::callAdvFuncWithResult(const string& funcName, const function<HRESULT(void)>& func) {
                try {
                    return func();
                }
                catch (const exception& ex) {
                    this->ChangeState(AddonState::ErroredState);
                    throw exceptions::AddonAdvFuncException(funcName, ex.what());
                }
                catch (const char* err) {
                    this->ChangeState(AddonState::ErroredState);
                    throw exceptions::AddonAdvFuncException(funcName, err);
                }
                catch (const wchar_t* err) {
                    this->ChangeState(AddonState::ErroredState);
                    throw exceptions::AddonAdvFuncException(funcName, err);
                }
                catch (...) {
                    this->ChangeState(AddonState::ErroredState);
                    throw exceptions::AddonAdvFuncException(funcName, "Unknown error");
                }
            }
        }
    }
}
