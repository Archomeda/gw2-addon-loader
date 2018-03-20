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

            class AddonFuncException : public AddonException {
            public:
                AddonFuncException(const std::string& funcName, const std::string& message) :
                    AddonException("(" + funcName + ") " + message),
                    funcName(funcName) { }
                AddonFuncException(const std::string& funcName, const std::wstring& message) :
                    AddonFuncException(funcName, utils::u8(message)) { }
                AddonFuncException(const std::wstring& funcName, const std::string& message) :
                    AddonFuncException(utils::u8(funcName), message) { }
                AddonFuncException(const std::wstring& funcName, const std::wstring& message) :
                    AddonFuncException(utils::u8(funcName), utils::u8(message)) { }

                const std::string& GetFuncName() const { return this->funcName; }

            private:
                std::string funcName;
            };

        }
    }
}
