#include "LegacyAddon.h"
#include <algorithm>
#include <filesystem>
#include "addons_manager.h"
#include "ProxyAddon.h"
#include "../minhook.h"
#include "../globals.h"
#include "../log.h"
#include "../utils/encoding.h"
#include "../utils/file.h"

using namespace std;
using namespace std::experimental::filesystem;
using namespace loader::hooks;
using namespace loader::utils;

namespace loader {
    namespace addons {

        namespace hooks {

            typedef HMODULE(WINAPI LoadLibraryA_t)(LPCSTR lpFileName);
            typedef HMODULE(WINAPI LoadLibraryW_t)(LPCWSTR lpFileName);
            typedef HMODULE(WINAPI LoadLibraryExA_t)(LPCSTR lpFileName, HANDLE hFile, DWORD dwFlags);
            typedef HMODULE(WINAPI LoadLibraryExW_t)(LPCWSTR lpFileName, HANDLE hFile, DWORD dwFlags);
            typedef HMODULE(WINAPI GetModuleHandleA_t)(LPCSTR lpFileName);
            typedef HMODULE(WINAPI GetModuleHandleW_t)(LPCWSTR lpFileName);
            typedef BOOL(WINAPI GetModuleHandleExA_t)(DWORD dwFlags, LPCSTR lpFileName, HMODULE* phModule);
            typedef BOOL(WINAPI GetModuleHandleExW_t)(DWORD dwFlags, LPCWSTR lpFileName, HMODULE* phModule);

            LoadLibraryA_t* SystemLoadLibraryA;
            LoadLibraryW_t* SystemLoadLibraryW;
            LoadLibraryExA_t* SystemLoadLibraryExA;
            LoadLibraryExW_t* SystemLoadLibraryExW;
            GetModuleHandleA_t* SystemGetModuleHandleA;
            GetModuleHandleW_t* SystemGetModuleHandleW;
            GetModuleHandleExA_t* SystemGetModuleHandleExA;
            GetModuleHandleExW_t* SystemGetModuleHandleExW;

           
            path RedirectModuleFileName(string fileName) {
                // 1) Redirect system d3d9.dll
                string fileNameLower = fileName;
                string systemD3D9 = GetSystemFolder("d3d9").u8string();
                transform(fileNameLower.begin(), fileNameLower.end(), fileNameLower.begin(), tolower);
                transform(systemD3D9.begin(), systemD3D9.end(), systemD3D9.begin(), tolower);
                if (fileNameLower.find(systemD3D9) == 0) {
                    // It tries to load the system d3d9.dll, replace with proxy DLL
                    return ProxyAddon::Instance->GetFilePath();
                }

                // 2) Redirect third-party modules inside the Guild Wars 2 bin(64) folder
                // Temporarily disabled for now until a more reliable way can be found
                // (addons that manually check if a file exists won't work with this way)

                //string fileNameString = filePath.filename().u8string();
                //string filePathString = path(filePath).remove_filename().u8string();
                //if (filePathString.back() == path::preferred_separator) {
                //    filePathString.erase(filePathString.size() - 1);
                //}
                //string binPath = path(BIN_FOLDER).u8string();
                //if (binPath.back() == path::preferred_separator) {
                //    binPath.erase(binPath.size() - 1);
                //}
                //if (equal(binPath.rbegin(), binPath.rend(), filePathString.rbegin())) {
                //    // It tries to load a module from the bin(64) folder, replace with bin(64)/addons
                //    filePathString.replace(filePathString.size() - binPath.size(), binPath.size(), ADDONS_FOLDER);
                //    return filePathString / fileNameString;
                //}

                return path(fileName);
            }


            HMODULE WINAPI HookLoadLibraryEx(string fileName, HANDLE hFile, DWORD dwFlags) {
                path filePath(fileName);
                if (filePath.has_parent_path()) {
                    path newFilePath = RedirectModuleFileName(fileName);
                    if (filePath != newFilePath) {
                        GetLog()->info("Redirecting path to {0}", newFilePath.u8string());
                    }
                    filePath = newFilePath;
                }
                return SystemLoadLibraryExW(filePath.c_str(), hFile, dwFlags);
            }

            HMODULE WINAPI HookLoadLibraryExA(LPCSTR lpFileName, HANDLE hFile, DWORD dwFlags) {
                GetLog()->info("Attempting to load {0} by using LoadLibraryExA with flags 0x{1:X}", lpFileName != nullptr ? lpFileName : "NULL", dwFlags);
                return HookLoadLibraryEx(lpFileName, hFile, dwFlags);
            }

            HMODULE WINAPI HookLoadLibraryExW(LPCWSTR lpFileName, HANDLE hFile, DWORD dwFlags) {
                GetLog()->info("Attempting to load {0} by using LoadLibraryExW with flags 0x{1:X}", lpFileName != nullptr ? u8(lpFileName) : "NULL", dwFlags);
                return HookLoadLibraryEx(u8(lpFileName), hFile, dwFlags);
            }

            HMODULE WINAPI HookLoadLibraryA(LPCSTR lpFileName) {
                GetLog()->info("Attempting to load {0} by using LoadLibraryA", lpFileName != nullptr ? lpFileName : "NULL");
                return HookLoadLibraryEx(lpFileName, NULL, 0);
            }

            HMODULE WINAPI HookLoadLibraryW(LPCWSTR lpFileName) {
                GetLog()->info("Attempting to load {0} by using LoadLibraryW", lpFileName != nullptr ? u8(lpFileName) : "NULL");
                return HookLoadLibraryEx(u8(lpFileName), NULL, 0);
            }


            BOOL WINAPI HookGetModuleHandleEx(DWORD dwFlags, string fileName, HMODULE* phModule) {
                path filePath(fileName);
                if (filePath.has_parent_path()) {
                    path newFilePath = RedirectModuleFileName(fileName);
                    if (filePath != newFilePath) {
                        GetLog()->info("Redirecting path to {0}", newFilePath.u8string());
                    }
                    filePath = newFilePath;
                }
                return SystemGetModuleHandleExW(dwFlags, filePath.c_str(), phModule);
            }

            BOOL WINAPI HookGetModuleHandleEx(DWORD dwFlags, LPCVOID fileName, HMODULE* phModule) {
                return SystemGetModuleHandleExW(dwFlags, reinterpret_cast<LPCWSTR>(fileName), phModule);
            }

            BOOL WINAPI HookGetModuleHandleExA(DWORD dwFlags, LPCSTR lpFileName, HMODULE* phModule) {
                if ((dwFlags & GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS) > 0 || lpFileName == NULL) {
                    GetLog()->info("Attempting to get module at 0x{0:X} by using GetModuleHandleExA with flags 0x{1:X}", reinterpret_cast<LPCVOID>(lpFileName), dwFlags);
                    return HookGetModuleHandleEx(dwFlags, reinterpret_cast<LPCVOID>(lpFileName), phModule);
                }
                else {
                    GetLog()->info("Attempting to get module {0} by using GetModuleHandleExA with flags 0x{1:X}", lpFileName, dwFlags);
                    return HookGetModuleHandleEx(dwFlags, lpFileName, phModule);
                }
            }

            BOOL WINAPI HookGetModuleHandleExW(DWORD dwFlags, LPCWSTR lpFileName, HMODULE* phModule) {
                if ((dwFlags & GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS) > 0 || lpFileName == NULL) {
                    GetLog()->info("Attempting to get module at 0x{0:X} by using GetModuleHandleExW with flags 0x{1:X}", reinterpret_cast<LPCVOID>(lpFileName), dwFlags);
                    return HookGetModuleHandleEx(dwFlags, reinterpret_cast<LPCVOID>(lpFileName), phModule);
                }
                else {
                    GetLog()->info("Attempting to get module {0} by using GetModuleHandleExW with flags 0x{1:X}", u8(lpFileName), dwFlags);
                    return HookGetModuleHandleEx(dwFlags, u8(lpFileName), phModule);
                }
            }

            HMODULE WINAPI HookGetModuleHandleA(LPCSTR lpFileName) {
                GetLog()->info("Attempting to get module {0} by using GetModuleHandleA", lpFileName != NULL ? lpFileName : "NULL");
                HMODULE phModule;
                HookGetModuleHandleEx(0, lpFileName, &phModule);
                return phModule;
            }

            HMODULE WINAPI HookGetModuleHandleW(LPCWSTR lpFileName) {
                GetLog()->info("Attempting to get module {0} by using GetModuleHandleW", lpFileName != NULL ? u8(lpFileName) : "NULL");
                HMODULE phModule;
                HookGetModuleHandleEx(0, lpFileName, &phModule);
                return phModule;
            }

        }


        bool LegacyAddon::ApplySafeEnv(SafeEnvType envType) {
            if (envType == SafeEnvType::Initialization) {
                // Mask the exports of our proxy so that ReShade doesn't overwrite them
                HMODULE hProxyAddon = LoadLibrary(ProxyAddon::Instance->GetFilePath().c_str());
                BYTE* imageBase = reinterpret_cast<BYTE*>(hProxyAddon);
                IMAGE_NT_HEADERS* imageHeader = reinterpret_cast<IMAGE_NT_HEADERS*>(imageBase + reinterpret_cast<IMAGE_DOS_HEADER*>(imageBase)->e_lfanew);
                if (imageHeader->Signature == IMAGE_NT_SIGNATURE && imageHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size > 0) {
                    IMAGE_EXPORT_DIRECTORY* exportDir = reinterpret_cast<IMAGE_EXPORT_DIRECTORY*>(imageBase + imageHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);
                    if (exportDir->NumberOfFunctions > 0) {
                        // Set this to 0
                        GetLog()->info("Masked proxy DLL exports for legacy addon {0}", this->GetFileName(), exportDir->NumberOfFunctions);
                        this->proxyAddonNumberOfExports = exportDir->NumberOfFunctions;
                        DWORD oldProtection;
                        VirtualProtect(reinterpret_cast<LPVOID>(exportDir), sizeof(IMAGE_EXPORT_DIRECTORY), PAGE_EXECUTE_READWRITE, &oldProtection);
                        exportDir->NumberOfFunctions = 0;
                        VirtualProtect(reinterpret_cast<LPVOID>(exportDir), sizeof(IMAGE_EXPORT_DIRECTORY), oldProtection, &oldProtection);
                    }
                }
                FreeLibrary(hProxyAddon);
            }
            else if (envType == SafeEnvType::Loading) {
                // Make sure to save some states to restore later
                // In order for addons to hook into D3D9, they either do one of the following:
                //  - Overwrite the virtual function table of D3DDevice9
                //  - Overwrite the CPU instructions inside the functions of the D3DDevice9 object
                //  - Return a new object that wraps the D3DDevice9 object (already covered by our ProxyDirect3DDevice9 object)
                // We catch the most common cases here

                // Case 1: Overwritten VFT (ReShade, Gw2Hook)
                ProxyDirect3DDevice9 proxyDevice;
                this->proxyVft = GetVftD3DDevice9(&proxyDevice);
                GetLog()->info("Stored state of legacy addon {0} ProxyD3DDevice9 VFT", this->GetFileName());

                // Case 2: Overwritten CPU instructions
                //  a) minhook (GW2Mounts)
                //TODO: Implement this

            }

            // Redirect some WinAPI calls
            MH_STATUS mhStatus;
            mhStatus = MH_CreateHookEx(&LoadLibraryA, hooks::HookLoadLibraryA, &hooks::SystemLoadLibraryA);
            mhStatus = MH_CreateHookEx(&LoadLibraryW, hooks::HookLoadLibraryW, &hooks::SystemLoadLibraryW);
            mhStatus = MH_CreateHookEx(&LoadLibraryExA, hooks::HookLoadLibraryExA, &hooks::SystemLoadLibraryExA);
            mhStatus = MH_CreateHookEx(&LoadLibraryExW, hooks::HookLoadLibraryExW, &hooks::SystemLoadLibraryExW);
            mhStatus = MH_CreateHookEx(&GetModuleHandleA, hooks::HookGetModuleHandleA, &hooks::SystemGetModuleHandleA);
            mhStatus = MH_CreateHookEx(&GetModuleHandleW, hooks::HookGetModuleHandleW, &hooks::SystemGetModuleHandleW);
            mhStatus = MH_CreateHookEx(&GetModuleHandleExA, hooks::HookGetModuleHandleExA, &hooks::SystemGetModuleHandleExA);
            mhStatus = MH_CreateHookEx(&GetModuleHandleExW, hooks::HookGetModuleHandleExW, &hooks::SystemGetModuleHandleExW);
            mhStatus = MH_QueueEnableHook(&LoadLibraryA);
            mhStatus = MH_QueueEnableHook(&LoadLibraryW);
            mhStatus = MH_QueueEnableHook(&LoadLibraryExA);
            mhStatus = MH_QueueEnableHook(&LoadLibraryExW);
            mhStatus = MH_QueueEnableHook(&GetModuleHandleA);
            mhStatus = MH_QueueEnableHook(&GetModuleHandleW);
            mhStatus = MH_QueueEnableHook(&GetModuleHandleExA);
            mhStatus = MH_QueueEnableHook(&GetModuleHandleExW);
            mhStatus = MH_ApplyQueued();
            GetLog()->info("Redirected WinAPI functions for legacy addon {0}", this->GetFileName());
            return mhStatus == MH_OK;
        }

        bool LegacyAddon::RevertSafeEnv(SafeEnvType envType) {
            // Revert redirection of WinAPI calls
            MH_STATUS mhStatus;
            mhStatus = MH_QueueDisableHook(&LoadLibraryA);
            mhStatus = MH_QueueDisableHook(&LoadLibraryW);
            mhStatus = MH_QueueDisableHook(&LoadLibraryExA);
            mhStatus = MH_QueueDisableHook(&LoadLibraryExW);
            mhStatus = MH_QueueDisableHook(&GetModuleHandleA);
            mhStatus = MH_QueueDisableHook(&GetModuleHandleW);
            mhStatus = MH_QueueDisableHook(&GetModuleHandleExA);
            mhStatus = MH_QueueDisableHook(&GetModuleHandleExW);
            mhStatus = MH_ApplyQueued();
            mhStatus = MH_RemoveHook(&LoadLibraryA);
            mhStatus = MH_RemoveHook(&LoadLibraryW);
            mhStatus = MH_RemoveHook(&LoadLibraryExA);
            mhStatus = MH_RemoveHook(&LoadLibraryExW);
            mhStatus = MH_RemoveHook(&GetModuleHandleA);
            mhStatus = MH_RemoveHook(&GetModuleHandleW);
            mhStatus = MH_RemoveHook(&GetModuleHandleExA);
            mhStatus = MH_RemoveHook(&GetModuleHandleExW);
            hooks::SystemLoadLibraryA = nullptr;
            hooks::SystemLoadLibraryW = nullptr;
            hooks::SystemLoadLibraryExA = nullptr;
            hooks::SystemLoadLibraryExW = nullptr;
            hooks::SystemGetModuleHandleA = nullptr;
            hooks::SystemGetModuleHandleW = nullptr;
            hooks::SystemGetModuleHandleExA = nullptr;
            hooks::SystemGetModuleHandleExW = nullptr;
            GetLog()->info("Reverted redirected WinAPI functions for legacy addon {0}", this->GetFileName());

            if (envType == SafeEnvType::Initialization) {
                // Unmask the exports of our proxy so that ReShade doesn't overwrite them
                if (this->proxyAddonNumberOfExports > 0) {
                    HMODULE hProxyAddon = LoadLibrary(ProxyAddon::Instance->GetFilePath().c_str());
                    BYTE* imageBase = reinterpret_cast<BYTE*>(hProxyAddon);
                    IMAGE_NT_HEADERS* imageHeader = reinterpret_cast<IMAGE_NT_HEADERS*>(imageBase + reinterpret_cast<IMAGE_DOS_HEADER*>(imageBase)->e_lfanew);
                    if (imageHeader->Signature == IMAGE_NT_SIGNATURE && imageHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size > 0) {
                        IMAGE_EXPORT_DIRECTORY* exportDir = reinterpret_cast<IMAGE_EXPORT_DIRECTORY*>(imageBase + imageHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);
                        // Revert this from 0
                        DWORD oldProtection;
                        VirtualProtect(reinterpret_cast<LPVOID>(exportDir), sizeof(IMAGE_EXPORT_DIRECTORY), PAGE_EXECUTE_READWRITE, &oldProtection);
                        exportDir->NumberOfFunctions = this->proxyAddonNumberOfExports;
                        VirtualProtect(reinterpret_cast<LPVOID>(exportDir), sizeof(IMAGE_EXPORT_DIRECTORY), oldProtection, &oldProtection);
                        GetLog()->info("Unmasked proxy DLL exports for legacy addon {0}", this->GetFileName(), exportDir->NumberOfFunctions);
                    }
                    FreeLibrary(hProxyAddon);
                }
            }
            else if (envType == SafeEnvType::Loading) {
                // Make sure to restore the states from earlier, check ApplySafeEnv
                // This is only needed whenever the addon returns the same pointer as the proxied D3D9 device
                // (in addon terms: the proxied D3D9 device is what the addon believes what is the system D3D9 device,
                // the addon device is what the addon wraps or changed the system D3D9 and returns to the game)
                if (this->AddonD3DDevice9 == this->ProxyD3DDevice9) {
                    GetLog()->info("Detected overwrites in ProxyD3DDevice9 object for legacy addon {0}", this->GetFileName());
                    
                    // Wrap the addon device first
                    ProxyDirect3DDevice9* newAddonDev = new ProxyDirect3DDevice9(this->ProxyD3DDevice9);

                    // Case 1: Overwritten VFT (ReShade, Gw2Hook)
                    // We get the new VFT first and then restore the original VFT to the proxied device
                    D3DDevice9_vft vft = GetVftD3DDevice9(this->ProxyD3DDevice9);
                    SetVftD3DDevice9(this->ProxyD3DDevice9, this->proxyVft);
                    // Compare the vfts and copy the function pointers to the addon device
                    newAddonDev->FunctionAddresses.QueryInterface = vft.QueryInterface != this->proxyVft.QueryInterface ? reinterpret_cast<d3d9_types::QueryInterface_t*>(vft.QueryInterface) : nullptr;
                    newAddonDev->FunctionAddresses.AddRef = vft.AddRef != this->proxyVft.AddRef ? reinterpret_cast<d3d9_types::AddRef_t*>(vft.AddRef) : nullptr;
                    newAddonDev->FunctionAddresses.Release = vft.Release != this->proxyVft.Release ? reinterpret_cast<d3d9_types::Release_t*>(vft.Release) : nullptr;
                    newAddonDev->FunctionAddresses.TestCooperativeLevel = vft.TestCooperativeLevel != this->proxyVft.TestCooperativeLevel ? reinterpret_cast<d3d9_types::TestCooperativeLevel_t*>(vft.TestCooperativeLevel) : nullptr;
                    newAddonDev->FunctionAddresses.GetAvailableTextureMem = vft.GetAvailableTextureMem != this->proxyVft.GetAvailableTextureMem ? reinterpret_cast<d3d9_types::GetAvailableTextureMem_t*>(vft.GetAvailableTextureMem) : nullptr;
                    newAddonDev->FunctionAddresses.EvictManagedResources = vft.EvictManagedResources != this->proxyVft.EvictManagedResources ? reinterpret_cast<d3d9_types::EvictManagedResources_t*>(vft.EvictManagedResources) : nullptr;
                    newAddonDev->FunctionAddresses.GetDirect3D = vft.GetDirect3D != this->proxyVft.GetDirect3D ? reinterpret_cast<d3d9_types::GetDirect3D_t*>(vft.GetDirect3D) : nullptr;
                    newAddonDev->FunctionAddresses.GetDeviceCaps = vft.GetDeviceCaps != this->proxyVft.GetDeviceCaps ? reinterpret_cast<d3d9_types::GetDeviceCaps_t*>(vft.GetDeviceCaps) : nullptr;
                    newAddonDev->FunctionAddresses.GetDisplayMode = vft.GetDisplayMode != this->proxyVft.GetDisplayMode ? reinterpret_cast<d3d9_types::GetDisplayMode_t*>(vft.GetDisplayMode) : nullptr;
                    newAddonDev->FunctionAddresses.GetCreationParameters = vft.GetCreationParameters != this->proxyVft.GetCreationParameters ? reinterpret_cast<d3d9_types::GetCreationParameters_t*>(vft.GetCreationParameters) : nullptr;
                    newAddonDev->FunctionAddresses.SetCursorProperties = vft.SetCursorProperties != this->proxyVft.SetCursorProperties ? reinterpret_cast<d3d9_types::SetCursorProperties_t*>(vft.SetCursorProperties) : nullptr;
                    newAddonDev->FunctionAddresses.SetCursorPosition = vft.SetCursorPosition != this->proxyVft.SetCursorPosition ? reinterpret_cast<d3d9_types::SetCursorPosition_t*>(vft.SetCursorPosition) : nullptr;
                    newAddonDev->FunctionAddresses.ShowCursor = vft.ShowCursor != this->proxyVft.ShowCursor ? reinterpret_cast<d3d9_types::ShowCursor_t*>(vft.ShowCursor) : nullptr;
                    newAddonDev->FunctionAddresses.CreateAdditionalSwapChain = vft.CreateAdditionalSwapChain != this->proxyVft.CreateAdditionalSwapChain ? reinterpret_cast<d3d9_types::CreateAdditionalSwapChain_t*>(vft.CreateAdditionalSwapChain) : nullptr;
                    newAddonDev->FunctionAddresses.GetSwapChain = vft.GetSwapChain != this->proxyVft.GetSwapChain ? reinterpret_cast<d3d9_types::GetSwapChain_t*>(vft.GetSwapChain) : nullptr;
                    newAddonDev->FunctionAddresses.GetNumberOfSwapChains = vft.GetNumberOfSwapChains != this->proxyVft.GetNumberOfSwapChains ? reinterpret_cast<d3d9_types::GetNumberOfSwapChains_t*>(vft.GetNumberOfSwapChains) : nullptr;
                    newAddonDev->FunctionAddresses.Reset = vft.Reset != this->proxyVft.Reset ? reinterpret_cast<d3d9_types::Reset_t*>(vft.Reset) : nullptr;
                    newAddonDev->FunctionAddresses.Present = vft.Present != this->proxyVft.Present ? reinterpret_cast<d3d9_types::Present_t*>(vft.Present) : nullptr;
                    newAddonDev->FunctionAddresses.GetBackBuffer = vft.GetBackBuffer != this->proxyVft.GetBackBuffer ? reinterpret_cast<d3d9_types::GetBackBuffer_t*>(vft.GetBackBuffer) : nullptr;
                    newAddonDev->FunctionAddresses.GetRasterStatus = vft.GetRasterStatus != this->proxyVft.GetRasterStatus ? reinterpret_cast<d3d9_types::GetRasterStatus_t*>(vft.GetRasterStatus) : nullptr;
                    newAddonDev->FunctionAddresses.SetDialogBoxMode = vft.SetDialogBoxMode != this->proxyVft.SetDialogBoxMode ? reinterpret_cast<d3d9_types::SetDialogBoxMode_t*>(vft.SetDialogBoxMode) : nullptr;
                    newAddonDev->FunctionAddresses.SetGammaRamp = vft.SetGammaRamp != this->proxyVft.SetGammaRamp ? reinterpret_cast<d3d9_types::SetGammaRamp_t*>(vft.SetGammaRamp) : nullptr;
                    newAddonDev->FunctionAddresses.GetGammaRamp = vft.GetGammaRamp != this->proxyVft.GetGammaRamp ? reinterpret_cast<d3d9_types::GetGammaRamp_t*>(vft.GetGammaRamp) : nullptr;
                    newAddonDev->FunctionAddresses.CreateTexture = vft.CreateTexture != this->proxyVft.CreateTexture ? reinterpret_cast<d3d9_types::CreateTexture_t*>(vft.CreateTexture) : nullptr;
                    newAddonDev->FunctionAddresses.CreateVolumeTexture = vft.CreateVolumeTexture != this->proxyVft.CreateVolumeTexture ? reinterpret_cast<d3d9_types::CreateVolumeTexture_t*>(vft.CreateVolumeTexture) : nullptr;
                    newAddonDev->FunctionAddresses.CreateCubeTexture = vft.CreateCubeTexture != this->proxyVft.CreateCubeTexture ? reinterpret_cast<d3d9_types::CreateCubeTexture_t*>(vft.CreateCubeTexture) : nullptr;
                    newAddonDev->FunctionAddresses.CreateVertexBuffer = vft.CreateVertexBuffer != this->proxyVft.CreateVertexBuffer ? reinterpret_cast<d3d9_types::CreateVertexBuffer_t*>(vft.CreateVertexBuffer) : nullptr;
                    newAddonDev->FunctionAddresses.CreateIndexBuffer = vft.CreateIndexBuffer != this->proxyVft.CreateIndexBuffer ? reinterpret_cast<d3d9_types::CreateIndexBuffer_t*>(vft.CreateIndexBuffer) : nullptr;
                    newAddonDev->FunctionAddresses.CreateRenderTarget = vft.CreateRenderTarget != this->proxyVft.CreateRenderTarget ? reinterpret_cast<d3d9_types::CreateRenderTarget_t*>(vft.CreateRenderTarget) : nullptr;
                    newAddonDev->FunctionAddresses.CreateDepthStencilSurface = vft.CreateDepthStencilSurface != this->proxyVft.CreateDepthStencilSurface ? reinterpret_cast<d3d9_types::CreateDepthStencilSurface_t*>(vft.CreateDepthStencilSurface) : nullptr;
                    newAddonDev->FunctionAddresses.UpdateSurface = vft.UpdateSurface != this->proxyVft.UpdateSurface ? reinterpret_cast<d3d9_types::UpdateSurface_t*>(vft.UpdateSurface) : nullptr;
                    newAddonDev->FunctionAddresses.UpdateTexture = vft.UpdateTexture != this->proxyVft.UpdateTexture ? reinterpret_cast<d3d9_types::UpdateTexture_t*>(vft.UpdateTexture) : nullptr;
                    newAddonDev->FunctionAddresses.GetRenderTargetData = vft.GetRenderTargetData != this->proxyVft.GetRenderTargetData ? reinterpret_cast<d3d9_types::GetRenderTargetData_t*>(vft.GetRenderTargetData) : nullptr;
                    newAddonDev->FunctionAddresses.GetFrontBufferData = vft.GetFrontBufferData != this->proxyVft.GetFrontBufferData ? reinterpret_cast<d3d9_types::GetFrontBufferData_t*>(vft.GetFrontBufferData) : nullptr;
                    newAddonDev->FunctionAddresses.StretchRect = vft.StretchRect != this->proxyVft.StretchRect ? reinterpret_cast<d3d9_types::StretchRect_t*>(vft.StretchRect) : nullptr;
                    newAddonDev->FunctionAddresses.ColorFill = vft.ColorFill != this->proxyVft.ColorFill ? reinterpret_cast<d3d9_types::ColorFill_t*>(vft.ColorFill) : nullptr;
                    newAddonDev->FunctionAddresses.CreateOffscreenPlainSurface = vft.CreateOffscreenPlainSurface != this->proxyVft.CreateOffscreenPlainSurface ? reinterpret_cast<d3d9_types::CreateOffscreenPlainSurface_t*>(vft.CreateOffscreenPlainSurface) : nullptr;
                    newAddonDev->FunctionAddresses.SetRenderTarget = vft.SetRenderTarget != this->proxyVft.SetRenderTarget ? reinterpret_cast<d3d9_types::SetRenderTarget_t*>(vft.SetRenderTarget) : nullptr;
                    newAddonDev->FunctionAddresses.GetRenderTarget = vft.GetRenderTarget != this->proxyVft.GetRenderTarget ? reinterpret_cast<d3d9_types::GetRenderTarget_t*>(vft.GetRenderTarget) : nullptr;
                    newAddonDev->FunctionAddresses.SetDepthStencilSurface = vft.SetDepthStencilSurface != this->proxyVft.SetDepthStencilSurface ? reinterpret_cast<d3d9_types::SetDepthStencilSurface_t*>(vft.SetDepthStencilSurface) : nullptr;
                    newAddonDev->FunctionAddresses.GetDepthStencilSurface = vft.GetDepthStencilSurface != this->proxyVft.GetDepthStencilSurface ? reinterpret_cast<d3d9_types::GetDepthStencilSurface_t*>(vft.GetDepthStencilSurface) : nullptr;
                    newAddonDev->FunctionAddresses.BeginScene = vft.BeginScene != this->proxyVft.BeginScene ? reinterpret_cast<d3d9_types::BeginScene_t*>(vft.BeginScene) : nullptr;
                    newAddonDev->FunctionAddresses.EndScene = vft.EndScene != this->proxyVft.EndScene ? reinterpret_cast<d3d9_types::EndScene_t*>(vft.EndScene) : nullptr;
                    newAddonDev->FunctionAddresses.Clear = vft.Clear != this->proxyVft.Clear ? reinterpret_cast<d3d9_types::Clear_t*>(vft.Clear) : nullptr;
                    newAddonDev->FunctionAddresses.SetTransform = vft.SetTransform != this->proxyVft.SetTransform ? reinterpret_cast<d3d9_types::SetTransform_t*>(vft.SetTransform) : nullptr;
                    newAddonDev->FunctionAddresses.GetTransform = vft.GetTransform != this->proxyVft.GetTransform ? reinterpret_cast<d3d9_types::GetTransform_t*>(vft.GetTransform) : nullptr;
                    newAddonDev->FunctionAddresses.MultiplyTransform = vft.MultiplyTransform != this->proxyVft.MultiplyTransform ? reinterpret_cast<d3d9_types::MultiplyTransform_t*>(vft.MultiplyTransform) : nullptr;
                    newAddonDev->FunctionAddresses.SetViewport = vft.SetViewport != this->proxyVft.SetViewport ? reinterpret_cast<d3d9_types::SetViewport_t*>(vft.SetViewport) : nullptr;
                    newAddonDev->FunctionAddresses.GetViewport = vft.GetViewport != this->proxyVft.GetViewport ? reinterpret_cast<d3d9_types::GetViewport_t*>(vft.GetViewport) : nullptr;
                    newAddonDev->FunctionAddresses.SetMaterial = vft.SetMaterial != this->proxyVft.SetMaterial ? reinterpret_cast<d3d9_types::SetMaterial_t*>(vft.SetMaterial) : nullptr;
                    newAddonDev->FunctionAddresses.GetMaterial = vft.GetMaterial != this->proxyVft.GetMaterial ? reinterpret_cast<d3d9_types::GetMaterial_t*>(vft.GetMaterial) : nullptr;
                    newAddonDev->FunctionAddresses.SetLight = vft.SetLight != this->proxyVft.SetLight ? reinterpret_cast<d3d9_types::SetLight_t*>(vft.SetLight) : nullptr;
                    newAddonDev->FunctionAddresses.GetLight = vft.GetLight != this->proxyVft.GetLight ? reinterpret_cast<d3d9_types::GetLight_t*>(vft.GetLight) : nullptr;
                    newAddonDev->FunctionAddresses.LightEnable = vft.LightEnable != this->proxyVft.LightEnable ? reinterpret_cast<d3d9_types::LightEnable_t*>(vft.LightEnable) : nullptr;
                    newAddonDev->FunctionAddresses.GetLightEnable = vft.GetLightEnable != this->proxyVft.GetLightEnable ? reinterpret_cast<d3d9_types::GetLightEnable_t*>(vft.GetLightEnable) : nullptr;
                    newAddonDev->FunctionAddresses.SetClipPlane = vft.SetClipPlane != this->proxyVft.SetClipPlane ? reinterpret_cast<d3d9_types::SetClipPlane_t*>(vft.SetClipPlane) : nullptr;
                    newAddonDev->FunctionAddresses.GetClipPlane = vft.GetClipPlane != this->proxyVft.GetClipPlane ? reinterpret_cast<d3d9_types::GetClipPlane_t*>(vft.GetClipPlane) : nullptr;
                    newAddonDev->FunctionAddresses.SetRenderState = vft.SetRenderState != this->proxyVft.SetRenderState ? reinterpret_cast<d3d9_types::SetRenderState_t*>(vft.SetRenderState) : nullptr;
                    newAddonDev->FunctionAddresses.GetRenderState = vft.GetRenderState != this->proxyVft.GetRenderState ? reinterpret_cast<d3d9_types::GetRenderState_t*>(vft.GetRenderState) : nullptr;
                    newAddonDev->FunctionAddresses.CreateStateBlock = vft.CreateStateBlock != this->proxyVft.CreateStateBlock ? reinterpret_cast<d3d9_types::CreateStateBlock_t*>(vft.CreateStateBlock) : nullptr;
                    newAddonDev->FunctionAddresses.BeginStateBlock = vft.BeginStateBlock != this->proxyVft.BeginStateBlock ? reinterpret_cast<d3d9_types::BeginStateBlock_t*>(vft.BeginStateBlock) : nullptr;
                    newAddonDev->FunctionAddresses.EndStateBlock = vft.EndStateBlock != this->proxyVft.EndStateBlock ? reinterpret_cast<d3d9_types::EndStateBlock_t*>(vft.EndStateBlock) : nullptr;
                    newAddonDev->FunctionAddresses.SetClipStatus = vft.SetClipStatus != this->proxyVft.SetClipStatus ? reinterpret_cast<d3d9_types::SetClipStatus_t*>(vft.SetClipStatus) : nullptr;
                    newAddonDev->FunctionAddresses.GetClipStatus = vft.GetClipStatus != this->proxyVft.GetClipStatus ? reinterpret_cast<d3d9_types::GetClipStatus_t*>(vft.GetClipStatus) : nullptr;
                    newAddonDev->FunctionAddresses.GetTexture = vft.GetTexture != this->proxyVft.GetTexture ? reinterpret_cast<d3d9_types::GetTexture_t*>(vft.GetTexture) : nullptr;
                    newAddonDev->FunctionAddresses.SetTexture = vft.SetTexture != this->proxyVft.SetTexture ? reinterpret_cast<d3d9_types::SetTexture_t*>(vft.SetTexture) : nullptr;
                    newAddonDev->FunctionAddresses.GetTextureStageState = vft.GetTextureStageState != this->proxyVft.GetTextureStageState ? reinterpret_cast<d3d9_types::GetTextureStageState_t*>(vft.GetTextureStageState) : nullptr;
                    newAddonDev->FunctionAddresses.SetTextureStageState = vft.SetTextureStageState != this->proxyVft.SetTextureStageState ? reinterpret_cast<d3d9_types::SetTextureStageState_t*>(vft.SetTextureStageState) : nullptr;
                    newAddonDev->FunctionAddresses.GetSamplerState = vft.GetSamplerState != this->proxyVft.GetSamplerState ? reinterpret_cast<d3d9_types::GetSamplerState_t*>(vft.GetSamplerState) : nullptr;
                    newAddonDev->FunctionAddresses.SetSamplerState = vft.SetSamplerState != this->proxyVft.SetSamplerState ? reinterpret_cast<d3d9_types::SetSamplerState_t*>(vft.SetSamplerState) : nullptr;
                    newAddonDev->FunctionAddresses.ValidateDevice = vft.ValidateDevice != this->proxyVft.ValidateDevice ? reinterpret_cast<d3d9_types::ValidateDevice_t*>(vft.ValidateDevice) : nullptr;
                    newAddonDev->FunctionAddresses.SetPaletteEntries = vft.SetPaletteEntries != this->proxyVft.SetPaletteEntries ? reinterpret_cast<d3d9_types::SetPaletteEntries_t*>(vft.SetPaletteEntries) : nullptr;
                    newAddonDev->FunctionAddresses.GetPaletteEntries = vft.GetPaletteEntries != this->proxyVft.GetPaletteEntries ? reinterpret_cast<d3d9_types::GetPaletteEntries_t*>(vft.GetPaletteEntries) : nullptr;
                    newAddonDev->FunctionAddresses.SetCurrentTexturePalette = vft.SetCurrentTexturePalette != this->proxyVft.SetCurrentTexturePalette ? reinterpret_cast<d3d9_types::SetCurrentTexturePalette_t*>(vft.SetCurrentTexturePalette) : nullptr;
                    newAddonDev->FunctionAddresses.GetCurrentTexturePalette = vft.GetCurrentTexturePalette != this->proxyVft.GetCurrentTexturePalette ? reinterpret_cast<d3d9_types::GetCurrentTexturePalette_t*>(vft.GetCurrentTexturePalette) : nullptr;
                    newAddonDev->FunctionAddresses.SetScissorRect = vft.SetScissorRect != this->proxyVft.SetScissorRect ? reinterpret_cast<d3d9_types::SetScissorRect_t*>(vft.SetScissorRect) : nullptr;
                    newAddonDev->FunctionAddresses.GetScissorRect = vft.GetScissorRect != this->proxyVft.GetScissorRect ? reinterpret_cast<d3d9_types::GetScissorRect_t*>(vft.GetScissorRect) : nullptr;
                    newAddonDev->FunctionAddresses.SetSoftwareVertexProcessing = vft.SetSoftwareVertexProcessing != this->proxyVft.SetSoftwareVertexProcessing ? reinterpret_cast<d3d9_types::SetSoftwareVertexProcessing_t*>(vft.SetSoftwareVertexProcessing) : nullptr;
                    newAddonDev->FunctionAddresses.GetSoftwareVertexProcessing = vft.GetSoftwareVertexProcessing != this->proxyVft.GetSoftwareVertexProcessing ? reinterpret_cast<d3d9_types::GetSoftwareVertexProcessing_t*>(vft.GetSoftwareVertexProcessing) : nullptr;
                    newAddonDev->FunctionAddresses.SetNPatchMode = vft.SetNPatchMode != this->proxyVft.SetNPatchMode ? reinterpret_cast<d3d9_types::SetNPatchMode_t*>(vft.SetNPatchMode) : nullptr;
                    newAddonDev->FunctionAddresses.GetNPatchMode = vft.GetNPatchMode != this->proxyVft.GetNPatchMode ? reinterpret_cast<d3d9_types::GetNPatchMode_t*>(vft.GetNPatchMode) : nullptr;
                    newAddonDev->FunctionAddresses.DrawPrimitive = vft.DrawPrimitive != this->proxyVft.DrawPrimitive ? reinterpret_cast<d3d9_types::DrawPrimitive_t*>(vft.DrawPrimitive) : nullptr;
                    newAddonDev->FunctionAddresses.DrawIndexedPrimitive = vft.DrawIndexedPrimitive != this->proxyVft.DrawIndexedPrimitive ? reinterpret_cast<d3d9_types::DrawIndexedPrimitive_t*>(vft.DrawIndexedPrimitive) : nullptr;
                    newAddonDev->FunctionAddresses.DrawPrimitiveUP = vft.DrawPrimitiveUP != this->proxyVft.DrawPrimitiveUP ? reinterpret_cast<d3d9_types::DrawPrimitiveUP_t*>(vft.DrawPrimitiveUP) : nullptr;
                    newAddonDev->FunctionAddresses.DrawIndexedPrimitiveUP = vft.DrawIndexedPrimitiveUP != this->proxyVft.DrawIndexedPrimitiveUP ? reinterpret_cast<d3d9_types::DrawIndexedPrimitiveUP_t*>(vft.DrawIndexedPrimitiveUP) : nullptr;
                    newAddonDev->FunctionAddresses.ProcessVertices = vft.ProcessVertices != this->proxyVft.ProcessVertices ? reinterpret_cast<d3d9_types::ProcessVertices_t*>(vft.ProcessVertices) : nullptr;
                    newAddonDev->FunctionAddresses.CreateVertexDeclaration = vft.CreateVertexDeclaration != this->proxyVft.CreateVertexDeclaration ? reinterpret_cast<d3d9_types::CreateVertexDeclaration_t*>(vft.CreateVertexDeclaration) : nullptr;
                    newAddonDev->FunctionAddresses.SetVertexDeclaration = vft.SetVertexDeclaration != this->proxyVft.SetVertexDeclaration ? reinterpret_cast<d3d9_types::SetVertexDeclaration_t*>(vft.SetVertexDeclaration) : nullptr;
                    newAddonDev->FunctionAddresses.GetVertexDeclaration = vft.GetVertexDeclaration != this->proxyVft.GetVertexDeclaration ? reinterpret_cast<d3d9_types::GetVertexDeclaration_t*>(vft.GetVertexDeclaration) : nullptr;
                    newAddonDev->FunctionAddresses.SetFVF = vft.SetFVF != this->proxyVft.SetFVF ? reinterpret_cast<d3d9_types::SetFVF_t*>(vft.SetFVF) : nullptr;
                    newAddonDev->FunctionAddresses.GetFVF = vft.GetFVF != this->proxyVft.GetFVF ? reinterpret_cast<d3d9_types::GetFVF_t*>(vft.GetFVF) : nullptr;
                    newAddonDev->FunctionAddresses.CreateVertexShader = vft.CreateVertexShader != this->proxyVft.CreateVertexShader ? reinterpret_cast<d3d9_types::CreateVertexShader_t*>(vft.CreateVertexShader) : nullptr;
                    newAddonDev->FunctionAddresses.SetVertexShader = vft.SetVertexShader != this->proxyVft.SetVertexShader ? reinterpret_cast<d3d9_types::SetVertexShader_t*>(vft.SetVertexShader) : nullptr;
                    newAddonDev->FunctionAddresses.GetVertexShader = vft.GetVertexShader != this->proxyVft.GetVertexShader ? reinterpret_cast<d3d9_types::GetVertexShader_t*>(vft.GetVertexShader) : nullptr;
                    newAddonDev->FunctionAddresses.SetVertexShaderConstantF = vft.SetVertexShaderConstantF != this->proxyVft.SetVertexShaderConstantF ? reinterpret_cast<d3d9_types::SetVertexShaderConstantF_t*>(vft.SetVertexShaderConstantF) : nullptr;
                    newAddonDev->FunctionAddresses.GetVertexShaderConstantF = vft.GetVertexShaderConstantF != this->proxyVft.GetVertexShaderConstantF ? reinterpret_cast<d3d9_types::GetVertexShaderConstantF_t*>(vft.GetVertexShaderConstantF) : nullptr;
                    newAddonDev->FunctionAddresses.SetVertexShaderConstantI = vft.SetVertexShaderConstantI != this->proxyVft.SetVertexShaderConstantI ? reinterpret_cast<d3d9_types::SetVertexShaderConstantI_t*>(vft.SetVertexShaderConstantI) : nullptr;
                    newAddonDev->FunctionAddresses.GetVertexShaderConstantI = vft.GetVertexShaderConstantI != this->proxyVft.GetVertexShaderConstantI ? reinterpret_cast<d3d9_types::GetVertexShaderConstantI_t*>(vft.GetVertexShaderConstantI) : nullptr;
                    newAddonDev->FunctionAddresses.SetVertexShaderConstantB = vft.SetVertexShaderConstantB != this->proxyVft.SetVertexShaderConstantB ? reinterpret_cast<d3d9_types::SetVertexShaderConstantB_t*>(vft.SetVertexShaderConstantB) : nullptr;
                    newAddonDev->FunctionAddresses.GetVertexShaderConstantB = vft.GetVertexShaderConstantB != this->proxyVft.GetVertexShaderConstantB ? reinterpret_cast<d3d9_types::GetVertexShaderConstantB_t*>(vft.GetVertexShaderConstantB) : nullptr;
                    newAddonDev->FunctionAddresses.SetStreamSource = vft.SetStreamSource != this->proxyVft.SetStreamSource ? reinterpret_cast<d3d9_types::SetStreamSource_t*>(vft.SetStreamSource) : nullptr;
                    newAddonDev->FunctionAddresses.GetStreamSource = vft.GetStreamSource != this->proxyVft.GetStreamSource ? reinterpret_cast<d3d9_types::GetStreamSource_t*>(vft.GetStreamSource) : nullptr;
                    newAddonDev->FunctionAddresses.SetStreamSourceFreq = vft.SetStreamSourceFreq != this->proxyVft.SetStreamSourceFreq ? reinterpret_cast<d3d9_types::SetStreamSourceFreq_t*>(vft.SetStreamSourceFreq) : nullptr;
                    newAddonDev->FunctionAddresses.GetStreamSourceFreq = vft.GetStreamSourceFreq != this->proxyVft.GetStreamSourceFreq ? reinterpret_cast<d3d9_types::GetStreamSourceFreq_t*>(vft.GetStreamSourceFreq) : nullptr;
                    newAddonDev->FunctionAddresses.SetIndices = vft.SetIndices != this->proxyVft.SetIndices ? reinterpret_cast<d3d9_types::SetIndices_t*>(vft.SetIndices) : nullptr;
                    newAddonDev->FunctionAddresses.GetIndices = vft.GetIndices != this->proxyVft.GetIndices ? reinterpret_cast<d3d9_types::GetIndices_t*>(vft.GetIndices) : nullptr;
                    newAddonDev->FunctionAddresses.CreatePixelShader = vft.CreatePixelShader != this->proxyVft.CreatePixelShader ? reinterpret_cast<d3d9_types::CreatePixelShader_t*>(vft.CreatePixelShader) : nullptr;
                    newAddonDev->FunctionAddresses.SetPixelShader = vft.SetPixelShader != this->proxyVft.SetPixelShader ? reinterpret_cast<d3d9_types::SetPixelShader_t*>(vft.SetPixelShader) : nullptr;
                    newAddonDev->FunctionAddresses.GetPixelShader = vft.GetPixelShader != this->proxyVft.GetPixelShader ? reinterpret_cast<d3d9_types::GetPixelShader_t*>(vft.GetPixelShader) : nullptr;
                    newAddonDev->FunctionAddresses.SetPixelShaderConstantF = vft.SetPixelShaderConstantF != this->proxyVft.SetPixelShaderConstantF ? reinterpret_cast<d3d9_types::SetPixelShaderConstantF_t*>(vft.SetPixelShaderConstantF) : nullptr;
                    newAddonDev->FunctionAddresses.GetPixelShaderConstantF = vft.GetPixelShaderConstantF != this->proxyVft.GetPixelShaderConstantF ? reinterpret_cast<d3d9_types::GetPixelShaderConstantF_t*>(vft.GetPixelShaderConstantF) : nullptr;
                    newAddonDev->FunctionAddresses.SetPixelShaderConstantI = vft.SetPixelShaderConstantI != this->proxyVft.SetPixelShaderConstantI ? reinterpret_cast<d3d9_types::SetPixelShaderConstantI_t*>(vft.SetPixelShaderConstantI) : nullptr;
                    newAddonDev->FunctionAddresses.GetPixelShaderConstantI = vft.GetPixelShaderConstantI != this->proxyVft.GetPixelShaderConstantI ? reinterpret_cast<d3d9_types::GetPixelShaderConstantI_t*>(vft.GetPixelShaderConstantI) : nullptr;
                    newAddonDev->FunctionAddresses.SetPixelShaderConstantB = vft.SetPixelShaderConstantB != this->proxyVft.SetPixelShaderConstantB ? reinterpret_cast<d3d9_types::SetPixelShaderConstantB_t*>(vft.SetPixelShaderConstantB) : nullptr;
                    newAddonDev->FunctionAddresses.GetPixelShaderConstantB = vft.GetPixelShaderConstantB != this->proxyVft.GetPixelShaderConstantB ? reinterpret_cast<d3d9_types::GetPixelShaderConstantB_t*>(vft.GetPixelShaderConstantB) : nullptr;
                    newAddonDev->FunctionAddresses.DrawRectPatch = vft.DrawRectPatch != this->proxyVft.DrawRectPatch ? reinterpret_cast<d3d9_types::DrawRectPatch_t*>(vft.DrawRectPatch) : nullptr;
                    newAddonDev->FunctionAddresses.DrawTriPatch = vft.DrawTriPatch != this->proxyVft.DrawTriPatch ? reinterpret_cast<d3d9_types::DrawTriPatch_t*>(vft.DrawTriPatch) : nullptr;
                    newAddonDev->FunctionAddresses.DeletePatch = vft.DeletePatch != this->proxyVft.DeletePatch ? reinterpret_cast<d3d9_types::DeletePatch_t*>(vft.DeletePatch) : nullptr;
                    newAddonDev->FunctionAddresses.CreateQuery = vft.CreateQuery != this->proxyVft.CreateQuery ? reinterpret_cast<d3d9_types::CreateQuery_t*>(vft.CreateQuery) : nullptr;
                    newAddonDev->FunctionAddresses.SetConvolutionMonoKernel = vft.SetConvolutionMonoKernel != this->proxyVft.SetConvolutionMonoKernel ? reinterpret_cast<d3d9_types::SetConvolutionMonoKernel_t*>(vft.SetConvolutionMonoKernel) : nullptr;
                    newAddonDev->FunctionAddresses.ComposeRects = vft.ComposeRects != this->proxyVft.ComposeRects ? reinterpret_cast<d3d9_types::ComposeRects_t*>(vft.ComposeRects) : nullptr;
                    newAddonDev->FunctionAddresses.PresentEx = vft.PresentEx != this->proxyVft.PresentEx ? reinterpret_cast<d3d9_types::PresentEx_t*>(vft.PresentEx) : nullptr;
                    newAddonDev->FunctionAddresses.GetGPUThreadPriority = vft.GetGPUThreadPriority != this->proxyVft.GetGPUThreadPriority ? reinterpret_cast<d3d9_types::GetGPUThreadPriority_t*>(vft.GetGPUThreadPriority) : nullptr;
                    newAddonDev->FunctionAddresses.SetGPUThreadPriority = vft.SetGPUThreadPriority != this->proxyVft.SetGPUThreadPriority ? reinterpret_cast<d3d9_types::SetGPUThreadPriority_t*>(vft.SetGPUThreadPriority) : nullptr;
                    newAddonDev->FunctionAddresses.WaitForVBlank = vft.WaitForVBlank != this->proxyVft.WaitForVBlank ? reinterpret_cast<d3d9_types::WaitForVBlank_t*>(vft.WaitForVBlank) : nullptr;
                    newAddonDev->FunctionAddresses.CheckResourceResidency = vft.CheckResourceResidency != this->proxyVft.CheckResourceResidency ? reinterpret_cast<d3d9_types::CheckResourceResidency_t*>(vft.CheckResourceResidency) : nullptr;
                    newAddonDev->FunctionAddresses.SetMaximumFrameLatency = vft.SetMaximumFrameLatency != this->proxyVft.SetMaximumFrameLatency ? reinterpret_cast<d3d9_types::SetMaximumFrameLatency_t*>(vft.SetMaximumFrameLatency) : nullptr;
                    newAddonDev->FunctionAddresses.GetMaximumFrameLatency = vft.GetMaximumFrameLatency != this->proxyVft.GetMaximumFrameLatency ? reinterpret_cast<d3d9_types::GetMaximumFrameLatency_t*>(vft.GetMaximumFrameLatency) : nullptr;
                    newAddonDev->FunctionAddresses.CheckDeviceState = vft.CheckDeviceState != this->proxyVft.CheckDeviceState ? reinterpret_cast<d3d9_types::CheckDeviceState_t*>(vft.CheckDeviceState) : nullptr;
                    newAddonDev->FunctionAddresses.CreateRenderTargetEx = vft.CreateRenderTargetEx != this->proxyVft.CreateRenderTargetEx ? reinterpret_cast<d3d9_types::CreateRenderTargetEx_t*>(vft.CreateRenderTargetEx) : nullptr;
                    newAddonDev->FunctionAddresses.CreateOffscreenPlainSurfaceEx = vft.CreateOffscreenPlainSurfaceEx != this->proxyVft.CreateOffscreenPlainSurfaceEx ? reinterpret_cast<d3d9_types::CreateOffscreenPlainSurfaceEx_t*>(vft.CreateOffscreenPlainSurfaceEx) : nullptr;
                    newAddonDev->FunctionAddresses.CreateDepthStencilSurfaceEx = vft.CreateDepthStencilSurfaceEx != this->proxyVft.CreateDepthStencilSurfaceEx ? reinterpret_cast<d3d9_types::CreateDepthStencilSurfaceEx_t*>(vft.CreateDepthStencilSurfaceEx) : nullptr;
                    newAddonDev->FunctionAddresses.ResetEx = vft.ResetEx != this->proxyVft.ResetEx ? reinterpret_cast<d3d9_types::ResetEx_t*>(vft.ResetEx) : nullptr;
                    newAddonDev->FunctionAddresses.GetDisplayModeEx = vft.GetDisplayModeEx != this->proxyVft.GetDisplayModeEx ? reinterpret_cast<d3d9_types::GetDisplayModeEx_t*>(vft.GetDisplayModeEx) : nullptr;

                    // Case 2: Overwritten CPU instructions
                    //  a) minhook (GW2Mounts)
                    //TODO: Implement this

                    GetLog()->info("Restored ProxyD3DDevice9 state for legacy addon {0}", this->GetFileName());
                    GetLog()->info("Legacy addon {0} new AddonD3DDevice9: 0x{1:X}", this->GetFileName(), reinterpret_cast<size_t>(newAddonDev));
                    this->AddonD3DDevice9 = newAddonDev;
                }
            }

            return mhStatus == MH_OK;
        }

        void LegacyAddon::SetNextAddonChain(LegacyAddon* addon) {
            if (this->ProxyD3DDevice9 != nullptr) {
                this->ProxyD3DDevice9->SetDevice(addon != nullptr ? addon->AddonD3DDevice9 : this->D3DDevice9->GetSystemDevice());
            }
        }


        bool LegacyAddon::Initialize() {
            // Make sure to load the proxy addon beforehand
            if (ProxyAddon::Instance == nullptr) {
                return false;
            }
            ProxyAddon::Instance->Load();

            if (!this->ApplySafeEnv(SafeEnvType::Initialization)) {
                this->ChangeState(AddonState::ErroredState);
                GetLog()->error("Could not initialize legacy addon {0}: Redirecting WinAPI functions failed", this->GetFileName());
                return false;
            }

            HMODULE h = hooks::SystemLoadLibraryExW(this->GetFilePath().c_str(), NULL, 0);
            if (h != NULL) {
                this->addonHandle = h;
                this->AddonCreate = reinterpret_cast<Direct3DCreate9_t*>(GetProcAddress(h, "Direct3DCreate9"));
            }
            else {
                this->ChangeState(AddonState::ErroredState);
                GetLog()->error("Could not initialize legacy addon {0}: Library handle is empty", this->GetFileName());
            }

            if (!this->RevertSafeEnv(SafeEnvType::Initialization)) {
                this->ChangeState(AddonState::ErroredState);
                GetLog()->error("Could not initialize legacy addon {0}: Reverting WinAPI functions failed", this->GetFileName());
            }

            return this->GetState() != AddonState::ErroredState;
        }

        bool LegacyAddon::Uninitialize() {
            this->Unload();
            
            this->AddonCreate = nullptr;

            // Make sure that a legacy addon doesn't leave itself dangling
            while (!FreeLibrary(this->addonHandle));
            this->addonHandle = NULL;

            return true;
        }

        bool LegacyAddon::Load() {
            if (this->AddonCreate == nullptr) {
                return false;
            }

            if (this->GetState() != AddonState::UnloadedState) {
                return false;
            }

            this->ChangeState(AddonState::LoadingState);

            if (!this->ApplySafeEnv(SafeEnvType::Loading)) {
                this->ChangeState(AddonState::ErroredState);
                GetLog()->error("Could not initialize legacy addon {0}: Redirecting WinAPI functions failed", this->GetFileName());
                return false;
            }

            // Make sure the addon is created and loaded properly.
            // At this point, the addon we are loading should use a reference to our proxy D3D9 DLL,
            // which in turn simulates the CreateDevice call to use an already created device.
            // This way we make sure it's not creating a duplicate device, but instead using the one we already have.
            this->AddonD3D9 = this->AddonCreate(this->D3D9SdkVersion);
            GetLog()->info("Legacy addon {0} AddonD3D9: 0x{1:X}", this->GetFileName(), reinterpret_cast<size_t>(this->AddonD3D9));
            Direct3DDevice9Information deviceInfo = GetGlobalDeviceInformation();
            ProxyAddon::Instance->LastProxiedDevice = nullptr; 
            HRESULT result = this->AddonD3D9->CreateDevice(deviceInfo.Adapter, deviceInfo.DeviceType, deviceInfo.hFocusWindow, deviceInfo.BehaviorFlags, &deviceInfo.PresentationParameters, &this->AddonD3DDevice9);
            GetLog()->info("Legacy addon {0} AddonD3DDevice9: 0x{1:X}", this->GetFileName(), reinterpret_cast<size_t>(this->AddonD3DDevice9));
            if (result != D3D_OK) {
                this->ChangeState(AddonState::ErroredState);
                GetLog()->error("Could not initialize legacy addon {0}: Obtaining the device failed", this->GetFileName());
            }
            this->ProxyD3DDevice9 = ProxyAddon::Instance->LastProxiedDevice;
            GetLog()->info("Legacy addon {0} ProxyD3DDevice9: 0x{1:X}", this->GetFileName(), reinterpret_cast<size_t>(this->ProxyD3DDevice9));

            if (!this->RevertSafeEnv(SafeEnvType::Loading)) {
                this->ChangeState(AddonState::ErroredState);
                GetLog()->error("Could not initialize legacy addon {0}: Reverting WinAPI functions failed", this->GetFileName());
            }

            if (this->GetState() == AddonState::ErroredState) {
                return false;
            }

            this->ChangeState(AddonState::LoadedState);
            ResetLegacyAddonChain();
            return true;
        }

        bool LegacyAddon::Unload() {
            if (this->GetState() != AddonState::LoadedState) {
                return false;
            }
            this->ChangeState(AddonState::DeactivatedOnRestartState);
            return true;
        }

    }
}
