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


        bool LegacyAddon::ApplySafeEnv() {
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
            memcpy(this->proxyFunctionInstructions.QueryInterface, this->proxyVtbl.QueryInterface, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.AddRef, this->proxyVtbl.AddRef, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.Release, this->proxyVtbl.Release, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.TestCooperativeLevel, this->proxyVtbl.TestCooperativeLevel, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.GetAvailableTextureMem, this->proxyVtbl.GetAvailableTextureMem, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.EvictManagedResources, this->proxyVtbl.EvictManagedResources, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.GetDirect3D, this->proxyVtbl.GetDirect3D, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.GetDeviceCaps, this->proxyVtbl.GetDeviceCaps, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.GetDisplayMode, this->proxyVtbl.GetDisplayMode, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.GetCreationParameters, this->proxyVtbl.GetCreationParameters, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.SetCursorProperties, this->proxyVtbl.SetCursorProperties, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.SetCursorPosition, this->proxyVtbl.SetCursorPosition, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.ShowCursor, this->proxyVtbl.ShowCursor, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.CreateAdditionalSwapChain, this->proxyVtbl.CreateAdditionalSwapChain, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.GetSwapChain, this->proxyVtbl.GetSwapChain, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.GetNumberOfSwapChains, this->proxyVtbl.GetNumberOfSwapChains, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.Reset, this->proxyVtbl.Reset, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.Present, this->proxyVtbl.Present, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.GetBackBuffer, this->proxyVtbl.GetBackBuffer, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.GetRasterStatus, this->proxyVtbl.GetRasterStatus, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.SetDialogBoxMode, this->proxyVtbl.SetDialogBoxMode, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.SetGammaRamp, this->proxyVtbl.SetGammaRamp, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.GetGammaRamp, this->proxyVtbl.GetGammaRamp, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.CreateTexture, this->proxyVtbl.CreateTexture, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.CreateVolumeTexture, this->proxyVtbl.CreateVolumeTexture, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.CreateCubeTexture, this->proxyVtbl.CreateCubeTexture, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.CreateVertexBuffer, this->proxyVtbl.CreateVertexBuffer, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.CreateIndexBuffer, this->proxyVtbl.CreateIndexBuffer, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.CreateRenderTarget, this->proxyVtbl.CreateRenderTarget, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.CreateDepthStencilSurface, this->proxyVtbl.CreateDepthStencilSurface, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.UpdateSurface, this->proxyVtbl.UpdateSurface, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.UpdateTexture, this->proxyVtbl.UpdateTexture, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.GetRenderTargetData, this->proxyVtbl.GetRenderTargetData, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.GetFrontBufferData, this->proxyVtbl.GetFrontBufferData, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.StretchRect, this->proxyVtbl.StretchRect, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.ColorFill, this->proxyVtbl.ColorFill, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.CreateOffscreenPlainSurface, this->proxyVtbl.CreateOffscreenPlainSurface, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.SetRenderTarget, this->proxyVtbl.SetRenderTarget, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.GetRenderTarget, this->proxyVtbl.GetRenderTarget, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.SetDepthStencilSurface, this->proxyVtbl.SetDepthStencilSurface, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.GetDepthStencilSurface, this->proxyVtbl.GetDepthStencilSurface, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.BeginScene, this->proxyVtbl.BeginScene, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.EndScene, this->proxyVtbl.EndScene, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.Clear, this->proxyVtbl.Clear, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.SetTransform, this->proxyVtbl.SetTransform, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.GetTransform, this->proxyVtbl.GetTransform, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.MultiplyTransform, this->proxyVtbl.MultiplyTransform, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.SetViewport, this->proxyVtbl.SetViewport, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.GetViewport, this->proxyVtbl.GetViewport, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.SetMaterial, this->proxyVtbl.SetMaterial, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.GetMaterial, this->proxyVtbl.GetMaterial, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.SetLight, this->proxyVtbl.SetLight, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.GetLight, this->proxyVtbl.GetLight, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.LightEnable, this->proxyVtbl.LightEnable, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.GetLightEnable, this->proxyVtbl.GetLightEnable, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.SetClipPlane, this->proxyVtbl.SetClipPlane, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.GetClipPlane, this->proxyVtbl.GetClipPlane, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.SetRenderState, this->proxyVtbl.SetRenderState, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.GetRenderState, this->proxyVtbl.GetRenderState, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.CreateStateBlock, this->proxyVtbl.CreateStateBlock, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.BeginStateBlock, this->proxyVtbl.BeginStateBlock, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.EndStateBlock, this->proxyVtbl.EndStateBlock, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.SetClipStatus, this->proxyVtbl.SetClipStatus, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.GetClipStatus, this->proxyVtbl.GetClipStatus, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.GetTexture, this->proxyVtbl.GetTexture, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.SetTexture, this->proxyVtbl.SetTexture, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.GetTextureStageState, this->proxyVtbl.GetTextureStageState, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.SetTextureStageState, this->proxyVtbl.SetTextureStageState, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.GetSamplerState, this->proxyVtbl.GetSamplerState, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.SetSamplerState, this->proxyVtbl.SetSamplerState, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.ValidateDevice, this->proxyVtbl.ValidateDevice, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.SetPaletteEntries, this->proxyVtbl.SetPaletteEntries, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.GetPaletteEntries, this->proxyVtbl.GetPaletteEntries, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.SetCurrentTexturePalette, this->proxyVtbl.SetCurrentTexturePalette, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.GetCurrentTexturePalette, this->proxyVtbl.GetCurrentTexturePalette, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.SetScissorRect, this->proxyVtbl.SetScissorRect, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.GetScissorRect, this->proxyVtbl.GetScissorRect, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.SetSoftwareVertexProcessing, this->proxyVtbl.SetSoftwareVertexProcessing, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.GetSoftwareVertexProcessing, this->proxyVtbl.GetSoftwareVertexProcessing, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.SetNPatchMode, this->proxyVtbl.SetNPatchMode, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.GetNPatchMode, this->proxyVtbl.GetNPatchMode, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.DrawPrimitive, this->proxyVtbl.DrawPrimitive, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.DrawIndexedPrimitive, this->proxyVtbl.DrawIndexedPrimitive, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.DrawPrimitiveUP, this->proxyVtbl.DrawPrimitiveUP, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.DrawIndexedPrimitiveUP, this->proxyVtbl.DrawIndexedPrimitiveUP, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.ProcessVertices, this->proxyVtbl.ProcessVertices, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.CreateVertexDeclaration, this->proxyVtbl.CreateVertexDeclaration, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.SetVertexDeclaration, this->proxyVtbl.SetVertexDeclaration, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.GetVertexDeclaration, this->proxyVtbl.GetVertexDeclaration, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.SetFVF, this->proxyVtbl.SetFVF, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.GetFVF, this->proxyVtbl.GetFVF, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.CreateVertexShader, this->proxyVtbl.CreateVertexShader, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.SetVertexShader, this->proxyVtbl.SetVertexShader, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.GetVertexShader, this->proxyVtbl.GetVertexShader, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.SetVertexShaderConstantF, this->proxyVtbl.SetVertexShaderConstantF, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.GetVertexShaderConstantF, this->proxyVtbl.GetVertexShaderConstantF, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.SetVertexShaderConstantI, this->proxyVtbl.SetVertexShaderConstantI, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.GetVertexShaderConstantI, this->proxyVtbl.GetVertexShaderConstantI, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.SetVertexShaderConstantB, this->proxyVtbl.SetVertexShaderConstantB, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.GetVertexShaderConstantB, this->proxyVtbl.GetVertexShaderConstantB, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.SetStreamSource, this->proxyVtbl.SetStreamSource, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.GetStreamSource, this->proxyVtbl.GetStreamSource, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.SetStreamSourceFreq, this->proxyVtbl.SetStreamSourceFreq, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.GetStreamSourceFreq, this->proxyVtbl.GetStreamSourceFreq, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.SetIndices, this->proxyVtbl.SetIndices, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.GetIndices, this->proxyVtbl.GetIndices, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.CreatePixelShader, this->proxyVtbl.CreatePixelShader, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.SetPixelShader, this->proxyVtbl.SetPixelShader, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.GetPixelShader, this->proxyVtbl.GetPixelShader, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.SetPixelShaderConstantF, this->proxyVtbl.SetPixelShaderConstantF, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.GetPixelShaderConstantF, this->proxyVtbl.GetPixelShaderConstantF, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.SetPixelShaderConstantI, this->proxyVtbl.SetPixelShaderConstantI, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.GetPixelShaderConstantI, this->proxyVtbl.GetPixelShaderConstantI, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.SetPixelShaderConstantB, this->proxyVtbl.SetPixelShaderConstantB, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.GetPixelShaderConstantB, this->proxyVtbl.GetPixelShaderConstantB, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.DrawRectPatch, this->proxyVtbl.DrawRectPatch, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.DrawTriPatch, this->proxyVtbl.DrawTriPatch, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.DeletePatch, this->proxyVtbl.DeletePatch, INSTRUCTION_BACKUP_SIZE);
            memcpy(this->proxyFunctionInstructions.CreateQuery, this->proxyVtbl.CreateQuery, INSTRUCTION_BACKUP_SIZE);

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

        bool LegacyAddon::RevertSafeEnv() {
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
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.QueryInterface, this->proxyFunctionInstructions.QueryInterface, vtbl.QueryInterface, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.AddRef, this->proxyFunctionInstructions.AddRef, vtbl.AddRef, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.Release, this->proxyFunctionInstructions.Release, vtbl.Release, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.TestCooperativeLevel, this->proxyFunctionInstructions.TestCooperativeLevel, vtbl.TestCooperativeLevel, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.GetAvailableTextureMem, this->proxyFunctionInstructions.GetAvailableTextureMem, vtbl.GetAvailableTextureMem, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.EvictManagedResources, this->proxyFunctionInstructions.EvictManagedResources, vtbl.EvictManagedResources, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.GetDirect3D, this->proxyFunctionInstructions.GetDirect3D, vtbl.GetDirect3D, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.GetDeviceCaps, this->proxyFunctionInstructions.GetDeviceCaps, vtbl.GetDeviceCaps, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.GetDisplayMode, this->proxyFunctionInstructions.GetDisplayMode, vtbl.GetDisplayMode, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.GetCreationParameters, this->proxyFunctionInstructions.GetCreationParameters, vtbl.GetCreationParameters, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.SetCursorProperties, this->proxyFunctionInstructions.SetCursorProperties, vtbl.SetCursorProperties, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.SetCursorPosition, this->proxyFunctionInstructions.SetCursorPosition, vtbl.SetCursorPosition, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.ShowCursor, this->proxyFunctionInstructions.ShowCursor, vtbl.ShowCursor, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.CreateAdditionalSwapChain, this->proxyFunctionInstructions.CreateAdditionalSwapChain, vtbl.CreateAdditionalSwapChain, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.GetSwapChain, this->proxyFunctionInstructions.GetSwapChain, vtbl.GetSwapChain, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.GetNumberOfSwapChains, this->proxyFunctionInstructions.GetNumberOfSwapChains, vtbl.GetNumberOfSwapChains, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.Reset, this->proxyFunctionInstructions.Reset, vtbl.Reset, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.Present, this->proxyFunctionInstructions.Present, vtbl.Present, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.GetBackBuffer, this->proxyFunctionInstructions.GetBackBuffer, vtbl.GetBackBuffer, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.GetRasterStatus, this->proxyFunctionInstructions.GetRasterStatus, vtbl.GetRasterStatus, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.SetDialogBoxMode, this->proxyFunctionInstructions.SetDialogBoxMode, vtbl.SetDialogBoxMode, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.SetGammaRamp, this->proxyFunctionInstructions.SetGammaRamp, vtbl.SetGammaRamp, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.GetGammaRamp, this->proxyFunctionInstructions.GetGammaRamp, vtbl.GetGammaRamp, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.CreateTexture, this->proxyFunctionInstructions.CreateTexture, vtbl.CreateTexture, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.CreateVolumeTexture, this->proxyFunctionInstructions.CreateVolumeTexture, vtbl.CreateVolumeTexture, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.CreateCubeTexture, this->proxyFunctionInstructions.CreateCubeTexture, vtbl.CreateCubeTexture, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.CreateVertexBuffer, this->proxyFunctionInstructions.CreateVertexBuffer, vtbl.CreateVertexBuffer, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.CreateIndexBuffer, this->proxyFunctionInstructions.CreateIndexBuffer, vtbl.CreateIndexBuffer, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.CreateRenderTarget, this->proxyFunctionInstructions.CreateRenderTarget, vtbl.CreateRenderTarget, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.CreateDepthStencilSurface, this->proxyFunctionInstructions.CreateDepthStencilSurface, vtbl.CreateDepthStencilSurface, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.UpdateSurface, this->proxyFunctionInstructions.UpdateSurface, vtbl.UpdateSurface, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.UpdateTexture, this->proxyFunctionInstructions.UpdateTexture, vtbl.UpdateTexture, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.GetRenderTargetData, this->proxyFunctionInstructions.GetRenderTargetData, vtbl.GetRenderTargetData, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.GetFrontBufferData, this->proxyFunctionInstructions.GetFrontBufferData, vtbl.GetFrontBufferData, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.StretchRect, this->proxyFunctionInstructions.StretchRect, vtbl.StretchRect, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.ColorFill, this->proxyFunctionInstructions.ColorFill, vtbl.ColorFill, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.CreateOffscreenPlainSurface, this->proxyFunctionInstructions.CreateOffscreenPlainSurface, vtbl.CreateOffscreenPlainSurface, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.SetRenderTarget, this->proxyFunctionInstructions.SetRenderTarget, vtbl.SetRenderTarget, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.GetRenderTarget, this->proxyFunctionInstructions.GetRenderTarget, vtbl.GetRenderTarget, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.SetDepthStencilSurface, this->proxyFunctionInstructions.SetDepthStencilSurface, vtbl.SetDepthStencilSurface, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.GetDepthStencilSurface, this->proxyFunctionInstructions.GetDepthStencilSurface, vtbl.GetDepthStencilSurface, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.BeginScene, this->proxyFunctionInstructions.BeginScene, vtbl.BeginScene, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.EndScene, this->proxyFunctionInstructions.EndScene, vtbl.EndScene, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.Clear, this->proxyFunctionInstructions.Clear, vtbl.Clear, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.SetTransform, this->proxyFunctionInstructions.SetTransform, vtbl.SetTransform, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.GetTransform, this->proxyFunctionInstructions.GetTransform, vtbl.GetTransform, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.MultiplyTransform, this->proxyFunctionInstructions.MultiplyTransform, vtbl.MultiplyTransform, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.SetViewport, this->proxyFunctionInstructions.SetViewport, vtbl.SetViewport, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.GetViewport, this->proxyFunctionInstructions.GetViewport, vtbl.GetViewport, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.SetMaterial, this->proxyFunctionInstructions.SetMaterial, vtbl.SetMaterial, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.GetMaterial, this->proxyFunctionInstructions.GetMaterial, vtbl.GetMaterial, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.SetLight, this->proxyFunctionInstructions.SetLight, vtbl.SetLight, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.GetLight, this->proxyFunctionInstructions.GetLight, vtbl.GetLight, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.LightEnable, this->proxyFunctionInstructions.LightEnable, vtbl.LightEnable, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.GetLightEnable, this->proxyFunctionInstructions.GetLightEnable, vtbl.GetLightEnable, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.SetClipPlane, this->proxyFunctionInstructions.SetClipPlane, vtbl.SetClipPlane, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.GetClipPlane, this->proxyFunctionInstructions.GetClipPlane, vtbl.GetClipPlane, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.SetRenderState, this->proxyFunctionInstructions.SetRenderState, vtbl.SetRenderState, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.GetRenderState, this->proxyFunctionInstructions.GetRenderState, vtbl.GetRenderState, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.CreateStateBlock, this->proxyFunctionInstructions.CreateStateBlock, vtbl.CreateStateBlock, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.BeginStateBlock, this->proxyFunctionInstructions.BeginStateBlock, vtbl.BeginStateBlock, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.EndStateBlock, this->proxyFunctionInstructions.EndStateBlock, vtbl.EndStateBlock, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.SetClipStatus, this->proxyFunctionInstructions.SetClipStatus, vtbl.SetClipStatus, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.GetClipStatus, this->proxyFunctionInstructions.GetClipStatus, vtbl.GetClipStatus, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.GetTexture, this->proxyFunctionInstructions.GetTexture, vtbl.GetTexture, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.SetTexture, this->proxyFunctionInstructions.SetTexture, vtbl.SetTexture, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.GetTextureStageState, this->proxyFunctionInstructions.GetTextureStageState, vtbl.GetTextureStageState, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.SetTextureStageState, this->proxyFunctionInstructions.SetTextureStageState, vtbl.SetTextureStageState, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.GetSamplerState, this->proxyFunctionInstructions.GetSamplerState, vtbl.GetSamplerState, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.SetSamplerState, this->proxyFunctionInstructions.SetSamplerState, vtbl.SetSamplerState, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.ValidateDevice, this->proxyFunctionInstructions.ValidateDevice, vtbl.ValidateDevice, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.SetPaletteEntries, this->proxyFunctionInstructions.SetPaletteEntries, vtbl.SetPaletteEntries, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.GetPaletteEntries, this->proxyFunctionInstructions.GetPaletteEntries, vtbl.GetPaletteEntries, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.SetCurrentTexturePalette, this->proxyFunctionInstructions.SetCurrentTexturePalette, vtbl.SetCurrentTexturePalette, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.GetCurrentTexturePalette, this->proxyFunctionInstructions.GetCurrentTexturePalette, vtbl.GetCurrentTexturePalette, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.SetScissorRect, this->proxyFunctionInstructions.SetScissorRect, vtbl.SetScissorRect, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.GetScissorRect, this->proxyFunctionInstructions.GetScissorRect, vtbl.GetScissorRect, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.SetSoftwareVertexProcessing, this->proxyFunctionInstructions.SetSoftwareVertexProcessing, vtbl.SetSoftwareVertexProcessing, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.GetSoftwareVertexProcessing, this->proxyFunctionInstructions.GetSoftwareVertexProcessing, vtbl.GetSoftwareVertexProcessing, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.SetNPatchMode, this->proxyFunctionInstructions.SetNPatchMode, vtbl.SetNPatchMode, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.GetNPatchMode, this->proxyFunctionInstructions.GetNPatchMode, vtbl.GetNPatchMode, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.DrawPrimitive, this->proxyFunctionInstructions.DrawPrimitive, vtbl.DrawPrimitive, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.DrawIndexedPrimitive, this->proxyFunctionInstructions.DrawIndexedPrimitive, vtbl.DrawIndexedPrimitive, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.DrawPrimitiveUP, this->proxyFunctionInstructions.DrawPrimitiveUP, vtbl.DrawPrimitiveUP, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.DrawIndexedPrimitiveUP, this->proxyFunctionInstructions.DrawIndexedPrimitiveUP, vtbl.DrawIndexedPrimitiveUP, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.ProcessVertices, this->proxyFunctionInstructions.ProcessVertices, vtbl.ProcessVertices, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.CreateVertexDeclaration, this->proxyFunctionInstructions.CreateVertexDeclaration, vtbl.CreateVertexDeclaration, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.SetVertexDeclaration, this->proxyFunctionInstructions.SetVertexDeclaration, vtbl.SetVertexDeclaration, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.GetVertexDeclaration, this->proxyFunctionInstructions.GetVertexDeclaration, vtbl.GetVertexDeclaration, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.SetFVF, this->proxyFunctionInstructions.SetFVF, vtbl.SetFVF, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.GetFVF, this->proxyFunctionInstructions.GetFVF, vtbl.GetFVF, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.CreateVertexShader, this->proxyFunctionInstructions.CreateVertexShader, vtbl.CreateVertexShader, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.SetVertexShader, this->proxyFunctionInstructions.SetVertexShader, vtbl.SetVertexShader, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.GetVertexShader, this->proxyFunctionInstructions.GetVertexShader, vtbl.GetVertexShader, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.SetVertexShaderConstantF, this->proxyFunctionInstructions.SetVertexShaderConstantF, vtbl.SetVertexShaderConstantF, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.GetVertexShaderConstantF, this->proxyFunctionInstructions.GetVertexShaderConstantF, vtbl.GetVertexShaderConstantF, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.SetVertexShaderConstantI, this->proxyFunctionInstructions.SetVertexShaderConstantI, vtbl.SetVertexShaderConstantI, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.GetVertexShaderConstantI, this->proxyFunctionInstructions.GetVertexShaderConstantI, vtbl.GetVertexShaderConstantI, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.SetVertexShaderConstantB, this->proxyFunctionInstructions.SetVertexShaderConstantB, vtbl.SetVertexShaderConstantB, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.GetVertexShaderConstantB, this->proxyFunctionInstructions.GetVertexShaderConstantB, vtbl.GetVertexShaderConstantB, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.SetStreamSource, this->proxyFunctionInstructions.SetStreamSource, vtbl.SetStreamSource, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.GetStreamSource, this->proxyFunctionInstructions.GetStreamSource, vtbl.GetStreamSource, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.SetStreamSourceFreq, this->proxyFunctionInstructions.SetStreamSourceFreq, vtbl.SetStreamSourceFreq, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.GetStreamSourceFreq, this->proxyFunctionInstructions.GetStreamSourceFreq, vtbl.GetStreamSourceFreq, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.SetIndices, this->proxyFunctionInstructions.SetIndices, vtbl.SetIndices, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.GetIndices, this->proxyFunctionInstructions.GetIndices, vtbl.GetIndices, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.CreatePixelShader, this->proxyFunctionInstructions.CreatePixelShader, vtbl.CreatePixelShader, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.SetPixelShader, this->proxyFunctionInstructions.SetPixelShader, vtbl.SetPixelShader, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.GetPixelShader, this->proxyFunctionInstructions.GetPixelShader, vtbl.GetPixelShader, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.SetPixelShaderConstantF, this->proxyFunctionInstructions.SetPixelShaderConstantF, vtbl.SetPixelShaderConstantF, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.GetPixelShaderConstantF, this->proxyFunctionInstructions.GetPixelShaderConstantF, vtbl.GetPixelShaderConstantF, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.SetPixelShaderConstantI, this->proxyFunctionInstructions.SetPixelShaderConstantI, vtbl.SetPixelShaderConstantI, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.GetPixelShaderConstantI, this->proxyFunctionInstructions.GetPixelShaderConstantI, vtbl.GetPixelShaderConstantI, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.SetPixelShaderConstantB, this->proxyFunctionInstructions.SetPixelShaderConstantB, vtbl.SetPixelShaderConstantB, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.GetPixelShaderConstantB, this->proxyFunctionInstructions.GetPixelShaderConstantB, vtbl.GetPixelShaderConstantB, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.DrawRectPatch, this->proxyFunctionInstructions.DrawRectPatch, vtbl.DrawRectPatch, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.DrawTriPatch, this->proxyFunctionInstructions.DrawTriPatch, vtbl.DrawTriPatch, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.DeletePatch, this->proxyFunctionInstructions.DeletePatch, vtbl.DeletePatch, INSTRUCTION_BACKUP_SIZE);
                this->CopyAndRestorePointerIfHooked(&newAddonDev->FunctionAddresses.CreateQuery, this->proxyFunctionInstructions.CreateQuery, vtbl.CreateQuery, INSTRUCTION_BACKUP_SIZE);

                GetLog()->info("Restored ProxyD3DDevice9 state for legacy addon {0}", this->GetFileName());
                GetLog()->info("Legacy addon {0} new AddonD3DDevice9: 0x{1:X}", this->GetFileName(), reinterpret_cast<size_t>(newAddonDev));
                this->AddonD3DDevice9 = newAddonDev;
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
            this->ChangeState(AddonState::UnloadedState);

            return true;
        }

        bool LegacyAddon::Uninitialize() {
            this->Unload();
            return true;
        }

        bool LegacyAddon::Load() {
            if (this->GetState() != AddonState::UnloadedState) {
                return false;
            }
            this->ChangeState(AddonState::LoadingState);

            // Ensure a safe environment that we can restore to later on
            if (!this->ApplySafeEnv()) {
                this->ChangeState(AddonState::ErroredState);
                GetLog()->error("Could not load legacy addon {0}: Redirecting WinAPI functions failed", this->GetFileName());
                return false;
            }

            // Start loading the addon
            HMODULE h = hooks::SystemLoadLibraryExW(this->GetFilePath().c_str(), NULL, 0);
            if (h != NULL) {
                this->addonHandle = h;
                this->AddonCreate = reinterpret_cast<Direct3DCreate9_t*>(GetProcAddress(h, "Direct3DCreate9"));
            }
            else {
                this->ChangeState(AddonState::ErroredState);
                GetLog()->error("Could not initialize load addon {0}: Library handle is empty", this->GetFileName());
                return false;
            }

            if (this->AddonCreate == NULL) {
                this->ChangeState(AddonState::ErroredState);
                GetLog()->error("Could not initialize load addon {0}: Addon doesn't have a Direct3DCreate9 export", this->GetFileName());
                FreeLibrary(this->addonHandle);
                this->addonHandle = NULL;
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
                GetLog()->error("Could not load legacy addon {0}: Obtaining the device failed", this->GetFileName());
            }
            this->ProxyD3DDevice9 = ProxyAddon::Instance->LastProxiedDevice;
            GetLog()->info("Legacy addon {0} ProxyD3DDevice9: 0x{1:X}", this->GetFileName(), reinterpret_cast<size_t>(this->ProxyD3DDevice9));

            if (!this->RevertSafeEnv()) {
                this->ChangeState(AddonState::ErroredState);
                GetLog()->error("Could not load legacy addon {0}: Reverting WinAPI functions failed", this->GetFileName());
            }

            if (this->GetState() == AddonState::ErroredState) {
                return false;
            }

            ResetLegacyAddonChain();

            this->ChangeState(AddonState::LoadedState);
            return true;
        }

        bool LegacyAddon::Unload() {
            if (this->GetState() != AddonState::LoadedState) {
                return false;
            }
            this->ChangeState(AddonState::UnloadingState);

            // This will flush the addon from the chain because it only takes *loaded* addons
            ResetLegacyAddonChain();

            this->AddonCreate = NULL;

            FreeLibrary(this->addonHandle);
            this->addonHandle = NULL;

            this->ChangeState(AddonState::UnloadedState);
            return true;
        }

    }
}
