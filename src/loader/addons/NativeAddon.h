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

                this->AddonInitialize = reinterpret_cast<GW2AddonInitialize_t*>(GetProcAddress(h, GW2ADDON_DLL_Initialize));
                this->AddonRelease = reinterpret_cast<GW2AddonRelease_t*>(GetProcAddress(h, GW2ADDON_DLL_Release));

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

                this->AddonInitialize = nullptr;
                this->AddonRelease = nullptr;
                this->AddonLoad = nullptr;
                this->AddonOpenSettings = nullptr;

                this->HandleWndProc.Func = nullptr;
                this->DrawFrame.Func = nullptr;
                this->DrawFrameBeforeGui.Func = nullptr;
                this->DrawFrameBeforePostProcessing.Func = nullptr;
                this->AdvPreBeginScene.Func = nullptr;
                this->AdvPostBeginScene.Func = nullptr;
                this->AdvPreEndScene.Func = nullptr;
                this->AdvPostEndScene.Func = nullptr;
                this->AdvPreClear.Func = nullptr;
                this->AdvPostClear.Func = nullptr;
                this->AdvPreReset.Func = nullptr;
                this->AdvPostReset.Func = nullptr;
                this->AdvPrePresent.Func = nullptr;
                this->AdvPostPresent.Func = nullptr;
                this->AdvPreCreateTexture.Func = nullptr;
                this->AdvPostCreateTexture.Func = nullptr;
                this->AdvPreCreateVertexShader.Func = nullptr;
                this->AdvPostCreateVertexShader.Func = nullptr;
                this->AdvPreCreatePixelShader.Func = nullptr;
                this->AdvPostCreatePixelShader.Func = nullptr;
                this->AdvPreCreateRenderTarget.Func = nullptr;
                this->AdvPostCreateRenderTarget.Func = nullptr;
                this->AdvPreSetTexture.Func = nullptr;
                this->AdvPostSetTexture.Func = nullptr;
                this->AdvPreSetVertexShader.Func = nullptr;
                this->AdvPostSetVertexShader.Func = nullptr;
                this->AdvPreSetPixelShader.Func = nullptr;
                this->AdvPostSetPixelShader.Func = nullptr;
                this->AdvPreSetRenderTarget.Func = nullptr;
                this->AdvPostSetRenderTarget.Func = nullptr;
                this->AdvPreSetRenderState.Func = nullptr;
                this->AdvPostSetRenderState.Func = nullptr;
                this->AdvPreDrawIndexedPrimitive.Func = nullptr;
                this->AdvPostDrawIndexedPrimitive.Func = nullptr;

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
                    this->GetMetricLoad().StartMeasurement();
                    GW2ADDON_RESULT result = this->AddonLoad(this->GetFocusWindow(), this->GetD3DDevice9());
                    this->GetMetricLoad().EndMeasurement();

                    if (result) {
                        this->ChangeState(AddonState::ErroredState);
                        GetLog()->error("Could not load native addon {0}: Addon returned {1}", this->GetFileName(), to_string(result));
                        return false;
                    }
                }

                if (this->HandleWndProc) this->AddHook(ActiveAddonHooks.HandleWndProc);
                if (this->DrawFrameBeforeGui) this->AddHook(ActiveAddonHooks.DrawFrameBeforeGui);
                if (this->DrawFrameBeforePostProcessing) this->AddHook(ActiveAddonHooks.DrawFrameBeforePostProcessing);
                if (this->DrawFrame) this->AddHook(ActiveAddonHooks.DrawFrame);
                if (this->AdvPreBeginScene) this->AddHook(ActiveAddonHooks.AdvPreBeginScene);
                if (this->AdvPostBeginScene) this->AddHook(ActiveAddonHooks.AdvPostBeginScene);
                if (this->AdvPreEndScene) this->AddHook(ActiveAddonHooks.AdvPreEndScene);
                if (this->AdvPostEndScene) this->AddHook(ActiveAddonHooks.AdvPostEndScene);
                if (this->AdvPreClear) this->AddHook(ActiveAddonHooks.AdvPreClear);
                if (this->AdvPostClear) this->AddHook(ActiveAddonHooks.AdvPostClear);
                if (this->AdvPreReset) this->AddHook(ActiveAddonHooks.AdvPreReset);
                if (this->AdvPostReset) this->AddHook(ActiveAddonHooks.AdvPostReset);
                if (this->AdvPrePresent) this->AddHook(ActiveAddonHooks.AdvPrePresent);
                if (this->AdvPostPresent) this->AddHook(ActiveAddonHooks.AdvPostPresent);
                if (this->AdvPreCreateTexture) this->AddHook(ActiveAddonHooks.AdvPreCreateTexture);
                if (this->AdvPostCreateTexture) this->AddHook(ActiveAddonHooks.AdvPostCreateTexture);
                if (this->AdvPreCreateVertexShader) this->AddHook(ActiveAddonHooks.AdvPreCreateVertexShader);
                if (this->AdvPostCreateVertexShader) this->AddHook(ActiveAddonHooks.AdvPostCreateVertexShader);
                if (this->AdvPreCreatePixelShader) this->AddHook(ActiveAddonHooks.AdvPreCreatePixelShader);
                if (this->AdvPostCreatePixelShader) this->AddHook(ActiveAddonHooks.AdvPostCreatePixelShader);
                if (this->AdvPreCreateRenderTarget) this->AddHook(ActiveAddonHooks.AdvPreCreateRenderTarget);
                if (this->AdvPostCreateRenderTarget) this->AddHook(ActiveAddonHooks.AdvPostCreateRenderTarget);
                if (this->AdvPreSetTexture) this->AddHook(ActiveAddonHooks.AdvPreSetTexture);
                if (this->AdvPostSetTexture) this->AddHook(ActiveAddonHooks.AdvPostSetTexture);
                if (this->AdvPreSetVertexShader) this->AddHook(ActiveAddonHooks.AdvPreSetVertexShader);
                if (this->AdvPostSetVertexShader) this->AddHook(ActiveAddonHooks.AdvPostSetVertexShader);
                if (this->AdvPreSetPixelShader) this->AddHook(ActiveAddonHooks.AdvPreSetPixelShader);
                if (this->AdvPostSetPixelShader) this->AddHook(ActiveAddonHooks.AdvPostSetPixelShader);
                if (this->AdvPreSetRenderTarget) this->AddHook(ActiveAddonHooks.AdvPreSetRenderTarget);
                if (this->AdvPostSetRenderTarget) this->AddHook(ActiveAddonHooks.AdvPostSetRenderTarget);
                if (this->AdvPreSetRenderState) this->AddHook(ActiveAddonHooks.AdvPreSetRenderState);
                if (this->AdvPostSetRenderState) this->AddHook(ActiveAddonHooks.AdvPostSetRenderState);
                if (this->AdvPreDrawIndexedPrimitive) this->AddHook(ActiveAddonHooks.AdvPreDrawIndexedPrimitive);
                if (this->AdvPostDrawIndexedPrimitive) this->AddHook(ActiveAddonHooks.AdvPostDrawIndexedPrimitive);

                this->ChangeState(AddonState::LoadedState);

                return this->addon.Load();
            }

            virtual bool Unload() override {
                if (this->GetState() != AddonState::LoadedState) {
                    return false;
                }

                this->ChangeState(AddonState::UnloadingState);

                if (this->HandleWndProc) this->RemoveHook(ActiveAddonHooks.HandleWndProc);
                if (this->DrawFrameBeforeGui) this->RemoveHook(ActiveAddonHooks.DrawFrameBeforeGui);
                if (this->DrawFrameBeforePostProcessing) this->RemoveHook(ActiveAddonHooks.DrawFrameBeforePostProcessing);
                if (this->DrawFrame) this->RemoveHook(ActiveAddonHooks.DrawFrame);
                if (this->AdvPreBeginScene) this->RemoveHook(ActiveAddonHooks.AdvPreBeginScene);
                if (this->AdvPostBeginScene) this->RemoveHook(ActiveAddonHooks.AdvPostBeginScene);
                if (this->AdvPreEndScene) this->RemoveHook(ActiveAddonHooks.AdvPreEndScene);
                if (this->AdvPostEndScene) this->RemoveHook(ActiveAddonHooks.AdvPostEndScene);
                if (this->AdvPreClear) this->RemoveHook(ActiveAddonHooks.AdvPreClear);
                if (this->AdvPostClear) this->RemoveHook(ActiveAddonHooks.AdvPostClear);
                if (this->AdvPreReset) this->RemoveHook(ActiveAddonHooks.AdvPreReset);
                if (this->AdvPostReset) this->RemoveHook(ActiveAddonHooks.AdvPostReset);
                if (this->AdvPrePresent) this->RemoveHook(ActiveAddonHooks.AdvPrePresent);
                if (this->AdvPostPresent) this->RemoveHook(ActiveAddonHooks.AdvPostPresent);
                if (this->AdvPreCreateTexture) this->RemoveHook(ActiveAddonHooks.AdvPreCreateTexture);
                if (this->AdvPostCreateTexture) this->RemoveHook(ActiveAddonHooks.AdvPostCreateTexture);
                if (this->AdvPreCreateVertexShader) this->RemoveHook(ActiveAddonHooks.AdvPreCreateVertexShader);
                if (this->AdvPostCreateVertexShader) this->RemoveHook(ActiveAddonHooks.AdvPostCreateVertexShader);
                if (this->AdvPreCreatePixelShader) this->RemoveHook(ActiveAddonHooks.AdvPreCreatePixelShader);
                if (this->AdvPostCreatePixelShader) this->RemoveHook(ActiveAddonHooks.AdvPostCreatePixelShader);
                if (this->AdvPreCreateRenderTarget) this->RemoveHook(ActiveAddonHooks.AdvPreCreateRenderTarget);
                if (this->AdvPostCreateRenderTarget) this->RemoveHook(ActiveAddonHooks.AdvPostCreateRenderTarget);
                if (this->AdvPreSetTexture) this->RemoveHook(ActiveAddonHooks.AdvPreSetTexture);
                if (this->AdvPostSetTexture) this->RemoveHook(ActiveAddonHooks.AdvPostSetTexture);
                if (this->AdvPreSetVertexShader) this->RemoveHook(ActiveAddonHooks.AdvPreSetVertexShader);
                if (this->AdvPostSetVertexShader) this->RemoveHook(ActiveAddonHooks.AdvPostSetVertexShader);
                if (this->AdvPreSetPixelShader) this->RemoveHook(ActiveAddonHooks.AdvPreSetPixelShader);
                if (this->AdvPostSetPixelShader) this->RemoveHook(ActiveAddonHooks.AdvPostSetPixelShader);
                if (this->AdvPreSetRenderTarget) this->RemoveHook(ActiveAddonHooks.AdvPreSetRenderTarget);
                if (this->AdvPostSetRenderTarget) this->RemoveHook(ActiveAddonHooks.AdvPostSetRenderTarget);
                if (this->AdvPreSetRenderState) this->RemoveHook(ActiveAddonHooks.AdvPreSetRenderState);
                if (this->AdvPostSetRenderState) this->RemoveHook(ActiveAddonHooks.AdvPostSetRenderState);
                if (this->AdvPreDrawIndexedPrimitive) this->RemoveHook(ActiveAddonHooks.AdvPreDrawIndexedPrimitive);
                if (this->AdvPostDrawIndexedPrimitive) this->RemoveHook(ActiveAddonHooks.AdvPostDrawIndexedPrimitive);
                
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


        protected:
            virtual const Addon* GetConstBaseAddon() const override { return &addon; }

            virtual Addon* GetBaseAddon() override { return &addon; }


        private:
            bool InitializeV1(GW2AddonAPIBase* base) {
                GW2AddonAPIV1* v1 = reinterpret_cast<GW2AddonAPIV1*>(base);
                this->id = v1->id;
                this->name = v1->name;
                this->author = v1->author;
                this->description = v1->description;
                this->version = v1->version;
                this->homepage = v1->homepage;

                this->AddonLoad = v1->Load;
                this->AddonOpenSettings = v1->OpenSettings;

                this->HandleWndProc.Func = v1->HandleWndProc;
                this->DrawFrameBeforePostProcessing.Func = v1->DrawFrameBeforePostProcessing;
                this->DrawFrameBeforeGui.Func = v1->DrawFrameBeforeGui;
                this->DrawFrame.Func = v1->DrawFrame;
                this->AdvPreBeginScene.Func = v1->AdvPreBeginScene;
                this->AdvPostBeginScene.Func = v1->AdvPostBeginScene;
                this->AdvPreEndScene.Func = v1->AdvPreEndScene;
                this->AdvPostEndScene.Func = v1->AdvPostEndScene;
                this->AdvPreClear.Func = v1->AdvPreClear;
                this->AdvPostClear.Func = v1->AdvPostClear;
                this->AdvPreReset.Func = v1->AdvPreReset;
                this->AdvPostReset.Func = v1->AdvPostReset;
                this->AdvPrePresent.Func = v1->AdvPrePresent;
                this->AdvPostPresent.Func = v1->AdvPostPresent;
                this->AdvPreCreateTexture.Func = v1->AdvPreCreateTexture;
                this->AdvPostCreateTexture.Func = v1->AdvPostCreateTexture;
                this->AdvPreCreateVertexShader.Func = v1->AdvPreCreateVertexShader;
                this->AdvPostCreateVertexShader.Func = v1->AdvPostCreateVertexShader;
                this->AdvPreCreatePixelShader.Func = v1->AdvPreCreatePixelShader;
                this->AdvPostCreatePixelShader.Func = v1->AdvPostCreatePixelShader;
                this->AdvPreCreateRenderTarget.Func = v1->AdvPreCreateRenderTarget;
                this->AdvPostCreateRenderTarget.Func = v1->AdvPostCreateRenderTarget;
                this->AdvPreSetTexture.Func = v1->AdvPreSetTexture;
                this->AdvPostSetTexture.Func = v1->AdvPostSetTexture;
                this->AdvPreSetVertexShader.Func = v1->AdvPreSetVertexShader;
                this->AdvPostSetVertexShader.Func = v1->AdvPostSetVertexShader;
                this->AdvPreSetPixelShader.Func = v1->AdvPreSetPixelShader;
                this->AdvPostSetPixelShader.Func = v1->AdvPostSetPixelShader;
                this->AdvPreSetRenderTarget.Func = v1->AdvPreSetRenderTarget;
                this->AdvPostSetRenderTarget.Func = v1->AdvPostSetRenderTarget;
                this->AdvPreSetRenderState.Func = v1->AdvPreSetRenderState;
                this->AdvPostSetRenderState.Func = v1->AdvPostSetRenderState;
                this->AdvPreDrawIndexedPrimitive.Func = v1->AdvPreDrawIndexedPrimitive;
                this->AdvPostDrawIndexedPrimitive.Func = v1->AdvPostDrawIndexedPrimitive;

                if (v1->icon) {
                    this->iconManaged = v1->iconSize > -1;
                    if (this->iconManaged) {
                        // Icon is just image data, we have to create the texture
                        D3DXCreateTextureFromFileInMemory(this->GetD3DDevice9(), v1->icon, v1->iconSize, &this->icon);
                        this->iconManaged = true;
                    }
                    else {
                        // Icon is already a loaded texture
                        this->icon = reinterpret_cast<IDirect3DTexture9*>(v1->icon);
                    }
                }

                return true;
            }

            void AddHook(std::vector<Addon*>& addons) {
                addons.push_back(this);
            }

            void RemoveHook(std::vector<Addon*>& addons) {
                addons.erase(std::remove(addons.begin(), addons.end(), this), addons.end());
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

            GW2AddonInitialize_t* AddonInitialize = nullptr;
            GW2AddonRelease_t* AddonRelease = nullptr;
            GW2AddonLoad_t* AddonLoad = nullptr;
            GW2AddonOpenSettings_t* AddonOpenSettings = nullptr;
        };

    }
}
