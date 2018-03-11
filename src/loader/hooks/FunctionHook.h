#pragma once
#include "../windows.h"
#include <stdint.h>

#define JMP32_SIZE 5

namespace loader {
    namespace hooks {

        class FunctionHook {
        public:
            FunctionHook() { }
            FunctionHook(uintptr_t origAddress) : origAddress(origAddress) { }

            bool SetOriginalAddress(uintptr_t origAddress);

            bool Hook(uintptr_t hookAddress);
            bool Unhook();
            bool IsHooked() const { return this->hooked; }

        protected:
            void DisableProtection();
            void ResetProtection();

        private:
            uintptr_t origAddress = 0;
            uintptr_t hookAddress = 0;
            bool hooked = false;
            uint8_t preHookData[JMP32_SIZE];

            bool unprotected = false;
            DWORD oldProtection;

        };

    }
}
