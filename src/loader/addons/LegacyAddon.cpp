#include "LegacyAddon.h"
#include "addons_manager.h"
#include "ProxyAddon.h"
#include "../minhook.h"
#include "../globals.h"
#include "../log.h"
#include "../disasm/opcodes.h"
#include "../utils/encoding.h"
#include "../utils/file.h"

using namespace std;
using namespace std::filesystem;
using namespace loader::disasm;
using namespace loader::hooks;
using namespace loader::utils;

namespace loader::addons {

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
            // (add-ons that manually check if a file exists won't work with this way)

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
                    ADDONS_LOG()->info("Redirecting path to {0}", newFilePath.u8string());
                }
                filePath = newFilePath;
            }
            return SystemLoadLibraryExW(filePath.c_str(), hFile, dwFlags);
        }

        HMODULE WINAPI HookLoadLibraryExA(LPCSTR lpFileName, HANDLE hFile, DWORD dwFlags) {
            ADDONS_LOG()->info("Loading {0} by using LoadLibraryExA with flags 0x{1:X}", lpFileName != nullptr ? lpFileName : "NULL", dwFlags);
            return HookLoadLibraryEx(lpFileName, hFile, dwFlags);
        }

        HMODULE WINAPI HookLoadLibraryExW(LPCWSTR lpFileName, HANDLE hFile, DWORD dwFlags) {
            ADDONS_LOG()->info("Loading {0} by using LoadLibraryExW with flags 0x{1:X}", lpFileName != nullptr ? u8(lpFileName) : "NULL", dwFlags);
            return HookLoadLibraryEx(u8(lpFileName), hFile, dwFlags);
        }

        HMODULE WINAPI HookLoadLibraryA(LPCSTR lpFileName) {
            ADDONS_LOG()->info("Loading {0} by using LoadLibraryA", lpFileName != nullptr ? lpFileName : "NULL");
            return HookLoadLibraryEx(lpFileName, NULL, 0);
        }

        HMODULE WINAPI HookLoadLibraryW(LPCWSTR lpFileName) {
            ADDONS_LOG()->info("Loading {0} by using LoadLibraryW", lpFileName != nullptr ? u8(lpFileName) : "NULL");
            return HookLoadLibraryEx(u8(lpFileName), NULL, 0);
        }


        BOOL WINAPI HookGetModuleHandleEx(DWORD dwFlags, string fileName, HMODULE* phModule) {
            path filePath(fileName);
            if (filePath.has_parent_path()) {
                path newFilePath = RedirectModuleFileName(fileName);
                if (filePath != newFilePath) {
                    ADDONS_LOG()->info("Redirecting path to {0}", newFilePath.u8string());
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
                ADDONS_LOG()->info("Getting module at 0x{0:X} by using GetModuleHandleExA with flags 0x{1:X}", reinterpret_cast<LPCVOID>(lpFileName), dwFlags);
                return HookGetModuleHandleEx(dwFlags, reinterpret_cast<LPCVOID>(lpFileName), phModule);
            }
            else {
                ADDONS_LOG()->info("Getting module {0} by using GetModuleHandleExA with flags 0x{1:X}", lpFileName, dwFlags);
                return HookGetModuleHandleEx(dwFlags, lpFileName, phModule);
            }
        }

        BOOL WINAPI HookGetModuleHandleExW(DWORD dwFlags, LPCWSTR lpFileName, HMODULE* phModule) {
            if ((dwFlags & GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS) > 0 || lpFileName == NULL) {
                ADDONS_LOG()->info("Getting module at 0x{0:X} by using GetModuleHandleExW with flags 0x{1:X}", reinterpret_cast<LPCVOID>(lpFileName), dwFlags);
                return HookGetModuleHandleEx(dwFlags, reinterpret_cast<LPCVOID>(lpFileName), phModule);
            }
            else {
                ADDONS_LOG()->info("Getting module {0} by using GetModuleHandleExW with flags 0x{1:X}", u8(lpFileName), dwFlags);
                return HookGetModuleHandleEx(dwFlags, u8(lpFileName), phModule);
            }
        }

        HMODULE WINAPI HookGetModuleHandleA(LPCSTR lpFileName) {
            ADDONS_LOG()->info("Getting module {0} by using GetModuleHandleA", lpFileName != NULL ? lpFileName : "NULL");
            HMODULE phModule;
            HookGetModuleHandleEx(0, lpFileName, &phModule);
            return phModule;
        }

        HMODULE WINAPI HookGetModuleHandleW(LPCWSTR lpFileName) {
            ADDONS_LOG()->info("Getting module {0} by using GetModuleHandleW", lpFileName != NULL ? u8(lpFileName) : "NULL");
            HMODULE phModule;
            HookGetModuleHandleEx(0, lpFileName, &phModule);
            return phModule;
        }

    }


    bool LegacyAddon::ApplySafeEnv() {
        // Make sure to save some states to restore later
        // In order for add-ons to hook into D3D9, they either do one of the following:
        //  - Overwrite the virtual table of D3DDevice9
        //  - Overwrite the CPU instructions inside the functions of the D3DDevice9 object
        //  - Return a new object that wraps the D3DDevice9 object (already covered by our ProxyDirect3DDevice9 object)
        // We catch the most common cases here

        // Case 1: Overwritten virtual table (ReShade, Gw2Hook)
        ProxyDirect3DDevice9 proxyDevice;
        this->proxyVtbl = GetD3DDevice9Vtbl(&proxyDevice);
        ADDONS_LOG()->info("Saved state of legacy add-on {0} ProxyD3DDevice9 vtbl", this->GetFileName());

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
        ADDONS_LOG()->info("Redirected WinAPI functions for legacy add-on {0}", this->GetFileName());
        ADDONS_LOG()->debug(" - LoadLibraryA: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(hooks::SystemLoadLibraryA), reinterpret_cast<size_t>(hooks::HookLoadLibraryA));
        ADDONS_LOG()->debug(" - LoadLibraryW: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(hooks::SystemLoadLibraryW), reinterpret_cast<size_t>(hooks::HookLoadLibraryW));
        ADDONS_LOG()->debug(" - LoadLibraryExA: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(hooks::SystemLoadLibraryExA), reinterpret_cast<size_t>(hooks::HookLoadLibraryExA));
        ADDONS_LOG()->debug(" - LoadLibraryExW: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(hooks::SystemLoadLibraryExW), reinterpret_cast<size_t>(hooks::HookLoadLibraryExW));
        ADDONS_LOG()->debug(" - GetModuleHandleA: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(hooks::SystemGetModuleHandleA), reinterpret_cast<size_t>(hooks::HookGetModuleHandleA));
        ADDONS_LOG()->debug(" - GetModuleHandleW: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(hooks::SystemGetModuleHandleW), reinterpret_cast<size_t>(hooks::HookGetModuleHandleW));
        ADDONS_LOG()->debug(" - GetModuleHandleExA: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(hooks::SystemGetModuleHandleExA), reinterpret_cast<size_t>(hooks::HookGetModuleHandleExA));
        ADDONS_LOG()->debug(" - GetModuleHandleExW: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(hooks::SystemGetModuleHandleExW), reinterpret_cast<size_t>(hooks::HookGetModuleHandleExW));
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
        ADDONS_LOG()->info("Reverted WinAPI functions redirects for legacy add-on {0}", this->GetFileName());

        // Make sure to restore the states from earlier, check ApplySafeEnv
        // This is only needed whenever the addon returns the same pointer as the proxied D3D9 device
        // (in add-on terms: the proxied D3D9 device is what the addon believes that is the system D3D9 device,
        // the add-on device is what the add-on wraps or changed the system D3D9 and returns to the game)
        if (this->AddonD3DDevice9 == this->ProxyD3DDevice9) {
            ADDONS_LOG()->info("Detected overwrites in ProxyD3DDevice9 object for legacy add-on {0}", this->GetFileName());

            // Wrap the add-on device first
            ProxyDirect3DDevice9* newAddonDev = new ProxyDirect3DDevice9(this->ProxyD3DDevice9);

            // Case 1: Overwritten virtual table (ReShade, Gw2Hook)
            // We get the new vtbl first and then restore the original vtbl to the proxied device
            D3DDevice9Vtbl vtbl = GetD3DDevice9Vtbl(this->ProxyD3DDevice9);
            SetD3DDevice9Vtbl(this->ProxyD3DDevice9, this->proxyVtbl);
            // Compare the pointers and copy them to the add-on device if they are different
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
            ADDONS_LOG()->debug(" - QueryInterface: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(this->proxyVtbl.QueryInterface), reinterpret_cast<size_t>(vtbl.QueryInterface));
            ADDONS_LOG()->debug(" - AddRef: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(this->proxyVtbl.AddRef), reinterpret_cast<size_t>(vtbl.AddRef));
            ADDONS_LOG()->debug(" - Release: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(this->proxyVtbl.Release), reinterpret_cast<size_t>(vtbl.Release));
            ADDONS_LOG()->debug(" - TestCooperativeLevel: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(this->proxyVtbl.TestCooperativeLevel), reinterpret_cast<size_t>(vtbl.TestCooperativeLevel));
            ADDONS_LOG()->debug(" - GetAvailableTextureMem: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(this->proxyVtbl.GetAvailableTextureMem), reinterpret_cast<size_t>(vtbl.GetAvailableTextureMem));
            ADDONS_LOG()->debug(" - EvictManagedResources: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(this->proxyVtbl.EvictManagedResources), reinterpret_cast<size_t>(vtbl.EvictManagedResources));
            ADDONS_LOG()->debug(" - GetDirect3D: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(this->proxyVtbl.GetDirect3D), reinterpret_cast<size_t>(vtbl.GetDirect3D));
            ADDONS_LOG()->debug(" - GetDeviceCaps: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(this->proxyVtbl.GetDeviceCaps), reinterpret_cast<size_t>(vtbl.GetDeviceCaps));
            ADDONS_LOG()->debug(" - GetDisplayMode: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(this->proxyVtbl.GetDisplayMode), reinterpret_cast<size_t>(vtbl.GetDisplayMode));
            ADDONS_LOG()->debug(" - GetCreationParameters: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(this->proxyVtbl.GetCreationParameters), reinterpret_cast<size_t>(vtbl.GetCreationParameters));
            ADDONS_LOG()->debug(" - SetCursorProperties: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(this->proxyVtbl.SetCursorProperties), reinterpret_cast<size_t>(vtbl.SetCursorProperties));
            ADDONS_LOG()->debug(" - SetCursorPosition: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(this->proxyVtbl.SetCursorPosition), reinterpret_cast<size_t>(vtbl.SetCursorPosition));
            ADDONS_LOG()->debug(" - ShowCursor: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(this->proxyVtbl.ShowCursor), reinterpret_cast<size_t>(vtbl.ShowCursor));
            ADDONS_LOG()->debug(" - CreateAdditionalSwapChain: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(this->proxyVtbl.CreateAdditionalSwapChain), reinterpret_cast<size_t>(vtbl.CreateAdditionalSwapChain));
            ADDONS_LOG()->debug(" - GetSwapChain: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(this->proxyVtbl.GetSwapChain), reinterpret_cast<size_t>(vtbl.GetSwapChain));
            ADDONS_LOG()->debug(" - GetNumberOfSwapChains: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(this->proxyVtbl.GetNumberOfSwapChains), reinterpret_cast<size_t>(vtbl.GetNumberOfSwapChains));
            ADDONS_LOG()->debug(" - Reset: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(this->proxyVtbl.Reset), reinterpret_cast<size_t>(vtbl.Reset));
            ADDONS_LOG()->debug(" - Present: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(this->proxyVtbl.Present), reinterpret_cast<size_t>(vtbl.Present));
            ADDONS_LOG()->debug(" - GetBackBuffer: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(this->proxyVtbl.GetBackBuffer), reinterpret_cast<size_t>(vtbl.GetBackBuffer));
            ADDONS_LOG()->debug(" - GetRasterStatus: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(this->proxyVtbl.GetRasterStatus), reinterpret_cast<size_t>(vtbl.GetRasterStatus));
            ADDONS_LOG()->debug(" - SetDialogBoxMode: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(this->proxyVtbl.SetDialogBoxMode), reinterpret_cast<size_t>(vtbl.SetDialogBoxMode));
            ADDONS_LOG()->debug(" - SetGammaRamp: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(this->proxyVtbl.SetGammaRamp), reinterpret_cast<size_t>(vtbl.SetGammaRamp));
            ADDONS_LOG()->debug(" - GetGammaRamp: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(this->proxyVtbl.GetGammaRamp), reinterpret_cast<size_t>(vtbl.GetGammaRamp));
            ADDONS_LOG()->debug(" - CreateTexture: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(this->proxyVtbl.CreateTexture), reinterpret_cast<size_t>(vtbl.CreateTexture));
            ADDONS_LOG()->debug(" - CreateVolumeTexture: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(this->proxyVtbl.CreateVolumeTexture), reinterpret_cast<size_t>(vtbl.CreateVolumeTexture));
            ADDONS_LOG()->debug(" - CreateCubeTexture: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(this->proxyVtbl.CreateCubeTexture), reinterpret_cast<size_t>(vtbl.CreateCubeTexture));
            ADDONS_LOG()->debug(" - CreateVertexBuffer: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(this->proxyVtbl.CreateVertexBuffer), reinterpret_cast<size_t>(vtbl.CreateVertexBuffer));
            ADDONS_LOG()->debug(" - CreateIndexBuffer: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(this->proxyVtbl.CreateIndexBuffer), reinterpret_cast<size_t>(vtbl.CreateIndexBuffer));
            ADDONS_LOG()->debug(" - CreateRenderTarget: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(this->proxyVtbl.CreateRenderTarget), reinterpret_cast<size_t>(vtbl.CreateRenderTarget));
            ADDONS_LOG()->debug(" - CreateDepthStencilSurface: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(this->proxyVtbl.CreateDepthStencilSurface), reinterpret_cast<size_t>(vtbl.CreateDepthStencilSurface));
            ADDONS_LOG()->debug(" - UpdateSurface: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(this->proxyVtbl.UpdateSurface), reinterpret_cast<size_t>(vtbl.UpdateSurface));
            ADDONS_LOG()->debug(" - UpdateTexture: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(this->proxyVtbl.UpdateTexture), reinterpret_cast<size_t>(vtbl.UpdateTexture));
            ADDONS_LOG()->debug(" - GetRenderTargetData: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(this->proxyVtbl.GetRenderTargetData), reinterpret_cast<size_t>(vtbl.GetRenderTargetData));
            ADDONS_LOG()->debug(" - GetFrontBufferData: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(this->proxyVtbl.GetFrontBufferData), reinterpret_cast<size_t>(vtbl.GetFrontBufferData));
            ADDONS_LOG()->debug(" - StretchRect: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(this->proxyVtbl.StretchRect), reinterpret_cast<size_t>(vtbl.StretchRect));
            ADDONS_LOG()->debug(" - ColorFill: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(this->proxyVtbl.ColorFill), reinterpret_cast<size_t>(vtbl.ColorFill));
            ADDONS_LOG()->debug(" - CreateOffscreenPlainSurface: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(this->proxyVtbl.CreateOffscreenPlainSurface), reinterpret_cast<size_t>(vtbl.CreateOffscreenPlainSurface));
            ADDONS_LOG()->debug(" - SetRenderTarget: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(this->proxyVtbl.SetRenderTarget), reinterpret_cast<size_t>(vtbl.SetRenderTarget));
            ADDONS_LOG()->debug(" - GetRenderTarget: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(this->proxyVtbl.GetRenderTarget), reinterpret_cast<size_t>(vtbl.GetRenderTarget));
            ADDONS_LOG()->debug(" - SetDepthStencilSurface: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(this->proxyVtbl.SetDepthStencilSurface), reinterpret_cast<size_t>(vtbl.SetDepthStencilSurface));
            ADDONS_LOG()->debug(" - GetDepthStencilSurface: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(this->proxyVtbl.GetDepthStencilSurface), reinterpret_cast<size_t>(vtbl.GetDepthStencilSurface));
            ADDONS_LOG()->debug(" - BeginScene: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(this->proxyVtbl.BeginScene), reinterpret_cast<size_t>(vtbl.BeginScene));
            ADDONS_LOG()->debug(" - EndScene: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(this->proxyVtbl.EndScene), reinterpret_cast<size_t>(vtbl.EndScene));
            ADDONS_LOG()->debug(" - Clear: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(this->proxyVtbl.Clear), reinterpret_cast<size_t>(vtbl.Clear));
            ADDONS_LOG()->debug(" - SetTransform: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(this->proxyVtbl.SetTransform), reinterpret_cast<size_t>(vtbl.SetTransform));
            ADDONS_LOG()->debug(" - GetTransform: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(this->proxyVtbl.GetTransform), reinterpret_cast<size_t>(vtbl.GetTransform));
            ADDONS_LOG()->debug(" - MultiplyTransform: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(this->proxyVtbl.MultiplyTransform), reinterpret_cast<size_t>(vtbl.MultiplyTransform));
            ADDONS_LOG()->debug(" - SetViewport: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(this->proxyVtbl.SetViewport), reinterpret_cast<size_t>(vtbl.SetViewport));
            ADDONS_LOG()->debug(" - GetViewport: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(this->proxyVtbl.GetViewport), reinterpret_cast<size_t>(vtbl.GetViewport));
            ADDONS_LOG()->debug(" - SetMaterial: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(this->proxyVtbl.SetMaterial), reinterpret_cast<size_t>(vtbl.SetMaterial));
            ADDONS_LOG()->debug(" - GetMaterial: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(this->proxyVtbl.GetMaterial), reinterpret_cast<size_t>(vtbl.GetMaterial));
            ADDONS_LOG()->debug(" - SetLight: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(this->proxyVtbl.SetLight), reinterpret_cast<size_t>(vtbl.SetLight));
            ADDONS_LOG()->debug(" - GetLight: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(this->proxyVtbl.GetLight), reinterpret_cast<size_t>(vtbl.GetLight));
            ADDONS_LOG()->debug(" - SetClipPlane: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(this->proxyVtbl.SetClipPlane), reinterpret_cast<size_t>(vtbl.SetClipPlane));
            ADDONS_LOG()->debug(" - GetClipPlane: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(this->proxyVtbl.GetClipPlane), reinterpret_cast<size_t>(vtbl.GetClipPlane));
            ADDONS_LOG()->debug(" - SetRenderState: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(this->proxyVtbl.SetRenderState), reinterpret_cast<size_t>(vtbl.SetRenderState));
            ADDONS_LOG()->debug(" - GetRenderState: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(this->proxyVtbl.GetRenderState), reinterpret_cast<size_t>(vtbl.GetRenderState));
            ADDONS_LOG()->debug(" - CreateStateBlock: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(this->proxyVtbl.CreateStateBlock), reinterpret_cast<size_t>(vtbl.CreateStateBlock));
            ADDONS_LOG()->debug(" - BeginStateBlock: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(this->proxyVtbl.BeginStateBlock), reinterpret_cast<size_t>(vtbl.BeginStateBlock));
            ADDONS_LOG()->debug(" - EndStateBlock: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(this->proxyVtbl.EndStateBlock), reinterpret_cast<size_t>(vtbl.EndStateBlock));
            ADDONS_LOG()->debug(" - SetClipStatus: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(this->proxyVtbl.SetClipStatus), reinterpret_cast<size_t>(vtbl.SetClipStatus));
            ADDONS_LOG()->debug(" - GetClipStatus: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(this->proxyVtbl.GetClipStatus), reinterpret_cast<size_t>(vtbl.GetClipStatus));
            ADDONS_LOG()->debug(" - GetTexture: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(this->proxyVtbl.GetTexture), reinterpret_cast<size_t>(vtbl.GetTexture));
            ADDONS_LOG()->debug(" - SetTexture: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(this->proxyVtbl.SetTexture), reinterpret_cast<size_t>(vtbl.SetTexture));
            ADDONS_LOG()->debug(" - GetTextureStageState: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(this->proxyVtbl.GetTextureStageState), reinterpret_cast<size_t>(vtbl.GetTextureStageState));
            ADDONS_LOG()->debug(" - SetTextureStageState: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(this->proxyVtbl.SetTextureStageState), reinterpret_cast<size_t>(vtbl.SetTextureStageState));
            ADDONS_LOG()->debug(" - GetSamplerState: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(this->proxyVtbl.GetSamplerState), reinterpret_cast<size_t>(vtbl.GetSamplerState));
            ADDONS_LOG()->debug(" - SetSamplerState: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(this->proxyVtbl.SetSamplerState), reinterpret_cast<size_t>(vtbl.SetSamplerState));
            ADDONS_LOG()->debug(" - ValidateDevice: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(this->proxyVtbl.ValidateDevice), reinterpret_cast<size_t>(vtbl.ValidateDevice));
            ADDONS_LOG()->debug(" - SetPaletteEntries: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(this->proxyVtbl.SetPaletteEntries), reinterpret_cast<size_t>(vtbl.SetPaletteEntries));
            ADDONS_LOG()->debug(" - GetPaletteEntries: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(this->proxyVtbl.GetPaletteEntries), reinterpret_cast<size_t>(vtbl.GetPaletteEntries));
            ADDONS_LOG()->debug(" - SetCurrentTexturePalette: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(this->proxyVtbl.SetCurrentTexturePalette), reinterpret_cast<size_t>(vtbl.SetCurrentTexturePalette));
            ADDONS_LOG()->debug(" - GetCurrentTexturePalette: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(this->proxyVtbl.GetCurrentTexturePalette), reinterpret_cast<size_t>(vtbl.GetCurrentTexturePalette));
            ADDONS_LOG()->debug(" - SetScissorRect: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(this->proxyVtbl.SetScissorRect), reinterpret_cast<size_t>(vtbl.SetScissorRect));
            ADDONS_LOG()->debug(" - GetScissorRect: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(this->proxyVtbl.GetScissorRect), reinterpret_cast<size_t>(vtbl.GetScissorRect));
            ADDONS_LOG()->debug(" - SetSoftwareVertexProcessing: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(this->proxyVtbl.SetSoftwareVertexProcessing), reinterpret_cast<size_t>(vtbl.SetSoftwareVertexProcessing));
            ADDONS_LOG()->debug(" - GetSoftwareVertexProcessing: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(this->proxyVtbl.GetSoftwareVertexProcessing), reinterpret_cast<size_t>(vtbl.GetSoftwareVertexProcessing));
            ADDONS_LOG()->debug(" - SetNPatchMode: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(this->proxyVtbl.SetNPatchMode), reinterpret_cast<size_t>(vtbl.SetNPatchMode));
            ADDONS_LOG()->debug(" - GetNPatchMode: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(this->proxyVtbl.GetNPatchMode), reinterpret_cast<size_t>(vtbl.GetNPatchMode));
            ADDONS_LOG()->debug(" - DrawPrimitive: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(this->proxyVtbl.DrawPrimitive), reinterpret_cast<size_t>(vtbl.DrawPrimitive));
            ADDONS_LOG()->debug(" - DrawIndexedPrimitive: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(this->proxyVtbl.DrawIndexedPrimitive), reinterpret_cast<size_t>(vtbl.DrawIndexedPrimitive));
            ADDONS_LOG()->debug(" - DrawPrimitiveUP: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(this->proxyVtbl.DrawPrimitiveUP), reinterpret_cast<size_t>(vtbl.DrawPrimitiveUP));
            ADDONS_LOG()->debug(" - DrawIndexedPrimitiveUP: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(this->proxyVtbl.DrawIndexedPrimitiveUP), reinterpret_cast<size_t>(vtbl.DrawIndexedPrimitiveUP));
            ADDONS_LOG()->debug(" - ProcessVertices: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(this->proxyVtbl.ProcessVertices), reinterpret_cast<size_t>(vtbl.ProcessVertices));
            ADDONS_LOG()->debug(" - CreateVertexDeclaration: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(this->proxyVtbl.CreateVertexDeclaration), reinterpret_cast<size_t>(vtbl.CreateVertexDeclaration));
            ADDONS_LOG()->debug(" - SetVertexDeclaration: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(this->proxyVtbl.SetVertexDeclaration), reinterpret_cast<size_t>(vtbl.SetVertexDeclaration));
            ADDONS_LOG()->debug(" - GetVertexDeclaration: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(this->proxyVtbl.GetVertexDeclaration), reinterpret_cast<size_t>(vtbl.GetVertexDeclaration));
            ADDONS_LOG()->debug(" - SetFVF: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(this->proxyVtbl.SetFVF), reinterpret_cast<size_t>(vtbl.SetFVF));
            ADDONS_LOG()->debug(" - GetFVF: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(this->proxyVtbl.GetFVF), reinterpret_cast<size_t>(vtbl.GetFVF));
            ADDONS_LOG()->debug(" - CreateVertexShader: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(this->proxyVtbl.CreateVertexShader), reinterpret_cast<size_t>(vtbl.CreateVertexShader));
            ADDONS_LOG()->debug(" - SetVertexShader: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(this->proxyVtbl.SetVertexShader), reinterpret_cast<size_t>(vtbl.SetVertexShader));
            ADDONS_LOG()->debug(" - GetVertexShader: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(this->proxyVtbl.GetVertexShader), reinterpret_cast<size_t>(vtbl.GetVertexShader));
            ADDONS_LOG()->debug(" - SetVertexShaderConstantF: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(this->proxyVtbl.SetVertexShaderConstantF), reinterpret_cast<size_t>(vtbl.SetVertexShaderConstantF));
            ADDONS_LOG()->debug(" - GetVertexShaderConstantF: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(this->proxyVtbl.GetVertexShaderConstantF), reinterpret_cast<size_t>(vtbl.GetVertexShaderConstantF));
            ADDONS_LOG()->debug(" - SetVertexShaderConstantI: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(this->proxyVtbl.SetVertexShaderConstantI), reinterpret_cast<size_t>(vtbl.SetVertexShaderConstantI));
            ADDONS_LOG()->debug(" - GetVertexShaderConstantI: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(this->proxyVtbl.GetVertexShaderConstantI), reinterpret_cast<size_t>(vtbl.GetVertexShaderConstantI));
            ADDONS_LOG()->debug(" - SetVertexShaderConstantB: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(this->proxyVtbl.SetVertexShaderConstantB), reinterpret_cast<size_t>(vtbl.SetVertexShaderConstantB));
            ADDONS_LOG()->debug(" - GetVertexShaderConstantB: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(this->proxyVtbl.GetVertexShaderConstantB), reinterpret_cast<size_t>(vtbl.GetVertexShaderConstantB));
            ADDONS_LOG()->debug(" - SetStreamSource: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(this->proxyVtbl.SetStreamSource), reinterpret_cast<size_t>(vtbl.SetStreamSource));
            ADDONS_LOG()->debug(" - GetStreamSource: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(this->proxyVtbl.GetStreamSource), reinterpret_cast<size_t>(vtbl.GetStreamSource));
            ADDONS_LOG()->debug(" - SetIndices: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(this->proxyVtbl.SetIndices), reinterpret_cast<size_t>(vtbl.SetIndices));
            ADDONS_LOG()->debug(" - GetIndices: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(this->proxyVtbl.GetIndices), reinterpret_cast<size_t>(vtbl.GetIndices));
            ADDONS_LOG()->debug(" - CreatePixelShader: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(this->proxyVtbl.CreatePixelShader), reinterpret_cast<size_t>(vtbl.CreatePixelShader));
            ADDONS_LOG()->debug(" - SetPixelShader: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(this->proxyVtbl.SetPixelShader), reinterpret_cast<size_t>(vtbl.SetPixelShader));
            ADDONS_LOG()->debug(" - GetPixelShader: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(this->proxyVtbl.GetPixelShader), reinterpret_cast<size_t>(vtbl.GetPixelShader));
            ADDONS_LOG()->debug(" - SetPixelShaderConstantF: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(this->proxyVtbl.SetPixelShaderConstantF), reinterpret_cast<size_t>(vtbl.SetPixelShaderConstantF));
            ADDONS_LOG()->debug(" - GetPixelShaderConstantF: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(this->proxyVtbl.GetPixelShaderConstantF), reinterpret_cast<size_t>(vtbl.GetPixelShaderConstantF));
            ADDONS_LOG()->debug(" - SetPixelShaderConstantI: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(this->proxyVtbl.SetPixelShaderConstantI), reinterpret_cast<size_t>(vtbl.SetPixelShaderConstantI));
            ADDONS_LOG()->debug(" - GetPixelShaderConstantI: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(this->proxyVtbl.GetPixelShaderConstantI), reinterpret_cast<size_t>(vtbl.GetPixelShaderConstantI));
            ADDONS_LOG()->debug(" - SetPixelShaderConstantB: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(this->proxyVtbl.SetPixelShaderConstantB), reinterpret_cast<size_t>(vtbl.SetPixelShaderConstantB));
            ADDONS_LOG()->debug(" - GetPixelShaderConstantB: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(this->proxyVtbl.GetPixelShaderConstantB), reinterpret_cast<size_t>(vtbl.GetPixelShaderConstantB));
            ADDONS_LOG()->debug(" - DrawRectPatch: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(this->proxyVtbl.DrawRectPatch), reinterpret_cast<size_t>(vtbl.DrawRectPatch));
            ADDONS_LOG()->debug(" - DrawTriPatch: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(this->proxyVtbl.DrawTriPatch), reinterpret_cast<size_t>(vtbl.DrawTriPatch));
            ADDONS_LOG()->debug(" - DeletePatch: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(this->proxyVtbl.DeletePatch), reinterpret_cast<size_t>(vtbl.DeletePatch));
            ADDONS_LOG()->debug(" - CreateQuery: 0x{0:X} -> 0x{1:X}", reinterpret_cast<size_t>(this->proxyVtbl.CreateQuery), reinterpret_cast<size_t>(vtbl.CreateQuery));

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

            ADDONS_LOG()->info("Restored ProxyD3DDevice9 state for legacy add-on {0}", this->GetFileName());
            ADDONS_LOG()->info("Legacy add-on {0} new AddonD3DDevice9: 0x{1:X}", this->GetFileName(), reinterpret_cast<size_t>(newAddonDev));
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
        // Make sure to load the proxy add-on beforehand
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

        this->GetMetricLoad().StartMeasurement();

        // Ensure a safe environment that we can restore to later on
        if (!this->ApplySafeEnv()) {
            this->GetMetricLoad().EndMeasurement();
            this->ChangeState(AddonState::ErroredState);
            ADDONS_LOG()->error("Could not load legacy add-on {0}: Redirecting WinAPI functions failed", this->GetFileName());
            return false;
        }

        // Start loading the add-on
        HMODULE h = hooks::SystemLoadLibraryExW(this->GetFilePath().c_str(), NULL, 0);
        if (h != NULL) {
            this->addonHandle = h;
            this->AddonCreate = reinterpret_cast<Direct3DCreate9_t*>(GetProcAddress(h, "Direct3DCreate9"));
        }
        else {
            this->GetMetricLoad().EndMeasurement();
            this->ChangeState(AddonState::ErroredState);
            ADDONS_LOG()->error("Could not initialize load add-on {0}: Library handle is empty", this->GetFileName());
            return false;
        }

        if (this->AddonCreate == NULL) {
            this->GetMetricLoad().EndMeasurement();
            this->ChangeState(AddonState::ErroredState);
            ADDONS_LOG()->error("Could not initialize load add-on {0}: Add-on doesn't have a Direct3DCreate9 export", this->GetFileName());
            FreeLibrary(this->addonHandle);
            this->addonHandle = NULL;
            return false;
        }

        // Make sure the add-on is created and loaded properly.
        // At this point, the add-on we are loading should use a reference to our proxy D3D9 DLL,
        // which in turn simulates the CreateDevice call to use an already created device.
        // This way we make sure it's not creating a duplicate device, but instead using the one we already have.
        this->AddonD3D9 = this->AddonCreate(this->D3D9SdkVersion);
        ADDONS_LOG()->info("Legacy add-on {0} AddonD3D9: 0x{1:X}", this->GetFileName(), reinterpret_cast<size_t>(this->AddonD3D9));
        Direct3DDevice9Information deviceInfo = GetGlobalDeviceInformation();
        ProxyAddon::Instance->LastProxiedDevice = nullptr;
        HRESULT result = this->AddonD3D9->CreateDevice(deviceInfo.Adapter, deviceInfo.DeviceType, deviceInfo.hFocusWindow, deviceInfo.BehaviorFlags, &deviceInfo.PresentationParameters, &this->AddonD3DDevice9);
        ADDONS_LOG()->info("Legacy add-on {0} AddonD3DDevice9: 0x{1:X}", this->GetFileName(), reinterpret_cast<size_t>(this->AddonD3DDevice9));
        if (result != D3D_OK) {
            this->ChangeState(AddonState::ErroredState);
            ADDONS_LOG()->error("Could not load legacy addon {0}: Obtaining the device failed", this->GetFileName());
        }
        this->ProxyD3DDevice9 = ProxyAddon::Instance->LastProxiedDevice;
        ADDONS_LOG()->info("Legacy add-on {0} ProxyD3DDevice9: 0x{1:X}", this->GetFileName(), reinterpret_cast<size_t>(this->ProxyD3DDevice9));

        if (!this->RevertSafeEnv()) {
            this->ChangeState(AddonState::ErroredState);
            ADDONS_LOG()->error("Could not load legacy add-on {0}: Reverting WinAPI functions failed", this->GetFileName());
        }

        if (this->GetState() == AddonState::ErroredState) {
            this->GetMetricLoad().EndMeasurement();
            return false;
        }

        ResetLegacyAddonChain();

        this->GetMetricLoad().EndMeasurement();
        this->ChangeState(AddonState::LoadedState);
        return true;
    }

    bool LegacyAddon::Unload() {
        if (this->GetState() != AddonState::LoadedState) {
            return false;
        }
        this->ChangeState(AddonState::UnloadingState);

        // This will flush the add-on from the chain because it only takes *loaded* addons
        ResetLegacyAddonChain();

        this->AddonCreate = NULL;

        FreeLibrary(this->addonHandle);
        this->addonHandle = NULL;

        this->ChangeState(AddonState::UnloadedState);
        return true;
    }

}
