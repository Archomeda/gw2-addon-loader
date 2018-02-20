#include "NativeAddonImpl.h"
#include <exception>
#include <filesystem>
#include "../Addon.h"
#include "../exceptions.h"
#include "../../utils.h"

using namespace std;
using namespace std::experimental::filesystem::v1;

namespace loader {
    namespace addons {
        namespace types {

            NativeAddonImpl::NativeAddonImpl(const wstring& filePath) {
                this->filePath = filePath;
                this->fileName = path(filePath).filename();
            }

            void NativeAddonImpl::Initialize() {
            }

            void NativeAddonImpl::Uninitialize() {
                if (this->handle) {
                    FreeLibrary(this->handle);
                    this->handle = NULL;
                }
            }

            void NativeAddonImpl::Load() {
                auto state = this->GetAddonState();
                if (state != AddonState::UnloadedState) {
                    return;
                }

                // Load the addon
                this->ChangeState(AddonState::LoadingState);
                HMODULE h = LoadLibrary(this->filePath.c_str());
                if (!h) {
                    this->ChangeState(AddonState::ErroredState);
                    throw exceptions::AddonLoadingException("Library handle is empty");
                }
                this->handle = h;

                this->GW2_Load = (GW2Load)GetProcAddress(h, GW2_DLL_Load);
                this->GW2_Unload = (GW2Unload)GetProcAddress(h, GW2_DLL_Unload);
                this->GW2_DrawFrameBeforeGui = (GW2DrawFrame)GetProcAddress(h, GW2_DLL_DrawFrameBeforeGui);
                this->GW2_DrawFrameBeforePostProcessing = (GW2DrawFrame)GetProcAddress(h, GW2_DLL_DrawFrameBeforePostProcessing);
                this->GW2_DrawFrame = (GW2DrawFrame)GetProcAddress(h, GW2_DLL_DrawFrame);
                this->GW2_HandleWndProc = (GW2HandleWndProc)GetProcAddress(h, GW2_DLL_HandleWndProc);

                if (this->GW2_Load) {
                    auto addon = this->GetAddon().lock();
                    GW2ADDON_RESULT result = this->GW2_Load(addon->GetFocusWindow(), addon->GetD3DDevice9());
                    if (result) {
                        this->ChangeState(AddonState::ErroredState);
                        throw exceptions::AddonLoadingException("Addon returned " + to_string(result));
                    }
                }

                if (this->GW2_DrawFrame) {
                    this->ChangeSubType(AddonSubType::GraphicsAddon);
                }
                else {
                    this->ChangeSubType(AddonSubType::NonGraphicsAddon);
                }

                // Done
                this->ChangeState(AddonState::LoadedState);
            }

            void NativeAddonImpl::Unload() {
                auto state = this->GetAddonState();
                if (state != AddonState::LoadedState) {
                    return;
                }

                // Unload the addon
                this->ChangeState(AddonState::UnloadingState);
                if (this->GW2_Unload) {
                    GW2ADDON_RESULT result = this->GW2_Unload();
                    if (result) {
                        this->ChangeState(AddonState::ErroredState);
                        throw exceptions::AddonUnloadingException("Addon returned " + to_string(result));
                    }
                }
                this->Uninitialize();

                this->GW2_Load = nullptr;
                this->GW2_Unload = nullptr;
                this->GW2_DrawFrame = nullptr;
                this->GW2_DrawFrameBeforeGui = nullptr;
                this->GW2_DrawFrameBeforePostProcessing = nullptr;
                this->GW2_HandleWndProc = nullptr;

                // Done
                this->ChangeState(AddonState::UnloadedState);
            }

            void NativeAddonImpl::DrawFrameBeforeGui(IDirect3DDevice9* device) {
                if (!this->GW2_DrawFrameBeforeGui) {
                    return;
                }
                try {
                    this->GW2_DrawFrameBeforeGui(device);
                }
                catch (const exception& ex) {
                    this->ChangeState(AddonState::ErroredState);
                    throw exceptions::AddonDrawFrameException(ex.what());
                }
                catch (const char* err) {
                    this->ChangeState(AddonState::ErroredState);
                    throw exceptions::AddonDrawFrameException(err);
                }
                catch (const wchar_t* err) {
                    this->ChangeState(AddonState::ErroredState);
                    throw exceptions::AddonDrawFrameException(ws2s(err));
                }
                catch (...) {
                    this->ChangeState(AddonState::ErroredState);
                    throw exceptions::AddonDrawFrameException("Unknown error");
                }
            }

            void NativeAddonImpl::DrawFrameBeforePostProcessing(IDirect3DDevice9* device) {
                if (!this->GW2_DrawFrameBeforePostProcessing) {
                    return;
                }
                try {
                    this->GW2_DrawFrameBeforePostProcessing(device);
                }
                catch (const exception& ex) {
                    this->ChangeState(AddonState::ErroredState);
                    throw exceptions::AddonDrawFrameException(ex.what());
                }
                catch (const char* err) {
                    this->ChangeState(AddonState::ErroredState);
                    throw exceptions::AddonDrawFrameException(err);
                }
                catch (const wchar_t* err) {
                    this->ChangeState(AddonState::ErroredState);
                    throw exceptions::AddonDrawFrameException(ws2s(err));
                }
                catch (...) {
                    this->ChangeState(AddonState::ErroredState);
                    throw exceptions::AddonDrawFrameException("Unknown error");
                }
            }

            void NativeAddonImpl::DrawFrame(IDirect3DDevice9* device) {
                if (!this->GW2_DrawFrame) {
                    return;
                }
                try {
                    this->GW2_DrawFrame(device);
                }
                catch (const exception& ex) {
                    this->ChangeState(AddonState::ErroredState);
                    throw exceptions::AddonDrawFrameException(ex.what());
                }
                catch (const char* err) {
                    this->ChangeState(AddonState::ErroredState);
                    throw exceptions::AddonDrawFrameException(err);
                }
                catch (const wchar_t* err) {
                    this->ChangeState(AddonState::ErroredState);
                    throw exceptions::AddonDrawFrameException(ws2s(err));
                }
                catch (...) {
                    this->ChangeState(AddonState::ErroredState);
                    throw exceptions::AddonDrawFrameException("Unknown error");
                }
            }

            bool NativeAddonImpl::HandleWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
                if (!this->GW2_HandleWndProc) {
                    return false;
                }
                try {
                    return this->GW2_HandleWndProc(hWnd, msg, wParam, lParam);
                }
                catch (const exception& ex) {
                    this->ChangeState(AddonState::ErroredState);
                    throw exceptions::AddonWndProcException(ex.what());
                }
                catch (const char* err) {
                    this->ChangeState(AddonState::ErroredState);
                    throw exceptions::AddonWndProcException(err);
                }
                catch (const wchar_t* err) {
                    this->ChangeState(AddonState::ErroredState);
                    throw exceptions::AddonWndProcException(ws2s(err));
                }
                catch (...) {
                    this->ChangeState(AddonState::ErroredState);
                    throw exceptions::AddonWndProcException("Unknown error");
                }
            }

        }
    }
}
