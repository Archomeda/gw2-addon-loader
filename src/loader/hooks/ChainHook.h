#pragma once
#include "../windows.h"
#include <map>
#include <string>

#define CALLN32_SIZE 6
#define HOOK_MAXSIZE 6

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

        std::string ChainHookTypeToString(ChainHookType type);

        class ChainHook {
        public:
            static ChainHook FindCurrentChainHook(ChainHookFunctionType functionType, void* func);

            ChainHookType GetType() const { return this->type; }
            ChainHookFunctionType GetFunctionType() const { return this->functionType; }
            void* GetOriginatingFunction() const { return this->origFunction; }
            void* GetChainFunction() const { return this->chainFunction; }

            bool HookCallback(void* callback);
            bool UnhookCallback();
            bool IsCallbackHooked() const { return this->callbackHookAddress != nullptr; }

        protected:
            DWORD DisableProtection(void* address, size_t length);
            void ResetProtection(void* address, size_t length, DWORD reset);

            uint8_t* FindMatch(uint8_t* startAddress, size_t searchLength, const uint8_t bytes[], size_t bytesSize) const;

        private:
            static std::map<HMODULE, ChainHookType> knownHooks;

            ChainHookType type = ChainHookType::NoHookType;
            ChainHookFunctionType functionType = ChainHookFunctionType::NoHookFunctionType;
            void* origFunction;
            void* chainFunction;

            void* callbackHookAddress = nullptr;
            uint8_t preHookCallbackData[HOOK_MAXSIZE];
            uint8_t preHookCallbackDataSize = 0;

            bool HookPresentCallback(void* callback);

        };

    }
}
