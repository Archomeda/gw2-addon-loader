#include "NativeAddonImpl.h"
#include <exception>
#include <filesystem>
#include "../Addon.h"
#include "../addons_manager.h"
#include "../exceptions.h"
#include "../../Config.h"
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


            void NativeAddonImpl::OnStartFrame(IDirect3DDevice9* device) {
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

            void NativeAddonImpl::OnEndFrame(IDirect3DDevice9* device) {
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
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->timeOverall.StartMeasurement();
                        this->timeDrawFrameBeforeGui.StartMeasurement();
                    }
                    this->AddonDrawFrameBeforeGui(device);
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->timeDrawFrameBeforeGui.EndMeasurement();
                        this->timeOverall.EndMeasurement();
                    }
                });
            }

            void NativeAddonImpl::DrawFrameBeforePostProcessing(IDirect3DDevice9* device) {
                if (!this->AddonDrawFrameBeforePostProcessing) {
                    return;
                }
                this->callDrawFunc([this, device]() {
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->timeOverall.StartMeasurement();
                        this->timeDrawFrameBeforePostProcessing.StartMeasurement();
                    }
                    this->AddonDrawFrameBeforePostProcessing(device);
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->timeDrawFrameBeforePostProcessing.EndMeasurement();
                        this->timeOverall.EndMeasurement();
                    }
                });
            }

            void NativeAddonImpl::DrawFrame(IDirect3DDevice9* device) {
                if (!this->AddonDrawFrame) {
                    return;
                }
                this->callDrawFunc([this, device]() {
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->timeOverall.StartMeasurement();
                        this->timeDrawFrame.StartMeasurement();
                    }
                    this->AddonDrawFrame(device);
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->timeDrawFrame.EndMeasurement();
                        this->timeOverall.EndMeasurement();
                    }
                });
            }


            void NativeAddonImpl::AdvPreBeginScene(IDirect3DDevice9* device) {
                if (!this->AddonAdvPreBeginScene) {
                    return;
                }
                this->callAdvFunc("PreBeginScene", [=]() {
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->timeOverall.StartMeasurement();
                        this->timeAdvPreBeginScene.StartMeasurement();
                    }
                    this->AddonAdvPreBeginScene(device);
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->timeAdvPreBeginScene.EndMeasurement();
                        this->timeOverall.EndMeasurement();
                    }
                });
            }

            void NativeAddonImpl::AdvPostBeginScene(IDirect3DDevice9* device) {
                if (!this->AddonAdvPostBeginScene) {
                    return;
                }
                this->callAdvFunc("PostBeginScene", [=]() {
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->timeOverall.StartMeasurement();
                        this->timeAdvPostBeginScene.StartMeasurement();
                    }
                    this->AddonAdvPostBeginScene(device);
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->timeAdvPostBeginScene.EndMeasurement();
                        this->timeOverall.EndMeasurement();
                    }
                });
            }

            void NativeAddonImpl::AdvPreEndScene(IDirect3DDevice9* device) {
                if (!this->AddonAdvPreEndScene) {
                    return;
                }
                this->callAdvFunc("PreEndScene", [=]() {
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->timeOverall.StartMeasurement();
                        this->timeAdvPreEndScene.StartMeasurement();
                    }
                    this->AddonAdvPreEndScene(device);
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->timeAdvPreEndScene.EndMeasurement();
                        this->timeOverall.EndMeasurement();
                    }
                });
            }

            void NativeAddonImpl::AdvPostEndScene(IDirect3DDevice9* device) {
                if (!this->AddonAdvPostEndScene) {
                    return;
                }
                this->callAdvFunc("PostEndScene", [=]() {
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->timeOverall.StartMeasurement();
                        this->timeAdvPostEndScene.StartMeasurement();
                    }
                    this->AddonAdvPostEndScene(device);
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->timeAdvPostEndScene.EndMeasurement();
                        this->timeOverall.EndMeasurement();
                    }
                });
            }

            void NativeAddonImpl::AdvPreClear(IDirect3DDevice9* device, DWORD Count, CONST D3DRECT* pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil) {
                if (!this->AddonAdvPreClear) {
                    return;
                }
                this->callAdvFunc("PreClear", [=]() {
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->timeOverall.StartMeasurement();
                        this->timeAdvPreClear.StartMeasurement();
                    }
                    this->AddonAdvPreClear(device, Count, pRects, Flags, Color, Z, Stencil);
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->timeAdvPreClear.EndMeasurement();
                        this->timeOverall.EndMeasurement();
                    }
                });
            }

            void NativeAddonImpl::AdvPostClear(IDirect3DDevice9* device, DWORD Count, CONST D3DRECT* pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil) {
                if (!this->AddonAdvPostClear) {
                    return;
                }
                this->callAdvFunc("PostClear", [=]() {
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->timeOverall.StartMeasurement();
                        this->timeAdvPostClear.StartMeasurement();
                    }
                    this->AddonAdvPostClear(device, Count, pRects, Flags, Color, Z, Stencil);
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->timeAdvPostClear.EndMeasurement();
                        this->timeOverall.EndMeasurement();
                    }
                });
            }

            void NativeAddonImpl::AdvPreReset(IDirect3DDevice9* device, D3DPRESENT_PARAMETERS* pPresentationParameters) {
                if (!this->AddonAdvPreReset) {
                    return;
                }
                this->callAdvFunc("PreReset", [=]() {
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->timeOverall.StartMeasurement();
                        this->timeAdvPreReset.StartMeasurement();
                    }
                    this->AddonAdvPreReset(device, pPresentationParameters);
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->timeAdvPreReset.EndMeasurement();
                        this->timeOverall.EndMeasurement();
                    }
                });
            }

            void NativeAddonImpl::AdvPostReset(IDirect3DDevice9* device, D3DPRESENT_PARAMETERS* pPresentationParameters) {
                if (!this->AddonAdvPostReset) {
                    return;
                }
                this->callAdvFunc("PostReset", [=]() {
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->timeOverall.StartMeasurement();
                        this->timeAdvPostReset.StartMeasurement();
                    }
                    this->AddonAdvPostReset(device, pPresentationParameters);
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->timeAdvPostReset.EndMeasurement();
                        this->timeOverall.EndMeasurement();
                    }
                });
            }

            void NativeAddonImpl::AdvPrePresent(IDirect3DDevice9* device, CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion) {
                if (!this->AddonAdvPrePresent) {
                    return;
                }
                this->callAdvFunc("PrePresent", [=]() {
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->timeOverall.StartMeasurement();
                        this->timeAdvPrePresent.StartMeasurement();
                    }
                    this->AddonAdvPrePresent(device, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->timeAdvPrePresent.EndMeasurement();
                        this->timeOverall.EndMeasurement();
                    }
                });
            }

            void NativeAddonImpl::AdvPostPresent(IDirect3DDevice9* device, CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion) {
                if (!this->AddonAdvPostPresent) {
                    return;
                }
                this->callAdvFunc("PostPresent", [=]() {
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->timeOverall.StartMeasurement();
                        this->timeAdvPostPresent.StartMeasurement();
                    }
                    this->AddonAdvPostPresent(device, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->timeAdvPostPresent.EndMeasurement();
                        this->timeOverall.EndMeasurement();
                    }
                });
            }

            HRESULT NativeAddonImpl::AdvPreCreateTexture(IDirect3DDevice9* device, UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DTexture9** ppTexture, HANDLE* pSharedHandle) {
                if (!this->AddonAdvPreCreateTexture) {
                    return D3D_OK;
                }
                return this->callAdvFuncWithResult("PreCreateTexture", [=]() {
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->timeOverall.StartMeasurement();
                        this->timeAdvPreCreateTexture.StartMeasurement();
                    }
                    return this->AddonAdvPreCreateTexture(device, Width, Height, Levels, Usage, Format, Pool, ppTexture, pSharedHandle);
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->timeAdvPreCreateTexture.EndMeasurement();
                        this->timeOverall.EndMeasurement();
                    }
                });
            }

            void NativeAddonImpl::AdvPostCreateTexture(IDirect3DDevice9* device, IDirect3DTexture9* pTexture, UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, HANDLE* pSharedHandle) {
                if (!this->AddonAdvPostCreateTexture) {
                    return;
                }
                this->callAdvFunc("PostCreateTexture", [=]() {
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->timeOverall.StartMeasurement();
                        this->timeAdvPostCreateTexture.StartMeasurement();
                    }
                    this->AddonAdvPostCreateTexture(device, pTexture, Width, Height, Levels, Usage, Format, Pool, pSharedHandle);
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->timeAdvPostCreateTexture.EndMeasurement();
                        this->timeOverall.EndMeasurement();
                    }
                });
            }

            HRESULT NativeAddonImpl::AdvPreCreateVertexShader(IDirect3DDevice9* device, CONST DWORD* pFunction, IDirect3DVertexShader9** ppShader) {
                if (!this->AddonAdvPreCreateVertexShader) {
                    return D3D_OK;
                }
                return this->callAdvFuncWithResult("PreCreateVertexShader", [=]() {
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->timeOverall.StartMeasurement();
                        this->timeAdvPreCreateVertexShader.StartMeasurement();
                    }
                    return this->AddonAdvPreCreateVertexShader(device, pFunction, ppShader);
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->timeAdvPreCreateVertexShader.EndMeasurement();
                        this->timeOverall.EndMeasurement();
                    }
                });
            }

            void NativeAddonImpl::AdvPostCreateVertexShader(IDirect3DDevice9* device, IDirect3DVertexShader9* pShader, CONST DWORD* pFunction) {
                if (!this->AddonAdvPostCreateVertexShader) {
                    return;
                }
                this->callAdvFunc("PostCreateVertexShader", [=]() {
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->timeOverall.StartMeasurement();
                        this->timeAdvPostCreateVertexShader.StartMeasurement();
                    }
                    this->AddonAdvPostCreateVertexShader(device, pShader, pFunction);
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->timeAdvPostCreateVertexShader.EndMeasurement();
                        this->timeOverall.EndMeasurement();
                    }
                });
            }

            HRESULT NativeAddonImpl::AdvPreCreatePixelShader(IDirect3DDevice9* device, CONST DWORD* pFunction, IDirect3DPixelShader9** ppShader) {
                if (!this->AddonAdvPreCreatePixelShader) {
                    return D3D_OK;
                }
                return this->callAdvFuncWithResult("PreCreatePixelShader", [=]() {
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->timeOverall.StartMeasurement();
                        this->timeAdvPreCreatePixelShader.StartMeasurement();
                    }
                    return this->AddonAdvPreCreatePixelShader(device, pFunction, ppShader);
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->timeAdvPreCreatePixelShader.EndMeasurement();
                        this->timeOverall.EndMeasurement();
                    }
                });
            }

            void NativeAddonImpl::AdvPostCreatePixelShader(IDirect3DDevice9* device, IDirect3DPixelShader9* pShader, CONST DWORD* pFunction) {
                if (!this->AddonAdvPostCreatePixelShader) {
                    return;
                }
                this->callAdvFunc("PostCreatePixelShader", [=]() {
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->timeOverall.StartMeasurement();
                        this->timeAdvPostCreatePixelShader.StartMeasurement();
                    }
                    this->AddonAdvPostCreatePixelShader(device, pShader, pFunction);
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->timeAdvPostCreatePixelShader.EndMeasurement();
                        this->timeOverall.EndMeasurement();
                    }
                });
            }

            HRESULT NativeAddonImpl::AdvPreCreateRenderTarget(IDirect3DDevice9* device, UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Lockable, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle) {
                if (!this->AddonAdvPreCreateRenderTarget) {
                    return D3D_OK;
                }
                return this->callAdvFuncWithResult("PreCreateRenderTarget", [=]() {
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->timeOverall.StartMeasurement();
                        this->timeAdvPreCreateRenderTarget.StartMeasurement();
                    }
                    return this->AddonAdvPreCreateRenderTarget(device, Width, Height, Format, MultiSample, MultisampleQuality, Lockable, ppSurface, pSharedHandle);
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->timeAdvPreCreateRenderTarget.EndMeasurement();
                        this->timeOverall.EndMeasurement();
                    }
                });
            }

            void NativeAddonImpl::AdvPostCreateRenderTarget(IDirect3DDevice9* device, IDirect3DSurface9* pSurface, UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Lockable, HANDLE* pSharedHandle) {
                if (!this->AddonAdvPostCreateRenderTarget) {
                    return;
                }
                this->callAdvFunc("PostCreateRenderTarget", [=]() {
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->timeOverall.StartMeasurement();
                        this->timeAdvPostCreateRenderTarget.StartMeasurement();
                    }
                    this->AddonAdvPostCreateRenderTarget(device, pSurface, Width, Height, Format, MultiSample, MultisampleQuality, Lockable, pSharedHandle);
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->timeAdvPostCreateRenderTarget.EndMeasurement();
                        this->timeOverall.EndMeasurement();
                    }
                });
            }

            void NativeAddonImpl::AdvPreSetTexture(IDirect3DDevice9* device, DWORD Stage, IDirect3DBaseTexture9* pTexture) {
                if (!this->AddonAdvPreSetTexture) {
                    return;
                }
                this->callAdvFunc("PreSetTexture", [=]() {
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->timeOverall.StartMeasurement();
                        this->timeAdvPreSetTexture.StartMeasurement();
                    }
                    this->AddonAdvPreSetTexture(device, Stage, pTexture);
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->timeAdvPreSetTexture.EndMeasurement();
                        this->timeOverall.EndMeasurement();
                    }
                });
            }

            void NativeAddonImpl::AdvPostSetTexture(IDirect3DDevice9* device, DWORD Stage, IDirect3DBaseTexture9* pTexture) {
                if (!this->AddonAdvPostSetTexture) {
                    return;
                }
                this->callAdvFunc("PostSetTexture", [=]() {
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->timeOverall.StartMeasurement();
                        this->timeAdvPostSetTexture.StartMeasurement();
                    }
                    this->AddonAdvPostSetTexture(device, Stage, pTexture);
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->timeAdvPostSetTexture.EndMeasurement();
                        this->timeOverall.EndMeasurement();
                    }
                });
            }

            void NativeAddonImpl::AdvPreSetVertexShader(IDirect3DDevice9* device, IDirect3DVertexShader9* pShader) {
                if (!this->AddonAdvPreSetVertexShader) {
                    return;
                }
                this->callAdvFunc("PreSetVertexShader", [=]() {
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->timeOverall.StartMeasurement();
                        this->timeAdvPreSetVertexShader.StartMeasurement();
                    }
                    this->AddonAdvPreSetVertexShader(device, pShader);
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->timeAdvPreSetVertexShader.EndMeasurement();
                        this->timeOverall.EndMeasurement();
                    }
                });
            }

            void NativeAddonImpl::AdvPostSetVertexShader(IDirect3DDevice9* device, IDirect3DVertexShader9* pShader) {
                if (!this->AddonAdvPostSetVertexShader) {
                    return;
                }
                this->callAdvFunc("PostSetVertexShader", [=]() {
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->timeOverall.StartMeasurement();
                        this->timeAdvPostSetVertexShader.StartMeasurement();
                    }
                    this->AddonAdvPostSetVertexShader(device, pShader);
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->timeAdvPostSetVertexShader.EndMeasurement();
                        this->timeOverall.EndMeasurement();
                    }
                });
            }

            void NativeAddonImpl::AdvPreSetPixelShader(IDirect3DDevice9* device, IDirect3DPixelShader9* pShader) {
                if (!this->AddonAdvPreSetPixelShader) {
                    return;
                }
                this->callAdvFunc("PreSetPixelShader", [=]() {
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->timeOverall.StartMeasurement();
                        this->timeAdvPreSetPixelShader.StartMeasurement();
                    }
                    this->AddonAdvPreSetPixelShader(device, pShader);
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->timeAdvPreSetPixelShader.EndMeasurement();
                        this->timeOverall.EndMeasurement();
                    }
                });
            }

            void NativeAddonImpl::AdvPostSetPixelShader(IDirect3DDevice9* device, IDirect3DPixelShader9* pShader) {
                if (!this->AddonAdvPostSetPixelShader) {
                    return;
                }
                this->callAdvFunc("PostSetPixelShader", [=]() {
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->timeOverall.StartMeasurement();
                        this->timeAdvPostSetPixelShader.StartMeasurement();
                    }
                    this->AddonAdvPostSetPixelShader(device, pShader);
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->timeAdvPostSetPixelShader.EndMeasurement();
                        this->timeOverall.EndMeasurement();
                    }
                });
            }

            void NativeAddonImpl::AdvPreSetRenderTarget(IDirect3DDevice9* device, DWORD RenderTargetIndex, IDirect3DSurface9* pRenderTarget) {
                if (!this->AddonAdvPreSetRenderTarget) {
                    return;
                }
                this->callAdvFunc("PreSetRenderTarget", [=]() {
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->timeOverall.StartMeasurement();
                        this->timeAdvPreSetRenderTarget.StartMeasurement();
                    }
                    this->AddonAdvPreSetRenderTarget(device, RenderTargetIndex, pRenderTarget);
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->timeAdvPreSetRenderTarget.EndMeasurement();
                        this->timeOverall.EndMeasurement();
                    }
                });
            }

            void NativeAddonImpl::AdvPostSetRenderTarget(IDirect3DDevice9* device, DWORD RenderTargetIndex, IDirect3DSurface9* pRenderTarget) {
                if (!this->AddonAdvPostSetRenderTarget) {
                    return;
                }
                this->callAdvFunc("PostSetRenderTarget", [=]() {
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->timeOverall.StartMeasurement();
                        this->timeAdvPostSetRenderTarget.StartMeasurement();
                    }
                    this->AddonAdvPostSetRenderTarget(device, RenderTargetIndex, pRenderTarget);
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->timeAdvPostSetRenderTarget.EndMeasurement();
                        this->timeOverall.EndMeasurement();
                    }
                });
            }

            void NativeAddonImpl::AdvPreSetRenderState(IDirect3DDevice9* device, D3DRENDERSTATETYPE State, DWORD Value) {
                if (!this->AddonAdvPreSetRenderState) {
                    return;
                }
                this->callAdvFunc("PreSetRenderState", [=]() {
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->timeOverall.StartMeasurement();
                        this->timeAdvPreSetRenderState.StartMeasurement();
                    }
                    this->AddonAdvPreSetRenderState(device, State, Value);
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->timeAdvPreSetRenderState.EndMeasurement();
                        this->timeOverall.EndMeasurement();
                    }
                });
            }

            void NativeAddonImpl::AdvPostSetRenderState(IDirect3DDevice9* device, D3DRENDERSTATETYPE State, DWORD Value) {
                if (!this->AddonAdvPostSetRenderState) {
                    return;
                }
                this->callAdvFunc("PostSetRenderState", [=]() {
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->timeOverall.StartMeasurement();
                        this->timeAdvPostSetRenderState.StartMeasurement();
                    }
                    this->AddonAdvPostSetRenderState(device, State, Value);
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->timeAdvPostSetRenderState.EndMeasurement();
                        this->timeOverall.EndMeasurement();
                    }
                });
            }

            void NativeAddonImpl::AdvPreDrawIndexedPrimitive(IDirect3DDevice9* device, D3DPRIMITIVETYPE PrimitiveType, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount) {
                if (!this->AddonAdvPreDrawIndexedPrimitive) {
                    return;
                }
                this->callAdvFunc("PreDrawIndexedPrimitive", [=]() {
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->timeOverall.StartMeasurement();
                        this->timeAdvPreDrawIndexedPrimitive.StartMeasurement();
                    }
                    this->AddonAdvPreDrawIndexedPrimitive(device, PrimitiveType, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount);
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->timeAdvPreDrawIndexedPrimitive.EndMeasurement();
                        this->timeOverall.EndMeasurement();
                    }
                });
            }

            void NativeAddonImpl::AdvPostDrawIndexedPrimitive(IDirect3DDevice9* device, D3DPRIMITIVETYPE PrimitiveType, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount) {
                if (!this->AddonAdvPostDrawIndexedPrimitive) {
                    return;
                }
                this->callAdvFunc("PostDrawIndexedPrimitive", [=]() {
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->timeOverall.StartMeasurement();
                        this->timeAdvPostDrawIndexedPrimitive.StartMeasurement();
                    }
                    this->AddonAdvPostDrawIndexedPrimitive(device, PrimitiveType, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount);
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->timeAdvPostDrawIndexedPrimitive.EndMeasurement();
                        this->timeOverall.EndMeasurement();
                    }
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
