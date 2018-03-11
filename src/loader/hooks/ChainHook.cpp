#include "ChainHook.h"
#include <filesystem>
#include <string>

using namespace std;
using namespace std::experimental::filesystem::v1;

namespace loader {
    namespace hooks {

        map<HMODULE, ChainHookType> ChainHook::knownHooks;


        string ChainHookTypeToString(ChainHookType type) {
            switch (type) {
            case ChainHookType::OBSHook:
                return "OBS";
            }
            return "None";
        }


        ChainHook ChainHook::FindCurrentChainHook(ChainHookFunctionType functionType, void* func) {
            uint8_t* pFunction = static_cast<uint8_t*>(func);
            ChainHook chainHook;
            chainHook.functionType = functionType;
            chainHook.origFunction = func;

            if (*pFunction == 0xE9) {
                // JMP
                int32_t jmpOffset = *reinterpret_cast<int32_t*>(&pFunction[1]);
                void* pJmpFunction = pFunction + 5 + jmpOffset;

                HMODULE hModule = NULL;
                if (GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, reinterpret_cast<LPCTSTR>(pJmpFunction), &hModule) == 0) {
                    return chainHook;
                }

                if (knownHooks.find(hModule) != knownHooks.end()) {
                    chainHook.type = knownHooks[hModule];
                    return chainHook;
                }


                wchar_t buff[MAX_PATH];
                if (GetModuleFileName(hModule, buff, MAX_PATH) == 0) {
                    return chainHook;
                }
                path modulePath(buff);
                string fileName = modulePath.filename().u8string();
                if (fileName == "graphics-hook64.dll" || fileName == "graphics-hook32.dll") {
                    chainHook.type = ChainHookType::OBSHook;
                    chainHook.chainFunction = pJmpFunction;
                }
            }

            return chainHook;
        }

        bool ChainHook::HookCallback(void* callback) {
            if (this->IsCallbackHooked()) {
                return false;
            }

            switch (this->functionType) {
            case ChainHookFunctionType::PresentFunction:
                return this->HookPresentCallback(callback);
            }

            return false;
        }

        bool ChainHook::HookPresentCallback(void* callback) {
            uint8_t hookData[HOOK_MAXSIZE];
            memset(hookData, 0x90, HOOK_MAXSIZE);

            switch (this->type) {
            case ChainHookType::OBSHook: {
                const uint8_t match[] = { 0xFF, 0x15, 0xFE, 0xF6, 0x03, 0x00 };
                this->callbackHookAddress = this->FindMatch(static_cast<uint8_t*>(this->chainFunction), 0x200, match, sizeof(match));
                if (this->callbackHookAddress != nullptr) {
                    this->preHookCallbackDataSize = CALLN32_SIZE;
                    intptr_t offset = reinterpret_cast<intptr_t>(callback) - (reinterpret_cast<intptr_t>(this->callbackHookAddress) + 5);
                    int32_t offset32 = static_cast<int32_t>(offset);
                    if (offset != offset32) {
                        // 64-bit offsets are not supported sadly, don't know how to solve this yet
                        this->callbackHookAddress = nullptr;
                        break;
                    }
                    hookData[0] = 0xE8;
                    *reinterpret_cast<int32_t*>(hookData + 1) = offset32;
                }
                break;
            }
            }

            if (this->callbackHookAddress != nullptr) {
                DWORD oldProtection = this->DisableProtection(this->callbackHookAddress, this->preHookCallbackDataSize);
                memcpy(this->preHookCallbackData, this->callbackHookAddress, this->preHookCallbackDataSize);
                memcpy(this->callbackHookAddress, hookData, this->preHookCallbackDataSize);
                this->ResetProtection(this->callbackHookAddress, this->preHookCallbackDataSize, oldProtection);
                return true;
            }
            return false;
        }

        bool ChainHook::UnhookCallback() {
            if (!this->IsCallbackHooked()) {
                return false;
            }

            DWORD oldProtection = this->DisableProtection(this->callbackHookAddress, this->preHookCallbackDataSize);
            memcpy(this->callbackHookAddress, this->preHookCallbackData, this->preHookCallbackDataSize);
            this->ResetProtection(this->callbackHookAddress, this->preHookCallbackDataSize, oldProtection);

            this->callbackHookAddress = nullptr;
            return true;
        }

        DWORD ChainHook::DisableProtection(void* address, size_t length) {
            DWORD oldProtection;
            VirtualProtect(address, length, PAGE_EXECUTE_READWRITE, &oldProtection);
            return oldProtection;
        }

        void ChainHook::ResetProtection(void* address, size_t length, DWORD reset) {
            DWORD oldProtection;
            VirtualProtect(address, length, reset, &oldProtection);
        }

        uint8_t* ChainHook::FindMatch(uint8_t* startAddress, size_t searchLength, const uint8_t bytes[], size_t bytesSize) const {
            uint8_t* currentAddress = startAddress;
            while (currentAddress < startAddress + searchLength) {
                for (size_t i = 0; i < bytesSize; ++i) {
                    if (*(currentAddress + i) != bytes[i]) {
                        break;
                    }
                    if (i + 1 == bytesSize) {
                        return currentAddress;
                    }
                }
                ++currentAddress;
            }
            return nullptr;
        }

    }
}
