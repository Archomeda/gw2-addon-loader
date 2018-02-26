#include "LegacyAddonImpl.h"
#include "../exceptions.h"

using namespace std;

namespace loader {
    namespace addons {
        namespace types {

            LegacyAddonImpl::LegacyAddonImpl(const wstring& filePath) : ITypeImpl() {

            }

            void LegacyAddonImpl::Initialize() {
                throw exceptions::AddonInitializationException("Addon type unsupported");
            }

            void LegacyAddonImpl::Uninitialize() {
           
            }

            void LegacyAddonImpl::Load() {
           
            }

            void LegacyAddonImpl::Unload() {
           
            }

        }
    }
}
