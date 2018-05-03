#pragma once
#include "../windows.h"
#include <map>
#include <string>

namespace loader {
    namespace hooks {

        enum ChainHookType {
            NoHookType,
            OBSHook
        };

        enum ChainHookFunctionType {
            NoHookFunctionType,
            PresentFunction
        };

        const std::string ChainHookTypeToString(ChainHookType type);

        class ChainHook {
        public:
            static ChainHook FindCurrentChainHook(ChainHookFunctionType functionType, void* func);

            ChainHookType GetType() const { return this->type; }
            const std::string GetTypeString() const { return ChainHookTypeToString(this->GetType()); }
            ChainHookFunctionType GetFunctionType() const { return this->functionType; }
            void* GetOriginatingFunction() const { return this->origFunction; }
            void* GetChainFunction() const { return this->chainFunction; }

            bool HookCallback(void* callback);
            bool UnhookCallback();
            bool IsCallbackHooked() const { return this->callbackHookAddress != 0; }

        protected:
            DWORD DisableProtection(void* address, size_t length);
            void ResetProtection(void* address, size_t length, DWORD reset);

        private:
            bool HookPresentCallback(void* callback);

            static std::map<HMODULE, ChainHookType> knownHooks;

            ChainHookType type = ChainHookType::NoHookType;
            ChainHookFunctionType functionType = ChainHookFunctionType::NoHookFunctionType;
            void* origFunction;
            void* chainFunction;

            uintptr_t callbackHookAddress = 0;
            uint8_t preHookCallbackData[16];
            uint8_t preHookCallbackDataSize = 0;
        };

    }
}
