#pragma once
#include "../stdafx.h"

namespace loader::hooks {

    extern UINT SDKVersion;

    void InitializeHooks();
    void UninitializeHooks();

}
