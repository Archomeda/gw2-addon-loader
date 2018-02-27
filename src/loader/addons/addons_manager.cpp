#include "addons_manager.h"
#include <filesystem>
#include <ShlObj.h>
#include <Shlwapi.h>
#include "./exceptions.h"
#include "../Config.h"
#include "../log.h"
#include "../utils.h"

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
            if ((!a->GetTypeImpl() && b->GetTypeImpl()) || (!a->SupportsLoading() && !b->SupportsLoading())) {
                return b->GetID() > a->GetID();
            }

            if (!a->GetTypeImpl()) {
                return false;
            }
            if (!b->GetTypeImpl()) {
                return true;
            }
            if (!a->SupportsLoading()) {
                return false;
            }
            if (!b->SupportsLoading()) {
                return true;
            }

            return AppConfig.GetAddonOrder(b->GetFileName()) > AppConfig.GetAddonOrder(a->GetFileName());
        }

        void RefreshAddonList() {
            GetLog()->debug("loader::addons::RefreshAddonList()");

            // Clear our list
            //TODO: Don't clear this, but instead check what's removed and what's new, because we do lose some references here otherwise
            // Until ^ isn't solved, refreshing can only be done by restarting
            AddonsList.clear();

            // Create path
            TCHAR fileName[MAX_PATH];
            GetModuleFileName(NULL, fileName, sizeof(fileName));
            PathRemoveFileSpec(fileName);
            path addonsFolder(fileName);
            addonsFolder /= ADDONS_FOLDER;
            GetLog()->info("Using addon folder {0}", addonsFolder.string());
            if (!PathFileExists(addonsFolder.c_str())) {
                SHCreateDirectoryEx(NULL, addonsFolder.c_str(), NULL);
            }

            // Iterate and find DLL files
            for (auto& pathFile : directory_iterator(addonsFolder)) {
                if (pathFile.path().extension() == ".dll") {
                    GetLog()->info("Found {0}", pathFile.path().string());
                    AddonsList.push_back(make_shared<Addon>(pathFile.path().wstring()));
                }
            }
        }


        void InitializeAddons(UINT sdkVersion, IDirect3D9* d3d9) {
            GetLog()->debug("loader::addons::InitializeAddons()");
            for (auto it = addons::AddonsList.begin(); it != addons::AddonsList.end(); ++it) {
                (*it)->SetSdkVersion(sdkVersion);
                (*it)->SetD3D9(d3d9);
                (*it)->Initialize();
            }
            sort(AddonsList.begin(), AddonsList.end(), sortAddonsFunc);
        }

        void UninitializeAddons() {
            GetLog()->debug("loader::addons::UninitializeAddons()");
            for (auto it = addons::AddonsList.begin(); it != addons::AddonsList.end(); ++it) {
                (*it)->Uninitialize();
            }
        }

        void LoadAddons(HWND hFocusWindow) {
            GetLog()->debug("loader::addons::LoadAddons()");
            for (auto it = addons::AddonsList.begin(); it != addons::AddonsList.end(); ++it) {
                (*it)->SetFocusWindow(hFocusWindow);
                if ((*it)->IsEnabledByConfig()) {
                    (*it)->Load();
                }
            }
        }

        void UnloadAddons() {
            GetLog()->debug("loader::addons::UnloadAddons()");
            for (auto it = addons::AddonsList.begin(); it != addons::AddonsList.end(); ++it) {
                (*it)->Unload();
            }
        }

        
        void DrawFrameBeforePostProcessing(IDirect3DDevice9* device) {
            if (ActiveAddonHookCounts.DrawFrameBeforePostProcessing > 0) {
                for (auto it = AddonsList.begin(); it != AddonsList.end(); ++it) {
                    if ((*it)->IsLoaded()) {
                        try {
                            (*it)->GetTypeImpl()->DrawFrameBeforePostProcessing(device);
                        }
                        catch (const exceptions::AddonDrawException& ex) {
                            GetLog()->error("Failed to draw frame before post processing in addon: {0} : {1}", ws2s((*it)->GetFileName()), ex.what());
                            GetLog()->error("Addon will be disabled on next restart");
                            AppConfig.SetAddonEnabled((*it)->GetFileName(), false);
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
                            (*it)->GetTypeImpl()->DrawFrameBeforeGui(device);
                        }
                        catch (const exceptions::AddonDrawException& ex) {
                            GetLog()->error("Failed to draw frame before GUI in addon: {0} : {1}", ws2s((*it)->GetFileName()), ex.what());
                            GetLog()->error("Addon will be disabled on next restart");
                            AppConfig.SetAddonEnabled((*it)->GetFileName(), false);
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
                            (*it)->GetTypeImpl()->DrawFrame(device);
                        }
                        catch (const exceptions::AddonDrawException& ex) {
                            GetLog()->error("Failed to draw frame in addon: {0} : {1}", ws2s((*it)->GetFileName()), ex.what());
                            GetLog()->error("Addon will be disabled on next restart");
                            AppConfig.SetAddonEnabled((*it)->GetFileName(), false);
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
                            (*it)->GetTypeImpl()->AdvPreBeginScene(device);
                        }
                        catch (const exceptions::AddonAdvFuncException& ex) {
                            GetLog()->error("Failed to call PreBeginScene in addon: {0} : {1}", ws2s((*it)->GetFileName()), ex.what());
                            GetLog()->error("Addon will be disabled on next restart");
                            AppConfig.SetAddonEnabled((*it)->GetFileName(), false);
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
                            (*it)->GetTypeImpl()->AdvPostBeginScene(device);
                        }
                        catch (const exceptions::AddonAdvFuncException& ex) {
                            GetLog()->error("Failed to call AdvPostBeginScene in addon: {0} : {1}", ws2s((*it)->GetFileName()), ex.what());
                            GetLog()->error("Addon will be disabled on next restart");
                            AppConfig.SetAddonEnabled((*it)->GetFileName(), false);
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
                            (*it)->GetTypeImpl()->AdvPreEndScene(device);
                        }
                        catch (const exceptions::AddonAdvFuncException& ex) {
                            GetLog()->error("Failed to call AdvPreEndScene in addon: {0} : {1}", ws2s((*it)->GetFileName()), ex.what());
                            GetLog()->error("Addon will be disabled on next restart");
                            AppConfig.SetAddonEnabled((*it)->GetFileName(), false);
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
                            (*it)->GetTypeImpl()->AdvPostEndScene(device);
                        }
                        catch (const exceptions::AddonAdvFuncException& ex) {
                            GetLog()->error("Failed to call AdvPostEndScene in addon: {0} : {1}", ws2s((*it)->GetFileName()), ex.what());
                            GetLog()->error("Addon will be disabled on next restart");
                            AppConfig.SetAddonEnabled((*it)->GetFileName(), false);
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
                            (*it)->GetTypeImpl()->AdvPreClear(device, Count, pRects, Flags, Color, Z, Stencil);
                        }
                        catch (const exceptions::AddonAdvFuncException& ex) {
                            GetLog()->error("Failed to call AdvPreClear in addon: {0} : {1}", ws2s((*it)->GetFileName()), ex.what());
                            GetLog()->error("Addon will be disabled on next restart");
                            AppConfig.SetAddonEnabled((*it)->GetFileName(), false);
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
                            (*it)->GetTypeImpl()->AdvPostClear(device, Count, pRects, Flags, Color, Z, Stencil);
                        }
                        catch (const exceptions::AddonAdvFuncException& ex) {
                            GetLog()->error("Failed to call AdvPostClear in addon: {0} : {1}", ws2s((*it)->GetFileName()), ex.what());
                            GetLog()->error("Addon will be disabled on next restart");
                            AppConfig.SetAddonEnabled((*it)->GetFileName(), false);
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
                            (*it)->GetTypeImpl()->AdvPreReset(device, pPresentationParameters);
                        }
                        catch (const exceptions::AddonAdvFuncException& ex) {
                            GetLog()->error("Failed to call AdvPreReset in addon: {0} : {1}", ws2s((*it)->GetFileName()), ex.what());
                            GetLog()->error("Addon will be disabled on next restart");
                            AppConfig.SetAddonEnabled((*it)->GetFileName(), false);
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
                            (*it)->GetTypeImpl()->AdvPostReset(device, pPresentationParameters);
                        }
                        catch (const exceptions::AddonAdvFuncException& ex) {
                            GetLog()->error("Failed to call AdvPostReset in addon: {0} : {1}", ws2s((*it)->GetFileName()), ex.what());
                            GetLog()->error("Addon will be disabled on next restart");
                            AppConfig.SetAddonEnabled((*it)->GetFileName(), false);
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
                            (*it)->GetTypeImpl()->AdvPrePresent(device, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
                        }
                        catch (const exceptions::AddonAdvFuncException& ex) {
                            GetLog()->error("Failed to call AdvPrePresent in addon: {0} : {1}", ws2s((*it)->GetFileName()), ex.what());
                            GetLog()->error("Addon will be disabled on next restart");
                            AppConfig.SetAddonEnabled((*it)->GetFileName(), false);
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
                            (*it)->GetTypeImpl()->AdvPostPresent(device, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
                        }
                        catch (const exceptions::AddonAdvFuncException& ex) {
                            GetLog()->error("Failed to call AdvPostPresent in addon: {0} : {1}", ws2s((*it)->GetFileName()), ex.what());
                            GetLog()->error("Addon will be disabled on next restart");
                            AppConfig.SetAddonEnabled((*it)->GetFileName(), false);
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
                            HRESULT hr = (*it)->GetTypeImpl()->AdvPreCreateTexture(device, Width, Height, Levels, Usage, Format, Pool, ppTexture, pSharedHandle);
                            if (hr != D3D_OK || *ppTexture != pOldTexture) {
                                return hr;
                            }
                        }
                        catch (const exceptions::AddonAdvFuncException& ex) {
                            GetLog()->error("Failed to call AdvPreCreateTexture in addon: {0} : {1}", ws2s((*it)->GetFileName()), ex.what());
                            GetLog()->error("Addon will be disabled on next restart");
                            AppConfig.SetAddonEnabled((*it)->GetFileName(), false);
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
                            (*it)->GetTypeImpl()->AdvPostCreateTexture(device, pTexture, Width, Height, Levels, Usage, Format, Pool, pSharedHandle);
                        }
                        catch (const exceptions::AddonAdvFuncException& ex) {
                            GetLog()->error("Failed to call AdvPostCreateTexture in addon: {0} : {1}", ws2s((*it)->GetFileName()), ex.what());
                            GetLog()->error("Addon will be disabled on next restart");
                            AppConfig.SetAddonEnabled((*it)->GetFileName(), false);
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
                            HRESULT hr = (*it)->GetTypeImpl()->AdvPreCreateVertexShader(device, pFunction, ppShader);
                            if (hr != D3D_OK || *ppShader != pOldShader) {
                                return hr;
                            }
                        }
                        catch (const exceptions::AddonAdvFuncException& ex) {
                            GetLog()->error("Failed to call AdvPreCreateVertexShader in addon: {0} : {1}", ws2s((*it)->GetFileName()), ex.what());
                            GetLog()->error("Addon will be disabled on next restart");
                            AppConfig.SetAddonEnabled((*it)->GetFileName(), false);
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
                            (*it)->GetTypeImpl()->AdvPostCreateVertexShader(device, pShader, pFunction);
                        }
                        catch (const exceptions::AddonAdvFuncException& ex) {
                            GetLog()->error("Failed to call AdvPostCreateVertexShader in addon: {0} : {1}", ws2s((*it)->GetFileName()), ex.what());
                            GetLog()->error("Addon will be disabled on next restart");
                            AppConfig.SetAddonEnabled((*it)->GetFileName(), false);
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
                            HRESULT hr = (*it)->GetTypeImpl()->AdvPreCreatePixelShader(device, pFunction, ppShader);
                            if (hr != D3D_OK || *ppShader != pOldShader) {
                                return hr;
                            }
                        }
                        catch (const exceptions::AddonAdvFuncException& ex) {
                            GetLog()->error("Failed to call AdvPreCreatePixelShader in addon: {0} : {1}", ws2s((*it)->GetFileName()), ex.what());
                            GetLog()->error("Addon will be disabled on next restart");
                            AppConfig.SetAddonEnabled((*it)->GetFileName(), false);
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
                            (*it)->GetTypeImpl()->AdvPostCreatePixelShader(device, pShader, pFunction);
                        }
                        catch (const exceptions::AddonAdvFuncException& ex) {
                            GetLog()->error("Failed to call AdvPostCreatePixelShader in addon: {0} : {1}", ws2s((*it)->GetFileName()), ex.what());
                            GetLog()->error("Addon will be disabled on next restart");
                            AppConfig.SetAddonEnabled((*it)->GetFileName(), false);
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
                            HRESULT hr = (*it)->GetTypeImpl()->AdvPreCreateRenderTarget(device, Width, Height, Format, MultiSample, MultisampleQuality, Lockable, ppSurface, pSharedHandle);
                            if (hr != D3D_OK || *ppSurface != pOldSurface) {
                                return hr;
                            }
                        }
                        catch (const exceptions::AddonAdvFuncException& ex) {
                            GetLog()->error("Failed to call AdvPreCreateRenderTarget in addon: {0} : {1}", ws2s((*it)->GetFileName()), ex.what());
                            GetLog()->error("Addon will be disabled on next restart");
                            AppConfig.SetAddonEnabled((*it)->GetFileName(), false);
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
                            (*it)->GetTypeImpl()->AdvPostCreateRenderTarget(device, pSurface, Width, Height, Format, MultiSample, MultisampleQuality, Lockable, pSharedHandle);
                        }
                        catch (const exceptions::AddonAdvFuncException& ex) {
                            GetLog()->error("Failed to call AdvPostCreateRenderTarget in addon: {0} : {1}", ws2s((*it)->GetFileName()), ex.what());
                            GetLog()->error("Addon will be disabled on next restart");
                            AppConfig.SetAddonEnabled((*it)->GetFileName(), false);
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
                            (*it)->GetTypeImpl()->AdvPreSetTexture(device, Stage, pTexture);
                        }
                        catch (const exceptions::AddonAdvFuncException& ex) {
                            GetLog()->error("Failed to call AdvPreSetTexture in addon: {0} : {1}", ws2s((*it)->GetFileName()), ex.what());
                            GetLog()->error("Addon will be disabled on next restart");
                            AppConfig.SetAddonEnabled((*it)->GetFileName(), false);
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
                            (*it)->GetTypeImpl()->AdvPostSetTexture(device, Stage, pTexture);
                        }
                        catch (const exceptions::AddonAdvFuncException& ex) {
                            GetLog()->error("Failed to call AdvPostSetTexture in addon: {0} : {1}", ws2s((*it)->GetFileName()), ex.what());
                            GetLog()->error("Addon will be disabled on next restart");
                            AppConfig.SetAddonEnabled((*it)->GetFileName(), false);
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
                            (*it)->GetTypeImpl()->AdvPreSetVertexShader(device, pShader);
                        }
                        catch (const exceptions::AddonAdvFuncException& ex) {
                            GetLog()->error("Failed to call AdvPreSetVertexShader in addon: {0} : {1}", ws2s((*it)->GetFileName()), ex.what());
                            GetLog()->error("Addon will be disabled on next restart");
                            AppConfig.SetAddonEnabled((*it)->GetFileName(), false);
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
                            (*it)->GetTypeImpl()->AdvPostSetVertexShader(device, pShader);
                        }
                        catch (const exceptions::AddonAdvFuncException& ex) {
                            GetLog()->error("Failed to call AdvPostSetVertexShader in addon: {0} : {1}", ws2s((*it)->GetFileName()), ex.what());
                            GetLog()->error("Addon will be disabled on next restart");
                            AppConfig.SetAddonEnabled((*it)->GetFileName(), false);
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
                            (*it)->GetTypeImpl()->AdvPreSetPixelShader(device, pShader);
                        }
                        catch (const exceptions::AddonAdvFuncException& ex) {
                            GetLog()->error("Failed to call AdvPreSetPixelShader in addon: {0} : {1}", ws2s((*it)->GetFileName()), ex.what());
                            GetLog()->error("Addon will be disabled on next restart");
                            AppConfig.SetAddonEnabled((*it)->GetFileName(), false);
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
                            (*it)->GetTypeImpl()->AdvPostSetPixelShader(device, pShader);
                        }
                        catch (const exceptions::AddonAdvFuncException& ex) {
                            GetLog()->error("Failed to call AdvPostSetPixelShader in addon: {0} : {1}", ws2s((*it)->GetFileName()), ex.what());
                            GetLog()->error("Addon will be disabled on next restart");
                            AppConfig.SetAddonEnabled((*it)->GetFileName(), false);
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
                            (*it)->GetTypeImpl()->AdvPreSetRenderTarget(device, RenderTargetIndex, pRenderTarget);
                        }
                        catch (const exceptions::AddonAdvFuncException& ex) {
                            GetLog()->error("Failed to call AdvPreSetRenderTarget in addon: {0} : {1}", ws2s((*it)->GetFileName()), ex.what());
                            GetLog()->error("Addon will be disabled on next restart");
                            AppConfig.SetAddonEnabled((*it)->GetFileName(), false);
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
                            (*it)->GetTypeImpl()->AdvPostSetRenderTarget(device, RenderTargetIndex, pRenderTarget);
                        }
                        catch (const exceptions::AddonAdvFuncException& ex) {
                            GetLog()->error("Failed to call AdvPostSetRenderTarget in addon: {0} : {1}", ws2s((*it)->GetFileName()), ex.what());
                            GetLog()->error("Addon will be disabled on next restart");
                            AppConfig.SetAddonEnabled((*it)->GetFileName(), false);
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
                            (*it)->GetTypeImpl()->AdvPreSetRenderState(device, State, Value);
                        }
                        catch (const exceptions::AddonAdvFuncException& ex) {
                            GetLog()->error("Failed to call AdvPreSetRenderState in addon: {0} : {1}", ws2s((*it)->GetFileName()), ex.what());
                            GetLog()->error("Addon will be disabled on next restart");
                            AppConfig.SetAddonEnabled((*it)->GetFileName(), false);
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
                            (*it)->GetTypeImpl()->AdvPostSetRenderState(device, State, Value);
                        }
                        catch (const exceptions::AddonAdvFuncException& ex) {
                            GetLog()->error("Failed to call AdvPostSetRenderState in addon: {0} : {1}", ws2s((*it)->GetFileName()), ex.what());
                            GetLog()->error("Addon will be disabled on next restart");
                            AppConfig.SetAddonEnabled((*it)->GetFileName(), false);
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
                            (*it)->GetTypeImpl()->AdvPreDrawIndexedPrimitive(device, PrimitiveType, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount);
                        }
                        catch (const exceptions::AddonAdvFuncException& ex) {
                            GetLog()->error("Failed to call AdvPreDrawIndexedPrimitive in addon: {0} : {1}", ws2s((*it)->GetFileName()), ex.what());
                            GetLog()->error("Addon will be disabled on next restart");
                            AppConfig.SetAddonEnabled((*it)->GetFileName(), false);
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
                            (*it)->GetTypeImpl()->AdvPostDrawIndexedPrimitive(device, PrimitiveType, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount);
                        }
                        catch (const exceptions::AddonAdvFuncException& ex) {
                            GetLog()->error("Failed to call AdvPostDrawIndexedPrimitive in addon: {0} : {1}", ws2s((*it)->GetFileName()), ex.what());
                            GetLog()->error("Addon will be disabled on next restart");
                            AppConfig.SetAddonEnabled((*it)->GetFileName(), false);
                        }
                    }
                }
            }
        }

    }
}
