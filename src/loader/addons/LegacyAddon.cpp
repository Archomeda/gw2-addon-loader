#include "LegacyAddon.h"
#include <algorithm>
#include <filesystem>
#include "addons_manager.h"
#include "ProxyAddon.h"
#include "../minhook.h"
#include "../globals.h"
#include "../log.h"
#include "../disasm/opcodes.h"
#include "../utils/encoding.h"
#include "../utils/file.h"

using namespace std;
using namespace std::experimental::filesystem;
using namespace loader::disasm;
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
                //  - Overwrite the virtual table of D3DDevice9
                //  - Overwrite the CPU instructions inside the functions of the D3DDevice9 object
                //  - Return a new object that wraps the D3DDevice9 object (already covered by our ProxyDirect3DDevice9 object)
                // We catch the most common cases here

                // Case 1: Overwritten virtual table (ReShade, Gw2Hook)
                ProxyDirect3DDevice9 proxyDevice;
                this->proxyVtbl = GetD3DDevice9Vtbl(&proxyDevice);
                GetLog()->info("Stored state of legacy addon {0} ProxyD3DDevice9 vtbl", this->GetFileName());

                // Case 2: Overwritten CPU instructions
                //  a) minhook (GW2Mounts)
                memcpy(this->proxyFunctionInstructions.QueryInterface, this->proxyVtbl.QueryInterface, 5);
                memcpy(this->proxyFunctionInstructions.AddRef, this->proxyVtbl.AddRef, 5);
                memcpy(this->proxyFunctionInstructions.Release, this->proxyVtbl.Release, 5);
                memcpy(this->proxyFunctionInstructions.TestCooperativeLevel, this->proxyVtbl.TestCooperativeLevel, 5);
                memcpy(this->proxyFunctionInstructions.GetAvailableTextureMem, this->proxyVtbl.GetAvailableTextureMem, 5);
                memcpy(this->proxyFunctionInstructions.EvictManagedResources, this->proxyVtbl.EvictManagedResources, 5);
                memcpy(this->proxyFunctionInstructions.GetDirect3D, this->proxyVtbl.GetDirect3D, 5);
                memcpy(this->proxyFunctionInstructions.GetDeviceCaps, this->proxyVtbl.GetDeviceCaps, 5);
                memcpy(this->proxyFunctionInstructions.GetDisplayMode, this->proxyVtbl.GetDisplayMode, 5);
                memcpy(this->proxyFunctionInstructions.GetCreationParameters, this->proxyVtbl.GetCreationParameters, 5);
                memcpy(this->proxyFunctionInstructions.SetCursorProperties, this->proxyVtbl.SetCursorProperties, 5);
                memcpy(this->proxyFunctionInstructions.SetCursorPosition, this->proxyVtbl.SetCursorPosition, 5);
                memcpy(this->proxyFunctionInstructions.ShowCursor, this->proxyVtbl.ShowCursor, 5);
                memcpy(this->proxyFunctionInstructions.CreateAdditionalSwapChain, this->proxyVtbl.CreateAdditionalSwapChain, 5);
                memcpy(this->proxyFunctionInstructions.GetSwapChain, this->proxyVtbl.GetSwapChain, 5);
                memcpy(this->proxyFunctionInstructions.GetNumberOfSwapChains, this->proxyVtbl.GetNumberOfSwapChains, 5);
                memcpy(this->proxyFunctionInstructions.Reset, this->proxyVtbl.Reset, 5);
                memcpy(this->proxyFunctionInstructions.Present, this->proxyVtbl.Present, 5);
                memcpy(this->proxyFunctionInstructions.GetBackBuffer, this->proxyVtbl.GetBackBuffer, 5);
                memcpy(this->proxyFunctionInstructions.GetRasterStatus, this->proxyVtbl.GetRasterStatus, 5);
                memcpy(this->proxyFunctionInstructions.SetDialogBoxMode, this->proxyVtbl.SetDialogBoxMode, 5);
                memcpy(this->proxyFunctionInstructions.SetGammaRamp, this->proxyVtbl.SetGammaRamp, 5);
                memcpy(this->proxyFunctionInstructions.GetGammaRamp, this->proxyVtbl.GetGammaRamp, 5);
                memcpy(this->proxyFunctionInstructions.CreateTexture, this->proxyVtbl.CreateTexture, 5);
                memcpy(this->proxyFunctionInstructions.CreateVolumeTexture, this->proxyVtbl.CreateVolumeTexture, 5);
                memcpy(this->proxyFunctionInstructions.CreateCubeTexture, this->proxyVtbl.CreateCubeTexture, 5);
                memcpy(this->proxyFunctionInstructions.CreateVertexBuffer, this->proxyVtbl.CreateVertexBuffer, 5);
                memcpy(this->proxyFunctionInstructions.CreateIndexBuffer, this->proxyVtbl.CreateIndexBuffer, 5);
                memcpy(this->proxyFunctionInstructions.CreateRenderTarget, this->proxyVtbl.CreateRenderTarget, 5);
                memcpy(this->proxyFunctionInstructions.CreateDepthStencilSurface, this->proxyVtbl.CreateDepthStencilSurface, 5);
                memcpy(this->proxyFunctionInstructions.UpdateSurface, this->proxyVtbl.UpdateSurface, 5);
                memcpy(this->proxyFunctionInstructions.UpdateTexture, this->proxyVtbl.UpdateTexture, 5);
                memcpy(this->proxyFunctionInstructions.GetRenderTargetData, this->proxyVtbl.GetRenderTargetData, 5);
                memcpy(this->proxyFunctionInstructions.GetFrontBufferData, this->proxyVtbl.GetFrontBufferData, 5);
                memcpy(this->proxyFunctionInstructions.StretchRect, this->proxyVtbl.StretchRect, 5);
                memcpy(this->proxyFunctionInstructions.ColorFill, this->proxyVtbl.ColorFill, 5);
                memcpy(this->proxyFunctionInstructions.CreateOffscreenPlainSurface, this->proxyVtbl.CreateOffscreenPlainSurface, 5);
                memcpy(this->proxyFunctionInstructions.SetRenderTarget, this->proxyVtbl.SetRenderTarget, 5);
                memcpy(this->proxyFunctionInstructions.GetRenderTarget, this->proxyVtbl.GetRenderTarget, 5);
                memcpy(this->proxyFunctionInstructions.SetDepthStencilSurface, this->proxyVtbl.SetDepthStencilSurface, 5);
                memcpy(this->proxyFunctionInstructions.GetDepthStencilSurface, this->proxyVtbl.GetDepthStencilSurface, 5);
                memcpy(this->proxyFunctionInstructions.BeginScene, this->proxyVtbl.BeginScene, 5);
                memcpy(this->proxyFunctionInstructions.EndScene, this->proxyVtbl.EndScene, 5);
                memcpy(this->proxyFunctionInstructions.Clear, this->proxyVtbl.Clear, 5);
                memcpy(this->proxyFunctionInstructions.SetTransform, this->proxyVtbl.SetTransform, 5);
                memcpy(this->proxyFunctionInstructions.GetTransform, this->proxyVtbl.GetTransform, 5);
                memcpy(this->proxyFunctionInstructions.MultiplyTransform, this->proxyVtbl.MultiplyTransform, 5);
                memcpy(this->proxyFunctionInstructions.SetViewport, this->proxyVtbl.SetViewport, 5);
                memcpy(this->proxyFunctionInstructions.GetViewport, this->proxyVtbl.GetViewport, 5);
                memcpy(this->proxyFunctionInstructions.SetMaterial, this->proxyVtbl.SetMaterial, 5);
                memcpy(this->proxyFunctionInstructions.GetMaterial, this->proxyVtbl.GetMaterial, 5);
                memcpy(this->proxyFunctionInstructions.SetLight, this->proxyVtbl.SetLight, 5);
                memcpy(this->proxyFunctionInstructions.GetLight, this->proxyVtbl.GetLight, 5);
                memcpy(this->proxyFunctionInstructions.LightEnable, this->proxyVtbl.LightEnable, 5);
                memcpy(this->proxyFunctionInstructions.GetLightEnable, this->proxyVtbl.GetLightEnable, 5);
                memcpy(this->proxyFunctionInstructions.SetClipPlane, this->proxyVtbl.SetClipPlane, 5);
                memcpy(this->proxyFunctionInstructions.GetClipPlane, this->proxyVtbl.GetClipPlane, 5);
                memcpy(this->proxyFunctionInstructions.SetRenderState, this->proxyVtbl.SetRenderState, 5);
                memcpy(this->proxyFunctionInstructions.GetRenderState, this->proxyVtbl.GetRenderState, 5);
                memcpy(this->proxyFunctionInstructions.CreateStateBlock, this->proxyVtbl.CreateStateBlock, 5);
                memcpy(this->proxyFunctionInstructions.BeginStateBlock, this->proxyVtbl.BeginStateBlock, 5);
                memcpy(this->proxyFunctionInstructions.EndStateBlock, this->proxyVtbl.EndStateBlock, 5);
                memcpy(this->proxyFunctionInstructions.SetClipStatus, this->proxyVtbl.SetClipStatus, 5);
                memcpy(this->proxyFunctionInstructions.GetClipStatus, this->proxyVtbl.GetClipStatus, 5);
                memcpy(this->proxyFunctionInstructions.GetTexture, this->proxyVtbl.GetTexture, 5);
                memcpy(this->proxyFunctionInstructions.SetTexture, this->proxyVtbl.SetTexture, 5);
                memcpy(this->proxyFunctionInstructions.GetTextureStageState, this->proxyVtbl.GetTextureStageState, 5);
                memcpy(this->proxyFunctionInstructions.SetTextureStageState, this->proxyVtbl.SetTextureStageState, 5);
                memcpy(this->proxyFunctionInstructions.GetSamplerState, this->proxyVtbl.GetSamplerState, 5);
                memcpy(this->proxyFunctionInstructions.SetSamplerState, this->proxyVtbl.SetSamplerState, 5);
                memcpy(this->proxyFunctionInstructions.ValidateDevice, this->proxyVtbl.ValidateDevice, 5);
                memcpy(this->proxyFunctionInstructions.SetPaletteEntries, this->proxyVtbl.SetPaletteEntries, 5);
                memcpy(this->proxyFunctionInstructions.GetPaletteEntries, this->proxyVtbl.GetPaletteEntries, 5);
                memcpy(this->proxyFunctionInstructions.SetCurrentTexturePalette, this->proxyVtbl.SetCurrentTexturePalette, 5);
                memcpy(this->proxyFunctionInstructions.GetCurrentTexturePalette, this->proxyVtbl.GetCurrentTexturePalette, 5);
                memcpy(this->proxyFunctionInstructions.SetScissorRect, this->proxyVtbl.SetScissorRect, 5);
                memcpy(this->proxyFunctionInstructions.GetScissorRect, this->proxyVtbl.GetScissorRect, 5);
                memcpy(this->proxyFunctionInstructions.SetSoftwareVertexProcessing, this->proxyVtbl.SetSoftwareVertexProcessing, 5);
                memcpy(this->proxyFunctionInstructions.GetSoftwareVertexProcessing, this->proxyVtbl.GetSoftwareVertexProcessing, 5);
                memcpy(this->proxyFunctionInstructions.SetNPatchMode, this->proxyVtbl.SetNPatchMode, 5);
                memcpy(this->proxyFunctionInstructions.GetNPatchMode, this->proxyVtbl.GetNPatchMode, 5);
                memcpy(this->proxyFunctionInstructions.DrawPrimitive, this->proxyVtbl.DrawPrimitive, 5);
                memcpy(this->proxyFunctionInstructions.DrawIndexedPrimitive, this->proxyVtbl.DrawIndexedPrimitive, 5);
                memcpy(this->proxyFunctionInstructions.DrawPrimitiveUP, this->proxyVtbl.DrawPrimitiveUP, 5);
                memcpy(this->proxyFunctionInstructions.DrawIndexedPrimitiveUP, this->proxyVtbl.DrawIndexedPrimitiveUP, 5);
                memcpy(this->proxyFunctionInstructions.ProcessVertices, this->proxyVtbl.ProcessVertices, 5);
                memcpy(this->proxyFunctionInstructions.CreateVertexDeclaration, this->proxyVtbl.CreateVertexDeclaration, 5);
                memcpy(this->proxyFunctionInstructions.SetVertexDeclaration, this->proxyVtbl.SetVertexDeclaration, 5);
                memcpy(this->proxyFunctionInstructions.GetVertexDeclaration, this->proxyVtbl.GetVertexDeclaration, 5);
                memcpy(this->proxyFunctionInstructions.SetFVF, this->proxyVtbl.SetFVF, 5);
                memcpy(this->proxyFunctionInstructions.GetFVF, this->proxyVtbl.GetFVF, 5);
                memcpy(this->proxyFunctionInstructions.CreateVertexShader, this->proxyVtbl.CreateVertexShader, 5);
                memcpy(this->proxyFunctionInstructions.SetVertexShader, this->proxyVtbl.SetVertexShader, 5);
                memcpy(this->proxyFunctionInstructions.GetVertexShader, this->proxyVtbl.GetVertexShader, 5);
                memcpy(this->proxyFunctionInstructions.SetVertexShaderConstantF, this->proxyVtbl.SetVertexShaderConstantF, 5);
                memcpy(this->proxyFunctionInstructions.GetVertexShaderConstantF, this->proxyVtbl.GetVertexShaderConstantF, 5);
                memcpy(this->proxyFunctionInstructions.SetVertexShaderConstantI, this->proxyVtbl.SetVertexShaderConstantI, 5);
                memcpy(this->proxyFunctionInstructions.GetVertexShaderConstantI, this->proxyVtbl.GetVertexShaderConstantI, 5);
                memcpy(this->proxyFunctionInstructions.SetVertexShaderConstantB, this->proxyVtbl.SetVertexShaderConstantB, 5);
                memcpy(this->proxyFunctionInstructions.GetVertexShaderConstantB, this->proxyVtbl.GetVertexShaderConstantB, 5);
                memcpy(this->proxyFunctionInstructions.SetStreamSource, this->proxyVtbl.SetStreamSource, 5);
                memcpy(this->proxyFunctionInstructions.GetStreamSource, this->proxyVtbl.GetStreamSource, 5);
                memcpy(this->proxyFunctionInstructions.SetStreamSourceFreq, this->proxyVtbl.SetStreamSourceFreq, 5);
                memcpy(this->proxyFunctionInstructions.GetStreamSourceFreq, this->proxyVtbl.GetStreamSourceFreq, 5);
                memcpy(this->proxyFunctionInstructions.SetIndices, this->proxyVtbl.SetIndices, 5);
                memcpy(this->proxyFunctionInstructions.GetIndices, this->proxyVtbl.GetIndices, 5);
                memcpy(this->proxyFunctionInstructions.CreatePixelShader, this->proxyVtbl.CreatePixelShader, 5);
                memcpy(this->proxyFunctionInstructions.SetPixelShader, this->proxyVtbl.SetPixelShader, 5);
                memcpy(this->proxyFunctionInstructions.GetPixelShader, this->proxyVtbl.GetPixelShader, 5);
                memcpy(this->proxyFunctionInstructions.SetPixelShaderConstantF, this->proxyVtbl.SetPixelShaderConstantF, 5);
                memcpy(this->proxyFunctionInstructions.GetPixelShaderConstantF, this->proxyVtbl.GetPixelShaderConstantF, 5);
                memcpy(this->proxyFunctionInstructions.SetPixelShaderConstantI, this->proxyVtbl.SetPixelShaderConstantI, 5);
                memcpy(this->proxyFunctionInstructions.GetPixelShaderConstantI, this->proxyVtbl.GetPixelShaderConstantI, 5);
                memcpy(this->proxyFunctionInstructions.SetPixelShaderConstantB, this->proxyVtbl.SetPixelShaderConstantB, 5);
                memcpy(this->proxyFunctionInstructions.GetPixelShaderConstantB, this->proxyVtbl.GetPixelShaderConstantB, 5);
                memcpy(this->proxyFunctionInstructions.DrawRectPatch, this->proxyVtbl.DrawRectPatch, 5);
                memcpy(this->proxyFunctionInstructions.DrawTriPatch, this->proxyVtbl.DrawTriPatch, 5);
                memcpy(this->proxyFunctionInstructions.DeletePatch, this->proxyVtbl.DeletePatch, 5);
                memcpy(this->proxyFunctionInstructions.CreateQuery, this->proxyVtbl.CreateQuery, 5);
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

                    // Case 1: Overwritten virtual table (ReShade, Gw2Hook)
                    // We get the new vtbl first and then restore the original vtbl to the proxied device
                    D3DDevice9Vtbl vtbl = GetD3DDevice9Vtbl(this->ProxyD3DDevice9);
                    SetD3DDevice9Vtbl(this->ProxyD3DDevice9, this->proxyVtbl);
                    // Compare the pointers and copy them to the addon device if they are different
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.QueryInterface, vtbl.QueryInterface, this->proxyVtbl.QueryInterface);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.AddRef, vtbl.AddRef, this->proxyVtbl.AddRef);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.Release, vtbl.Release, this->proxyVtbl.Release);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.TestCooperativeLevel, vtbl.TestCooperativeLevel, this->proxyVtbl.TestCooperativeLevel);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.GetAvailableTextureMem, vtbl.GetAvailableTextureMem, this->proxyVtbl.GetAvailableTextureMem);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.EvictManagedResources, vtbl.EvictManagedResources, this->proxyVtbl.EvictManagedResources);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.GetDirect3D, vtbl.GetDirect3D, this->proxyVtbl.GetDirect3D);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.GetDeviceCaps, vtbl.GetDeviceCaps, this->proxyVtbl.GetDeviceCaps);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.GetDisplayMode, vtbl.GetDisplayMode, this->proxyVtbl.GetDisplayMode);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.GetCreationParameters, vtbl.GetCreationParameters, this->proxyVtbl.GetCreationParameters);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.SetCursorProperties, vtbl.SetCursorProperties, this->proxyVtbl.SetCursorProperties);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.SetCursorPosition, vtbl.SetCursorPosition, this->proxyVtbl.SetCursorPosition);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.ShowCursor, vtbl.ShowCursor, this->proxyVtbl.ShowCursor);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.CreateAdditionalSwapChain, vtbl.CreateAdditionalSwapChain, this->proxyVtbl.CreateAdditionalSwapChain);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.GetSwapChain, vtbl.GetSwapChain, this->proxyVtbl.GetSwapChain);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.GetNumberOfSwapChains, vtbl.GetNumberOfSwapChains, this->proxyVtbl.GetNumberOfSwapChains);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.Reset, vtbl.Reset, this->proxyVtbl.Reset);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.Present, vtbl.Present, this->proxyVtbl.Present);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.GetBackBuffer, vtbl.GetBackBuffer, this->proxyVtbl.GetBackBuffer);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.GetRasterStatus, vtbl.GetRasterStatus, this->proxyVtbl.GetRasterStatus);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.SetDialogBoxMode, vtbl.SetDialogBoxMode, this->proxyVtbl.SetDialogBoxMode);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.SetGammaRamp, vtbl.SetGammaRamp, this->proxyVtbl.SetGammaRamp);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.GetGammaRamp, vtbl.GetGammaRamp, this->proxyVtbl.GetGammaRamp);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.CreateTexture, vtbl.CreateTexture, this->proxyVtbl.CreateTexture);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.CreateVolumeTexture, vtbl.CreateVolumeTexture, this->proxyVtbl.CreateVolumeTexture);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.CreateCubeTexture, vtbl.CreateCubeTexture, this->proxyVtbl.CreateCubeTexture);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.CreateVertexBuffer, vtbl.CreateVertexBuffer, this->proxyVtbl.CreateVertexBuffer);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.CreateIndexBuffer, vtbl.CreateIndexBuffer, this->proxyVtbl.CreateIndexBuffer);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.CreateRenderTarget, vtbl.CreateRenderTarget, this->proxyVtbl.CreateRenderTarget);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.CreateDepthStencilSurface, vtbl.CreateDepthStencilSurface, this->proxyVtbl.CreateDepthStencilSurface);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.UpdateSurface, vtbl.UpdateSurface, this->proxyVtbl.UpdateSurface);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.UpdateTexture, vtbl.UpdateTexture, this->proxyVtbl.UpdateTexture);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.GetRenderTargetData, vtbl.GetRenderTargetData, this->proxyVtbl.GetRenderTargetData);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.GetFrontBufferData, vtbl.GetFrontBufferData, this->proxyVtbl.GetFrontBufferData);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.StretchRect, vtbl.StretchRect, this->proxyVtbl.StretchRect);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.ColorFill, vtbl.ColorFill, this->proxyVtbl.ColorFill);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.CreateOffscreenPlainSurface, vtbl.CreateOffscreenPlainSurface, this->proxyVtbl.CreateOffscreenPlainSurface);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.SetRenderTarget, vtbl.SetRenderTarget, this->proxyVtbl.SetRenderTarget);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.GetRenderTarget, vtbl.GetRenderTarget, this->proxyVtbl.GetRenderTarget);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.SetDepthStencilSurface, vtbl.SetDepthStencilSurface, this->proxyVtbl.SetDepthStencilSurface);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.GetDepthStencilSurface, vtbl.GetDepthStencilSurface, this->proxyVtbl.GetDepthStencilSurface);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.BeginScene, vtbl.BeginScene, this->proxyVtbl.BeginScene);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.EndScene, vtbl.EndScene, this->proxyVtbl.EndScene);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.Clear, vtbl.Clear, this->proxyVtbl.Clear);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.SetTransform, vtbl.SetTransform, this->proxyVtbl.SetTransform);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.GetTransform, vtbl.GetTransform, this->proxyVtbl.GetTransform);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.MultiplyTransform, vtbl.MultiplyTransform, this->proxyVtbl.MultiplyTransform);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.SetViewport, vtbl.SetViewport, this->proxyVtbl.SetViewport);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.GetViewport, vtbl.GetViewport, this->proxyVtbl.GetViewport);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.SetMaterial, vtbl.SetMaterial, this->proxyVtbl.SetMaterial);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.GetMaterial, vtbl.GetMaterial, this->proxyVtbl.GetMaterial);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.SetLight, vtbl.SetLight, this->proxyVtbl.SetLight);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.GetLight, vtbl.GetLight, this->proxyVtbl.GetLight);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.LightEnable, vtbl.LightEnable, this->proxyVtbl.LightEnable);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.GetLightEnable, vtbl.GetLightEnable, this->proxyVtbl.GetLightEnable);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.SetClipPlane, vtbl.SetClipPlane, this->proxyVtbl.SetClipPlane);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.GetClipPlane, vtbl.GetClipPlane, this->proxyVtbl.GetClipPlane);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.SetRenderState, vtbl.SetRenderState, this->proxyVtbl.SetRenderState);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.GetRenderState, vtbl.GetRenderState, this->proxyVtbl.GetRenderState);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.CreateStateBlock, vtbl.CreateStateBlock, this->proxyVtbl.CreateStateBlock);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.BeginStateBlock, vtbl.BeginStateBlock, this->proxyVtbl.BeginStateBlock);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.EndStateBlock, vtbl.EndStateBlock, this->proxyVtbl.EndStateBlock);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.SetClipStatus, vtbl.SetClipStatus, this->proxyVtbl.SetClipStatus);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.GetClipStatus, vtbl.GetClipStatus, this->proxyVtbl.GetClipStatus);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.GetTexture, vtbl.GetTexture, this->proxyVtbl.GetTexture);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.SetTexture, vtbl.SetTexture, this->proxyVtbl.SetTexture);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.GetTextureStageState, vtbl.GetTextureStageState, this->proxyVtbl.GetTextureStageState);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.SetTextureStageState, vtbl.SetTextureStageState, this->proxyVtbl.SetTextureStageState);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.GetSamplerState, vtbl.GetSamplerState, this->proxyVtbl.GetSamplerState);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.SetSamplerState, vtbl.SetSamplerState, this->proxyVtbl.SetSamplerState);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.ValidateDevice, vtbl.ValidateDevice, this->proxyVtbl.ValidateDevice);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.SetPaletteEntries, vtbl.SetPaletteEntries, this->proxyVtbl.SetPaletteEntries);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.GetPaletteEntries, vtbl.GetPaletteEntries, this->proxyVtbl.GetPaletteEntries);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.SetCurrentTexturePalette, vtbl.SetCurrentTexturePalette, this->proxyVtbl.SetCurrentTexturePalette);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.GetCurrentTexturePalette, vtbl.GetCurrentTexturePalette, this->proxyVtbl.GetCurrentTexturePalette);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.SetScissorRect, vtbl.SetScissorRect, this->proxyVtbl.SetScissorRect);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.GetScissorRect, vtbl.GetScissorRect, this->proxyVtbl.GetScissorRect);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.SetSoftwareVertexProcessing, vtbl.SetSoftwareVertexProcessing, this->proxyVtbl.SetSoftwareVertexProcessing);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.GetSoftwareVertexProcessing, vtbl.GetSoftwareVertexProcessing, this->proxyVtbl.GetSoftwareVertexProcessing);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.SetNPatchMode, vtbl.SetNPatchMode, this->proxyVtbl.SetNPatchMode);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.GetNPatchMode, vtbl.GetNPatchMode, this->proxyVtbl.GetNPatchMode);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.DrawPrimitive, vtbl.DrawPrimitive, this->proxyVtbl.DrawPrimitive);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.DrawIndexedPrimitive, vtbl.DrawIndexedPrimitive, this->proxyVtbl.DrawIndexedPrimitive);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.DrawPrimitiveUP, vtbl.DrawPrimitiveUP, this->proxyVtbl.DrawPrimitiveUP);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.DrawIndexedPrimitiveUP, vtbl.DrawIndexedPrimitiveUP, this->proxyVtbl.DrawIndexedPrimitiveUP);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.ProcessVertices, vtbl.ProcessVertices, this->proxyVtbl.ProcessVertices);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.CreateVertexDeclaration, vtbl.CreateVertexDeclaration, this->proxyVtbl.CreateVertexDeclaration);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.SetVertexDeclaration, vtbl.SetVertexDeclaration, this->proxyVtbl.SetVertexDeclaration);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.GetVertexDeclaration, vtbl.GetVertexDeclaration, this->proxyVtbl.GetVertexDeclaration);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.SetFVF, vtbl.SetFVF, this->proxyVtbl.SetFVF);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.GetFVF, vtbl.GetFVF, this->proxyVtbl.GetFVF);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.CreateVertexShader, vtbl.CreateVertexShader, this->proxyVtbl.CreateVertexShader);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.SetVertexShader, vtbl.SetVertexShader, this->proxyVtbl.SetVertexShader);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.GetVertexShader, vtbl.GetVertexShader, this->proxyVtbl.GetVertexShader);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.SetVertexShaderConstantF, vtbl.SetVertexShaderConstantF, this->proxyVtbl.SetVertexShaderConstantF);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.GetVertexShaderConstantF, vtbl.GetVertexShaderConstantF, this->proxyVtbl.GetVertexShaderConstantF);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.SetVertexShaderConstantI, vtbl.SetVertexShaderConstantI, this->proxyVtbl.SetVertexShaderConstantI);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.GetVertexShaderConstantI, vtbl.GetVertexShaderConstantI, this->proxyVtbl.GetVertexShaderConstantI);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.SetVertexShaderConstantB, vtbl.SetVertexShaderConstantB, this->proxyVtbl.SetVertexShaderConstantB);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.GetVertexShaderConstantB, vtbl.GetVertexShaderConstantB, this->proxyVtbl.GetVertexShaderConstantB);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.SetStreamSource, vtbl.SetStreamSource, this->proxyVtbl.SetStreamSource);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.GetStreamSource, vtbl.GetStreamSource, this->proxyVtbl.GetStreamSource);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.SetStreamSourceFreq, vtbl.SetStreamSourceFreq, this->proxyVtbl.SetStreamSourceFreq);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.GetStreamSourceFreq, vtbl.GetStreamSourceFreq, this->proxyVtbl.GetStreamSourceFreq);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.SetIndices, vtbl.SetIndices, this->proxyVtbl.SetIndices);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.GetIndices, vtbl.GetIndices, this->proxyVtbl.GetIndices);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.CreatePixelShader, vtbl.CreatePixelShader, this->proxyVtbl.CreatePixelShader);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.SetPixelShader, vtbl.SetPixelShader, this->proxyVtbl.SetPixelShader);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.GetPixelShader, vtbl.GetPixelShader, this->proxyVtbl.GetPixelShader);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.SetPixelShaderConstantF, vtbl.SetPixelShaderConstantF, this->proxyVtbl.SetPixelShaderConstantF);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.GetPixelShaderConstantF, vtbl.GetPixelShaderConstantF, this->proxyVtbl.GetPixelShaderConstantF);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.SetPixelShaderConstantI, vtbl.SetPixelShaderConstantI, this->proxyVtbl.SetPixelShaderConstantI);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.GetPixelShaderConstantI, vtbl.GetPixelShaderConstantI, this->proxyVtbl.GetPixelShaderConstantI);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.SetPixelShaderConstantB, vtbl.SetPixelShaderConstantB, this->proxyVtbl.SetPixelShaderConstantB);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.GetPixelShaderConstantB, vtbl.GetPixelShaderConstantB, this->proxyVtbl.GetPixelShaderConstantB);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.DrawRectPatch, vtbl.DrawRectPatch, this->proxyVtbl.DrawRectPatch);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.DrawTriPatch, vtbl.DrawTriPatch, this->proxyVtbl.DrawTriPatch);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.DeletePatch, vtbl.DeletePatch, this->proxyVtbl.DeletePatch);
                    this->CopyPointerIfNotEqual(&newAddonDev->FunctionAddresses.CreateQuery, vtbl.CreateQuery, this->proxyVtbl.CreateQuery);

                    // Case 2: Overwritten CPU instructions
                    //  a) minhook (GW2Mounts)
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.QueryInterface, this->proxyFunctionInstructions.QueryInterface, vtbl.QueryInterface, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.AddRef, this->proxyFunctionInstructions.AddRef, vtbl.AddRef, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.Release, this->proxyFunctionInstructions.Release, vtbl.Release, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.TestCooperativeLevel, this->proxyFunctionInstructions.TestCooperativeLevel, vtbl.TestCooperativeLevel, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.GetAvailableTextureMem, this->proxyFunctionInstructions.GetAvailableTextureMem, vtbl.GetAvailableTextureMem, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.EvictManagedResources, this->proxyFunctionInstructions.EvictManagedResources, vtbl.EvictManagedResources, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.GetDirect3D, this->proxyFunctionInstructions.GetDirect3D, vtbl.GetDirect3D, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.GetDeviceCaps, this->proxyFunctionInstructions.GetDeviceCaps, vtbl.GetDeviceCaps, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.GetDisplayMode, this->proxyFunctionInstructions.GetDisplayMode, vtbl.GetDisplayMode, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.GetCreationParameters, this->proxyFunctionInstructions.GetCreationParameters, vtbl.GetCreationParameters, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.SetCursorProperties, this->proxyFunctionInstructions.SetCursorProperties, vtbl.SetCursorProperties, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.SetCursorPosition, this->proxyFunctionInstructions.SetCursorPosition, vtbl.SetCursorPosition, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.ShowCursor, this->proxyFunctionInstructions.ShowCursor, vtbl.ShowCursor, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.CreateAdditionalSwapChain, this->proxyFunctionInstructions.CreateAdditionalSwapChain, vtbl.CreateAdditionalSwapChain, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.GetSwapChain, this->proxyFunctionInstructions.GetSwapChain, vtbl.GetSwapChain, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.GetNumberOfSwapChains, this->proxyFunctionInstructions.GetNumberOfSwapChains, vtbl.GetNumberOfSwapChains, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.Reset, this->proxyFunctionInstructions.Reset, vtbl.Reset, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.Present, this->proxyFunctionInstructions.Present, vtbl.Present, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.GetBackBuffer, this->proxyFunctionInstructions.GetBackBuffer, vtbl.GetBackBuffer, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.GetRasterStatus, this->proxyFunctionInstructions.GetRasterStatus, vtbl.GetRasterStatus, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.SetDialogBoxMode, this->proxyFunctionInstructions.SetDialogBoxMode, vtbl.SetDialogBoxMode, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.SetGammaRamp, this->proxyFunctionInstructions.SetGammaRamp, vtbl.SetGammaRamp, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.GetGammaRamp, this->proxyFunctionInstructions.GetGammaRamp, vtbl.GetGammaRamp, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.CreateTexture, this->proxyFunctionInstructions.CreateTexture, vtbl.CreateTexture, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.CreateVolumeTexture, this->proxyFunctionInstructions.CreateVolumeTexture, vtbl.CreateVolumeTexture, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.CreateCubeTexture, this->proxyFunctionInstructions.CreateCubeTexture, vtbl.CreateCubeTexture, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.CreateVertexBuffer, this->proxyFunctionInstructions.CreateVertexBuffer, vtbl.CreateVertexBuffer, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.CreateIndexBuffer, this->proxyFunctionInstructions.CreateIndexBuffer, vtbl.CreateIndexBuffer, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.CreateRenderTarget, this->proxyFunctionInstructions.CreateRenderTarget, vtbl.CreateRenderTarget, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.CreateDepthStencilSurface, this->proxyFunctionInstructions.CreateDepthStencilSurface, vtbl.CreateDepthStencilSurface, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.UpdateSurface, this->proxyFunctionInstructions.UpdateSurface, vtbl.UpdateSurface, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.UpdateTexture, this->proxyFunctionInstructions.UpdateTexture, vtbl.UpdateTexture, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.GetRenderTargetData, this->proxyFunctionInstructions.GetRenderTargetData, vtbl.GetRenderTargetData, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.GetFrontBufferData, this->proxyFunctionInstructions.GetFrontBufferData, vtbl.GetFrontBufferData, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.StretchRect, this->proxyFunctionInstructions.StretchRect, vtbl.StretchRect, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.ColorFill, this->proxyFunctionInstructions.ColorFill, vtbl.ColorFill, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.CreateOffscreenPlainSurface, this->proxyFunctionInstructions.CreateOffscreenPlainSurface, vtbl.CreateOffscreenPlainSurface, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.SetRenderTarget, this->proxyFunctionInstructions.SetRenderTarget, vtbl.SetRenderTarget, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.GetRenderTarget, this->proxyFunctionInstructions.GetRenderTarget, vtbl.GetRenderTarget, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.SetDepthStencilSurface, this->proxyFunctionInstructions.SetDepthStencilSurface, vtbl.SetDepthStencilSurface, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.GetDepthStencilSurface, this->proxyFunctionInstructions.GetDepthStencilSurface, vtbl.GetDepthStencilSurface, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.BeginScene, this->proxyFunctionInstructions.BeginScene, vtbl.BeginScene, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.EndScene, this->proxyFunctionInstructions.EndScene, vtbl.EndScene, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.Clear, this->proxyFunctionInstructions.Clear, vtbl.Clear, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.SetTransform, this->proxyFunctionInstructions.SetTransform, vtbl.SetTransform, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.GetTransform, this->proxyFunctionInstructions.GetTransform, vtbl.GetTransform, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.MultiplyTransform, this->proxyFunctionInstructions.MultiplyTransform, vtbl.MultiplyTransform, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.SetViewport, this->proxyFunctionInstructions.SetViewport, vtbl.SetViewport, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.GetViewport, this->proxyFunctionInstructions.GetViewport, vtbl.GetViewport, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.SetMaterial, this->proxyFunctionInstructions.SetMaterial, vtbl.SetMaterial, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.GetMaterial, this->proxyFunctionInstructions.GetMaterial, vtbl.GetMaterial, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.SetLight, this->proxyFunctionInstructions.SetLight, vtbl.SetLight, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.GetLight, this->proxyFunctionInstructions.GetLight, vtbl.GetLight, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.LightEnable, this->proxyFunctionInstructions.LightEnable, vtbl.LightEnable, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.GetLightEnable, this->proxyFunctionInstructions.GetLightEnable, vtbl.GetLightEnable, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.SetClipPlane, this->proxyFunctionInstructions.SetClipPlane, vtbl.SetClipPlane, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.GetClipPlane, this->proxyFunctionInstructions.GetClipPlane, vtbl.GetClipPlane, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.SetRenderState, this->proxyFunctionInstructions.SetRenderState, vtbl.SetRenderState, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.GetRenderState, this->proxyFunctionInstructions.GetRenderState, vtbl.GetRenderState, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.CreateStateBlock, this->proxyFunctionInstructions.CreateStateBlock, vtbl.CreateStateBlock, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.BeginStateBlock, this->proxyFunctionInstructions.BeginStateBlock, vtbl.BeginStateBlock, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.EndStateBlock, this->proxyFunctionInstructions.EndStateBlock, vtbl.EndStateBlock, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.SetClipStatus, this->proxyFunctionInstructions.SetClipStatus, vtbl.SetClipStatus, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.GetClipStatus, this->proxyFunctionInstructions.GetClipStatus, vtbl.GetClipStatus, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.GetTexture, this->proxyFunctionInstructions.GetTexture, vtbl.GetTexture, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.SetTexture, this->proxyFunctionInstructions.SetTexture, vtbl.SetTexture, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.GetTextureStageState, this->proxyFunctionInstructions.GetTextureStageState, vtbl.GetTextureStageState, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.SetTextureStageState, this->proxyFunctionInstructions.SetTextureStageState, vtbl.SetTextureStageState, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.GetSamplerState, this->proxyFunctionInstructions.GetSamplerState, vtbl.GetSamplerState, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.SetSamplerState, this->proxyFunctionInstructions.SetSamplerState, vtbl.SetSamplerState, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.ValidateDevice, this->proxyFunctionInstructions.ValidateDevice, vtbl.ValidateDevice, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.SetPaletteEntries, this->proxyFunctionInstructions.SetPaletteEntries, vtbl.SetPaletteEntries, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.GetPaletteEntries, this->proxyFunctionInstructions.GetPaletteEntries, vtbl.GetPaletteEntries, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.SetCurrentTexturePalette, this->proxyFunctionInstructions.SetCurrentTexturePalette, vtbl.SetCurrentTexturePalette, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.GetCurrentTexturePalette, this->proxyFunctionInstructions.GetCurrentTexturePalette, vtbl.GetCurrentTexturePalette, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.SetScissorRect, this->proxyFunctionInstructions.SetScissorRect, vtbl.SetScissorRect, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.GetScissorRect, this->proxyFunctionInstructions.GetScissorRect, vtbl.GetScissorRect, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.SetSoftwareVertexProcessing, this->proxyFunctionInstructions.SetSoftwareVertexProcessing, vtbl.SetSoftwareVertexProcessing, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.GetSoftwareVertexProcessing, this->proxyFunctionInstructions.GetSoftwareVertexProcessing, vtbl.GetSoftwareVertexProcessing, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.SetNPatchMode, this->proxyFunctionInstructions.SetNPatchMode, vtbl.SetNPatchMode, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.GetNPatchMode, this->proxyFunctionInstructions.GetNPatchMode, vtbl.GetNPatchMode, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.DrawPrimitive, this->proxyFunctionInstructions.DrawPrimitive, vtbl.DrawPrimitive, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.DrawIndexedPrimitive, this->proxyFunctionInstructions.DrawIndexedPrimitive, vtbl.DrawIndexedPrimitive, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.DrawPrimitiveUP, this->proxyFunctionInstructions.DrawPrimitiveUP, vtbl.DrawPrimitiveUP, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.DrawIndexedPrimitiveUP, this->proxyFunctionInstructions.DrawIndexedPrimitiveUP, vtbl.DrawIndexedPrimitiveUP, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.ProcessVertices, this->proxyFunctionInstructions.ProcessVertices, vtbl.ProcessVertices, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.CreateVertexDeclaration, this->proxyFunctionInstructions.CreateVertexDeclaration, vtbl.CreateVertexDeclaration, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.SetVertexDeclaration, this->proxyFunctionInstructions.SetVertexDeclaration, vtbl.SetVertexDeclaration, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.GetVertexDeclaration, this->proxyFunctionInstructions.GetVertexDeclaration, vtbl.GetVertexDeclaration, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.SetFVF, this->proxyFunctionInstructions.SetFVF, vtbl.SetFVF, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.GetFVF, this->proxyFunctionInstructions.GetFVF, vtbl.GetFVF, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.CreateVertexShader, this->proxyFunctionInstructions.CreateVertexShader, vtbl.CreateVertexShader, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.SetVertexShader, this->proxyFunctionInstructions.SetVertexShader, vtbl.SetVertexShader, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.GetVertexShader, this->proxyFunctionInstructions.GetVertexShader, vtbl.GetVertexShader, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.SetVertexShaderConstantF, this->proxyFunctionInstructions.SetVertexShaderConstantF, vtbl.SetVertexShaderConstantF, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.GetVertexShaderConstantF, this->proxyFunctionInstructions.GetVertexShaderConstantF, vtbl.GetVertexShaderConstantF, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.SetVertexShaderConstantI, this->proxyFunctionInstructions.SetVertexShaderConstantI, vtbl.SetVertexShaderConstantI, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.GetVertexShaderConstantI, this->proxyFunctionInstructions.GetVertexShaderConstantI, vtbl.GetVertexShaderConstantI, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.SetVertexShaderConstantB, this->proxyFunctionInstructions.SetVertexShaderConstantB, vtbl.SetVertexShaderConstantB, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.GetVertexShaderConstantB, this->proxyFunctionInstructions.GetVertexShaderConstantB, vtbl.GetVertexShaderConstantB, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.SetStreamSource, this->proxyFunctionInstructions.SetStreamSource, vtbl.SetStreamSource, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.GetStreamSource, this->proxyFunctionInstructions.GetStreamSource, vtbl.GetStreamSource, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.SetStreamSourceFreq, this->proxyFunctionInstructions.SetStreamSourceFreq, vtbl.SetStreamSourceFreq, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.GetStreamSourceFreq, this->proxyFunctionInstructions.GetStreamSourceFreq, vtbl.GetStreamSourceFreq, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.SetIndices, this->proxyFunctionInstructions.SetIndices, vtbl.SetIndices, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.GetIndices, this->proxyFunctionInstructions.GetIndices, vtbl.GetIndices, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.CreatePixelShader, this->proxyFunctionInstructions.CreatePixelShader, vtbl.CreatePixelShader, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.SetPixelShader, this->proxyFunctionInstructions.SetPixelShader, vtbl.SetPixelShader, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.GetPixelShader, this->proxyFunctionInstructions.GetPixelShader, vtbl.GetPixelShader, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.SetPixelShaderConstantF, this->proxyFunctionInstructions.SetPixelShaderConstantF, vtbl.SetPixelShaderConstantF, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.GetPixelShaderConstantF, this->proxyFunctionInstructions.GetPixelShaderConstantF, vtbl.GetPixelShaderConstantF, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.SetPixelShaderConstantI, this->proxyFunctionInstructions.SetPixelShaderConstantI, vtbl.SetPixelShaderConstantI, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.GetPixelShaderConstantI, this->proxyFunctionInstructions.GetPixelShaderConstantI, vtbl.GetPixelShaderConstantI, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.SetPixelShaderConstantB, this->proxyFunctionInstructions.SetPixelShaderConstantB, vtbl.SetPixelShaderConstantB, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.GetPixelShaderConstantB, this->proxyFunctionInstructions.GetPixelShaderConstantB, vtbl.GetPixelShaderConstantB, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.DrawRectPatch, this->proxyFunctionInstructions.DrawRectPatch, vtbl.DrawRectPatch, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.DrawTriPatch, this->proxyFunctionInstructions.DrawTriPatch, vtbl.DrawTriPatch, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.DeletePatch, this->proxyFunctionInstructions.DeletePatch, vtbl.DeletePatch, 5);
                    this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.CreateQuery, this->proxyFunctionInstructions.CreateQuery, vtbl.CreateQuery, 5);

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
