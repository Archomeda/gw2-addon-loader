#pragma once
#include <Windows.h>
#include <d3d9.h>
#include <memory>
#include <string>

namespace loader {
    namespace addons {

        // Forward declare Addon, because we use weak_ptr
        class Addon;

        namespace types {

            enum AddonSubType {
                NoSubType,
                GraphicsAddon,
                NonGraphicsAddon,
                PointerReplacingAddon,
                VirtualTableReplacingAddon,
                WrapperAddon
            };

            enum AddonState {
                UnknownState,
                DeactivatedOnRestartState,
                ActivatedOnRestartState,
                ErroredState,
                UnloadingState,
                UnloadedState,
                LoadingState,
                LoadedState
            };

            class ITypeImpl {
            public:
                std::weak_ptr<Addon> GetAddon() const { return this->addon; }
                void SetAddon(std::weak_ptr<Addon> addon) { this->addon = addon; }

                virtual HMODULE GetHandle() const { return this->handle; }

                virtual const AddonSubType GetAddonSubType() const { return this->subType; }
                virtual const std::wstring GetAddonSubTypeString() const;

                virtual const AddonState GetAddonState() const { return this->state; }
                virtual const std::wstring GetAddonStateString() const;

                virtual bool SupportsHotLoading() const = 0;
                virtual void Initialize() = 0;
                virtual void Uninitialize() = 0;
                virtual void Load() = 0;
                virtual void Unload() = 0;
                virtual void DrawFrameBeforeGui(IDirect3DDevice9* device) = 0;
                virtual void DrawFrameBeforePostProcessing(IDirect3DDevice9* device) = 0;
                virtual void DrawFrame(IDirect3DDevice9* device) = 0;
                virtual bool HandleWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) = 0;

            protected:
                virtual void ChangeSubType(AddonSubType type) { this->subType = type; }
                virtual void ChangeState(AddonState state) { this->state = state; }

                HMODULE handle = nullptr;

            private:
                AddonSubType subType;
                AddonState state = AddonState::UnloadedState;
                std::weak_ptr<Addon> addon;
            };

        }
    }
}
