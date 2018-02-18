#include "LegacyAddonImpl.h"
#include <filesystem>
#include "../exceptions.h"

using namespace std;
using namespace std::experimental::filesystem::v1;

namespace loader {
    namespace addons {
        namespace types {

            LegacyAddonImpl::LegacyAddonImpl(const wstring& filePath) {
                this->filePath = filePath;
                this->fileName = path(filePath).filename();
            }

            void LegacyAddonImpl::Initialize() {
                /*if (active) {
                    // Get handle
                    HMODULE h = LoadLibrary(this->filePath.c_str());
                    if (!h) {
                        this->ChangeState(AddonState::ErroredState);
                        throw exceptions::AddonInitializationException("Library handle is empty");
                    }
                    this->handle = h;

                    //TODO: LOAD STUFF

                    // Done
                    this->ChangeState(AddonState::LoadedState);
                }*/
            }

            void LegacyAddonImpl::Uninitialize() {
                /*if (this->handle) {
                    FreeLibrary(this->handle);
                    this->handle = NULL;
                }
                this->ChangeState(AddonState::UnloadedState);*/
            }

            void LegacyAddonImpl::Load() {
                /*auto state = this->GetAddonState();
                if (state != AddonState::UnloadedState && state != AddonState::DeactivatedOnRestartState) {
                    return;
                }

                this->ChangeState(AddonState::ActivatedOnRestartState);*/
            }

            void LegacyAddonImpl::Unload() {
                /*auto state = this->GetAddonState();
                if (state != AddonState::LoadedState && state != AddonState::ActivatedOnRestartState) {
                    return;
                }

                this->ChangeState(AddonState::DeactivatedOnRestartState);*/
            }

            void LegacyAddonImpl::DrawFrame(IDirect3DDevice9* device) {

            }

            bool LegacyAddonImpl::HandleWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
                return false;
            }

        }
    }
}
