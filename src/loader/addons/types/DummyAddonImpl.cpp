#include "DummyAddonImpl.h"
#include <filesystem>
#include "../exceptions.h"

using namespace std;
using namespace std::experimental::filesystem::v1;

namespace loader {
    namespace addons {
        namespace types {

            DummyAddonImpl::DummyAddonImpl(const wstring& filePath) : ITypeImpl() {
                this->fileName = path(filePath).filename();
            }

            void DummyAddonImpl::Initialize() {
                throw exceptions::AddonInitializationException("Addon type unsupported");
            }

            void DummyAddonImpl::Uninitialize() {

            }

            void DummyAddonImpl::Load() {
                throw exceptions::AddonLoadingException("Addon type unsupported");
            }

            void DummyAddonImpl::Unload() {

            }

        }
    }
}
