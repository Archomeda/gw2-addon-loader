#include "addons_manager.h"
#include <filesystem>
#include <ShlObj.h>
#include <Shlwapi.h>
#include "exceptions.h"
#include "../Config.h"
#include "../log.h"

#ifdef _WIN64
#define ADDONS_FOLDER "bin64/addons/"
#else
#define ADDONS_FOLDER "bin/addons/"
#endif

using namespace std;
using namespace std::experimental::filesystem::v1;

namespace loader {
    namespace addons {

        vector<shared_ptr<Addon>> AddonsList;
        AddonHookCounts ActiveAddonHookCounts = { };

        bool sortAddonsFunc(shared_ptr<Addon> a, shared_ptr<Addon> b) {
            if (!a->SupportsLoading() && !b->SupportsLoading()) {
                return b->GetID() > a->GetID();
            }
            if (!a->SupportsLoading()) {
                return false;
            }
            if (!b->SupportsLoading()) {
                return true;
            }

            return AppConfig.GetAddonOrder(b.get()) > AppConfig.GetAddonOrder(a.get());
        }

        void RefreshAddonList() {
            GetLog()->debug("loader::addons::RefreshAddonList()");

            // Clear our list
            //TODO: Don't clear this, but instead check what's removed and what's new, because we do lose some references here otherwise
            // Until ^ is solved, refreshing can only be done by restarting
            AddonsList.clear();

            // Create path
            wchar_t fileName[MAX_PATH];
            GetModuleFileName(NULL, fileName, sizeof(fileName));
            PathRemoveFileSpec(fileName);
            path addonsFolder(fileName);
            addonsFolder /= ADDONS_FOLDER;
            GetLog()->info("Refresh addons list in {0}", addonsFolder.u8string());
            if (!PathFileExists(addonsFolder.c_str())) {
                SHCreateDirectoryEx(NULL, addonsFolder.c_str(), NULL);
            }

            // Iterate and find DLL files
            for (auto& pathFile : directory_iterator(addonsFolder)) {
                if (pathFile.path().extension() == ".dll") {
                    GetLog()->info("Found {0}", pathFile.path().string());
                    AddonsList.push_back(move(Addon::GetAddon(pathFile.path().u8string())));
                }
            }
        }


        void InitializeAddons(UINT sdkVersion, IDirect3D9* d3d9, IDirect3DDevice9* device) {
            GetLog()->debug("loader::addons::InitializeAddons()");
            for (auto it = AddonsList.begin(); it != AddonsList.end(); ++it) {
                GetLog()->info("Initializing addon {0}", (*it)->GetFileName());
                (*it)->SetSdkVersion(sdkVersion);
                (*it)->SetD3D9(d3d9);
                (*it)->SetD3DDevice9(device);
                (*it)->Initialize();
                GetLog()->info("Addon {0} is {1}", (*it)->GetTypeString());
            }
            sort(AddonsList.begin(), AddonsList.end(), sortAddonsFunc);
        }

        void UninitializeAddons() {
            GetLog()->debug("loader::addons::UninitializeAddons()");
            for (auto it = AddonsList.begin(); it != AddonsList.end(); ++it) {
                GetLog()->info("Uninitializing addon {0}", (*it)->GetFileName());
                (*it)->Uninitialize();
            }
        }

        void LoadAddons(HWND hFocusWindow) {
            GetLog()->debug("loader::addons::LoadAddons()");
            for (auto it = AddonsList.begin(); it != AddonsList.end(); ++it) {
                (*it)->SetFocusWindow(hFocusWindow);
                if ((*it)->IsEnabledByConfig()) {
                    GetLog()->info("Loading enabled addon {0}", (*it)->GetFileName());
                    (*it)->Load();
                }
                else {
                    GetLog()->info("Addon {0} is disabled", (*it)->GetFileName());
                }
            }
        }

        void UnloadAddons() {
            GetLog()->debug("loader::addons::UnloadAddons()");
            for (auto it = AddonsList.begin(); it != AddonsList.end(); ++it) {
                GetLog()->info("Unloading addon {0}", (*it)->GetFileName());
                (*it)->Unload();
            }
        }

        
        void OnStartFrame(IDirect3DDevice9* device) {
            for (auto it = AddonsList.begin(); it != AddonsList.end(); ++it) {
                if ((*it)->IsLoaded()) {
                    (*it)->OnStartFrame(device);
                }
            }
        }

        void OnEndFrame(IDirect3DDevice9* device) {
            for (auto it = AddonsList.begin(); it != AddonsList.end(); ++it) {
                if ((*it)->IsLoaded()) {
                    (*it)->OnEndFrame(device);
                }
            }
        }


        void DrawFrameBeforePostProcessing(IDirect3DDevice9* device) {
            if (ActiveAddonHookCounts.DrawFrameBeforePostProcessing > 0) {
                for (auto it = AddonsList.begin(); it != AddonsList.end(); ++it) {
                    if ((*it)->IsLoaded()) {
                        try {
                            (*it)->DrawFrameBeforePostProcessing(device);
                        }
                        catch (const exceptions::AddonException& ex) {
                            GetLog()->error("Failed to call function in addon {0}: {1}", (*it)->GetFileName(), ex.what());
                            GetLog()->error("Addon will be disabled on next restart");
                            AppConfig.SetAddonEnabled(it->get(), false);
                        }
                    }
                }
            }
        }

        void DrawFrameBeforeGui(IDirect3DDevice9* device) {
            if (ActiveAddonHookCounts.DrawFrameBeforeGui > 0) {
                for (auto it = AddonsList.begin(); it != AddonsList.end(); ++it) {
                    if ((*it)->IsLoaded()) {
                        try {
                            (*it)->DrawFrameBeforeGui(device);
                        }
                        catch (const exceptions::AddonException& ex) {
                            GetLog()->error("Failed to call function in addon {0}: {1}", (*it)->GetFileName(), ex.what());
                            GetLog()->error("Addon will be disabled on next restart");
                            AppConfig.SetAddonEnabled(it->get(), false);
                        }
                    }
                }
            }
        }

        void DrawFrame(IDirect3DDevice9* device) {
            if (ActiveAddonHookCounts.DrawFrame > 0) {
                for (auto it = AddonsList.begin(); it != AddonsList.end(); ++it) {
                    if ((*it)->IsLoaded()) {
                        try {
                            (*it)->DrawFrame(device);
                        }
                        catch (const exceptions::AddonException& ex) {
                            GetLog()->error("Failed to call function in addon {0}: {1}", (*it)->GetFileName(), ex.what());
                            GetLog()->error("Addon will be disabled on next restart");
                            AppConfig.SetAddonEnabled(it->get(), false);
                        }
                    }
                }
            }
        }

        void AdvPreBeginScene(IDirect3DDevice9* device) {
            if (ActiveAddonHookCounts.AdvPreBeginScene > 0) {
                for (auto it = AddonsList.begin(); it != AddonsList.end(); ++it) {
                    if ((*it)->IsLoaded()) {
                        try {
                            (*it)->AdvPreBeginScene(device);
                        }
                        catch (const exceptions::AddonException& ex) {
                            GetLog()->error("Failed to call function in addon {0}: {1}", (*it)->GetFileName(), ex.what());
                            GetLog()->error("Addon will be disabled on next restart");
                            AppConfig.SetAddonEnabled(it->get(), false);
                        }
                    }
                }
            }
        }

        void AdvPostBeginScene(IDirect3DDevice9* device) {
            if (ActiveAddonHookCounts.AdvPostBeginScene > 0) {
                for (auto it = AddonsList.begin(); it != AddonsList.end(); ++it) {
                    if ((*it)->IsLoaded()) {
                        try {
                            (*it)->AdvPostBeginScene(device);
                        }
                        catch (const exceptions::AddonException& ex) {
                            GetLog()->error("Failed to call function in addon {0}: {1}", (*it)->GetFileName(), ex.what());
                            GetLog()->error("Addon will be disabled on next restart");
                            AppConfig.SetAddonEnabled(it->get(), false);
                        }
                    }
                }
            }
        }

        void AdvPreEndScene(IDirect3DDevice9* device) {
            if (ActiveAddonHookCounts.AdvPreEndScene > 0) {
                for (auto it = AddonsList.begin(); it != AddonsList.end(); ++it) {
                    if ((*it)->IsLoaded()) {
                        try {
                            (*it)->AdvPreEndScene(device);
                        }
                        catch (const exceptions::AddonException& ex) {
                            GetLog()->error("Failed to call function in addon {0}: {1}", (*it)->GetFileName(), ex.what());
                            GetLog()->error("Addon will be disabled on next restart");
                            AppConfig.SetAddonEnabled(it->get(), false);
                        }
                    }
                }
            }
        }

        void AdvPostEndScene(IDirect3DDevice9* device) {
            if (ActiveAddonHookCounts.AdvPostEndScene > 0) {
                for (auto it = AddonsList.begin(); it != AddonsList.end(); ++it) {
                    if ((*it)->IsLoaded()) {
                        try {
                            (*it)->AdvPostEndScene(device);
                        }
                        catch (const exceptions::AddonException& ex) {
                            GetLog()->error("Failed to call function in addon {0}: {1}", (*it)->GetFileName(), ex.what());
                            GetLog()->error("Addon will be disabled on next restart");
                            AppConfig.SetAddonEnabled(it->get(), false);
                        }
                    }
                }
            }
        }

        void AdvPreClear(IDirect3DDevice9* device, DWORD Count, CONST D3DRECT* pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil) {
            if (ActiveAddonHookCounts.AdvPreClear > 0) {
                for (auto it = AddonsList.begin(); it != AddonsList.end(); ++it) {
                    if ((*it)->IsLoaded()) {
                        try {
                            (*it)->AdvPreClear(device, Count, pRects, Flags, Color, Z, Stencil);
                        }
                        catch (const exceptions::AddonException& ex) {
                            GetLog()->error("Failed to call function in addon {0}: {1}", (*it)->GetFileName(), ex.what());
                            GetLog()->error("Addon will be disabled on next restart");
                            AppConfig.SetAddonEnabled(it->get(), false);
                        }
                    }
                }
            }
        }

        void AdvPostClear(IDirect3DDevice9* device, DWORD Count, CONST D3DRECT* pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil) {
            if (ActiveAddonHookCounts.AdvPostClear > 0) {
                for (auto it = AddonsList.begin(); it != AddonsList.end(); ++it) {
                    if ((*it)->IsLoaded()) {
                        try {
                            (*it)->AdvPostClear(device, Count, pRects, Flags, Color, Z, Stencil);
                        }
                        catch (const exceptions::AddonException& ex) {
                            GetLog()->error("Failed to call function in addon {0}: {1}", (*it)->GetFileName(), ex.what());
                            GetLog()->error("Addon will be disabled on next restart");
                            AppConfig.SetAddonEnabled(it->get(), false);
                        }
                    }
                }
            }
        }

        void AdvPreReset(IDirect3DDevice9* device, D3DPRESENT_PARAMETERS* pPresentationParameters) {
            if (ActiveAddonHookCounts.AdvPreReset > 0) {
                for (auto it = AddonsList.begin(); it != AddonsList.end(); ++it) {
                    if ((*it)->IsLoaded()) {
                        try {
                            (*it)->AdvPreReset(device, pPresentationParameters);
                        }
                        catch (const exceptions::AddonException& ex) {
                            GetLog()->error("Failed to call function in addon {0}: {1}", (*it)->GetFileName(), ex.what());
                            GetLog()->error("Addon will be disabled on next restart");
                            AppConfig.SetAddonEnabled(it->get(), false);
                        }
                    }
                }
            }
        }

        void AdvPostReset(IDirect3DDevice9* device, D3DPRESENT_PARAMETERS* pPresentationParameters) {
            if (ActiveAddonHookCounts.AdvPostReset > 0) {
                for (auto it = AddonsList.begin(); it != AddonsList.end(); ++it) {
                    if ((*it)->IsLoaded()) {
                        try {
                            (*it)->AdvPostReset(device, pPresentationParameters);
                        }
                        catch (const exceptions::AddonException& ex) {
                            GetLog()->error("Failed to call function in addon {0}: {1}", (*it)->GetFileName(), ex.what());
                            GetLog()->error("Addon will be disabled on next restart");
                            AppConfig.SetAddonEnabled(it->get(), false);
                        }
                    }
                }
            }
        }

        void AdvPrePresent(IDirect3DDevice9* device, CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion) {
            if (ActiveAddonHookCounts.AdvPrePresent > 0) {
                for (auto it = AddonsList.begin(); it != AddonsList.end(); ++it) {
                    if ((*it)->IsLoaded()) {
                        try {
                            (*it)->AdvPrePresent(device, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
                        }
                        catch (const exceptions::AddonException& ex) {
                            GetLog()->error("Failed to call function in addon {0}: {1}", (*it)->GetFileName(), ex.what());
                            GetLog()->error("Addon will be disabled on next restart");
                            AppConfig.SetAddonEnabled(it->get(), false);
                        }
                    }
                }
            }
        }

        void AdvPostPresent(IDirect3DDevice9* device, CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion) {
            if (ActiveAddonHookCounts.AdvPostPresent > 0) {
                for (auto it = AddonsList.begin(); it != AddonsList.end(); ++it) {
                    if ((*it)->IsLoaded()) {
                        try {
                            (*it)->AdvPostPresent(device, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
                        }
                        catch (const exceptions::AddonException& ex) {
                            GetLog()->error("Failed to call function in addon {0}: {1}", (*it)->GetFileName(), ex.what());
                            GetLog()->error("Addon will be disabled on next restart");
                            AppConfig.SetAddonEnabled(it->get(), false);
                        }
                    }
                }
            }
        }

        HRESULT AdvPreCreateTexture(IDirect3DDevice9* device, UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DTexture9** ppTexture, HANDLE* pSharedHandle) {
            if (ActiveAddonHookCounts.AdvPreCreateTexture > 0) {
                for (auto it = AddonsList.begin(); it != AddonsList.end(); ++it) {
                    if ((*it)->IsLoaded()) {
                        try {
                            IDirect3DTexture9* pOldTexture = *ppTexture;
                            HRESULT hr = (*it)->AdvPreCreateTexture(device, Width, Height, Levels, Usage, Format, Pool, ppTexture, pSharedHandle);
                            if (hr != D3D_OK || *ppTexture != pOldTexture) {
                                return hr;
                            }
                        }
                        catch (const exceptions::AddonException& ex) {
                            GetLog()->error("Failed to call function in addon {0}: {1}", (*it)->GetFileName(), ex.what());
                            GetLog()->error("Addon will be disabled on next restart");
                            AppConfig.SetAddonEnabled(it->get(), false);
                        }
                    }
                }
            }
            return D3D_OK;
        }

        void AdvPostCreateTexture(IDirect3DDevice9* device, IDirect3DTexture9* pTexture, UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, HANDLE* pSharedHandle) {
            if (ActiveAddonHookCounts.AdvPostCreateTexture > 0) {
                for (auto it = AddonsList.begin(); it != AddonsList.end(); ++it) {
                    if ((*it)->IsLoaded()) {
                        try {
                            (*it)->AdvPostCreateTexture(device, pTexture, Width, Height, Levels, Usage, Format, Pool, pSharedHandle);
                        }
                        catch (const exceptions::AddonException& ex) {
                            GetLog()->error("Failed to call function in addon {0}: {1}", (*it)->GetFileName(), ex.what());
                            GetLog()->error("Addon will be disabled on next restart");
                            AppConfig.SetAddonEnabled(it->get(), false);
                        }
                    }
                }
            }
        }

        HRESULT AdvPreCreateVertexShader(IDirect3DDevice9* device, CONST DWORD* pFunction, IDirect3DVertexShader9** ppShader) {
            if (ActiveAddonHookCounts.AdvPreCreateVertexShader > 0) {
                for (auto it = AddonsList.begin(); it != AddonsList.end(); ++it) {
                    if ((*it)->IsLoaded()) {
                        try {
                            IDirect3DVertexShader9* pOldShader = *ppShader;
                            HRESULT hr = (*it)->AdvPreCreateVertexShader(device, pFunction, ppShader);
                            if (hr != D3D_OK || *ppShader != pOldShader) {
                                return hr;
                            }
                        }
                        catch (const exceptions::AddonException& ex) {
                            GetLog()->error("Failed to call function in addon {0}: {1}", (*it)->GetFileName(), ex.what());
                            GetLog()->error("Addon will be disabled on next restart");
                            AppConfig.SetAddonEnabled(it->get(), false);
                        }
                    }
                }
            }
            return D3D_OK;
        }

        void AdvPostCreateVertexShader(IDirect3DDevice9* device, IDirect3DVertexShader9* pShader, CONST DWORD* pFunction) {
            if (ActiveAddonHookCounts.AdvPostCreateVertexShader > 0) {
                for (auto it = AddonsList.begin(); it != AddonsList.end(); ++it) {
                    if ((*it)->IsLoaded()) {
                        try {
                            (*it)->AdvPostCreateVertexShader(device, pShader, pFunction);
                        }
                        catch (const exceptions::AddonException& ex) {
                            GetLog()->error("Failed to call function in addon {0}: {1}", (*it)->GetFileName(), ex.what());
                            GetLog()->error("Addon will be disabled on next restart");
                            AppConfig.SetAddonEnabled(it->get(), false);
                        }
                    }
                }
            }
        }

        HRESULT AdvPreCreatePixelShader(IDirect3DDevice9* device, CONST DWORD* pFunction, IDirect3DPixelShader9** ppShader) {
            if (ActiveAddonHookCounts.AdvPreCreatePixelShader > 0) {
                for (auto it = AddonsList.begin(); it != AddonsList.end(); ++it) {
                    if ((*it)->IsLoaded()) {
                        try {
                            IDirect3DPixelShader9* pOldShader = *ppShader;
                            HRESULT hr = (*it)->AdvPreCreatePixelShader(device, pFunction, ppShader);
                            if (hr != D3D_OK || *ppShader != pOldShader) {
                                return hr;
                            }
                        }
                        catch (const exceptions::AddonException& ex) {
                            GetLog()->error("Failed to call function in addon {0}: {1}", (*it)->GetFileName(), ex.what());
                            GetLog()->error("Addon will be disabled on next restart");
                            AppConfig.SetAddonEnabled(it->get(), false);
                        }
                    }
                }
            }
            return D3D_OK;
        }

        void AdvPostCreatePixelShader(IDirect3DDevice9* device, IDirect3DPixelShader9* pShader, CONST DWORD* pFunction) {
            if (ActiveAddonHookCounts.AdvPostCreatePixelShader > 0) {
                for (auto it = AddonsList.begin(); it != AddonsList.end(); ++it) {
                    if ((*it)->IsLoaded()) {
                        try {
                            (*it)->AdvPostCreatePixelShader(device, pShader, pFunction);
                        }
                        catch (const exceptions::AddonException& ex) {
                            GetLog()->error("Failed to call function in addon {0}: {1}", (*it)->GetFileName(), ex.what());
                            GetLog()->error("Addon will be disabled on next restart");
                            AppConfig.SetAddonEnabled(it->get(), false);
                        }
                    }
                }
            }
        }

        HRESULT AdvPreCreateRenderTarget(IDirect3DDevice9* device, UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Lockable, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle) {
            if (ActiveAddonHookCounts.AdvPreCreateRenderTarget > 0) {
                for (auto it = AddonsList.begin(); it != AddonsList.end(); ++it) {
                    if ((*it)->IsLoaded()) {
                        try {
                            IDirect3DSurface9* pOldSurface = *ppSurface;
                            HRESULT hr = (*it)->AdvPreCreateRenderTarget(device, Width, Height, Format, MultiSample, MultisampleQuality, Lockable, ppSurface, pSharedHandle);
                            if (hr != D3D_OK || *ppSurface != pOldSurface) {
                                return hr;
                            }
                        }
                        catch (const exceptions::AddonException& ex) {
                            GetLog()->error("Failed to call function in addon {0}: {1}", (*it)->GetFileName(), ex.what());
                            GetLog()->error("Addon will be disabled on next restart");
                            AppConfig.SetAddonEnabled(it->get(), false);
                        }
                    }
                }
            }
            return D3D_OK;
        }

        void AdvPostCreateRenderTarget(IDirect3DDevice9* device, IDirect3DSurface9* pSurface, UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Lockable, HANDLE* pSharedHandle) {
            if (ActiveAddonHookCounts.AdvPostCreateRenderTarget > 0) {
                for (auto it = AddonsList.begin(); it != AddonsList.end(); ++it) {
                    if ((*it)->IsLoaded()) {
                        try {
                            (*it)->AdvPostCreateRenderTarget(device, pSurface, Width, Height, Format, MultiSample, MultisampleQuality, Lockable, pSharedHandle);
                        }
                        catch (const exceptions::AddonException& ex) {
                            GetLog()->error("Failed to call function in addon {0}: {1}", (*it)->GetFileName(), ex.what());
                            GetLog()->error("Addon will be disabled on next restart");
                            AppConfig.SetAddonEnabled(it->get(), false);
                        }
                    }
                }
            }
        }

        void AdvPreSetTexture(IDirect3DDevice9* device, DWORD Stage, IDirect3DBaseTexture9* pTexture) {
            if (ActiveAddonHookCounts.AdvPreSetTexture > 0) {
                for (auto it = AddonsList.begin(); it != AddonsList.end(); ++it) {
                    if ((*it)->IsLoaded()) {
                        try {
                            (*it)->AdvPreSetTexture(device, Stage, pTexture);
                        }
                        catch (const exceptions::AddonException& ex) {
                            GetLog()->error("Failed to call function in addon {0}: {1}", (*it)->GetFileName(), ex.what());
                            GetLog()->error("Addon will be disabled on next restart");
                            AppConfig.SetAddonEnabled(it->get(), false);
                        }
                    }
                }
            }
        }

        void AdvPostSetTexture(IDirect3DDevice9* device, DWORD Stage, IDirect3DBaseTexture9* pTexture) {
            if (ActiveAddonHookCounts.AdvPostSetTexture > 0) {
                for (auto it = AddonsList.begin(); it != AddonsList.end(); ++it) {
                    if ((*it)->IsLoaded()) {
                        try {
                            (*it)->AdvPostSetTexture(device, Stage, pTexture);
                        }
                        catch (const exceptions::AddonException& ex) {
                            GetLog()->error("Failed to call function in addon {0}: {1}", (*it)->GetFileName(), ex.what());
                            GetLog()->error("Addon will be disabled on next restart");
                            AppConfig.SetAddonEnabled(it->get(), false);
                        }
                    }
                }
            }
        }

        void AdvPreSetVertexShader(IDirect3DDevice9* device, IDirect3DVertexShader9* pShader) {
            if (ActiveAddonHookCounts.AdvPreSetVertexShader > 0) {
                for (auto it = AddonsList.begin(); it != AddonsList.end(); ++it) {
                    if ((*it)->IsLoaded()) {
                        try {
                            (*it)->AdvPreSetVertexShader(device, pShader);
                        }
                        catch (const exceptions::AddonException& ex) {
                            GetLog()->error("Failed to call function in addon {0}: {1}", (*it)->GetFileName(), ex.what());
                            GetLog()->error("Addon will be disabled on next restart");
                            AppConfig.SetAddonEnabled(it->get(), false);
                        }
                    }
                }
            }
        }

        void AdvPostSetVertexShader(IDirect3DDevice9* device, IDirect3DVertexShader9* pShader) {
            if (ActiveAddonHookCounts.AdvPostSetVertexShader > 0) {
                for (auto it = AddonsList.begin(); it != AddonsList.end(); ++it) {
                    if ((*it)->IsLoaded()) {
                        try {
                            (*it)->AdvPostSetVertexShader(device, pShader);
                        }
                        catch (const exceptions::AddonException& ex) {
                            GetLog()->error("Failed to call function in addon {0}: {1}", (*it)->GetFileName(), ex.what());
                            GetLog()->error("Addon will be disabled on next restart");
                            AppConfig.SetAddonEnabled(it->get(), false);
                        }
                    }
                }
            }
        }

        void AdvPreSetPixelShader(IDirect3DDevice9* device, IDirect3DPixelShader9* pShader) {
            if (ActiveAddonHookCounts.AdvPreSetPixelShader > 0) {
                for (auto it = AddonsList.begin(); it != AddonsList.end(); ++it) {
                    if ((*it)->IsLoaded()) {
                        try {
                            (*it)->AdvPreSetPixelShader(device, pShader);
                        }
                        catch (const exceptions::AddonException& ex) {
                            GetLog()->error("Failed to call function in addon {0}: {1}", (*it)->GetFileName(), ex.what());
                            GetLog()->error("Addon will be disabled on next restart");
                            AppConfig.SetAddonEnabled(it->get(), false);
                        }
                    }
                }
            }
        }

        void AdvPostSetPixelShader(IDirect3DDevice9* device, IDirect3DPixelShader9* pShader) {
            if (ActiveAddonHookCounts.AdvPostSetPixelShader > 0) {
                for (auto it = AddonsList.begin(); it != AddonsList.end(); ++it) {
                    if ((*it)->IsLoaded()) {
                        try {
                            (*it)->AdvPostSetPixelShader(device, pShader);
                        }
                        catch (const exceptions::AddonException& ex) {
                            GetLog()->error("Failed to call function in addon {0}: {1}", (*it)->GetFileName(), ex.what());
                            GetLog()->error("Addon will be disabled on next restart");
                            AppConfig.SetAddonEnabled(it->get(), false);
                        }
                    }
                }
            }
        }

        void AdvPreSetRenderTarget(IDirect3DDevice9* device, DWORD RenderTargetIndex, IDirect3DSurface9* pRenderTarget) {
            if (ActiveAddonHookCounts.AdvPreSetRenderTarget > 0) {
                for (auto it = AddonsList.begin(); it != AddonsList.end(); ++it) {
                    if ((*it)->IsLoaded()) {
                        try {
                            (*it)->AdvPreSetRenderTarget(device, RenderTargetIndex, pRenderTarget);
                        }
                        catch (const exceptions::AddonException& ex) {
                            GetLog()->error("Failed to call function in addon {0}: {1}", (*it)->GetFileName(), ex.what());
                            GetLog()->error("Addon will be disabled on next restart");
                            AppConfig.SetAddonEnabled(it->get(), false);
                        }
                    }
                }
            }
        }

        void AdvPostSetRenderTarget(IDirect3DDevice9* device, DWORD RenderTargetIndex, IDirect3DSurface9* pRenderTarget) {
            if (ActiveAddonHookCounts.AdvPostSetRenderTarget > 0) {
                for (auto it = AddonsList.begin(); it != AddonsList.end(); ++it) {
                    if ((*it)->IsLoaded()) {
                        try {
                            (*it)->AdvPostSetRenderTarget(device, RenderTargetIndex, pRenderTarget);
                        }
                        catch (const exceptions::AddonException& ex) {
                            GetLog()->error("Failed to call function in addon {0}: {1}", (*it)->GetFileName(), ex.what());
                            GetLog()->error("Addon will be disabled on next restart");
                            AppConfig.SetAddonEnabled(it->get(), false);
                        }
                    }
                }
            }
        }

        void AdvPreSetRenderState(IDirect3DDevice9* device, D3DRENDERSTATETYPE State, DWORD Value) {
            if (ActiveAddonHookCounts.AdvPreSetRenderState > 0) {
                for (auto it = AddonsList.begin(); it != AddonsList.end(); ++it) {
                    if ((*it)->IsLoaded()) {
                        try {
                            (*it)->AdvPreSetRenderState(device, State, Value);
                        }
                        catch (const exceptions::AddonException& ex) {
                            GetLog()->error("Failed to call function in addon {0}: {1}", (*it)->GetFileName(), ex.what());
                            GetLog()->error("Addon will be disabled on next restart");
                            AppConfig.SetAddonEnabled(it->get(), false);
                        }
                    }
                }
            }
        }

        void AdvPostSetRenderState(IDirect3DDevice9* device, D3DRENDERSTATETYPE State, DWORD Value) {
            if (ActiveAddonHookCounts.AdvPostSetRenderState > 0) {
                for (auto it = AddonsList.begin(); it != AddonsList.end(); ++it) {
                    if ((*it)->IsLoaded()) {
                        try {
                            (*it)->AdvPostSetRenderState(device, State, Value);
                        }
                        catch (const exceptions::AddonException& ex) {
                            GetLog()->error("Failed to call function in addon {0}: {1}", (*it)->GetFileName(), ex.what());
                            GetLog()->error("Addon will be disabled on next restart");
                            AppConfig.SetAddonEnabled(it->get(), false);
                        }
                    }
                }
            }
        }

        void AdvPreDrawIndexedPrimitive(IDirect3DDevice9* device, D3DPRIMITIVETYPE PrimitiveType, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount) {
            if (ActiveAddonHookCounts.AdvPreDrawIndexedPrimitive > 0) {
                for (auto it = AddonsList.begin(); it != AddonsList.end(); ++it) {
                    if ((*it)->IsLoaded()) {
                        try {
                            (*it)->AdvPreDrawIndexedPrimitive(device, PrimitiveType, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount);
                        }
                        catch (const exceptions::AddonException& ex) {
                            GetLog()->error("Failed to call function in addon {0}: {1}", (*it)->GetFileName(), ex.what());
                            GetLog()->error("Addon will be disabled on next restart");
                            AppConfig.SetAddonEnabled(it->get(), false);
                        }
                    }
                }
            }
        }

        void AdvPostDrawIndexedPrimitive(IDirect3DDevice9* device, D3DPRIMITIVETYPE PrimitiveType, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount) {
            if (ActiveAddonHookCounts.AdvPostDrawIndexedPrimitive > 0) {
                for (auto it = AddonsList.begin(); it != AddonsList.end(); ++it) {
                    if ((*it)->IsLoaded()) {
                        try {
                            (*it)->AdvPostDrawIndexedPrimitive(device, PrimitiveType, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount);
                        }
                        catch (const exceptions::AddonException& ex) {
                            GetLog()->error("Failed to call function in addon {0}: {1}", (*it)->GetFileName(), ex.what());
                            GetLog()->error("Addon will be disabled on next restart");
                            AppConfig.SetAddonEnabled(it->get(), false);
                        }
                    }
                }
            }
        }

    }
}
