#include "vftable.h"

D3DDevice9_vft GetVftD3DDevice9(IDirect3DDevice9* obj) {
    D3DDevice9_vft vft = { 0 };
    vft.Present = obj->lpVtbl->Present;
    vft.Reset = obj->lpVtbl->Reset;
    vft.Release = obj->lpVtbl->Release;
    vft.AddRef = obj->lpVtbl->AddRef;
    return vft;
}
