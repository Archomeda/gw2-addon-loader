#pragma once
#include <stdexcept>
#include <string>
#include "../utils/encoding.h"

namespace loader {
    namespace addons {
        namespace exceptions {

            class AddonException : public std::runtime_error {
            public:
                AddonException(const std::string& message) : 
                    runtime_error(message) { }
                AddonException(const std::wstring& message) :
                    runtime_error(utils::u8(message)) { }
            };

            class AddonInitializationException : public AddonException {
            public:
                AddonInitializationException(const std::string& message) :
                    AddonException(message) { }
                AddonInitializationException(const std::wstring& message) :
                    AddonException(message) { }
            };

            class AddonLoadingException : public AddonException {
            public:
                AddonLoadingException(const std::string& message) :
                    AddonException(message) { }
                AddonLoadingException(const std::wstring& message) :
                    AddonException(message) { }
            };

            class AddonUnloadingException : public AddonException {
            public:
                AddonUnloadingException(const std::string& message) :
                    AddonException(message) { }
                AddonUnloadingException(const std::wstring& message) :
                    AddonException(message) { }
            };

            class AddonDrawException : public AddonException {
            public:
                AddonDrawException(const std::string& message) :
                    AddonException(message) { }
                AddonDrawException(const std::wstring& message) :
                    AddonException(message) { }
            };

            class AddonWndProcException : public AddonException {
            public:
                AddonWndProcException(const std::string& message) :
                    AddonException(message) { }
                AddonWndProcException(const std::wstring& message) :
                    AddonException(message) { }
            };

            class AddonAdvFuncException : public AddonException {
            public:
                AddonAdvFuncException(const std::string& funcName, const std::string& message) :
                    AddonException("(" + funcName + ") " + message),
                    funcName(funcName) { }
                AddonAdvFuncException(const std::string& funcName, const std::wstring& message) :
                    AddonAdvFuncException(funcName, utils::u8(message)) { }
                AddonAdvFuncException(const std::wstring& funcName, const std::string& message) :
                    AddonAdvFuncException(utils::u8(funcName), message) { }
                AddonAdvFuncException(const std::wstring& funcName, const std::wstring& message) :
                    AddonAdvFuncException(utils::u8(funcName), utils::u8(message)) { }

                const std::string& GetFuncName() const { return this->funcName; }

            private:
                std::string funcName;
            };

        }
    }
}
