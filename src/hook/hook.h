#pragma once

bool InitializeHook();
void UninitializeHook();

void PreCreateDevice(HWND hFocusWindow);
void PostCreateDevice(IDirect3DDevice9* temp_device, D3DPRESENT_PARAMETERS *pPresentationParameters);
void Draw(IDirect3DDevice9* dev);
void PreReset();
void PostReset(IDirect3DDevice9* dev, D3DPRESENT_PARAMETERS *pPresentationParameters);
