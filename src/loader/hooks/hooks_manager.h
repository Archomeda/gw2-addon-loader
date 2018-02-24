#pragma once
#include "../windows.h"

namespace loader {
    namespace hooks {

        extern UINT SDKVersion;

        void InitializeHooks();
        void UninitializeHooks();

    }
}
