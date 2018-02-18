#pragma once
#include <d3d9.h>

#ifdef __cplusplus
extern "C" {
#endif

    typedef struct {
        LPVOID QueryInterface;
        LPVOID AddRef;
        LPVOID Release;
        LPVOID RegisterSoftwareDevice;
        LPVOID GetAdapterCount;
        LPVOID GetAdapterIdentifier;
        LPVOID GetAdapterModeCount;
        LPVOID EnumAdapterModes;
        LPVOID GetAdapterDisplayMode;
        LPVOID CheckDeviceType;
        LPVOID CheckDeviceFormat;
        LPVOID CheckDeviceMultiSampleType;
        LPVOID CheckDepthStencilMatch;
        LPVOID CheckDeviceFormatConversion;
        LPVOID GetDeviceCaps;
        LPVOID GetAdapterMonitor;
        LPVOID CreateDevice;
    } D3D9_vft;
    D3D9_vft GetVftD3D9(IDirect3D9* obj);
    void SetVftD3D9(IDirect3D9* obj, D3D9_vft vft);

    typedef struct {
        LPVOID QueryInterface;
        LPVOID AddRef;
        LPVOID Release;
        LPVOID RegisterSoftwareDevice;
        LPVOID GetAdapterCount;
        LPVOID GetAdapterIdentifier;
        LPVOID GetAdapterModeCount;
        LPVOID EnumAdapterModes;
        LPVOID GetAdapterDisplayMode;
        LPVOID CheckDeviceType;
        LPVOID CheckDeviceFormat;
        LPVOID CheckDeviceMultiSampleType;
        LPVOID CheckDepthStencilMatch;
        LPVOID CheckDeviceFormatConversion;
        LPVOID GetDeviceCaps;
        LPVOID GetAdapterMonitor;
        LPVOID CreateDevice;
        LPVOID GetAdapterModeCountEx;
        LPVOID EnumAdapterModesEx;
        LPVOID GetAdapterDisplayModeEx;
        LPVOID CreateDeviceEx;
        LPVOID GetAdapterLUID;
    } D3D9Ex_vft;
    D3D9Ex_vft GetVftD3D9Ex(IDirect3D9Ex* obj);
    void SetVftD3D9Ex(IDirect3D9Ex* obj, D3D9Ex_vft vft);

    BOOL CompareVftPtr(IUnknown* objA, IUnknown* objB);
    BOOL CompareD3D9Vft(IDirect3D9* obj, D3D9_vft vft);
    BOOL CompareD3D9ExVft(IDirect3D9Ex* obj, D3D9Ex_vft vft);
    void CloneD3D9Vft(IDirect3D9* objFrom, IDirect3D9* objTo);
    void CloneD3D9ExVft(IDirect3D9Ex* objFrom, IDirect3D9Ex* objTo);

    typedef HRESULT(WINAPI *Present_t)(IDirect3DDevice9* _this, CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion);
    typedef HRESULT(WINAPI *PresentEx_t)(IDirect3DDevice9Ex* _this, CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion, DWORD dwFlags);
    typedef HRESULT(WINAPI *Reset_t)(IDirect3DDevice9* _this, D3DPRESENT_PARAMETERS* pPresentationParameters);
    typedef HRESULT(WINAPI *ResetEx_t)(IDirect3DDevice9Ex* _this, D3DPRESENT_PARAMETERS* pPresentationParameters, D3DDISPLAYMODEEX *pFullscreenDisplayMode);

    typedef struct {
        LPVOID Present;
        LPVOID PresentEx;
        LPVOID Reset;
        LPVOID ResetEx;
        LPVOID Release;
        LPVOID AddRef;
    } D3DDevice9_vft;

    D3DDevice9_vft GetVftD3DDevice9(IDirect3DDevice9* obj);
    D3DDevice9_vft GetVftD3DDevice9Ex(IDirect3DDevice9Ex* obj);

#ifdef __cplusplus
}
#endif
