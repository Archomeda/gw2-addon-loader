#pragma once
#include <string>
#include <gw2addon-native.h>
#include "ITypeImpl.h"
#include "../../d3d9types.h"

namespace loader {
    namespace addons {
        namespace types {

            class NativeAddonImpl : public ITypeImpl {
            public:
                NativeAddonImpl(const std::wstring& filePath);

                virtual bool SupportsHotLoading() const override { return true; };
                virtual void Initialize() override;
                virtual void Uninitialize() override;
                virtual void Load() override;
                virtual void Unload() override;
                virtual void DrawFrame(IDirect3DDevice9* device) override;
                virtual void DrawFrameBeforeGui(IDirect3DDevice9* device) override;
                virtual bool HandleWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) override;

            private:
                GW2Load GW2_Load = nullptr;
                GW2Unload GW2_Unload = nullptr;
                GW2DrawFrame GW2_DrawFrame = nullptr;
                GW2DrawFrameBeforeGui GW2_DrawFrameBeforeGui = nullptr;
                GW2HandleWndProc GW2_HandleWndProc = nullptr;

                std::wstring filePath;
                std::wstring fileName;
            };

        }
    }
}
