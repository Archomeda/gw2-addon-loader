#include "ITypeImpl.h"
#include "../../Config.h"

using namespace std;

namespace loader {
    namespace addons {
        namespace types {

            const string ITypeImpl::GetAddonStateString() const {
                switch (this->GetAddonState()) {
                case AddonState::DeactivatedOnRestartState:
                    return "Deactivates on restart";
                case AddonState::ActivatedOnRestartState:
                    return "Activates on restart";
                case AddonState::ErroredState:
                    return "Errored";
                case AddonState::UnloadingState:
                    return "Unloading";
                case AddonState::UnloadedState:
                    return "Not active";
                case AddonState::LoadingState:
                    return "Loading";
                case AddonState::LoadedState:
                    return "Active";
                default:
                    return "Unknown";
                }
            }

        }
    }
}
