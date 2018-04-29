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

template<typename T>
struct D3DDevice9Functions {
    T QueryInterface;
    T AddRef;
    T Release;
    T TestCooperativeLevel;
    T GetAvailableTextureMem;
    T EvictManagedResources;
    T GetDirect3D;
    T GetDeviceCaps;
    T GetDisplayMode;
    T GetCreationParameters;
    T SetCursorProperties;
    T SetCursorPosition;
    T ShowCursor;
    T CreateAdditionalSwapChain;
    T GetSwapChain;
    T GetNumberOfSwapChains;
    T Reset;
    T Present;
    T GetBackBuffer;
    T GetRasterStatus;
    T SetDialogBoxMode;
    T SetGammaRamp;
    T GetGammaRamp;
    T CreateTexture;
    T CreateVolumeTexture;
    T CreateCubeTexture;
    T CreateVertexBuffer;
    T CreateIndexBuffer;
    T CreateRenderTarget;
    T CreateDepthStencilSurface;
    T UpdateSurface;
    T UpdateTexture;
    T GetRenderTargetData;
    T GetFrontBufferData;
    T StretchRect;
    T ColorFill;
    T CreateOffscreenPlainSurface;
    T SetRenderTarget;
    T GetRenderTarget;
    T SetDepthStencilSurface;
    T GetDepthStencilSurface;
    T BeginScene;
    T EndScene;
    T Clear;
    T SetTransform;
    T GetTransform;
    T MultiplyTransform;
    T SetViewport;
    T GetViewport;
    T SetMaterial;
    T GetMaterial;
    T SetLight;
    T GetLight;
    T LightEnable;
    T GetLightEnable;
    T SetClipPlane;
    T GetClipPlane;
    T SetRenderState;
    T GetRenderState;
    T CreateStateBlock;
    T BeginStateBlock;
    T EndStateBlock;
    T SetClipStatus;
    T GetClipStatus;
    T GetTexture;
    T SetTexture;
    T GetTextureStageState;
    T SetTextureStageState;
    T GetSamplerState;
    T SetSamplerState;
    T ValidateDevice;
    T SetPaletteEntries;
    T GetPaletteEntries;
    T SetCurrentTexturePalette;
    T GetCurrentTexturePalette;
    T SetScissorRect;
    T GetScissorRect;
    T SetSoftwareVertexProcessing;
    T GetSoftwareVertexProcessing;
    T SetNPatchMode;
    T GetNPatchMode;
    T DrawPrimitive;
    T DrawIndexedPrimitive;
    T DrawPrimitiveUP;
    T DrawIndexedPrimitiveUP;
    T ProcessVertices;
    T CreateVertexDeclaration;
    T SetVertexDeclaration;
    T GetVertexDeclaration;
    T SetFVF;
    T GetFVF;
    T CreateVertexShader;
    T SetVertexShader;
    T GetVertexShader;
    T SetVertexShaderConstantF;
    T GetVertexShaderConstantF;
    T SetVertexShaderConstantI;
    T GetVertexShaderConstantI;
    T SetVertexShaderConstantB;
    T GetVertexShaderConstantB;
    T SetStreamSource;
    T GetStreamSource;
    T SetStreamSourceFreq;
    T GetStreamSourceFreq;
    T SetIndices;
    T GetIndices;
    T CreatePixelShader;
    T SetPixelShader;
    T GetPixelShader;
    T SetPixelShaderConstantF;
    T GetPixelShaderConstantF;
    T SetPixelShaderConstantI;
    T GetPixelShaderConstantI;
    T SetPixelShaderConstantB;
    T GetPixelShaderConstantB;
    T DrawRectPatch;
    T DrawTriPatch;
    T DeletePatch;
    T CreateQuery;
    T SetConvolutionMonoKernel;
    T ComposeRects;
    T PresentEx;
    T GetGPUThreadPriority;
    T SetGPUThreadPriority;
    T WaitForVBlank;
    T CheckResourceResidency;
    T SetMaximumFrameLatency;
    T GetMaximumFrameLatency;
    T CheckDeviceState;
    T CreateRenderTargetEx;
    T CreateOffscreenPlainSurfaceEx;
    T CreateDepthStencilSurfaceEx;
    T ResetEx;
    T GetDisplayModeEx;
};

