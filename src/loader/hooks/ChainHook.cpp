#include "ChainHook.h"
#include "../disasm/opcodes.h"

using namespace std;
using namespace std::filesystem;
using namespace loader::disasm;

namespace loader::hooks {

    map<HMODULE, ChainHookType> ChainHook::knownHooks;


    const string ChainHookTypeToString(ChainHookType type) {
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
            JMP_REL jmpRel = *reinterpret_cast<JMP_REL*>(pFunction);
            void* pJmpFunction = pFunction + sizeof(JMP_REL) + jmpRel.operand;

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
        uint8_t hookData[16];
        memset(hookData, 0, 16);

        switch (this->type) {
        case ChainHookType::OBSHook: {
            uint8_t* currentAddress = static_cast<uint8_t*>(this->chainFunction);
            while (currentAddress < static_cast<uint8_t*>(this->chainFunction) + 0x200) {
                if (*currentAddress == 0xFF && *(currentAddress + 1) == 0x15) {
                    // CALL instruction with ptr to absolute address
                    // This ptr is absolute on x86, and relative on x64
                    CALL_ABS callAbs = *reinterpret_cast<CALL_ABS*>(currentAddress);
                    uintptr_t ptr = callAbs.operand;
#ifdef _WIN64
                    ptr += reinterpret_cast<uintptr_t>(currentAddress) + sizeof(CALL_ABS);
#endif
                    uintptr_t address = reinterpret_cast<uintptr_t>(*reinterpret_cast<void**>(ptr));
                    if (address == reinterpret_cast<uintptr_t>(this->origFunction)) {
                        // This is the location of the absolute address we want to change
                        this->callbackHookAddress = ptr;
                        this->preHookCallbackDataSize = sizeof(uintptr_t);
                        *reinterpret_cast<uintptr_t*>(hookData) = reinterpret_cast<uintptr_t>(callback);
                    }
                    break;
                }
                ++currentAddress;
            }
            break;
        }
        }

        if (this->callbackHookAddress != 0) {
            void* address = reinterpret_cast<void*>(this->callbackHookAddress);
            DWORD oldProtection = this->DisableProtection(address, this->preHookCallbackDataSize);
            memcpy(this->preHookCallbackData, address, this->preHookCallbackDataSize);
            memcpy(address, hookData, this->preHookCallbackDataSize);
            this->ResetProtection(address, this->preHookCallbackDataSize, oldProtection);
            return true;
        }
        return false;
    }

    bool ChainHook::UnhookCallback() {
        if (!this->IsCallbackHooked()) {
            return false;
        }

        void* address = reinterpret_cast<void*>(this->callbackHookAddress);
        DWORD oldProtection = this->DisableProtection(address, this->preHookCallbackDataSize);
        memcpy(address, this->preHookCallbackData, this->preHookCallbackDataSize);
        this->ResetProtection(address, this->preHookCallbackDataSize, oldProtection);

        this->callbackHookAddress = 0;
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

}
