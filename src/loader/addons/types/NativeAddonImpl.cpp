#include "NativeAddonImpl.h"
#include <exception>
#include <filesystem>
#include "../Addon.h"
#include "../addons_manager.h"
#include "../exceptions.h"
#include "../../utils.h"

using namespace std;
using namespace std::experimental::filesystem::v1;

namespace loader {
    namespace addons {
        namespace types {

            NativeAddonImpl::NativeAddonImpl(const wstring& filePath) : ITypeImpl() {
                this->filePath = filePath;
                this->fileName = path(filePath).filename();
            }

            void NativeAddonImpl::Initialize() {
            }

            void NativeAddonImpl::Uninitialize() {
                if (this->handle) {
                    FreeLibrary(this->handle);
                    this->handle = NULL;
                }
            }

            void NativeAddonImpl::Load() {
                auto state = this->GetAddonState();
                if (state != AddonState::UnloadedState) {
                    return;
                }

                // Load the addon
                this->ChangeState(AddonState::LoadingState);
                HMODULE h = LoadLibrary(this->filePath.c_str());
                if (!h) {
                    this->ChangeState(AddonState::ErroredState);
                    throw exceptions::AddonLoadingException("Library handle is empty");
                }
                this->handle = h;

                this->GW2_Load = reinterpret_cast<GW2_Load_t>(GetProcAddress(h, GW2_DLL_Load));
                this->GW2_Unload = reinterpret_cast<GW2_Unload_t>(GetProcAddress(h, GW2_DLL_Unload));

                this->GW2_HandleWndProc = reinterpret_cast<GW2_HandleWndProc_t>(GetProcAddress(h, GW2_DLL_HandleWndProc));

                this->GW2_DrawFrameBeforeGui = reinterpret_cast<GW2_DrawFrame_t>(GetProcAddress(h, GW2_DLL_DrawFrameBeforeGui));
                this->GW2_DrawFrameBeforePostProcessing = reinterpret_cast<GW2_DrawFrame_t>(GetProcAddress(h, GW2_DLL_DrawFrameBeforePostProcessing));
                this->GW2_DrawFrame = reinterpret_cast<GW2_DrawFrame_t>(GetProcAddress(h, GW2_DLL_DrawFrame));

                this->GW2_AdvPreBeginScene = reinterpret_cast<GW2_AdvPreBeginScene_t>(GetProcAddress(h, GW2_DLL_AdvPreBeginScene));
                this->GW2_AdvPostBeginScene = reinterpret_cast<GW2_AdvPostBeginScene_t>(GetProcAddress(h, GW2_DLL_AdvPostBeginScene));
                this->GW2_AdvPreEndScene = reinterpret_cast<GW2_AdvPreEndScene_t>(GetProcAddress(h, GW2_DLL_AdvPreEndScene));
                this->GW2_AdvPostEndScene = reinterpret_cast<GW2_AdvPostEndScene_t>(GetProcAddress(h, GW2_DLL_AdvPostEndScene));
                this->GW2_AdvPreClear = reinterpret_cast<GW2_AdvPreClear_t>(GetProcAddress(h, GW2_DLL_AdvPreClear));
                this->GW2_AdvPostClear = reinterpret_cast<GW2_AdvPostClear_t>(GetProcAddress(h, GW2_DLL_AdvPostClear));
                this->GW2_AdvPreReset = reinterpret_cast<GW2_AdvPreReset_t>(GetProcAddress(h, GW2_DLL_AdvPreReset));
                this->GW2_AdvPostReset = reinterpret_cast<GW2_AdvPostReset_t>(GetProcAddress(h, GW2_DLL_AdvPostReset));
                this->GW2_AdvPrePresent = reinterpret_cast<GW2_AdvPrePresent_t>(GetProcAddress(h, GW2_DLL_AdvPrePresent));
                this->GW2_AdvPostPresent = reinterpret_cast<GW2_AdvPostPresent_t>(GetProcAddress(h, GW2_DLL_AdvPostPresent));
                this->GW2_AdvPreCreateTexture = reinterpret_cast<GW2_AdvPreCreateTexture_t>(GetProcAddress(h, GW2_DLL_AdvPreCreateTexture));
                this->GW2_AdvPostCreateTexture = reinterpret_cast<GW2_AdvPostCreateTexture_t>(GetProcAddress(h, GW2_DLL_AdvPostCreateTexture));
                this->GW2_AdvPreCreateVertexShader = reinterpret_cast<GW2_AdvPreCreateVertexShader_t>(GetProcAddress(h, GW2_DLL_AdvPreCreateVertexShader));
                this->GW2_AdvPostCreateVertexShader = reinterpret_cast<GW2_AdvPostCreateVertexShader_t>(GetProcAddress(h, GW2_DLL_AdvPostCreateVertexShader));
                this->GW2_AdvPreCreatePixelShader = reinterpret_cast<GW2_AdvPreCreatePixelShader_t>(GetProcAddress(h, GW2_DLL_AdvPreCreatePixelShader));
                this->GW2_AdvPostCreatePixelShader = reinterpret_cast<GW2_AdvPostCreatePixelShader_t>(GetProcAddress(h, GW2_DLL_AdvPostCreatePixelShader));
                this->GW2_AdvPreCreateRenderTarget = reinterpret_cast<GW2_AdvPreCreateRenderTarget_t>(GetProcAddress(h, GW2_DLL_AdvPreCreateRenderTarget));
                this->GW2_AdvPostCreateRenderTarget = reinterpret_cast<GW2_AdvPostCreateRenderTarget_t>(GetProcAddress(h, GW2_DLL_AdvPostCreateRenderTarget));
                this->GW2_AdvPreSetTexture = reinterpret_cast<GW2_AdvPreSetTexture_t>(GetProcAddress(h, GW2_DLL_AdvPreSetTexture));
                this->GW2_AdvPostSetTexture = reinterpret_cast<GW2_AdvPostSetTexture_t>(GetProcAddress(h, GW2_DLL_AdvPostSetTexture));
                this->GW2_AdvPreSetVertexShader = reinterpret_cast<GW2_AdvPreSetVertexShader_t>(GetProcAddress(h, GW2_DLL_AdvPreSetVertexShader));
                this->GW2_AdvPostSetVertexShader = reinterpret_cast<GW2_AdvPostSetVertexShader_t>(GetProcAddress(h, GW2_DLL_AdvPostSetVertexShader));
                this->GW2_AdvPreSetPixelShader = reinterpret_cast<GW2_AdvPreSetPixelShader_t>(GetProcAddress(h, GW2_DLL_AdvPreSetPixelShader));
                this->GW2_AdvPostSetPixelShader = reinterpret_cast<GW2_AdvPostSetPixelShader_t>(GetProcAddress(h, GW2_DLL_AdvPostSetPixelShader));
                this->GW2_AdvPreSetRenderTarget = reinterpret_cast<GW2_AdvPreSetRenderTarget_t>(GetProcAddress(h, GW2_DLL_AdvPreSetRenderTarget));
                this->GW2_AdvPostSetRenderTarget = reinterpret_cast<GW2_AdvPostSetRenderTarget_t>(GetProcAddress(h, GW2_DLL_AdvPostSetRenderTarget));
                this->GW2_AdvPreSetRenderState = reinterpret_cast<GW2_AdvPreSetRenderState_t>(GetProcAddress(h, GW2_DLL_AdvPreSetRenderState));
                this->GW2_AdvPostSetRenderState = reinterpret_cast<GW2_AdvPostSetRenderState_t>(GetProcAddress(h, GW2_DLL_AdvPostSetRenderState));
                this->GW2_AdvPreDrawIndexedPrimitive = reinterpret_cast<GW2_AdvPreDrawIndexedPrimitive_t>(GetProcAddress(h, GW2_DLL_AdvPreDrawIndexedPrimitive));
                this->GW2_AdvPostDrawIndexedPrimitive = reinterpret_cast<GW2_AdvPostDrawIndexedPrimitive_t>(GetProcAddress(h, GW2_DLL_AdvPostDrawIndexedPrimitive));

                if (this->GW2_DrawFrameBeforeGui) ++ActiveAddonHookCounts.DrawFrameBeforeGui;
                if (this->GW2_DrawFrameBeforePostProcessing) ++ActiveAddonHookCounts.DrawFrameBeforePostProcessing;
                if (this->GW2_DrawFrame) ++ActiveAddonHookCounts.DrawFrame;

                if (this->GW2_AdvPreBeginScene) ++ActiveAddonHookCounts.AdvPreBeginScene;
                if (this->GW2_AdvPostBeginScene) ++ActiveAddonHookCounts.AdvPostBeginScene;
                if (this->GW2_AdvPreEndScene) ++ActiveAddonHookCounts.AdvPreEndScene;
                if (this->GW2_AdvPostEndScene) ++ActiveAddonHookCounts.AdvPostEndScene;
                if (this->GW2_AdvPreClear) ++ActiveAddonHookCounts.AdvPreClear;
                if (this->GW2_AdvPostClear) ++ActiveAddonHookCounts.AdvPostClear;
                if (this->GW2_AdvPreReset) ++ActiveAddonHookCounts.AdvPreReset;
                if (this->GW2_AdvPostReset) ++ActiveAddonHookCounts.AdvPostReset;
                if (this->GW2_AdvPrePresent) ++ActiveAddonHookCounts.AdvPrePresent;
                if (this->GW2_AdvPostPresent) ++ActiveAddonHookCounts.AdvPostPresent;
                if (this->GW2_AdvPreCreateTexture) ++ActiveAddonHookCounts.AdvPreCreateTexture;
                if (this->GW2_AdvPostCreateTexture) ++ActiveAddonHookCounts.AdvPostCreateTexture;
                if (this->GW2_AdvPreCreateVertexShader) ++ActiveAddonHookCounts.AdvPreCreateVertexShader;
                if (this->GW2_AdvPostCreateVertexShader) ++ActiveAddonHookCounts.AdvPostCreateVertexShader;
                if (this->GW2_AdvPreCreatePixelShader) ++ActiveAddonHookCounts.AdvPreCreatePixelShader;
                if (this->GW2_AdvPostCreatePixelShader) ++ActiveAddonHookCounts.AdvPostCreatePixelShader;
                if (this->GW2_AdvPreCreateRenderTarget) ++ActiveAddonHookCounts.AdvPreCreateRenderTarget;
                if (this->GW2_AdvPostCreateRenderTarget) ++ActiveAddonHookCounts.AdvPostCreateRenderTarget;
                if (this->GW2_AdvPreSetTexture) ++ActiveAddonHookCounts.AdvPreSetTexture;
                if (this->GW2_AdvPostSetTexture) ++ActiveAddonHookCounts.AdvPostSetTexture;
                if (this->GW2_AdvPreSetVertexShader) ++ActiveAddonHookCounts.AdvPreSetVertexShader;
                if (this->GW2_AdvPostSetVertexShader) ++ActiveAddonHookCounts.AdvPostSetVertexShader;
                if (this->GW2_AdvPreSetPixelShader) ++ActiveAddonHookCounts.AdvPreSetPixelShader;
                if (this->GW2_AdvPostSetPixelShader) ++ActiveAddonHookCounts.AdvPostSetPixelShader;
                if (this->GW2_AdvPreSetRenderTarget) ++ActiveAddonHookCounts.AdvPreSetRenderTarget;
                if (this->GW2_AdvPostSetRenderTarget) ++ActiveAddonHookCounts.AdvPostSetRenderTarget;
                if (this->GW2_AdvPreSetRenderState) ++ActiveAddonHookCounts.AdvPreSetRenderState;
                if (this->GW2_AdvPostSetRenderState) ++ActiveAddonHookCounts.AdvPostSetRenderState;
                if (this->GW2_AdvPreDrawIndexedPrimitive) ++ActiveAddonHookCounts.AdvPreDrawIndexedPrimitive;
                if (this->GW2_AdvPostDrawIndexedPrimitive) ++ActiveAddonHookCounts.AdvPostDrawIndexedPrimitive;

                if (this->GW2_Load) {
                    auto addon = this->GetAddon().lock();
                    GW2ADDON_RESULT result = this->GW2_Load(addon->GetFocusWindow(), addon->GetD3DDevice9());
                    if (result) {
                        this->ChangeState(AddonState::ErroredState);
                        throw exceptions::AddonLoadingException("Addon returned " + to_string(result));
                    }
                }

                if (this->GW2_DrawFrame) {
                    this->ChangeSubType(AddonSubType::GraphicsAddon);
                }
                else {
                    this->ChangeSubType(AddonSubType::NonGraphicsAddon);
                }

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
                if (this->GW2_Unload) {
                    GW2ADDON_RESULT result = this->GW2_Unload();
                    if (result) {
                        this->ChangeState(AddonState::ErroredState);
                        throw exceptions::AddonUnloadingException("Addon returned " + to_string(result));
                    }
                }
                this->Uninitialize();

                this->GW2_Load = nullptr;
                this->GW2_Unload = nullptr;

                this->GW2_HandleWndProc = nullptr;

                if (this->GW2_DrawFrameBeforeGui) --ActiveAddonHookCounts.DrawFrameBeforeGui;
                if (this->GW2_DrawFrameBeforePostProcessing) --ActiveAddonHookCounts.DrawFrameBeforePostProcessing;
                if (this->GW2_DrawFrame) --ActiveAddonHookCounts.DrawFrame;

                if (this->GW2_AdvPreBeginScene) --ActiveAddonHookCounts.AdvPreBeginScene;
                if (this->GW2_AdvPostBeginScene) --ActiveAddonHookCounts.AdvPostBeginScene;
                if (this->GW2_AdvPreEndScene) --ActiveAddonHookCounts.AdvPreEndScene;
                if (this->GW2_AdvPostEndScene) --ActiveAddonHookCounts.AdvPostEndScene;
                if (this->GW2_AdvPreClear) --ActiveAddonHookCounts.AdvPreClear;
                if (this->GW2_AdvPostClear) --ActiveAddonHookCounts.AdvPostClear;
                if (this->GW2_AdvPreReset) --ActiveAddonHookCounts.AdvPreReset;
                if (this->GW2_AdvPostReset) --ActiveAddonHookCounts.AdvPostReset;
                if (this->GW2_AdvPrePresent) --ActiveAddonHookCounts.AdvPrePresent;
                if (this->GW2_AdvPostPresent) --ActiveAddonHookCounts.AdvPostPresent;
                if (this->GW2_AdvPreCreateTexture) --ActiveAddonHookCounts.AdvPreCreateTexture;
                if (this->GW2_AdvPostCreateTexture) --ActiveAddonHookCounts.AdvPostCreateTexture;
                if (this->GW2_AdvPreCreateVertexShader) --ActiveAddonHookCounts.AdvPreCreateVertexShader;
                if (this->GW2_AdvPostCreateVertexShader) --ActiveAddonHookCounts.AdvPostCreateVertexShader;
                if (this->GW2_AdvPreCreatePixelShader) --ActiveAddonHookCounts.AdvPreCreatePixelShader;
                if (this->GW2_AdvPostCreatePixelShader) --ActiveAddonHookCounts.AdvPostCreatePixelShader;
                if (this->GW2_AdvPreCreateRenderTarget) --ActiveAddonHookCounts.AdvPreCreateRenderTarget;
                if (this->GW2_AdvPostCreateRenderTarget) --ActiveAddonHookCounts.AdvPostCreateRenderTarget;
                if (this->GW2_AdvPreSetTexture) --ActiveAddonHookCounts.AdvPreSetTexture;
                if (this->GW2_AdvPostSetTexture) --ActiveAddonHookCounts.AdvPostSetTexture;
                if (this->GW2_AdvPreSetVertexShader) --ActiveAddonHookCounts.AdvPreSetVertexShader;
                if (this->GW2_AdvPostSetVertexShader) --ActiveAddonHookCounts.AdvPostSetVertexShader;
                if (this->GW2_AdvPreSetPixelShader) --ActiveAddonHookCounts.AdvPreSetPixelShader;
                if (this->GW2_AdvPostSetPixelShader) --ActiveAddonHookCounts.AdvPostSetPixelShader;
                if (this->GW2_AdvPreSetRenderTarget) --ActiveAddonHookCounts.AdvPreSetRenderTarget;
                if (this->GW2_AdvPostSetRenderTarget) --ActiveAddonHookCounts.AdvPostSetRenderTarget;
                if (this->GW2_AdvPreSetRenderState) --ActiveAddonHookCounts.AdvPreSetRenderState;
                if (this->GW2_AdvPostSetRenderState) --ActiveAddonHookCounts.AdvPostSetRenderState;
                if (this->GW2_AdvPreDrawIndexedPrimitive) --ActiveAddonHookCounts.AdvPreDrawIndexedPrimitive;
                if (this->GW2_AdvPostDrawIndexedPrimitive) --ActiveAddonHookCounts.AdvPostDrawIndexedPrimitive;
                
                this->GW2_DrawFrame = nullptr;
                this->GW2_DrawFrameBeforeGui = nullptr;
                this->GW2_DrawFrameBeforePostProcessing = nullptr;

                this->GW2_AdvPreBeginScene = nullptr;
                this->GW2_AdvPostBeginScene = nullptr;
                this->GW2_AdvPreEndScene = nullptr;
                this->GW2_AdvPostEndScene = nullptr;
                this->GW2_AdvPreClear = nullptr;
                this->GW2_AdvPostClear = nullptr;
                this->GW2_AdvPreReset = nullptr;
                this->GW2_AdvPostReset = nullptr;
                this->GW2_AdvPrePresent = nullptr;
                this->GW2_AdvPostPresent = nullptr;
                this->GW2_AdvPreCreateTexture = nullptr;
                this->GW2_AdvPostCreateTexture = nullptr;
                this->GW2_AdvPreCreateVertexShader = nullptr;
                this->GW2_AdvPostCreateVertexShader = nullptr;
                this->GW2_AdvPreCreatePixelShader = nullptr;
                this->GW2_AdvPostCreatePixelShader = nullptr;
                this->GW2_AdvPreCreateRenderTarget = nullptr;
                this->GW2_AdvPostCreateRenderTarget = nullptr;
                this->GW2_AdvPreSetTexture = nullptr;
                this->GW2_AdvPostSetTexture = nullptr;
                this->GW2_AdvPreSetVertexShader = nullptr;
                this->GW2_AdvPostSetVertexShader = nullptr;
                this->GW2_AdvPreSetPixelShader = nullptr;
                this->GW2_AdvPostSetPixelShader = nullptr;
                this->GW2_AdvPreSetRenderTarget = nullptr;
                this->GW2_AdvPostSetRenderTarget = nullptr;
                this->GW2_AdvPreSetRenderState = nullptr;
                this->GW2_AdvPostSetRenderState = nullptr;
                this->GW2_AdvPreDrawIndexedPrimitive = nullptr;
                this->GW2_AdvPostDrawIndexedPrimitive = nullptr;

                // Done
                this->ChangeState(AddonState::UnloadedState);
            }


            bool NativeAddonImpl::HandleWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
                if (!this->GW2_HandleWndProc) {
                    return false;
                }
                try {
                    return this->GW2_HandleWndProc(hWnd, msg, wParam, lParam);
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
                    throw exceptions::AddonWndProcException(ws2s(err));
                }
                catch (...) {
                    this->ChangeState(AddonState::ErroredState);
                    throw exceptions::AddonWndProcException("Unknown error");
                }
            }


            void NativeAddonImpl::DrawFrameBeforeGui(IDirect3DDevice9* device) {
                if (!this->GW2_DrawFrameBeforeGui) {
                    return;
                }
                this->callDrawFunc([this, device]() {
                    this->StartTimeMeasure();
                    this->GW2_DrawFrameBeforeGui(device);
                    this->AddDurationHistory(&this->durationHistoryDrawFrameBeforeGui, this->EndTimeMeasure());
                });
            }

            void NativeAddonImpl::DrawFrameBeforePostProcessing(IDirect3DDevice9* device) {
                if (!this->GW2_DrawFrameBeforePostProcessing) {
                    return;
                }
                this->callDrawFunc([this, device]() {
                    this->StartTimeMeasure();
                    this->GW2_DrawFrameBeforePostProcessing(device);
                    this->AddDurationHistory(&this->durationHistoryDrawFrameBeforePostProcessing, this->EndTimeMeasure());
                });
            }

            void NativeAddonImpl::DrawFrame(IDirect3DDevice9* device) {
                if (!this->GW2_DrawFrame) {
                    return;
                }
                this->callDrawFunc([this, device]() {
                    this->StartTimeMeasure();
                    this->GW2_DrawFrame(device);
                    this->AddDurationHistory(&this->durationHistoryDrawFrame, this->EndTimeMeasure());
                });
            }


            void NativeAddonImpl::AdvPreBeginScene(IDirect3DDevice9* device) {
                if (!this->GW2_AdvPreBeginScene) {
                    return;
                }
                this->callAdvFunc("PreBeginScene", [=]() {
                    this->GW2_AdvPreBeginScene(device);
                });
            }

            void NativeAddonImpl::AdvPostBeginScene(IDirect3DDevice9* device) {
                if (!this->GW2_AdvPostBeginScene) {
                    return;
                }
                this->callAdvFunc("PostBeginScene", [=]() {
                    this->GW2_AdvPostBeginScene(device);
                });
            }

            void NativeAddonImpl::AdvPreEndScene(IDirect3DDevice9* device) {
                if (!this->GW2_AdvPreEndScene) {
                    return;
                }
                this->callAdvFunc("PreEndScene", [=]() {
                    this->GW2_AdvPreEndScene(device);
                });
            }

            void NativeAddonImpl::AdvPostEndScene(IDirect3DDevice9* device) {
                if (!this->GW2_AdvPostEndScene) {
                    return;
                }
                this->callAdvFunc("PostEndScene", [=]() {
                    this->GW2_AdvPostEndScene(device);
                });
            }

            void NativeAddonImpl::AdvPreClear(IDirect3DDevice9* device, DWORD Count, CONST D3DRECT* pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil) {
                if (!this->GW2_AdvPreClear) {
                    return;
                }
                this->callAdvFunc("PreClear", [=]() {
                    this->GW2_AdvPreClear(device, Count, pRects, Flags, Color, Z, Stencil);
                });
            }

            void NativeAddonImpl::AdvPostClear(IDirect3DDevice9* device, DWORD Count, CONST D3DRECT* pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil) {
                if (!this->GW2_AdvPostClear) {
                    return;
                }
                this->callAdvFunc("PostClear", [=]() {
                    this->GW2_AdvPostClear(device, Count, pRects, Flags, Color, Z, Stencil);
                });
            }

            void NativeAddonImpl::AdvPreReset(IDirect3DDevice9* device, D3DPRESENT_PARAMETERS* pPresentationParameters) {
                if (!this->GW2_AdvPreReset) {
                    return;
                }
                this->callAdvFunc("PreReset", [=]() {
                    this->GW2_AdvPreReset(device, pPresentationParameters);
                });
            }

            void NativeAddonImpl::AdvPostReset(IDirect3DDevice9* device, D3DPRESENT_PARAMETERS* pPresentationParameters) {
                if (!this->GW2_AdvPostReset) {
                    return;
                }
                this->callAdvFunc("PostReset", [=]() {
                    this->GW2_AdvPostReset(device, pPresentationParameters);
                });
            }

            void NativeAddonImpl::AdvPrePresent(IDirect3DDevice9* device, CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion) {
                if (!this->GW2_AdvPrePresent) {
                    return;
                }
                this->callAdvFunc("PrePresent", [=]() {
                    this->GW2_AdvPrePresent(device, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
                });
            }

            void NativeAddonImpl::AdvPostPresent(IDirect3DDevice9* device, CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion) {
                if (!this->GW2_AdvPostPresent) {
                    return;
                }
                this->callAdvFunc("PostPresent", [=]() {
                    this->GW2_AdvPostPresent(device, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
                });
            }

            HRESULT NativeAddonImpl::AdvPreCreateTexture(IDirect3DDevice9* device, UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DTexture9** ppTexture, HANDLE* pSharedHandle) {
                if (!this->GW2_AdvPreCreateTexture) {
                    return D3D_OK;
                }
                return this->callAdvFuncWithResult("PreCreateTexture", [=]() {
                    return this->GW2_AdvPreCreateTexture(device, Width, Height, Levels, Usage, Format, Pool, ppTexture, pSharedHandle);
                });
            }

            void NativeAddonImpl::AdvPostCreateTexture(IDirect3DDevice9* device, IDirect3DTexture9* pTexture, UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, HANDLE* pSharedHandle) {
                if (!this->GW2_AdvPostCreateTexture) {
                    return;
                }
                this->callAdvFunc("PostCreateTexture", [=]() {
                    this->GW2_AdvPostCreateTexture(device, pTexture, Width, Height, Levels, Usage, Format, Pool, pSharedHandle);
                });
            }

            HRESULT NativeAddonImpl::AdvPreCreateVertexShader(IDirect3DDevice9* device, CONST DWORD* pFunction, IDirect3DVertexShader9** ppShader) {
                if (!this->GW2_AdvPreCreateVertexShader) {
                    return D3D_OK;
                }
                return this->callAdvFuncWithResult("PreCreateVertexShader", [=]() {
                    return this->GW2_AdvPreCreateVertexShader(device, pFunction, ppShader);
                });
            }

            void NativeAddonImpl::AdvPostCreateVertexShader(IDirect3DDevice9* device, IDirect3DVertexShader9* pShader, CONST DWORD* pFunction) {
                if (!this->GW2_AdvPostCreateVertexShader) {
                    return;
                }
                this->callAdvFunc("PostCreateVertexShader", [=]() {
                    this->GW2_AdvPostCreateVertexShader(device, pShader, pFunction);
                });
            }

            HRESULT NativeAddonImpl::AdvPreCreatePixelShader(IDirect3DDevice9* device, CONST DWORD* pFunction, IDirect3DPixelShader9** ppShader) {
                if (!this->GW2_AdvPreCreatePixelShader) {
                    return D3D_OK;
                }
                return this->callAdvFuncWithResult("PreCraetePixelShader", [=]() {
                    return this->GW2_AdvPreCreatePixelShader(device, pFunction, ppShader);
                });
            }

            void NativeAddonImpl::AdvPostCreatePixelShader(IDirect3DDevice9* device, IDirect3DPixelShader9* pShader, CONST DWORD* pFunction) {
                if (!this->GW2_AdvPostCreatePixelShader) {
                    return;
                }
                this->callAdvFunc("PostCraetePixelShader", [=]() {
                    this->GW2_AdvPostCreatePixelShader(device, pShader, pFunction);
                });
            }

            HRESULT NativeAddonImpl::AdvPreCreateRenderTarget(IDirect3DDevice9* device, UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Lockable, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle) {
                if (!this->GW2_AdvPreCreateRenderTarget) {
                    return D3D_OK;
                }
                return this->callAdvFuncWithResult("PreCreateRenderTarget", [=]() {
                    return this->GW2_AdvPreCreateRenderTarget(device, Width, Height, Format, MultiSample, MultisampleQuality, Lockable, ppSurface, pSharedHandle);
                });
            }

            void NativeAddonImpl::AdvPostCreateRenderTarget(IDirect3DDevice9* device, IDirect3DSurface9* pSurface, UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Lockable, HANDLE* pSharedHandle) {
                if (!this->GW2_AdvPostCreateRenderTarget) {
                    return;
                }
                this->callAdvFunc("PostCreateRenderTarget", [=]() {
                    this->GW2_AdvPostCreateRenderTarget(device, pSurface, Width, Height, Format, MultiSample, MultisampleQuality, Lockable, pSharedHandle);
                });
            }

            void NativeAddonImpl::AdvPreSetTexture(IDirect3DDevice9* device, DWORD Stage, IDirect3DBaseTexture9* pTexture) {
                if (!this->GW2_AdvPreSetTexture) {
                    return;
                }
                this->callAdvFunc("PreSetTexture", [=]() {
                    this->GW2_AdvPreSetTexture(device, Stage, pTexture);
                });
            }

            void NativeAddonImpl::AdvPostSetTexture(IDirect3DDevice9* device, DWORD Stage, IDirect3DBaseTexture9* pTexture) {
                if (!this->GW2_AdvPostSetTexture) {
                    return;
                }
                this->callAdvFunc("PostSetTexture", [=]() {
                    this->GW2_AdvPostSetTexture(device, Stage, pTexture);
                });
            }

            void NativeAddonImpl::AdvPreSetVertexShader(IDirect3DDevice9* device, IDirect3DVertexShader9* pShader) {
                if (!this->GW2_AdvPreSetVertexShader) {
                    return;
                }
                this->callAdvFunc("PreSetVertexShader", [=]() {
                    this->GW2_AdvPreSetVertexShader(device, pShader);
                });
            }

            void NativeAddonImpl::AdvPostSetVertexShader(IDirect3DDevice9* device, IDirect3DVertexShader9* pShader) {
                if (!this->GW2_AdvPostSetVertexShader) {
                    return;
                }
                this->callAdvFunc("PostSetVertexShader", [=]() {
                    this->GW2_AdvPostSetVertexShader(device, pShader);
                });
            }

            void NativeAddonImpl::AdvPreSetPixelShader(IDirect3DDevice9* device, IDirect3DPixelShader9* pShader) {
                if (!this->GW2_AdvPreSetPixelShader) {
                    return;
                }
                this->callAdvFunc("PreSetPixelShader", [=]() {
                    this->GW2_AdvPreSetPixelShader(device, pShader);
                });
            }

            void NativeAddonImpl::AdvPostSetPixelShader(IDirect3DDevice9* device, IDirect3DPixelShader9* pShader) {
                if (!this->GW2_AdvPostSetPixelShader) {
                    return;
                }
                this->callAdvFunc("PostSetPixelShader", [=]() {
                    this->GW2_AdvPostSetPixelShader(device, pShader);
                });
            }

            void NativeAddonImpl::AdvPreSetRenderTarget(IDirect3DDevice9* device, DWORD RenderTargetIndex, IDirect3DSurface9* pRenderTarget) {
                if (!this->GW2_AdvPreSetRenderTarget) {
                    return;
                }
                this->callAdvFunc("PreSetRenderTarget", [=]() {
                    this->GW2_AdvPreSetRenderTarget(device, RenderTargetIndex, pRenderTarget);
                });
            }

            void NativeAddonImpl::AdvPostSetRenderTarget(IDirect3DDevice9* device, DWORD RenderTargetIndex, IDirect3DSurface9* pRenderTarget) {
                if (!this->GW2_AdvPostSetRenderTarget) {
                    return;
                }
                this->callAdvFunc("PostSetRenderTarget", [=]() {
                    this->GW2_AdvPostSetRenderTarget(device, RenderTargetIndex, pRenderTarget);
                });
            }

            void NativeAddonImpl::AdvPreSetRenderState(IDirect3DDevice9* device, D3DRENDERSTATETYPE State, DWORD Value) {
                if (!this->GW2_AdvPreSetRenderState) {
                    return;
                }
                this->callAdvFunc("PreSetRenderState", [=]() {
                    this->GW2_AdvPreSetRenderState(device, State, Value);
                });
            }

            void NativeAddonImpl::AdvPostSetRenderState(IDirect3DDevice9* device, D3DRENDERSTATETYPE State, DWORD Value) {
                if (!this->GW2_AdvPostSetRenderState) {
                    return;
                }
                this->callAdvFunc("PostSetRenderState", [=]() {
                    this->GW2_AdvPostSetRenderState(device, State, Value);
                });
            }

            void NativeAddonImpl::AdvPreDrawIndexedPrimitive(IDirect3DDevice9* device, D3DPRIMITIVETYPE PrimitiveType, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount) {
                if (!this->GW2_AdvPreDrawIndexedPrimitive) {
                    return;
                }
                this->callAdvFunc("PreDrawIndexedPrimitive", [=]() {
                    this->GW2_AdvPreDrawIndexedPrimitive(device, PrimitiveType, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount);
                });
            }

            void NativeAddonImpl::AdvPostDrawIndexedPrimitive(IDirect3DDevice9* device, D3DPRIMITIVETYPE PrimitiveType, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount) {
                if (!this->GW2_AdvPostDrawIndexedPrimitive) {
                    return;
                }
                this->callAdvFunc("PostDrawIndexedPrimitive", [=]() {
                    this->GW2_AdvPostDrawIndexedPrimitive(device, PrimitiveType, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount);
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
                    throw exceptions::AddonDrawException(ws2s(err));
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
                    throw exceptions::AddonAdvFuncException(funcName, ws2s(err));
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
                    throw exceptions::AddonAdvFuncException(funcName, ws2s(err));
                }
                catch (...) {
                    this->ChangeState(AddonState::ErroredState);
                    throw exceptions::AddonAdvFuncException(funcName, "Unknown error");
                }
            }
        }
    }
}
