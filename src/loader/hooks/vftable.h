#pragma once
#include "../windows.h"
#include <d3d9.h>

#ifdef __cplusplus
extern "C" {
#endif

    typedef struct {
        // Expand to full vtable when needed
        LPVOID Present;
        LPVOID Reset;
        LPVOID Release;
        LPVOID AddRef;
    } D3DDevice9_vft;

    D3DDevice9_vft GetVftD3DDevice9(IDirect3DDevice9* obj);

#ifdef __cplusplus
}
#endif
