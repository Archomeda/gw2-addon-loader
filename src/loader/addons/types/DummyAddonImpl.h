#pragma once
#include <string>
#include "ITypeImpl.h"
#include "../../d3d9types.h"

namespace loader {
    namespace addons {
        namespace types {

            class DummyAddonImpl : public ITypeImpl {
            public:
                DummyAddonImpl(const std::wstring& filePath);

                virtual bool SupportsHotLoading() const override { return false; };
                virtual void Initialize() override;
                virtual void Uninitialize() override;
                virtual void Load() override;
                virtual void Unload() override;
                virtual void DrawFrame(IDirect3DDevice9* device) override;
                virtual bool HandleWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) override;
            };

        }
    }
}
