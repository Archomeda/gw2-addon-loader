#pragma once
#include <stdexcept>
#include <string>

namespace loader {
    namespace addons {
        namespace exceptions {

            class AddonException : public std::runtime_error {
            public:
                AddonException(const std::string& message) : 
                    runtime_error(message) { }
            };

            class AddonInitializationException : public AddonException {
            public:
                AddonInitializationException(const std::string& message) :
                    AddonException(message) { }
            };

            class AddonLoadingException : public AddonException {
            public:
                AddonLoadingException(const std::string& message) :
                    AddonException(message) { }
            };

            class AddonUnloadingException : public AddonException {
            public:
                AddonUnloadingException(const std::string& message) :
                    AddonException(message) { }
            };

            class AddonDrawFrameException : public AddonException {
            public:
                AddonDrawFrameException(const std::string& message) :
                    AddonException(message) { }
            };

            class AddonWndProcException : public AddonException {
            public:
                AddonWndProcException(const std::string& message) :
                    AddonException(message) { }
            };
        }
    }
}
