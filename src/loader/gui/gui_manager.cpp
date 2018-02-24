#include "gui_manager.h"
#include <map>
#include <set>
#include <d3dx9tex.h>
#include <imgui.h>
#include "Window.h"
#include "AddonInfoWindow.h"
#include "SettingsWindow.h"
#include "../resource.h"
#include "../utils.h"

using namespace std;

namespace loader {
    namespace gui {

        ImFont* FontMain;
        ImFont* FontIconButtons;

        map<shared_ptr<Window>, bool> openWindows;


        shared_ptr<AddonInfoWindow> AddonInfoWnd = make_shared<AddonInfoWindow>();
        shared_ptr<SettingsWindow> SettingsWnd = make_shared<SettingsWindow>();


        void ShowWindow(const shared_ptr<Window>& window) {
            openWindows[window] = true;
        }

        void CloseWindow(const shared_ptr<Window>& window) {
            openWindows[window] = false;
        }

        bool IsWindowOpen(const shared_ptr<Window>& window) {
            return openWindows.count(window) > 0 && openWindows[window];
        }


        void* LoadEmbeddedResource(HMODULE hModule, LPCWSTR lpName, LPCWSTR lpType, UINT* pSize) {
            HRSRC resource = FindResource(hModule, lpName, lpType);
            if (!resource) {
                return nullptr;
            }
            *pSize = SizeofResource(hModule, resource);
            if (!*pSize) {
                return nullptr;
            }
            HGLOBAL resourceData = LoadResource(hModule, resource);
            if (!resourceData) {
                return nullptr;
            }
            void* binaryData = LockResource(resourceData);
            if (!binaryData) {
                return nullptr;
            }
            return binaryData;
        }

        ImFontConfig GetFontConfig(HMODULE hModule, LPCWSTR lpName, LPCWSTR lpType, float fontSize) {
            UINT size;
            ImFontConfig fontConfig = {};
            fontConfig.FontData = LoadEmbeddedResource(hModule, lpName, lpType, &size);
            fontConfig.FontDataSize = size;
            fontConfig.FontDataOwnedByAtlas = false; // Don't let ImGui take over our resource memory, the OS handles this
            fontConfig.SizePixels = fontSize;
            return fontConfig;
        }

        void LoadFonts(HMODULE hModule) {
            ImGuiIO io = ImGui::GetIO();
            ImFontConfig fontConfigLato;
            ImFontConfig fontConfigMaterial;
      
            fontConfigLato = GetFontConfig(hModule, MAKEINTRESOURCE(IDR_LATO), L"TTF", 16);
            FontMain = io.Fonts->AddFont(&fontConfigLato);

            fontConfigMaterial = GetFontConfig(hModule, MAKEINTRESOURCE(IDR_MATERIAL), L"TTF", 16);
            fontConfigMaterial.GlyphRanges = FontMaterialIconsRange;
            fontConfigMaterial.GlyphOffset.y = 3;
            fontConfigMaterial.MergeMode = true;
            fontConfigMaterial.PixelSnapH = true;
            fontConfigMaterial.OversampleH = 1;
            io.Fonts->AddFont(&fontConfigMaterial);

            fontConfigMaterial = GetFontConfig(hModule, MAKEINTRESOURCE(IDR_MATERIAL), L"TTF", 32);
            fontConfigMaterial.GlyphRanges = FontMaterialIconsRange;
            fontConfigMaterial.GlyphOffset.y = -1;
            fontConfigMaterial.GlyphExtraSpacing.x = 2;
            fontConfigMaterial.PixelSnapH = true;
            fontConfigMaterial.OversampleH = 1;
            FontIconButtons = io.Fonts->AddFont(&fontConfigMaterial);

            fontConfigLato = GetFontConfig(hModule, MAKEINTRESOURCE(IDR_LATO), L"TTF", 16);
            fontConfigLato.GlyphOffset.y = -8;
            fontConfigLato.MergeMode = true;
            io.Fonts->AddFont(&fontConfigLato);
        }

        HRESULT LoadTexture(HMODULE hModule, IDirect3DDevice9* pDev, LPCWSTR lpName, LPCWSTR lpType, LPDIRECT3DTEXTURE9* ppTexture) {
            UINT size;
            void* data = LoadEmbeddedResource(hModule, lpName, lpType, &size);
            if (!data) {
                return D3DERR_NOTFOUND;
            }
            return D3DXCreateTextureFromFileInMemory(pDev, data, size, ppTexture);
        }

        void LoadTextures(HMODULE hModule, IDirect3DDevice9* pDev) {
            // If we ever want to have our own textures, this is the place to load them
        }

        void UnloadTextures() {
            // If we ever want to have our own textures, this is the place to unload them
        }


        void Render() {
            for (auto it = openWindows.begin(); it != openWindows.end(); ) {
                if (!it->second) {
                    it = openWindows.erase(it);
                    continue;
                }
                auto wnd = it->first;
                wnd->BeginStyle();
                switch (wnd->GetType()) {
                case WindowType::GenericWindow:
                    ImGui::Begin(ws2s(wnd->GetTitle()).c_str(), &it->second, wnd->GetFlags());
                    wnd->Render();
                    ImGui::End();
                    break;
                case WindowType::ModalWindow: {
                    string title = ws2s(wnd->GetTitle());
                    if (!ImGui::IsPopupOpen(title.c_str())) {
                        ImGui::OpenPopup(title.c_str());
                    }
                    if (ImGui::BeginPopupModal(title.c_str(), &it->second, wnd->GetFlags())) {
                        wnd->Render();
                        ImGui::EndPopup();
                    }
                    break;
                }
                }
                wnd->EndStyle();
                ++it;
            }
        }

    }
}
