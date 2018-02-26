#pragma once
#include "../windows.h"
#include <d3dx9.h>
#include <memory>
#include <imgui.h>
#include <IconsMaterialDesign.h>

namespace loader {
    namespace gui {

        class Window;
        class AddonInfoWindow;
        class SettingsWindow;

        static const ImWchar FontMaterialIconsRange[] = { ICON_MIN_MD, ICON_MAX_MD, 0 };
        extern ImFont* FontMain;
        extern ImFont* FontIconButtons;

        extern std::shared_ptr<AddonInfoWindow> AddonInfoWnd;
        extern std::shared_ptr<SettingsWindow> SettingsWnd;


        void ShowWindow(const std::shared_ptr<Window>& window);
        void CloseWindow(const std::shared_ptr<Window>& window);
        bool IsWindowOpen(const std::shared_ptr<Window>& window);

        void* LoadEmbeddedResource(HMODULE hModule, LPCWSTR lpName, LPCWSTR lpType, UINT* pSize);
        ImFontConfig GetFontConfig(HMODULE hModule, LPCWSTR lpName, LPCWSTR lpType, float fontSize);
        void LoadFonts(HMODULE hModule);
        HRESULT LoadTexture(HMODULE hModule, IDirect3DDevice9* pDev, LPCWSTR lpName, LPCWSTR lpType, LPDIRECT3DTEXTURE9* ppTexture);
        void LoadTextures(HMODULE hModule, IDirect3DDevice9* pDev);
        void UnloadTextures();
        
        void Render();

    }
}
