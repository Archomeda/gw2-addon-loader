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


        bool LegacyAddon::ApplySafeEnv(bool isInitialize) {
            if (isInitialize) {
                // Mask the exports of our proxy so that ReShade doesn't overwrite them
                HMODULE hProxyAddon = LoadLibrary(ProxyAddon::Instance->GetFilePath().c_str());
                BYTE* imageBase = reinterpret_cast<BYTE*>(hProxyAddon);
                IMAGE_NT_HEADERS* imageHeader = reinterpret_cast<IMAGE_NT_HEADERS*>(imageBase + reinterpret_cast<IMAGE_DOS_HEADER*>(imageBase)->e_lfanew);
                if (imageHeader->Signature == IMAGE_NT_SIGNATURE && imageHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size > 0) {
                    IMAGE_EXPORT_DIRECTORY* exportDir = reinterpret_cast<IMAGE_EXPORT_DIRECTORY*>(imageBase + imageHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);
                    if (exportDir->NumberOfFunctions > 0) {
                        // Set this to 0
                        this->proxyAddonNumberOfExports = exportDir->NumberOfFunctions;
                        DWORD oldProtection;
                        VirtualProtect(reinterpret_cast<LPVOID>(exportDir), sizeof(IMAGE_EXPORT_DIRECTORY), PAGE_EXECUTE_READWRITE, &oldProtection);
                        exportDir->NumberOfFunctions = 0;
                        VirtualProtect(reinterpret_cast<LPVOID>(exportDir), sizeof(IMAGE_EXPORT_DIRECTORY), oldProtection, &oldProtection);
                    }
                }
                FreeLibrary(hProxyAddon);
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
            return mhStatus == MH_OK;
        }

        bool LegacyAddon::RevertSafeEnv(bool isInitialize) {
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
        
            if (isInitialize) {
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
                    }
                    FreeLibrary(hProxyAddon);
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

            if (!this->ApplySafeEnv(true)) {
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

            if (!this->RevertSafeEnv(true)) {
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

            if (!this->ApplySafeEnv()) {
                this->ChangeState(AddonState::ErroredState);
                GetLog()->error("Could not initialize legacy addon {0}: Redirecting WinAPI functions failed", this->GetFileName());
                return false;
            }

            // Make sure the addon is created and loaded properly.
            // At this point, the addon we are loading should use a reference to our proxy D3D9 DLL,
            // which in turn simulates the CreateDevice call to use an already created device.
            // This way we make sure it's not creating a duplicate device, but instead using the one we already have.
            this->AddonD3D9 = this->AddonCreate(this->D3D9SdkVersion);
            Direct3DDevice9Information deviceInfo = GetGlobalDeviceInformation();
            ProxyAddon::Instance->LastProxiedDevice = nullptr; 
            HRESULT result = this->AddonD3D9->CreateDevice(deviceInfo.Adapter, deviceInfo.DeviceType, deviceInfo.hFocusWindow, deviceInfo.BehaviorFlags, &deviceInfo.PresentationParameters, &this->AddonD3DDevice9);
            if (result != D3D_OK) {
                this->ChangeState(AddonState::ErroredState);
                GetLog()->error("Could not initialize legacy addon {0}: Obtaining the device failed", this->GetFileName());
            }
            this->ProxyD3DDevice9 = ProxyAddon::Instance->LastProxiedDevice;

            if (!this->RevertSafeEnv()) {
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
