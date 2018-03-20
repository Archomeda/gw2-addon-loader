#include "addons_manager.h"
#include <filesystem>
#include <ShlObj.h>
#include <Shlwapi.h>
#include "exceptions.h"
#include "../Config.h"
#include "../globals.h"
#include "../log.h"
#include "../utils/file.h"

using namespace std;
using namespace std::experimental::filesystem;
using namespace loader::utils;

namespace loader {
    namespace addons {

        vector<shared_ptr<Addon>> AddonsList;
        AddonHooks ActiveAddonHooks = {};

        bool sortAddonsRawPtrFunc(Addon* a, Addon* b) {
            if (!a->SupportsLoading() && !b->SupportsLoading()) {
                return b->GetID() > a->GetID();
            }
            if (!a->SupportsLoading()) {
                return false;
            }
            if (!b->SupportsLoading()) {
                return true;
            }

            return AppConfig.GetAddonOrder(b) > AppConfig.GetAddonOrder(a);
        }

        bool sortAddonsFunc(const shared_ptr<Addon>& a, const shared_ptr<Addon>& b) {
            return sortAddonsRawPtrFunc(a.get(), b.get());
        }

        void RefreshAddonList() {
            GetLog()->debug("loader::addons::RefreshAddonList()");

            // Clear our list
            //TODO: Don't clear this, but instead check what's removed and what's new, because we do lose some references here otherwise
            // Until ^ is solved, refreshing can only be done by restarting
            AddonsList.clear();

            // Create path
            path addonsFolder = GetGuildWars2Folder(ADDONS_FOLDER);
            GetLog()->info("Refresh addons list in {0}", addonsFolder.u8string());
            if (!PathFileExists(addonsFolder.c_str())) {
                SHCreateDirectoryEx(NULL, addonsFolder.c_str(), NULL);
            }

            // Iterate and find DLL files
            for (const auto& pathFile : directory_iterator(addonsFolder)) {
                if (pathFile.path().extension() == ".dll") {
                    GetLog()->info("Found {0}", pathFile.path().u8string());
                    AddonsList.push_back(move(Addon::GetAddon(pathFile.path().u8string())));
                }
            }
        }


        void MoveAddonUp(const Addon* const addon) {
            int index = -1;
            for (auto it = AddonsList.rbegin(); it != AddonsList.rend(); ++it) {
                if ((*it)->GetID() == addon->GetID()) {
                    index = static_cast<int>(it - AddonsList.rbegin());
                }
                else if (index > -1) {
                    AppConfig.SetAddonOrder(it->get(), static_cast<int>(AddonsList.size() - (index + 1)));
                    iter_swap(AddonsList.rbegin() + index, it);
                    AppConfig.SetAddonOrder(it->get(), static_cast<int>(AddonsList.size() - (index + 2)));
                    break;
                }
            }
            ReorderAddonHooks();
        }
        
        void MoveAddonDown(const Addon* const addon) {
            int index = -1;
            for (auto it = AddonsList.begin(); it != AddonsList.end(); ++it) {
                if ((*it)->GetID() == addon->GetID()) {
                    index = static_cast<int>(it - AddonsList.begin());
                }
                else if (index > -1) {
                    AppConfig.SetAddonOrder(it->get(), index);
                    iter_swap(AddonsList.begin() + index, it);
                    AppConfig.SetAddonOrder(it->get(), index + 1);
                    break;
                }
            }
            ReorderAddonHooks();
        }

        void ReorderAddonHooks() {
            sort(ActiveAddonHooks.HandleWndProc.begin(), ActiveAddonHooks.HandleWndProc.end(), sortAddonsRawPtrFunc);
            sort(ActiveAddonHooks.DrawFrameBeforePostProcessing.begin(), ActiveAddonHooks.DrawFrameBeforePostProcessing.end(), sortAddonsRawPtrFunc);
            sort(ActiveAddonHooks.DrawFrameBeforeGui.begin(), ActiveAddonHooks.DrawFrameBeforeGui.end(), sortAddonsRawPtrFunc);
            sort(ActiveAddonHooks.DrawFrame.begin(), ActiveAddonHooks.DrawFrame.end(), sortAddonsRawPtrFunc);
            sort(ActiveAddonHooks.AdvPreBeginScene.begin(), ActiveAddonHooks.AdvPreBeginScene.end(), sortAddonsRawPtrFunc);
            sort(ActiveAddonHooks.AdvPostBeginScene.begin(), ActiveAddonHooks.AdvPostBeginScene.end(), sortAddonsRawPtrFunc);
            sort(ActiveAddonHooks.AdvPreEndScene.begin(), ActiveAddonHooks.AdvPreEndScene.end(), sortAddonsRawPtrFunc);
            sort(ActiveAddonHooks.AdvPostEndScene.begin(), ActiveAddonHooks.AdvPostEndScene.end(), sortAddonsRawPtrFunc);
            sort(ActiveAddonHooks.AdvPreClear.begin(), ActiveAddonHooks.AdvPreClear.end(), sortAddonsRawPtrFunc);
            sort(ActiveAddonHooks.AdvPostClear.begin(), ActiveAddonHooks.AdvPostClear.end(), sortAddonsRawPtrFunc);
            sort(ActiveAddonHooks.AdvPreReset.begin(), ActiveAddonHooks.AdvPreReset.end(), sortAddonsRawPtrFunc);
            sort(ActiveAddonHooks.AdvPostReset.begin(), ActiveAddonHooks.AdvPostReset.end(), sortAddonsRawPtrFunc);
            sort(ActiveAddonHooks.AdvPrePresent.begin(), ActiveAddonHooks.AdvPrePresent.end(), sortAddonsRawPtrFunc);
            sort(ActiveAddonHooks.AdvPostPresent.begin(), ActiveAddonHooks.AdvPostPresent.end(), sortAddonsRawPtrFunc);
            sort(ActiveAddonHooks.AdvPreCreateTexture.begin(), ActiveAddonHooks.AdvPreCreateTexture.end(), sortAddonsRawPtrFunc);
            sort(ActiveAddonHooks.AdvPostCreateTexture.begin(), ActiveAddonHooks.AdvPostCreateTexture.end(), sortAddonsRawPtrFunc);
            sort(ActiveAddonHooks.AdvPreCreateVertexShader.begin(), ActiveAddonHooks.AdvPreCreateVertexShader.end(), sortAddonsRawPtrFunc);
            sort(ActiveAddonHooks.AdvPostCreateVertexShader.begin(), ActiveAddonHooks.AdvPostCreateVertexShader.end(), sortAddonsRawPtrFunc);
            sort(ActiveAddonHooks.AdvPreCreatePixelShader.begin(), ActiveAddonHooks.AdvPreCreatePixelShader.end(), sortAddonsRawPtrFunc);
            sort(ActiveAddonHooks.AdvPostCreatePixelShader.begin(), ActiveAddonHooks.AdvPostCreatePixelShader.end(), sortAddonsRawPtrFunc);
            sort(ActiveAddonHooks.AdvPreCreateRenderTarget.begin(), ActiveAddonHooks.AdvPreCreateRenderTarget.end(), sortAddonsRawPtrFunc);
            sort(ActiveAddonHooks.AdvPostCreateRenderTarget.begin(), ActiveAddonHooks.AdvPostCreateRenderTarget.end(), sortAddonsRawPtrFunc);
            sort(ActiveAddonHooks.AdvPreSetTexture.begin(), ActiveAddonHooks.AdvPreSetTexture.end(), sortAddonsRawPtrFunc);
            sort(ActiveAddonHooks.AdvPostSetTexture.begin(), ActiveAddonHooks.AdvPostSetTexture.end(), sortAddonsRawPtrFunc);
            sort(ActiveAddonHooks.AdvPreSetVertexShader.begin(), ActiveAddonHooks.AdvPreSetVertexShader.end(), sortAddonsRawPtrFunc);
            sort(ActiveAddonHooks.AdvPostSetVertexShader.begin(), ActiveAddonHooks.AdvPostSetVertexShader.end(), sortAddonsRawPtrFunc);
            sort(ActiveAddonHooks.AdvPreSetPixelShader.begin(), ActiveAddonHooks.AdvPreSetPixelShader.end(), sortAddonsRawPtrFunc);
            sort(ActiveAddonHooks.AdvPostSetPixelShader.begin(), ActiveAddonHooks.AdvPostSetPixelShader.end(), sortAddonsRawPtrFunc);
            sort(ActiveAddonHooks.AdvPreSetRenderTarget.begin(), ActiveAddonHooks.AdvPreSetRenderTarget.end(), sortAddonsRawPtrFunc);
            sort(ActiveAddonHooks.AdvPostSetRenderTarget.begin(), ActiveAddonHooks.AdvPostSetRenderTarget.end(), sortAddonsRawPtrFunc);
            sort(ActiveAddonHooks.AdvPreSetRenderState.begin(), ActiveAddonHooks.AdvPreSetRenderState.end(), sortAddonsRawPtrFunc);
            sort(ActiveAddonHooks.AdvPostSetRenderState.begin(), ActiveAddonHooks.AdvPostSetRenderState.end(), sortAddonsRawPtrFunc);
            sort(ActiveAddonHooks.AdvPreDrawIndexedPrimitive.begin(), ActiveAddonHooks.AdvPreDrawIndexedPrimitive.end(), sortAddonsRawPtrFunc);
            sort(ActiveAddonHooks.AdvPostDrawIndexedPrimitive.begin(), ActiveAddonHooks.AdvPostDrawIndexedPrimitive.end(), sortAddonsRawPtrFunc);
        }


        void InitializeAddons(UINT sdkVersion, IDirect3D9* d3d9, IDirect3DDevice9* device) {
            GetLog()->debug("loader::addons::InitializeAddons()");
            for (auto& addon : AddonsList) {
                GetLog()->info("Initializing addon {0}", addon->GetFileName());
                addon->SetSdkVersion(sdkVersion);
                addon->SetD3D9(d3d9);
                addon->SetD3DDevice9(device);
                addon->Initialize();
                GetLog()->info("Addon {0} is {1}", addon->GetFileName(), addon->GetTypeString());
            }
            sort(AddonsList.begin(), AddonsList.end(), sortAddonsFunc);
        }

        void UninitializeAddons() {
            GetLog()->debug("loader::addons::UninitializeAddons()");
            for (auto& addon : AddonsList) {
                GetLog()->info("Uninitializing addon {0}", addon->GetFileName());
                addon->Uninitialize();
            }
        }

        void LoadAddons(HWND hFocusWindow) {
            GetLog()->debug("loader::addons::LoadAddons()");
            for (auto& addon : AddonsList) {
                addon->SetFocusWindow(hFocusWindow);
                if (addon->IsEnabledByConfig()) {
                    GetLog()->info("Loading enabled addon {0}", addon->GetFileName());
                    addon->Load();
                }
                else {
                    GetLog()->info("Addon {0} is disabled", addon->GetFileName());
                }
            }
            ReorderAddonHooks();
        }

        void UnloadAddons() {
            GetLog()->debug("loader::addons::UnloadAddons()");
            for (auto& addon : AddonsList) {
                GetLog()->info("Unloading addon {0}", addon->GetFileName());
                addon->Unload();
            }
        }


        void OnStartFrame(IDirect3DDevice9* device) {
            for (auto& addon : AddonsList) {
                if (addon->IsLoaded()) {
                    addon->OnStartFrame(device);
                }
            }
        }

        void OnEndFrame(IDirect3DDevice9* device) {
            for (auto& addon : AddonsList) {
                if (addon->IsLoaded()) {
                    addon->OnEndFrame(device);
                }
            }
        }


        bool HandleWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
            for (auto& addon : ActiveAddonHooks.HandleWndProc) {
                try {
                    bool result = addon->HandleWndProc(hWnd, msg, wParam, lParam);
                    if (result) {
                        return true;
                    }
                }
                catch (const exceptions::AddonException& ex) {
                    GetLog()->error("Failed to call HandleWndProc in addon {0}: {1}", addon->GetFileName(), ex.what());
                    GetLog()->error("Addon will be disabled on next restart");
                    AppConfig.SetAddonEnabled(addon, false);
                }
            }
            return false;
        }
        
        void DrawFrameBeforePostProcessing(IDirect3DDevice9* device) {
            for (auto& addon : ActiveAddonHooks.DrawFrameBeforePostProcessing) {
                try {
                    addon->DrawFrameBeforePostProcessing(device);
                }
                catch (const exceptions::AddonException& ex) {
                    GetLog()->error("Failed to call DrawFrameBeforePostProcessing in addon {0}: {1}", addon->GetFileName(), ex.what());
                    GetLog()->error("Addon will be disabled on next restart");
                    AppConfig.SetAddonEnabled(addon, false);
                }
            }
        }

        void DrawFrameBeforeGui(IDirect3DDevice9* device) {
            for (auto& addon : ActiveAddonHooks.DrawFrameBeforeGui) {
                try {
                    addon->DrawFrameBeforeGui(device);
                }
                catch (const exceptions::AddonException& ex) {
                    GetLog()->error("Failed to call DrawFrameBeforeGui in addon {0}: {1}", addon->GetFileName(), ex.what());
                    GetLog()->error("Addon will be disabled on next restart");
                    AppConfig.SetAddonEnabled(addon, false);
                }
            }
        }

        void DrawFrame(IDirect3DDevice9* device) {
            for (auto& addon : ActiveAddonHooks.DrawFrame) {
                try {
                    addon->DrawFrame(device);
                }
                catch (const exceptions::AddonException& ex) {
                    GetLog()->error("Failed to call DrawFrame in addon {0}: {1}", addon->GetFileName(), ex.what());
                    GetLog()->error("Addon will be disabled on next restart");
                    AppConfig.SetAddonEnabled(addon, false);
                }
            }
        }

        void AdvPreBeginScene(IDirect3DDevice9* device) {
            for (auto& addon : ActiveAddonHooks.AdvPreBeginScene) {
                try {
                    addon->AdvPreBeginScene(device);
                }
                catch (const exceptions::AddonException& ex) {
                    GetLog()->error("Failed to call AdvPreBeginScene in addon {0}: {1}", addon->GetFileName(), ex.what());
                    GetLog()->error("Addon will be disabled on next restart");
                    AppConfig.SetAddonEnabled(addon, false);
                }
            }
        }

        void AdvPostBeginScene(IDirect3DDevice9* device) {
            for (auto& addon : ActiveAddonHooks.AdvPostBeginScene) {
                try {
                    addon->AdvPostBeginScene(device);
                }
                catch (const exceptions::AddonException& ex) {
                    GetLog()->error("Failed to call AdvPostBeginScene in addon {0}: {1}", addon->GetFileName(), ex.what());
                    GetLog()->error("Addon will be disabled on next restart");
                    AppConfig.SetAddonEnabled(addon, false);
                }
            }
        }

        void AdvPreEndScene(IDirect3DDevice9* device) {
            for (auto& addon : ActiveAddonHooks.AdvPreEndScene) {
                try {
                    addon->AdvPreEndScene(device);
                }
                catch (const exceptions::AddonException& ex) {
                    GetLog()->error("Failed to call AdvPreEndScene in addon {0}: {1}", addon->GetFileName(), ex.what());
                    GetLog()->error("Addon will be disabled on next restart");
                    AppConfig.SetAddonEnabled(addon, false);
                }
            }
        }

        void AdvPostEndScene(IDirect3DDevice9* device) {
            for (auto& addon : ActiveAddonHooks.AdvPostEndScene) {
                try {
                    addon->AdvPostEndScene(device);
                }
                catch (const exceptions::AddonException& ex) {
                    GetLog()->error("Failed to call AdvPostEndScene in addon {0}: {1}", addon->GetFileName(), ex.what());
                    GetLog()->error("Addon will be disabled on next restart");
                    AppConfig.SetAddonEnabled(addon, false);
                }
            }
        }

        void AdvPreClear(IDirect3DDevice9* device, DWORD Count, CONST D3DRECT* pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil) {
            for (auto& addon : ActiveAddonHooks.AdvPreClear) {
                try {
                    addon->AdvPreClear(device, Count, pRects, Flags, Color, Z, Stencil);
                }
                catch (const exceptions::AddonException& ex) {
                    GetLog()->error("Failed to call AdvPreClear in addon {0}: {1}", addon->GetFileName(), ex.what());
                    GetLog()->error("Addon will be disabled on next restart");
                    AppConfig.SetAddonEnabled(addon, false);
                }
            }
        }

        void AdvPostClear(IDirect3DDevice9* device, DWORD Count, CONST D3DRECT* pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil) {
            for (auto& addon : ActiveAddonHooks.AdvPostClear) {
                try {
                    addon->AdvPostClear(device, Count, pRects, Flags, Color, Z, Stencil);
                }
                catch (const exceptions::AddonException& ex) {
                    GetLog()->error("Failed to call AdvPostClear in addon {0}: {1}", addon->GetFileName(), ex.what());
                    GetLog()->error("Addon will be disabled on next restart");
                    AppConfig.SetAddonEnabled(addon, false);
                }
            }
        }

        void AdvPreReset(IDirect3DDevice9* device, D3DPRESENT_PARAMETERS* pPresentationParameters) {
            for (auto& addon : ActiveAddonHooks.AdvPreReset) {
                try {
                    addon->AdvPreReset(device, pPresentationParameters);
                }
                catch (const exceptions::AddonException& ex) {
                    GetLog()->error("Failed to call AdvPreReset in addon {0}: {1}", addon->GetFileName(), ex.what());
                    GetLog()->error("Addon will be disabled on next restart");
                    AppConfig.SetAddonEnabled(addon, false);
                }
            }
        }

        void AdvPostReset(IDirect3DDevice9* device, D3DPRESENT_PARAMETERS* pPresentationParameters) {
            for (auto& addon : ActiveAddonHooks.AdvPostReset) {
                try {
                    addon->AdvPostReset(device, pPresentationParameters);
                }
                catch (const exceptions::AddonException& ex) {
                    GetLog()->error("Failed to call AdvPostReset in addon {0}: {1}", addon->GetFileName(), ex.what());
                    GetLog()->error("Addon will be disabled on next restart");
                    AppConfig.SetAddonEnabled(addon, false);
                }
            }
        }

        void AdvPrePresent(IDirect3DDevice9* device, CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion) {
            for (auto& addon : ActiveAddonHooks.AdvPrePresent) {
                try {
                    addon->AdvPrePresent(device, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
                }
                catch (const exceptions::AddonException& ex) {
                    GetLog()->error("Failed to call AdvPrePresent in addon {0}: {1}", addon->GetFileName(), ex.what());
                    GetLog()->error("Addon will be disabled on next restart");
                    AppConfig.SetAddonEnabled(addon, false);
                }
            }
        }

        void AdvPostPresent(IDirect3DDevice9* device, CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion) {
            for (auto& addon : ActiveAddonHooks.AdvPostPresent) {
                try {
                    addon->AdvPostPresent(device, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
                }
                catch (const exceptions::AddonException& ex) {
                    GetLog()->error("Failed to call AdvPostPresent in addon {0}: {1}", addon->GetFileName(), ex.what());
                    GetLog()->error("Addon will be disabled on next restart");
                    AppConfig.SetAddonEnabled(addon, false);
                }
            }
        }

        HRESULT AdvPreCreateTexture(IDirect3DDevice9* device, UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DTexture9** ppTexture, HANDLE* pSharedHandle) {
            for (auto& addon : ActiveAddonHooks.AdvPreCreateTexture) {
                try {
                    IDirect3DTexture9* pOldTexture = *ppTexture;
                    HRESULT hr = addon->AdvPreCreateTexture(device, Width, Height, Levels, Usage, Format, Pool, ppTexture, pSharedHandle);
                    if (hr != D3D_OK || *ppTexture != pOldTexture) {
                        return hr;
                    }
                }
                catch (const exceptions::AddonException& ex) {
                    GetLog()->error("Failed to call AdvPreCreateTexture in addon {0}: {1}", addon->GetFileName(), ex.what());
                    GetLog()->error("Addon will be disabled on next restart");
                    AppConfig.SetAddonEnabled(addon, false);
                }
            }
            return D3D_OK;
        }

        void AdvPostCreateTexture(IDirect3DDevice9* device, IDirect3DTexture9* pTexture, UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, HANDLE* pSharedHandle) {
            for (auto& addon : ActiveAddonHooks.AdvPostCreateTexture) {
                try {
                    addon->AdvPostCreateTexture(device, pTexture, Width, Height, Levels, Usage, Format, Pool, pSharedHandle);
                }
                catch (const exceptions::AddonException& ex) {
                    GetLog()->error("Failed to call AdvPostCreateTexture in addon {0}: {1}", addon->GetFileName(), ex.what());
                    GetLog()->error("Addon will be disabled on next restart");
                    AppConfig.SetAddonEnabled(addon, false);
                }
            }
        }

        HRESULT AdvPreCreateVertexShader(IDirect3DDevice9* device, CONST DWORD* pFunction, IDirect3DVertexShader9** ppShader) {
            for (auto& addon : ActiveAddonHooks.AdvPreCreateVertexShader) {
                try {
                    IDirect3DVertexShader9* pOldShader = *ppShader;
                    HRESULT hr = addon->AdvPreCreateVertexShader(device, pFunction, ppShader);
                    if (hr != D3D_OK || *ppShader != pOldShader) {
                        return hr;
                    }
                }
                catch (const exceptions::AddonException& ex) {
                    GetLog()->error("Failed to call AdvPreCreateVertexShader in addon {0}: {1}", addon->GetFileName(), ex.what());
                    GetLog()->error("Addon will be disabled on next restart");
                    AppConfig.SetAddonEnabled(addon, false);
                }
            }
            return D3D_OK;
        }

        void AdvPostCreateVertexShader(IDirect3DDevice9* device, IDirect3DVertexShader9* pShader, CONST DWORD* pFunction) {
            for (auto& addon : ActiveAddonHooks.AdvPostCreateVertexShader) {
                try {
                    addon->AdvPostCreateVertexShader(device, pShader, pFunction);
                }
                catch (const exceptions::AddonException& ex) {
                    GetLog()->error("Failed to call AdvPostCreateVertexShader in addon {0}: {1}", addon->GetFileName(), ex.what());
                    GetLog()->error("Addon will be disabled on next restart");
                    AppConfig.SetAddonEnabled(addon, false);
                }
            }
        }

        HRESULT AdvPreCreatePixelShader(IDirect3DDevice9* device, CONST DWORD* pFunction, IDirect3DPixelShader9** ppShader) {
            for (auto& addon : ActiveAddonHooks.AdvPreCreatePixelShader) {
                try {
                    IDirect3DPixelShader9* pOldShader = *ppShader;
                    HRESULT hr = addon->AdvPreCreatePixelShader(device, pFunction, ppShader);
                    if (hr != D3D_OK || *ppShader != pOldShader) {
                        return hr;
                    }
                }
                catch (const exceptions::AddonException& ex) {
                    GetLog()->error("Failed to call AdvPreCreatePixelShader in addon {0}: {1}", addon->GetFileName(), ex.what());
                    GetLog()->error("Addon will be disabled on next restart");
                    AppConfig.SetAddonEnabled(addon, false);
                }
            }
            return D3D_OK;
        }

        void AdvPostCreatePixelShader(IDirect3DDevice9* device, IDirect3DPixelShader9* pShader, CONST DWORD* pFunction) {
            for (auto& addon : ActiveAddonHooks.AdvPostCreatePixelShader) {
                try {
                    addon->AdvPostCreatePixelShader(device, pShader, pFunction);
                }
                catch (const exceptions::AddonException& ex) {
                    GetLog()->error("Failed to call AdvPostCreatePixelShader in addon {0}: {1}", addon->GetFileName(), ex.what());
                    GetLog()->error("Addon will be disabled on next restart");
                    AppConfig.SetAddonEnabled(addon, false);
                }
            }
        }

        HRESULT AdvPreCreateRenderTarget(IDirect3DDevice9* device, UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Lockable, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle) {
            for (auto& addon : ActiveAddonHooks.AdvPreCreateRenderTarget) {
                try {
                    IDirect3DSurface9* pOldSurface = *ppSurface;
                    HRESULT hr = addon->AdvPreCreateRenderTarget(device, Width, Height, Format, MultiSample, MultisampleQuality, Lockable, ppSurface, pSharedHandle);
                    if (hr != D3D_OK || *ppSurface != pOldSurface) {
                        return hr;
                    }
                }
                catch (const exceptions::AddonException& ex) {
                    GetLog()->error("Failed to call AdvPreCreateRenderTarget in addon {0}: {1}", addon->GetFileName(), ex.what());
                    GetLog()->error("Addon will be disabled on next restart");
                    AppConfig.SetAddonEnabled(addon, false);
                }
            }
            return D3D_OK;
        }

        void AdvPostCreateRenderTarget(IDirect3DDevice9* device, IDirect3DSurface9* pSurface, UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Lockable, HANDLE* pSharedHandle) {
            for (auto& addon : ActiveAddonHooks.AdvPostCreateRenderTarget) {
                try {
                    addon->AdvPostCreateRenderTarget(device, pSurface, Width, Height, Format, MultiSample, MultisampleQuality, Lockable, pSharedHandle);
                }
                catch (const exceptions::AddonException& ex) {
                    GetLog()->error("Failed to call AdvPostCreateRenderTarget in addon {0}: {1}", addon->GetFileName(), ex.what());
                    GetLog()->error("Addon will be disabled on next restart");
                    AppConfig.SetAddonEnabled(addon, false);
                }
            }
        }

        void AdvPreSetTexture(IDirect3DDevice9* device, DWORD Stage, IDirect3DBaseTexture9* pTexture) {
            for (auto& addon : ActiveAddonHooks.AdvPreSetTexture) {
                try {
                    addon->AdvPreSetTexture(device, Stage, pTexture);
                }
                catch (const exceptions::AddonException& ex) {
                    GetLog()->error("Failed to call AdvPreSetTexture in addon {0}: {1}", addon->GetFileName(), ex.what());
                    GetLog()->error("Addon will be disabled on next restart");
                    AppConfig.SetAddonEnabled(addon, false);
                }
            }
        }

        void AdvPostSetTexture(IDirect3DDevice9* device, DWORD Stage, IDirect3DBaseTexture9* pTexture) {
            for (auto& addon : ActiveAddonHooks.AdvPostSetTexture) {
                try {
                    addon->AdvPostSetTexture(device, Stage, pTexture);
                }
                catch (const exceptions::AddonException& ex) {
                    GetLog()->error("Failed to call AdvPostSetTexture in addon {0}: {1}", addon->GetFileName(), ex.what());
                    GetLog()->error("Addon will be disabled on next restart");
                    AppConfig.SetAddonEnabled(addon, false);
                }
            }
        }

        void AdvPreSetVertexShader(IDirect3DDevice9* device, IDirect3DVertexShader9* pShader) {
            for (auto& addon : ActiveAddonHooks.AdvPreSetVertexShader) {
                try {
                    addon->AdvPreSetVertexShader(device, pShader);
                }
                catch (const exceptions::AddonException& ex) {
                    GetLog()->error("Failed to call AdvPreSetVertexShader in addon {0}: {1}", addon->GetFileName(), ex.what());
                    GetLog()->error("Addon will be disabled on next restart");
                    AppConfig.SetAddonEnabled(addon, false);
                }
            }
        }

        void AdvPostSetVertexShader(IDirect3DDevice9* device, IDirect3DVertexShader9* pShader) {
            for (auto& addon : ActiveAddonHooks.AdvPostSetVertexShader) {
                try {
                    addon->AdvPostSetVertexShader(device, pShader);
                }
                catch (const exceptions::AddonException& ex) {
                    GetLog()->error("Failed to call AdvPostSetVertexShader in addon {0}: {1}", addon->GetFileName(), ex.what());
                    GetLog()->error("Addon will be disabled on next restart");
                    AppConfig.SetAddonEnabled(addon, false);
                }
            }
        }

        void AdvPreSetPixelShader(IDirect3DDevice9* device, IDirect3DPixelShader9* pShader) {
            for (auto& addon : ActiveAddonHooks.AdvPreSetPixelShader) {
                try {
                    addon->AdvPreSetPixelShader(device, pShader);
                }
                catch (const exceptions::AddonException& ex) {
                    GetLog()->error("Failed to call AdvPreSetPixelShader in addon {0}: {1}", addon->GetFileName(), ex.what());
                    GetLog()->error("Addon will be disabled on next restart");
                    AppConfig.SetAddonEnabled(addon, false);
                }
            }
        }

        void AdvPostSetPixelShader(IDirect3DDevice9* device, IDirect3DPixelShader9* pShader) {
            for (auto& addon : ActiveAddonHooks.AdvPostSetPixelShader) {
                try {
                    addon->AdvPostSetPixelShader(device, pShader);
                }
                catch (const exceptions::AddonException& ex) {
                    GetLog()->error("Failed to call AdvPostSetPixelShader in addon {0}: {1}", addon->GetFileName(), ex.what());
                    GetLog()->error("Addon will be disabled on next restart");
                    AppConfig.SetAddonEnabled(addon, false);
                }
            }
        }

        void AdvPreSetRenderTarget(IDirect3DDevice9* device, DWORD RenderTargetIndex, IDirect3DSurface9* pRenderTarget) {
            for (auto& addon : ActiveAddonHooks.AdvPreSetRenderTarget) {
                try {
                    addon->AdvPreSetRenderTarget(device, RenderTargetIndex, pRenderTarget);
                }
                catch (const exceptions::AddonException& ex) {
                    GetLog()->error("Failed to call AdvPreSetRenderTarget in addon {0}: {1}", addon->GetFileName(), ex.what());
                    GetLog()->error("Addon will be disabled on next restart");
                    AppConfig.SetAddonEnabled(addon, false);
                }
            }
        }

        void AdvPostSetRenderTarget(IDirect3DDevice9* device, DWORD RenderTargetIndex, IDirect3DSurface9* pRenderTarget) {
            for (auto& addon : ActiveAddonHooks.AdvPostSetRenderTarget) {
                try {
                    addon->AdvPostSetRenderTarget(device, RenderTargetIndex, pRenderTarget);
                }
                catch (const exceptions::AddonException& ex) {
                    GetLog()->error("Failed to call AdvPostSetRenderTarget in addon {0}: {1}", addon->GetFileName(), ex.what());
                    GetLog()->error("Addon will be disabled on next restart");
                    AppConfig.SetAddonEnabled(addon, false);
                }
            }
        }

        void AdvPreSetRenderState(IDirect3DDevice9* device, D3DRENDERSTATETYPE State, DWORD Value) {
            for (auto& addon : ActiveAddonHooks.AdvPreSetRenderState) {
                try {
                    addon->AdvPreSetRenderState(device, State, Value);
                }
                catch (const exceptions::AddonException& ex) {
                    GetLog()->error("Failed to call AdvPreSetRenderState in addon {0}: {1}", addon->GetFileName(), ex.what());
                    GetLog()->error("Addon will be disabled on next restart");
                    AppConfig.SetAddonEnabled(addon, false);
                }
            }
        }

        void AdvPostSetRenderState(IDirect3DDevice9* device, D3DRENDERSTATETYPE State, DWORD Value) {
            for (auto& addon : ActiveAddonHooks.AdvPostSetRenderState) {
                try {
                    addon->AdvPostSetRenderState(device, State, Value);
                }
                catch (const exceptions::AddonException& ex) {
                    GetLog()->error("Failed to call function in addon {0}: {1}", addon->GetFileName(), ex.what());
                    GetLog()->error("Addon will be disabled on next restart");
                    AppConfig.SetAddonEnabled(addon, false);
                }
            }
        }

        void AdvPreDrawIndexedPrimitive(IDirect3DDevice9* device, D3DPRIMITIVETYPE PrimitiveType, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount) {
            for (auto& addon : ActiveAddonHooks.AdvPreDrawIndexedPrimitive) {
                try {
                    addon->AdvPreDrawIndexedPrimitive(device, PrimitiveType, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount);
                }
                catch (const exceptions::AddonException& ex) {
                    GetLog()->error("Failed to call AdvPreDrawIndexedPrimitive in addon {0}: {1}", addon->GetFileName(), ex.what());
                    GetLog()->error("Addon will be disabled on next restart");
                    AppConfig.SetAddonEnabled(addon, false);
                }
            }
        }

        void AdvPostDrawIndexedPrimitive(IDirect3DDevice9* device, D3DPRIMITIVETYPE PrimitiveType, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount) {
            for (auto& addon : ActiveAddonHooks.AdvPostDrawIndexedPrimitive) {
                try {
                    addon->AdvPostDrawIndexedPrimitive(device, PrimitiveType, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount);
                }
                catch (const exceptions::AddonException& ex) {
                    GetLog()->error("Failed to call AdvPostDrawIndexedPrimitive in addon {0}: {1}", addon->GetFileName(), ex.what());
                    GetLog()->error("Addon will be disabled on next restart");
                    AppConfig.SetAddonEnabled(addon, false);
                }
            }
        }

    }
}
