#pragma once
#include "../windows.h"
#include <d3d9.h>
#include <chrono>
#include <memory>
#include <string>
#include "types/ITypeImpl.h"

namespace loader {
    namespace addons {

        enum AddonType {
            UnknownAddon,
            NativeAddon
        };

        class Addon : public std::enable_shared_from_this<Addon> {
        public:
            Addon(const std::string& filePath);

            bool IsEnabledByConfig();

            const std::string GetFilePath() const { return this->filePath; }
            const std::string GetFileName() const { return this->fileName; }
            const std::string GetID() const { return this->GetTypeImpl()->GetID(); }
            const std::string GetName() const { return this->GetTypeImpl()->GetName(); }
            const std::string GetAuthor() const { return this->GetTypeImpl()->GetAuthor(); }
            const std::string GetDescription() const { return this->GetTypeImpl()->GetDescription(); }
            const std::string GetVersion() const { return this->GetTypeImpl()->GetVersion(); }
            const std::string GetHomepage() const { return this->GetTypeImpl()->GetHomepage(); }
            IDirect3DTexture9* GetIcon() const { return this->GetTypeImpl()->GetIcon(); }

            const AddonType GetAddonType() const;
            const std::string GetAddonTypeString() const;

            const std::chrono::steady_clock::rep GetLoadDuration() const { return this->durationLoad; }

            bool IsLoaded() const;

            const std::shared_ptr<types::ITypeImpl>& GetTypeImpl() const { return this->typeImpl; }
            void SetTypeImpl(const AddonType addonType);

            UINT GetSdkVersion() const { return this->sdkVersion; }
            void SetSdkVersion(UINT sdkVersion) { this->sdkVersion = sdkVersion; }
            IDirect3D9* GetD3D9() const { return this->d3d9; }
            void SetD3D9(IDirect3D9* d3d9) { this->d3d9 = d3d9; }
            IDirect3DDevice9* GetD3DDevice9() const { return this->d3ddevice9; }
            void SetD3DDevice9(IDirect3DDevice9* device) { this->d3ddevice9 = device; }
            HWND GetFocusWindow() const { return this->focusWindow; }
            void SetFocusWindow(HWND focusWindow) { this->focusWindow = focusWindow; }

            bool Initialize();
            bool Uninitialize();
            bool Load();
            bool Unload();

            bool SupportsLoading() const;
            bool SupportsSettings() const;
            bool SupportsHomepage() const;

            bool HandleWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

        private:
            std::string filePath;
            std::string fileName;
            AddonType addonType = UnknownAddon;
            std::shared_ptr<types::ITypeImpl> typeImpl;

            void LoadAddonType();

            std::chrono::steady_clock::rep durationLoad;

            UINT sdkVersion;
            IDirect3D9* d3d9;
            IDirect3D9Ex* d3d9Ex;
            IDirect3DDevice9* d3ddevice9;
            HWND focusWindow;
        };

    }
}
