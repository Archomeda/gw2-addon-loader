#include "FunctionHook.h"

namespace loader::hooks {

    bool FunctionHook::SetOriginalAddress(uintptr_t origAddress) {
        if (this->hooked) {
            return false;
        }

        this->origAddress = origAddress;
        return true;
    }

    bool FunctionHook::Hook(uintptr_t hookAddress) {
        if (this->hooked) {
            return false;
        }

        this->hookAddress = hookAddress;
        this->DisableProtection();
        this->hooked = true;

        void* origAddress = reinterpret_cast<void*>(this->origAddress);
        int32_t offset = static_cast<int32_t>(this->origAddress - hookAddress + JMP32_SIZE);
        uint8_t hookData[JMP32_SIZE];
        hookData[0] = 0xE9;
        *reinterpret_cast<int32_t*>(hookData) = offset;

        memcpy(this->preHookData, origAddress, JMP32_SIZE);
        memcpy(origAddress, hookData, JMP32_SIZE);

        this->ResetProtection();
        return true;
    }

    bool FunctionHook::Unhook() {
        if (!this->hooked) {
            return false;
        }

        this->DisableProtection();

        void* origAddress = reinterpret_cast<void*>(this->origAddress);
        memcpy(origAddress, this->preHookData, JMP32_SIZE);

        this->hooked = false;
        this->ResetProtection();
        return true;
    }

    void FunctionHook::DisableProtection() {
        if (!this->unprotected) {
            VirtualProtect(reinterpret_cast<void*>(this->origAddress), JMP32_SIZE, PAGE_EXECUTE_READWRITE, &this->oldProtection);
        }
    }

    void FunctionHook::ResetProtection() {
        if (this->unprotected) {
            DWORD replaceProtection;
            VirtualProtect(reinterpret_cast<void*>(this->origAddress), JMP32_SIZE, this->oldProtection, &replaceProtection);
        }
    }

}
