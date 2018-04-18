/**
This is an include file specifically for the GW2 Addon Loader.
*/

#pragma once
#include <d3d9.h>

typedef void* (WINAPI Direct3DShaderValidatorCreate9_t)(void); // Unsure, no documentation available
typedef void(WINAPI PSGPError_t)(void); // Unsure, no documentation available
typedef void(WINAPI PSGPSampleTexture_t)(void); // Unsure, no documentation available
typedef int(WINAPI D3DPERF_BeginEvent_t)(D3DCOLOR col, LPCWSTR wszName);
typedef int(WINAPI D3DPERF_EndEvent_t)(void);
typedef DWORD(WINAPI D3DPERF_GetStatus_t)(void);
typedef BOOL(WINAPI D3DPERF_QueryRepeatFrame_t)(void);
typedef void(WINAPI D3DPERF_SetMarker_t)(D3DCOLOR col, LPCWSTR wszName);
typedef void(WINAPI D3DPERF_SetOptions_t)(DWORD dwOptions);
typedef void(WINAPI D3DPERF_SetRegion_t)(D3DCOLOR col, LPCWSTR wszName);
typedef void(WINAPI DebugSetLevel_t)(LONG level); // Unsure, no documentation available
typedef void(WINAPI DebugSetMute_t)(void);
typedef void(WINAPI Direct3D9EnableMaximizedWindowedModeShim_t)(void); // Unsure, no documentation available
typedef IDirect3D9* (WINAPI Direct3DCreate9_t)(UINT SDKVersion);
typedef IDirect3D9Ex* (WINAPI Direct3DCreate9Ex_t)(UINT SDKVersion);

struct D3D9Exports {
    Direct3DShaderValidatorCreate9_t* Direct3DShaderValidatorCreate9;
    PSGPError_t* PSGPError;
    PSGPSampleTexture_t* PSGPSampleTexture;
    D3DPERF_BeginEvent_t* D3DPERF_BeginEvent;
    D3DPERF_EndEvent_t* D3DPERF_EndEvent;
    D3DPERF_GetStatus_t* D3DPERF_GetStatus;
    D3DPERF_QueryRepeatFrame_t* D3DPERF_QueryRepeatFrame;
    D3DPERF_SetMarker_t* D3DPERF_SetMarker;
    D3DPERF_SetOptions_t* D3DPERF_SetOptions;
    D3DPERF_SetRegion_t* D3DPERF_SetRegion;
    DebugSetLevel_t* DebugSetLevel;
    DebugSetMute_t* DebugSetMute;
    Direct3D9EnableMaximizedWindowedModeShim_t* Direct3D9EnableMaximizedWindowedModeShim;
    Direct3DCreate9_t* Direct3DCreate9;
    Direct3DCreate9Ex_t* Direct3DCreate9Ex;
};
