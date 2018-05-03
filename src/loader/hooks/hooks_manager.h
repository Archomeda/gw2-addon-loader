#pragma once
#include "../stdafx.h"

namespace loader {
    namespace hooks {

        extern UINT SDKVersion;

        void InitializeHooks();
        void UninitializeHooks();

    }
}
