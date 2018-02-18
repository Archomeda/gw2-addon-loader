#include <d3d9.h>
#include "vftable.h"

BOOL CompareVftPtr(IUnknown* objA, IUnknown* objB) {
    return objA->lpVtbl == objB->lpVtbl;
}

BOOL CompareD3D9Vft(IDirect3D9* obj, D3D9_vft vft) {
    return obj->lpVtbl->QueryInterface == vft.QueryInterface &&
        obj->lpVtbl->AddRef == vft.AddRef &&
        obj->lpVtbl->Release == vft.Release &&
        obj->lpVtbl->RegisterSoftwareDevice == vft.RegisterSoftwareDevice &&
        obj->lpVtbl->GetAdapterCount == vft.GetAdapterCount &&
        obj->lpVtbl->GetAdapterIdentifier == vft.GetAdapterIdentifier &&
        obj->lpVtbl->GetAdapterModeCount == vft.GetAdapterModeCount &&
        obj->lpVtbl->EnumAdapterModes == vft.EnumAdapterModes &&
        obj->lpVtbl->GetAdapterDisplayMode == vft.GetAdapterDisplayMode &&
        obj->lpVtbl->CheckDeviceType == vft.CheckDeviceType &&
        obj->lpVtbl->CheckDeviceFormat == vft.CheckDeviceFormat &&
        obj->lpVtbl->CheckDeviceMultiSampleType == vft.CheckDeviceMultiSampleType &&
        obj->lpVtbl->CheckDepthStencilMatch == vft.CheckDepthStencilMatch &&
        obj->lpVtbl->CheckDeviceFormatConversion == vft.CheckDeviceFormatConversion &&
        obj->lpVtbl->GetDeviceCaps == vft.GetDeviceCaps &&
        obj->lpVtbl->GetAdapterMonitor == vft.GetAdapterMonitor &&
        obj->lpVtbl->CreateDevice == vft.CreateDevice;
}

BOOL CompareD3D9ExVft(IDirect3D9Ex* obj, D3D9Ex_vft vft) {
    return obj->lpVtbl->QueryInterface == vft.QueryInterface &&
        obj->lpVtbl->AddRef == vft.AddRef &&
        obj->lpVtbl->Release == vft.Release &&
        obj->lpVtbl->RegisterSoftwareDevice == vft.RegisterSoftwareDevice &&
        obj->lpVtbl->GetAdapterCount == vft.GetAdapterCount &&
        obj->lpVtbl->GetAdapterIdentifier == vft.GetAdapterIdentifier &&
        obj->lpVtbl->GetAdapterModeCount == vft.GetAdapterModeCount &&
        obj->lpVtbl->EnumAdapterModes == vft.EnumAdapterModes &&
        obj->lpVtbl->GetAdapterDisplayMode == vft.GetAdapterDisplayMode &&
        obj->lpVtbl->CheckDeviceType == vft.CheckDeviceType &&
        obj->lpVtbl->CheckDeviceFormat == vft.CheckDeviceFormat &&
        obj->lpVtbl->CheckDeviceMultiSampleType == vft.CheckDeviceMultiSampleType &&
        obj->lpVtbl->CheckDepthStencilMatch == vft.CheckDepthStencilMatch &&
        obj->lpVtbl->CheckDeviceFormatConversion == vft.CheckDeviceFormatConversion &&
        obj->lpVtbl->GetDeviceCaps == vft.GetDeviceCaps &&
        obj->lpVtbl->GetAdapterMonitor == vft.GetAdapterMonitor &&
        obj->lpVtbl->GetAdapterModeCountEx == vft.GetAdapterModeCountEx &&
        obj->lpVtbl->EnumAdapterModesEx == vft.EnumAdapterModesEx &&
        obj->lpVtbl->GetAdapterDisplayModeEx == vft.GetAdapterDisplayModeEx &&
        obj->lpVtbl->CreateDeviceEx == vft.CreateDeviceEx &&
        obj->lpVtbl->GetAdapterLUID == vft.GetAdapterLUID;
}

void CloneD3D9Vft(IDirect3D9* objFrom, IDirect3D9* objTo) {
    IDirect3D9Vtbl* newVtbl = malloc(sizeof(*objFrom->lpVtbl));
    *newVtbl = *objFrom->lpVtbl;
    objTo->lpVtbl = newVtbl;
}

void CloneD3D9ExVft(IDirect3D9Ex* objFrom, IDirect3D9Ex* objTo) {
    IDirect3D9ExVtbl* newVtbl = malloc(sizeof(*objFrom->lpVtbl));
    *newVtbl = *objFrom->lpVtbl;
    objTo->lpVtbl = newVtbl;
}

D3D9_vft GetVftD3D9(IDirect3D9* obj) {
    D3D9_vft vft = { 0 };
    vft.QueryInterface = obj->lpVtbl->QueryInterface;
    vft.AddRef = obj->lpVtbl->AddRef;
    vft.Release = obj->lpVtbl->Release;
    vft.RegisterSoftwareDevice = obj->lpVtbl->RegisterSoftwareDevice;
    vft.GetAdapterCount = obj->lpVtbl->GetAdapterCount;
    vft.GetAdapterIdentifier = obj->lpVtbl->GetAdapterIdentifier;
    vft.GetAdapterModeCount = obj->lpVtbl->GetAdapterModeCount;
    vft.EnumAdapterModes = obj->lpVtbl->EnumAdapterModes;
    vft.GetAdapterDisplayMode = obj->lpVtbl->GetAdapterDisplayMode;
    vft.CheckDeviceType = obj->lpVtbl->CheckDeviceType;
    vft.CheckDeviceFormat = obj->lpVtbl->CheckDeviceFormat;
    vft.CheckDeviceMultiSampleType = obj->lpVtbl->CheckDeviceMultiSampleType;
    vft.CheckDepthStencilMatch = obj->lpVtbl->CheckDepthStencilMatch;
    vft.CheckDeviceFormatConversion = obj->lpVtbl->CheckDeviceFormatConversion;
    vft.GetDeviceCaps = obj->lpVtbl->GetDeviceCaps;
    vft.GetAdapterMonitor = obj->lpVtbl->GetAdapterMonitor;
    vft.CreateDevice = obj->lpVtbl->CreateDevice;
    return vft;
}

void SetVftD3D9(IDirect3D9* obj, D3D9_vft vft) {
    DWORD protection = PAGE_READWRITE;
    if (VirtualProtect(obj->lpVtbl, sizeof(*obj->lpVtbl), protection, &protection)) {
        obj->lpVtbl->QueryInterface = vft.QueryInterface;
        obj->lpVtbl->AddRef = vft.AddRef;
        obj->lpVtbl->Release = vft.Release;
        obj->lpVtbl->RegisterSoftwareDevice = vft.RegisterSoftwareDevice;
        obj->lpVtbl->GetAdapterCount = vft.GetAdapterCount;
        obj->lpVtbl->GetAdapterIdentifier = vft.GetAdapterIdentifier;
        obj->lpVtbl->GetAdapterModeCount = vft.GetAdapterModeCount;
        obj->lpVtbl->EnumAdapterModes = vft.EnumAdapterModes;
        obj->lpVtbl->GetAdapterDisplayMode = vft.GetAdapterDisplayMode;
        obj->lpVtbl->CheckDeviceType = vft.CheckDeviceType;
        obj->lpVtbl->CheckDeviceFormat = vft.CheckDeviceFormat;
        obj->lpVtbl->CheckDeviceMultiSampleType = vft.CheckDeviceMultiSampleType;
        obj->lpVtbl->CheckDepthStencilMatch = vft.CheckDepthStencilMatch;
        obj->lpVtbl->CheckDeviceFormatConversion = vft.CheckDeviceFormatConversion;
        obj->lpVtbl->GetDeviceCaps = vft.GetDeviceCaps;
        obj->lpVtbl->GetAdapterMonitor = vft.GetAdapterMonitor;
        obj->lpVtbl->CreateDevice = vft.CreateDevice;
        VirtualProtect(obj->lpVtbl, sizeof(*obj->lpVtbl), protection, &protection);
    }
}

D3D9Ex_vft GetVftD3D9Ex(IDirect3D9Ex* obj) {
    D3D9Ex_vft vft = { 0 };
    vft.QueryInterface = obj->lpVtbl->QueryInterface;
    vft.AddRef = obj->lpVtbl->AddRef;
    vft.Release = obj->lpVtbl->Release;
    vft.RegisterSoftwareDevice = obj->lpVtbl->RegisterSoftwareDevice;
    vft.GetAdapterCount = obj->lpVtbl->GetAdapterCount;
    vft.GetAdapterIdentifier = obj->lpVtbl->GetAdapterIdentifier;
    vft.GetAdapterModeCount = obj->lpVtbl->GetAdapterModeCount;
    vft.EnumAdapterModes = obj->lpVtbl->EnumAdapterModes;
    vft.GetAdapterDisplayMode = obj->lpVtbl->GetAdapterDisplayMode;
    vft.CheckDeviceType = obj->lpVtbl->CheckDeviceType;
    vft.CheckDeviceFormat = obj->lpVtbl->CheckDeviceFormat;
    vft.CheckDeviceMultiSampleType = obj->lpVtbl->CheckDeviceMultiSampleType;
    vft.CheckDepthStencilMatch = obj->lpVtbl->CheckDepthStencilMatch;
    vft.CheckDeviceFormatConversion = obj->lpVtbl->CheckDeviceFormatConversion;
    vft.GetDeviceCaps = obj->lpVtbl->GetDeviceCaps;
    vft.GetAdapterMonitor = obj->lpVtbl->GetAdapterMonitor;
    vft.CreateDevice = obj->lpVtbl->CreateDevice;
    vft.GetAdapterModeCountEx = obj->lpVtbl->GetAdapterModeCountEx;
    vft.EnumAdapterModesEx = obj->lpVtbl->EnumAdapterModesEx;
    vft.GetAdapterDisplayModeEx = obj->lpVtbl->GetAdapterDisplayModeEx;
    vft.CreateDeviceEx = obj->lpVtbl->CreateDeviceEx;
    vft.GetAdapterLUID = obj->lpVtbl->GetAdapterLUID;
    return vft;
}

void SetVftD3D9Ex(IDirect3D9Ex* obj, D3D9Ex_vft vft) {
    DWORD protection = PAGE_READWRITE;
    if (VirtualProtect(obj->lpVtbl, sizeof(*obj->lpVtbl), protection, &protection)) {
        obj->lpVtbl->QueryInterface = vft.QueryInterface;
        obj->lpVtbl->AddRef = vft.AddRef;
        obj->lpVtbl->Release = vft.Release;
        obj->lpVtbl->RegisterSoftwareDevice = vft.RegisterSoftwareDevice;
        obj->lpVtbl->GetAdapterCount = vft.GetAdapterCount;
        obj->lpVtbl->GetAdapterIdentifier = vft.GetAdapterIdentifier;
        obj->lpVtbl->GetAdapterModeCount = vft.GetAdapterModeCount;
        obj->lpVtbl->EnumAdapterModes = vft.EnumAdapterModes;
        obj->lpVtbl->GetAdapterDisplayMode = vft.GetAdapterDisplayMode;
        obj->lpVtbl->CheckDeviceType = vft.CheckDeviceType;
        obj->lpVtbl->CheckDeviceFormat = vft.CheckDeviceFormat;
        obj->lpVtbl->CheckDeviceMultiSampleType = vft.CheckDeviceMultiSampleType;
        obj->lpVtbl->CheckDepthStencilMatch = vft.CheckDepthStencilMatch;
        obj->lpVtbl->CheckDeviceFormatConversion = vft.CheckDeviceFormatConversion;
        obj->lpVtbl->GetDeviceCaps = vft.GetDeviceCaps;
        obj->lpVtbl->GetAdapterMonitor = vft.GetAdapterMonitor;
        obj->lpVtbl->CreateDevice = vft.CreateDevice;
        obj->lpVtbl->GetAdapterModeCountEx = vft.GetAdapterModeCountEx;
        obj->lpVtbl->EnumAdapterModesEx = vft.EnumAdapterModesEx;
        obj->lpVtbl->GetAdapterDisplayModeEx = vft.GetAdapterDisplayModeEx;
        obj->lpVtbl->CreateDeviceEx = vft.CreateDeviceEx;
        obj->lpVtbl->GetAdapterLUID = vft.GetAdapterLUID;
        VirtualProtect(obj->lpVtbl, sizeof(*obj->lpVtbl), protection, &protection);
    }
}

D3DDevice9_vft GetVftD3DDevice9(IDirect3DDevice9 * obj) {
    D3DDevice9_vft vft = { 0 };
    vft.Present = obj->lpVtbl->Present;
    vft.Reset = obj->lpVtbl->Reset;
    vft.Release = obj->lpVtbl->Release;
    vft.AddRef = obj->lpVtbl->AddRef;
    return vft;
}

D3DDevice9_vft GetVftD3DDevice9Ex(IDirect3DDevice9Ex * obj) {
    D3DDevice9_vft vft = { 0 };
    vft.Present = obj->lpVtbl->Present;
    vft.PresentEx = obj->lpVtbl->PresentEx;
    vft.Reset = obj->lpVtbl->Reset;
    vft.ResetEx = obj->lpVtbl->ResetEx;
    vft.Release = obj->lpVtbl->Release;
    vft.AddRef = obj->lpVtbl->AddRef;
    return vft;
}
