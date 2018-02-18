#include "DummyAddonImpl.h"
#include "../exceptions.h"

using namespace std;

namespace loader {
    namespace addons {
        namespace types {

            DummyAddonImpl::DummyAddonImpl(const wstring& filePath) {

            }

            void DummyAddonImpl::Initialize() {
                throw exceptions::AddonInitializationException("Addon type unsupported");
            }

            void DummyAddonImpl::Uninitialize() {

            }

            void DummyAddonImpl::Load() {

            }

            void DummyAddonImpl::Unload() {

            }

            void DummyAddonImpl::DrawFrame(IDirect3DDevice9* device) {

            }

            bool DummyAddonImpl::HandleWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
                return false;
            }

        }
    }
}
