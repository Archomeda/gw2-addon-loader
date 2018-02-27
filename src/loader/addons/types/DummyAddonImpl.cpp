#include "DummyAddonImpl.h"
#include <filesystem>
#include "../exceptions.h"

using namespace std;
using namespace std::experimental::filesystem::v1;

namespace loader {
    namespace addons {
        namespace types {

            DummyAddonImpl::DummyAddonImpl(const string& filePath) : ITypeImpl() {
                this->fileName = u8path(filePath).filename().u8string();
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
