#pragma once
#include "../windows.h"
#include <d3dx9tex.h>
#include <chrono>
#include <string>
#include <gw2addon-native.h>
#include "Addon.h"
#include "exceptions.h"
#include "../Config.h"
#include "../log.h"
#include "../utils/encoding.h"

namespace loader {
    namespace addons {

        template<class T>
        class NativeAddon : public T {
        public:
            NativeAddon() = delete;
            NativeAddon(const T& addon) : addon(addon) { }

            virtual bool Initialize() override {
                using namespace std;
                using namespace loader::utils;

                HMODULE h = LoadLibrary(this->GetFilePath().c_str());
                if (h == NULL) {
                    this->ChangeState(AddonState::ErroredState);
                    GetLog()->error("Could not initialize native addon {0}: Libray handle is empty", this->GetFileName());
                    return false;
                }
                this->addonHandle = h;

                this->AddonInitialize = reinterpret_cast<GW2AddonInitialize_t>(GetProcAddress(h, GW2ADDON_DLL_Initialize));
                this->AddonRelease = reinterpret_cast<GW2AddonRelease_t>(GetProcAddress(h, GW2ADDON_DLL_Release));

                if (this->AddonInitialize == nullptr) {
                    this->ChangeState(AddonState::ErroredState);
                    GetLog()->error("Could not initialize native addon {0}: Addon doesn't contain a valid " GW2ADDON_DLL_Initialize " export", this->GetFileName());
                    return false;
                }
                if (this->AddonRelease == nullptr) {
                    this->ChangeState(AddonState::ErroredState);
                    GetLog()->error("Could not initialize native addon {0}: Addon doesn't contain a valid " GW2ADDON_DLL_Release " export", this->GetFileName());
                    return false;
                }

                GW2AddonAPIBase* addonBase = this->AddonInitialize(GW2ADDON_VER);
                if (addonBase == nullptr) {
                    this->ChangeState(AddonState::ErroredState);
                    GetLog()->error("Could not initialize native addon {0}: Addon didn't return a valid GW2AddonAPIBase pointer when calling " GW2ADDON_DLL_Initialize, this->GetFileName());
                    return false;
                }

                if (addonBase->ver == 1) {
                    if (!this->InitializeV1(addonBase)) {
                        return false;
                    }
                }
                else {
                    this->ChangeState(AddonState::ErroredState);
                    GetLog()->error(("Could not initialize native addon {0}: Addon uses a version (" + to_string(addonBase->ver) + ") that is not compatible. Are both the addon and the addon loader updated?").c_str(), this->GetFileName());
                    return false;
                }

                return this->addon.Initialize();
            }

            virtual bool Uninitialize() override {
                this->Unload();

                this->AddonLoad = nullptr;
                this->AddonDrawFrame = nullptr;
                this->AddonDrawFrameBeforeGui = nullptr;
                this->AddonDrawFrameBeforePostProcessing = nullptr;
                this->AddonHandleWndProc = nullptr;
                this->AddonOpenSettings = nullptr;
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

                if (this->iconManaged) {
                    this->icon->Release();
                    this->icon = nullptr;
                    this->iconManaged = false;
                }

                if (this->addonHandle) {
                    FreeLibrary(this->addonHandle);
                    this->addonHandle = NULL;
                }

                return this->addon.Uninitialize();
            }

            virtual bool Load() override {
                if (this->GetState() != AddonState::UnloadedState) {
                    return false;
                }

                this->ChangeState(AddonState::LoadingState);

                if (this->AddonLoad != nullptr) {
                    this->GetLoadDuration().Start();
                    GW2ADDON_RESULT result = this->AddonLoad(this->GetFocusWindow(), this->GetD3DDevice9());
                    this->GetLoadDuration().End();

                    if (result) {
                        this->ChangeState(AddonState::ErroredState);
                        GetLog()->error("Could not load native addon {0}: Addon returned {1}", this->GetFileName(), to_string(result));
                        return false;
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

                this->ChangeState(AddonState::LoadedState);

                return this->addon.Load();
            }

            virtual bool Unload() override {
                if (this->GetState() != AddonState::LoadedState) {
                    return false;
                }

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

                this->ChangeState(AddonState::UnloadedState);

                return this->addon.Unload();
            }


            virtual bool SupportsLoading() const override {
                return this->AddonInitialize != nullptr && this->addon.SupportsLoading();
            }

            virtual bool SupportsSettings() const override {
                return this->AddonOpenSettings != nullptr && this->addon.SupportsSettings();
            }


            virtual AddonType GetType() const override {
                return AddonType::AddonTypeNative;
            }


            virtual const std::string GetID() const override {
                return this->id;
            }

            virtual const std::string GetName() const override {
                return this->name;
            }

            virtual const std::string GetAuthor() const override {
                return this->author;
            }

            virtual const std::string GetDescription() const override {
                return this->description;
            }

            virtual const std::string GetVersion() const override {
                return this->version;
            }

            virtual const std::string GetHomepage() const override {
                return this->homepage;
            }

            virtual IDirect3DTexture9* GetIcon() const override {
                return this->icon;
            }


            virtual void OpenSettings() override {
                if (this->AddonOpenSettings != nullptr) {
                    this->AddonOpenSettings();
                }
            }


            virtual bool HandleWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) override {
                if (this->AddonHandleWndProc == nullptr) {
                    return false;
                }

                bool result = this->callFunc<bool>("HandleWndProc", [=]() {
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->GetTimeWndProc().StartFrame();
                        this->GetTimeWndProc().StartMeasurement();
                    }
                    bool result = this->AddonHandleWndProc(hWnd, msg, wParam, lParam);
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->GetTimeWndProc().EndMeasurement();
                        this->GetTimeWndProc().EndFrame();
                    }
                    return result;
                });
                
                if (result) {
                    return result;
                }
                return this->addon.HandleWndProc(hWnd, msg, wParam, lParam);
            }

            virtual void DrawFrameBeforeGui(IDirect3DDevice9* device) override {
                if (!this->AddonDrawFrameBeforeGui) {
                    return;
                }

                this->callFunc<void>("DrawFrameBeforeGui", [=]() {
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->GetTimeOverall().StartMeasurement();
                        this->GetTimeDrawFrameBeforeGui().StartMeasurement();
                    }
                    this->AddonDrawFrameBeforeGui(device);
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->GetTimeDrawFrameBeforeGui().EndMeasurement();
                        this->GetTimeOverall().EndMeasurement();
                    }
                });

                return this->addon.DrawFrameBeforeGui(device);
            }

            virtual void DrawFrameBeforePostProcessing(IDirect3DDevice9* device) override {
                if (!this->AddonDrawFrameBeforePostProcessing) {
                    return;
                }

                this->callFunc<void>("DrawFrameBeforePostProcessing", [=]() {
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->GetTimeOverall().StartMeasurement();
                        this->GetTimeDrawFrameBeforePostProcessing().StartMeasurement();
                    }
                    this->AddonDrawFrameBeforePostProcessing(device);
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->GetTimeDrawFrameBeforePostProcessing().EndMeasurement();
                        this->GetTimeOverall().EndMeasurement();
                    }
                });
           
                return this->addon.DrawFrameBeforePostProcessing(device);
            }

            virtual void DrawFrame(IDirect3DDevice9* device) override {
                if (!this->AddonDrawFrame) {
                    return;
                }

                this->callFunc<void>("DrawFrame", [=]() {
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->GetTimeOverall().StartMeasurement();
                        this->GetTimeDrawFrame().StartMeasurement();
                    }
                    this->AddonDrawFrame(device);
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->GetTimeDrawFrame().EndMeasurement();
                        this->GetTimeOverall().EndMeasurement();
                    }
                });

                return this->addon.DrawFrame(device);
            }


            virtual void AdvPreBeginScene(IDirect3DDevice9* device) {
                if (!this->AddonAdvPreBeginScene) {
                    return;
                }

                this->callFunc<void>("PreBeginScene", [=]() {
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->GetTimeOverall().StartMeasurement();
                        this->GetTimeAdvPreBeginScene().StartMeasurement();
                    }
                    this->AddonAdvPreBeginScene(device);
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->GetTimeAdvPreBeginScene().EndMeasurement();
                        this->GetTimeOverall().EndMeasurement();
                    }
                });

                return this->addon.AdvPreBeginScene(device);
            }

            virtual void AdvPostBeginScene(IDirect3DDevice9* device) {
                if (!this->AddonAdvPostBeginScene) {
                    return;
                }

                this->callFunc<void>("PostBeginScene", [=]() {
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->GetTimeOverall().StartMeasurement();
                        this->GetTimeAdvPostBeginScene().StartMeasurement();
                    }
                    this->AddonAdvPostBeginScene(device);
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->GetTimeAdvPostBeginScene().EndMeasurement();
                        this->GetTimeOverall().EndMeasurement();
                    }
                });

                return this->addon.AdvPostBeginScene(device);
            }

            virtual void AdvPreEndScene(IDirect3DDevice9* device) {
                if (!this->AddonAdvPreEndScene) {
                    return;
                }

                this->callFunc<void>("PreEndScene", [=]() {
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->GetTimeOverall().StartMeasurement();
                        this->GetTimeAdvPreEndScene().StartMeasurement();
                    }
                    this->AddonAdvPreEndScene(device);
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->GetTimeAdvPreEndScene().EndMeasurement();
                        this->GetTimeOverall().EndMeasurement();
                    }
                });

                return this->addon.AdvPreEndScene(device);
            }

            virtual void AdvPostEndScene(IDirect3DDevice9* device) {
                if (!this->AddonAdvPostEndScene) {
                    return;
                }

                this->callFunc<void>("PostEndScene", [=]() {
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->GetTimeOverall().StartMeasurement();
                        this->GetTimeAdvPostEndScene().StartMeasurement();
                    }
                    this->AddonAdvPostEndScene(device);
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->GetTimeAdvPostEndScene().EndMeasurement();
                        this->GetTimeOverall().EndMeasurement();
                    }
                });

                return this->addon.AdvPostEndScene(device);
            }

            virtual void AdvPreClear(IDirect3DDevice9* device, DWORD Count, CONST D3DRECT* pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil) {
                if (!this->AddonAdvPreClear) {
                    return;
                }

                this->callFunc<void>("PreClear", [=]() {
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->GetTimeOverall().StartMeasurement();
                        this->GetTimeAdvPreClear().StartMeasurement();
                    }
                    this->AddonAdvPreClear(device, Count, pRects, Flags, Color, Z, Stencil);
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->GetTimeAdvPreClear().EndMeasurement();
                        this->GetTimeOverall().EndMeasurement();
                    }
                });

                return this->addon.AdvPreClear(device, Count, pRects, Flags, Color, Z, Stencil);
            }

            virtual void AdvPostClear(IDirect3DDevice9* device, DWORD Count, CONST D3DRECT* pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil) {
                if (!this->AddonAdvPostClear) {
                    return;
                }

                this->callFunc<void>("PostClear", [=]() {
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->GetTimeOverall().StartMeasurement();
                        this->GetTimeAdvPostClear().StartMeasurement();
                    }
                    this->AddonAdvPostClear(device, Count, pRects, Flags, Color, Z, Stencil);
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->GetTimeAdvPostClear().EndMeasurement();
                        this->GetTimeOverall().EndMeasurement();
                    }
                });

                return this->addon.AdvPostClear(device, Count, pRects, Flags, Color, Z, Stencil);
            }

            virtual void AdvPreReset(IDirect3DDevice9* device, D3DPRESENT_PARAMETERS* pPresentationParameters) {
                if (!this->AddonAdvPreReset) {
                    return;
                }

                this->callFunc<void>("PreReset", [=]() {
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->GetTimeOverall().StartMeasurement();
                        this->GetTimeAdvPreReset().StartMeasurement();
                    }
                    this->AddonAdvPreReset(device, pPresentationParameters);
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->GetTimeAdvPreReset().EndMeasurement();
                        this->GetTimeOverall().EndMeasurement();
                    }
                });

                return this->addon.AdvPreReset(device, pPresentationParameters);
            }

            virtual void AdvPostReset(IDirect3DDevice9* device, D3DPRESENT_PARAMETERS* pPresentationParameters) {
                if (!this->AddonAdvPostReset) {
                    return;
                }

                this->callFunc<void>("PostReset", [=]() {
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->GetTimeOverall().StartMeasurement();
                        this->GetTimeAdvPostReset().StartMeasurement();
                    }
                    this->AddonAdvPostReset(device, pPresentationParameters);
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->GetTimeAdvPostReset().EndMeasurement();
                        this->GetTimeOverall().EndMeasurement();
                    }
                });

                return this->addon.AdvPostReset(device, pPresentationParameters);
            }

            virtual void AdvPrePresent(IDirect3DDevice9* device, CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion) {
                if (!this->AddonAdvPrePresent) {
                    return;
                }

                this->callFunc<void>("PrePresent", [=]() {
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->GetTimeOverall().StartMeasurement();
                        this->GetTimeAdvPrePresent().StartMeasurement();
                    }
                    this->AddonAdvPrePresent(device, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->GetTimeAdvPrePresent().EndMeasurement();
                        this->GetTimeOverall().EndMeasurement();
                    }
                });

                return this->addon.AdvPrePresent(device, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
            }

            virtual void AdvPostPresent(IDirect3DDevice9* device, CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion) {
                if (!this->AddonAdvPostPresent) {
                    return;
                }

                this->callFunc<void>("PostPresent", [=]() {
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->GetTimeOverall().StartMeasurement();
                        this->GetTimeAdvPostPresent().StartMeasurement();
                    }
                    this->AddonAdvPostPresent(device, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->GetTimeAdvPostPresent().EndMeasurement();
                        this->GetTimeOverall().EndMeasurement();
                    }
                });

                return this->addon.AdvPostPresent(device, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
            }

            virtual HRESULT AdvPreCreateTexture(IDirect3DDevice9* device, UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DTexture9** ppTexture, HANDLE* pSharedHandle) {
                if (!this->AddonAdvPreCreateTexture) {
                    return D3D_OK;
                }

                HRESULT result = this->callFunc<HRESULT>("PreCreateTexture", [=]() {
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->GetTimeOverall().StartMeasurement();
                        this->GetTimeAdvPreCreateTexture().StartMeasurement();
                    }
                    HRESULT result = this->AddonAdvPreCreateTexture(device, Width, Height, Levels, Usage, Format, Pool, ppTexture, pSharedHandle);
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->GetTimeAdvPreCreateTexture().EndMeasurement();
                        this->GetTimeOverall().EndMeasurement();
                    }
                    return result;
                });

                if (result != D3D_OK) {
                    return this->addon.AdvPreCreateTexture(device, Width, Height, Levels, Usage, Format, Pool, ppTexture, pSharedHandle);
                }
                return result;
            }

            virtual void AdvPostCreateTexture(IDirect3DDevice9* device, IDirect3DTexture9* pTexture, UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, HANDLE* pSharedHandle) {
                if (!this->AddonAdvPostCreateTexture) {
                    return;
                }

                this->callFunc<void>("PostCreateTexture", [=]() {
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->GetTimeOverall().StartMeasurement();
                        this->GetTimeAdvPostCreateTexture().StartMeasurement();
                    }
                    this->AddonAdvPostCreateTexture(device, pTexture, Width, Height, Levels, Usage, Format, Pool, pSharedHandle);
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->GetTimeAdvPostCreateTexture().EndMeasurement();
                        this->GetTimeOverall().EndMeasurement();
                    }
                });

                return this->addon.AdvPostCreateTexture(device, pTexture, Width, Height, Levels, Usage, Format, Pool, pSharedHandle);
            }

            virtual HRESULT AdvPreCreateVertexShader(IDirect3DDevice9* device, CONST DWORD* pFunction, IDirect3DVertexShader9** ppShader) {
                if (!this->AddonAdvPreCreateVertexShader) {
                    return D3D_OK;
                }

                HRESULT result = this->callFunc<HRESULT>("PreCreateVertexShader", [=]() {
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->GetTimeOverall().StartMeasurement();
                        this->GetTimeAdvPreCreateVertexShader().StartMeasurement();
                    }
                    HRESULT result = this->AddonAdvPreCreateVertexShader(device, pFunction, ppShader);
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->GetTimeAdvPreCreateVertexShader().EndMeasurement();
                        this->GetTimeOverall().EndMeasurement();
                    }
                    return result;
                });

                if (result != D3D_OK) {
                    return this->addon.AdvPreCreateVertexShader(device, pFunction, ppShader);
                }
                return result;
            }

            virtual void AdvPostCreateVertexShader(IDirect3DDevice9* device, IDirect3DVertexShader9* pShader, CONST DWORD* pFunction) {
                if (!this->AddonAdvPostCreateVertexShader) {
                    return;
                }

                this->callFunc<void>("PostCreateVertexShader", [=]() {
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->GetTimeOverall().StartMeasurement();
                        this->GetTimeAdvPostCreateVertexShader().StartMeasurement();
                    }
                    this->AddonAdvPostCreateVertexShader(device, pShader, pFunction);
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->GetTimeAdvPostCreateVertexShader().EndMeasurement();
                        this->GetTimeOverall().EndMeasurement();
                    }
                });

                return this->addon.AdvPostCreateVertexShader(device, pShader, pFunction);
            }

            virtual HRESULT AdvPreCreatePixelShader(IDirect3DDevice9* device, CONST DWORD* pFunction, IDirect3DPixelShader9** ppShader) {
                if (!this->AddonAdvPreCreatePixelShader) {
                    return D3D_OK;
                }

                HRESULT result = this->callFunc<HRESULT>("PreCreatePixelShader", [=]() {
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->GetTimeOverall().StartMeasurement();
                        this->GetTimeAdvPreCreatePixelShader().StartMeasurement();
                    }
                    HRESULT result = this->AddonAdvPreCreatePixelShader(device, pFunction, ppShader);
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->GetTimeAdvPreCreatePixelShader().EndMeasurement();
                        this->GetTimeOverall().EndMeasurement();
                    }
                    return result;
                });

                if (result != D3D_OK) {
                    return this->addon.AdvPreCreatePixelShader(device, pFunction, ppShader);
                }
                return result;
            }

            virtual void AdvPostCreatePixelShader(IDirect3DDevice9* device, IDirect3DPixelShader9* pShader, CONST DWORD* pFunction) {
                if (!this->AddonAdvPostCreatePixelShader) {
                    return;
                }

                this->callFunc<void>("PostCreatePixelShader", [=]() {
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->GetTimeOverall().StartMeasurement();
                        this->GetTimeAdvPostCreatePixelShader().StartMeasurement();
                    }
                    this->AddonAdvPostCreatePixelShader(device, pShader, pFunction);
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->GetTimeAdvPostCreatePixelShader().EndMeasurement();
                        this->GetTimeOverall().EndMeasurement();
                    }
                });

                return this->addon.AdvPostCreatePixelShader(device, pShader, pFunction);
            }

            virtual HRESULT AdvPreCreateRenderTarget(IDirect3DDevice9* device, UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Lockable, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle) {
                if (!this->AddonAdvPreCreateRenderTarget) {
                    return D3D_OK;
                }

                HRESULT result = this->callFunc<HRESULT>("PreCreateRenderTarget", [=]() {
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->GetTimeOverall().StartMeasurement();
                        this->GetTimeAdvPreCreateRenderTarget().StartMeasurement();
                    }
                    HRESULT result = this->AddonAdvPreCreateRenderTarget(device, Width, Height, Format, MultiSample, MultisampleQuality, Lockable, ppSurface, pSharedHandle);
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->GetTimeAdvPreCreateRenderTarget().EndMeasurement();
                        this->GetTimeOverall().EndMeasurement();
                    }
                    return result;
                });

                if (result != D3D_OK) {
                    return this->addon.AdvPreCreateRenderTarget(device, Width, Height, Format, MultiSample, MultisampleQuality, Lockable, ppSurface, pSharedHandle);
                }
                return result;
            }

            virtual void AdvPostCreateRenderTarget(IDirect3DDevice9* device, IDirect3DSurface9* pSurface, UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Lockable, HANDLE* pSharedHandle) {
                if (!this->AddonAdvPostCreateRenderTarget) {
                    return;
                }

                this->callFunc<void>("PostCreateRenderTarget", [=]() {
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->GetTimeOverall().StartMeasurement();
                        this->GetTimeAdvPostCreateRenderTarget().StartMeasurement();
                    }
                    this->AddonAdvPostCreateRenderTarget(device, pSurface, Width, Height, Format, MultiSample, MultisampleQuality, Lockable, pSharedHandle);
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->GetTimeAdvPostCreateRenderTarget().EndMeasurement();
                        this->GetTimeOverall().EndMeasurement();
                    }
                });

                return this->addon.AdvPostCreateRenderTarget(device, pSurface, Width, Height, Format, MultiSample, MultisampleQuality, Lockable, pSharedHandle);
            }

            virtual void AdvPreSetTexture(IDirect3DDevice9* device, DWORD Stage, IDirect3DBaseTexture9* pTexture) {
                if (!this->AddonAdvPreSetTexture) {
                    return;
                }

                this->callFunc<void>("PreSetTexture", [=]() {
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->GetTimeOverall().StartMeasurement();
                        this->GetTimeAdvPreSetTexture().StartMeasurement();
                    }
                    this->AddonAdvPreSetTexture(device, Stage, pTexture);
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->GetTimeAdvPreSetTexture().EndMeasurement();
                        this->GetTimeOverall().EndMeasurement();
                    }
                });

                return this->addon.AdvPreSetTexture(device, Stage, pTexture);
            }

            virtual void AdvPostSetTexture(IDirect3DDevice9* device, DWORD Stage, IDirect3DBaseTexture9* pTexture) {
                if (!this->AddonAdvPostSetTexture) {
                    return;
                }

                this->callFunc<void>("PostSetTexture", [=]() {
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->GetTimeOverall().StartMeasurement();
                        this->GetTimeAdvPostSetTexture().StartMeasurement();
                    }
                    this->AddonAdvPostSetTexture(device, Stage, pTexture);
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->GetTimeAdvPostSetTexture().EndMeasurement();
                        this->GetTimeOverall().EndMeasurement();
                    }
                });

                return this->addon.AdvPostSetTexture(device, Stage, pTexture);
            }

            virtual void AdvPreSetVertexShader(IDirect3DDevice9* device, IDirect3DVertexShader9* pShader) {
                if (!this->AddonAdvPreSetVertexShader) {
                    return;
                }

                this->callFunc<void>("PreSetVertexShader", [=]() {
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->GetTimeOverall().StartMeasurement();
                        this->GetTimeAdvPreSetVertexShader().StartMeasurement();
                    }
                    this->AddonAdvPreSetVertexShader(device, pShader);
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->GetTimeAdvPreSetVertexShader().EndMeasurement();
                        this->GetTimeOverall().EndMeasurement();
                    }
                });

                return this->addon.AdvPreSetVertexShader(device, pShader);
            }

            virtual void AdvPostSetVertexShader(IDirect3DDevice9* device, IDirect3DVertexShader9* pShader) {
                if (!this->AddonAdvPostSetVertexShader) {
                    return;
                }

                this->callFunc<void>("PostSetVertexShader", [=]() {
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->GetTimeOverall().StartMeasurement();
                        this->GetTimeAdvPostSetVertexShader().StartMeasurement();
                    }
                    this->AddonAdvPostSetVertexShader(device, pShader);
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->GetTimeAdvPostSetVertexShader().EndMeasurement();
                        this->GetTimeOverall().EndMeasurement();
                    }
                });

                return this->addon.AdvPostSetVertexShader(device, pShader);
            }

            virtual void AdvPreSetPixelShader(IDirect3DDevice9* device, IDirect3DPixelShader9* pShader) {
                if (!this->AddonAdvPreSetPixelShader) {
                    return;
                }

                this->callFunc<void>("PreSetPixelShader", [=]() {
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->GetTimeOverall().StartMeasurement();
                        this->GetTimeAdvPreSetPixelShader().StartMeasurement();
                    }
                    this->AddonAdvPreSetPixelShader(device, pShader);
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->GetTimeAdvPreSetPixelShader().EndMeasurement();
                        this->GetTimeOverall().EndMeasurement();
                    }
                });

                return this->addon.AdvPreSetPixelShader(device, pShader);
            }

            virtual void AdvPostSetPixelShader(IDirect3DDevice9* device, IDirect3DPixelShader9* pShader) {
                if (!this->AddonAdvPostSetPixelShader) {
                    return;
                }

                this->callFunc<void>("PostSetPixelShader", [=]() {
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->GetTimeOverall().StartMeasurement();
                        this->GetTimeAdvPostSetPixelShader().StartMeasurement();
                    }
                    this->AddonAdvPostSetPixelShader(device, pShader);
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->GetTimeAdvPostSetPixelShader().EndMeasurement();
                        this->GetTimeOverall().EndMeasurement();
                    }
                });

                return this->addon.AdvPostSetPixelShader(device, pShader);
            }

            virtual void AdvPreSetRenderTarget(IDirect3DDevice9* device, DWORD RenderTargetIndex, IDirect3DSurface9* pRenderTarget) {
                if (!this->AddonAdvPreSetRenderTarget) {
                    return;
                }

                this->callFunc<void>("PreSetRenderTarget", [=]() {
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->GetTimeOverall().StartMeasurement();
                        this->GetTimeAdvPreSetRenderTarget().StartMeasurement();
                    }
                    this->AddonAdvPreSetRenderTarget(device, RenderTargetIndex, pRenderTarget);
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->GetTimeAdvPreSetRenderTarget().EndMeasurement();
                        this->GetTimeOverall().EndMeasurement();
                    }
                });

                return this->addon.AdvPreSetRenderTarget(device, RenderTargetIndex, pRenderTarget);
            }

            virtual void AdvPostSetRenderTarget(IDirect3DDevice9* device, DWORD RenderTargetIndex, IDirect3DSurface9* pRenderTarget) {
                if (!this->AddonAdvPostSetRenderTarget) {
                    return;
                }

                this->callFunc<void>("PostSetRenderTarget", [=]() {
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->GetTimeOverall().StartMeasurement();
                        this->GetTimeAdvPostSetRenderTarget().StartMeasurement();
                    }
                    this->AddonAdvPostSetRenderTarget(device, RenderTargetIndex, pRenderTarget);
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->GetTimeAdvPostSetRenderTarget().EndMeasurement();
                        this->GetTimeOverall().EndMeasurement();
                    }
                });

                return this->addon.AdvPostSetRenderTarget(device, RenderTargetIndex, pRenderTarget);
            }

            virtual void AdvPreSetRenderState(IDirect3DDevice9* device, D3DRENDERSTATETYPE State, DWORD Value) {
                if (!this->AddonAdvPreSetRenderState) {
                    return;
                }

                this->callFunc<void>("PreSetRenderState", [=]() {
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->GetTimeOverall().StartMeasurement();
                        this->GetTimeAdvPreSetRenderState().StartMeasurement();
                    }
                    this->AddonAdvPreSetRenderState(device, State, Value);
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->GetTimeAdvPreSetRenderState().EndMeasurement();
                        this->GetTimeOverall().EndMeasurement();
                    }
                });

                return this->addon.AdvPreSetRenderState(device, State, Value);
            }

            virtual void AdvPostSetRenderState(IDirect3DDevice9* device, D3DRENDERSTATETYPE State, DWORD Value) {
                if (!this->AddonAdvPostSetRenderState) {
                    return;
                }

                this->callFunc<void>("PostSetRenderState", [=]() {
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->GetTimeOverall().StartMeasurement();
                        this->GetTimeAdvPostSetRenderState().StartMeasurement();
                    }
                    this->AddonAdvPostSetRenderState(device, State, Value);
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->GetTimeAdvPostSetRenderState().EndMeasurement();
                        this->GetTimeOverall().EndMeasurement();
                    }
                });

                return this->addon.AdvPostSetRenderState(device, State, Value);
            }

            virtual void AdvPreDrawIndexedPrimitive(IDirect3DDevice9* device, D3DPRIMITIVETYPE PrimitiveType, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount) {
                if (!this->AddonAdvPreDrawIndexedPrimitive) {
                    return;
                }

                this->callFunc<void>("PreDrawIndexedPrimitive", [=]() {
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->GetTimeOverall().StartMeasurement();
                        this->GetTimeAdvPreDrawIndexedPrimitive().StartMeasurement();
                    }
                    this->AddonAdvPreDrawIndexedPrimitive(device, PrimitiveType, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount);
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->GetTimeAdvPreDrawIndexedPrimitive().EndMeasurement();
                        this->GetTimeOverall().EndMeasurement();
                    }
                });

                return this->addon.AdvPreDrawIndexedPrimitive(device, PrimitiveType, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount);
            }

            virtual void AdvPostDrawIndexedPrimitive(IDirect3DDevice9* device, D3DPRIMITIVETYPE PrimitiveType, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount) {
                if (!this->AddonAdvPostDrawIndexedPrimitive) {
                    return;
                }

                this->callFunc<void>("PostDrawIndexedPrimitive", [=]() {
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->GetTimeOverall().StartMeasurement();
                        this->GetTimeAdvPostDrawIndexedPrimitive().StartMeasurement();
                    }
                    this->AddonAdvPostDrawIndexedPrimitive(device, PrimitiveType, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount);
                    if (AppConfig.GetShowDebugFeatures()) {
                        this->GetTimeAdvPostDrawIndexedPrimitive().EndMeasurement();
                        this->GetTimeOverall().EndMeasurement();
                    }
                });

                return this->addon.AdvPostDrawIndexedPrimitive(device, PrimitiveType, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount);
            }



        protected:
            virtual const Addon* GetConstBaseAddon() const override { return &addon; }
            virtual Addon* GetBaseAddon() override { return &addon; }


        private:
            template<typename R>
            R callFunc(const std::string& funcName, const std::function<R(void)>& func) {
                try {
                    return func();
                }
                catch (const std::exception& ex) {
                    this->ChangeState(AddonState::ErroredState);
                    throw exceptions::AddonFuncException(funcName, ex.what());
                }
                catch (const char* err) {
                    this->ChangeState(AddonState::ErroredState);
                    throw exceptions::AddonFuncException(funcName, err);
                }
                catch (const wchar_t* err) {
                    this->ChangeState(AddonState::ErroredState);
                    throw exceptions::AddonFuncException(funcName, err);
                }
                catch (...) {
                    this->ChangeState(AddonState::ErroredState);
                    throw exceptions::AddonFuncException(funcName, "Unknown error");
                }
            }

            bool InitializeV1(GW2AddonAPIBase* base) {
                GW2AddonAPIV1* v1 = reinterpret_cast<GW2AddonAPIV1*>(base);
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
                this->AddonOpenSettings = v1->OpenSettings;
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

                if (v1->icon) {
                    this->iconManaged = v1->iconSize > -1;
                    if (this->iconManaged) {
                        // Icon is just image data, we have to create the texture
                        D3DXCreateTextureFromFileInMemory(this->addon.GetD3DDevice9(), v1->icon, v1->iconSize, &this->icon);
                        this->iconManaged = true;
                    }
                    else {
                        // Icon is already a loaded texture
                        this->icon = reinterpret_cast<IDirect3DTexture9*>(v1->icon);
                    }
                }

                return true;
            }

            T addon;
            HMODULE addonHandle;

            std::string id = "";
            std::string name = "";
            std::string author = "";
            std::string description = "";
            std::string version = "";
            std::string homepage = "";
            IDirect3DTexture9* icon = nullptr;
            bool iconManaged = false;

            GW2AddonInitialize_t AddonInitialize = nullptr;
            GW2AddonRelease_t AddonRelease = nullptr;

            GW2AddonLoad_t AddonLoad = nullptr;
            GW2AddonDrawFrameBeforeGui_t AddonDrawFrameBeforeGui = nullptr;
            GW2AddonDrawFrameBeforePostProcessing_t AddonDrawFrameBeforePostProcessing = nullptr;
            GW2AddonDrawFrame_t AddonDrawFrame = nullptr;
            GW2AddonHandleWndProc_t AddonHandleWndProc = nullptr;
            GW2AddonOpenSettings_t AddonOpenSettings = nullptr;

            GW2AddonAdvPreBeginScene_t AddonAdvPreBeginScene = nullptr;
            GW2AddonAdvPostBeginScene_t AddonAdvPostBeginScene = nullptr;
            GW2AddonAdvPreEndScene_t AddonAdvPreEndScene = nullptr;
            GW2AddonAdvPostEndScene_t AddonAdvPostEndScene = nullptr;
            GW2AddonAdvPreClear_t AddonAdvPreClear = nullptr;
            GW2AddonAdvPostClear_t AddonAdvPostClear = nullptr;
            GW2AddonAdvPreReset_t AddonAdvPreReset = nullptr;
            GW2AddonAdvPostReset_t AddonAdvPostReset = nullptr;
            GW2AddonAdvPrePresent_t AddonAdvPrePresent = nullptr;
            GW2AddonAdvPostPresent_t AddonAdvPostPresent = nullptr;
            GW2AddonAdvPreCreateTexture_t AddonAdvPreCreateTexture = nullptr;
            GW2AddonAdvPostCreateTexture_t AddonAdvPostCreateTexture = nullptr;
            GW2AddonAdvPreCreateVertexShader_t AddonAdvPreCreateVertexShader = nullptr;
            GW2AddonAdvPostCreateVertexShader_t AddonAdvPostCreateVertexShader = nullptr;
            GW2AddonAdvPreCreatePixelShader_t AddonAdvPreCreatePixelShader = nullptr;
            GW2AddonAdvPostCreatePixelShader_t AddonAdvPostCreatePixelShader = nullptr;
            GW2AddonAdvPreCreateRenderTarget_t AddonAdvPreCreateRenderTarget = nullptr;
            GW2AddonAdvPostCreateRenderTarget_t AddonAdvPostCreateRenderTarget = nullptr;
            GW2AddonAdvPreSetTexture_t AddonAdvPreSetTexture = nullptr;
            GW2AddonAdvPostSetTexture_t AddonAdvPostSetTexture = nullptr;
            GW2AddonAdvPreSetVertexShader_t AddonAdvPreSetVertexShader = nullptr;
            GW2AddonAdvPostSetVertexShader_t AddonAdvPostSetVertexShader = nullptr;
            GW2AddonAdvPreSetPixelShader_t AddonAdvPreSetPixelShader = nullptr;
            GW2AddonAdvPostSetPixelShader_t AddonAdvPostSetPixelShader = nullptr;
            GW2AddonAdvPreSetRenderTarget_t AddonAdvPreSetRenderTarget = nullptr;
            GW2AddonAdvPostSetRenderTarget_t AddonAdvPostSetRenderTarget = nullptr;
            GW2AddonAdvPreSetRenderState_t AddonAdvPreSetRenderState = nullptr;
            GW2AddonAdvPostSetRenderState_t AddonAdvPostSetRenderState = nullptr;
            GW2AddonAdvPreDrawIndexedPrimitive_t AddonAdvPreDrawIndexedPrimitive = nullptr;
            GW2AddonAdvPostDrawIndexedPrimitive_t AddonAdvPostDrawIndexedPrimitive = nullptr;
        };

    }
}
