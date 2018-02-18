#include "ITypeImpl.h"

using namespace std;

namespace loader {
    namespace addons {
        namespace types {

            const wstring ITypeImpl::GetAddonStateString() const {
                switch (this->GetAddonState()) {
                case AddonState::DeactivatedOnRestartState:
                    return L"Deactivates on restart";
                case AddonState::ActivatedOnRestartState:
                    return L"Activates on restart";
                case AddonState::ErroredState:
                    return L"Errored";
                case AddonState::UnloadingState:
                    return L"Unloading";
                case AddonState::UnloadedState:
                    return L"Not active";
                case AddonState::LoadingState:
                    return L"Loading";
                case AddonState::LoadedState:
                    return L"Active";
                default:
                    return L"Unknown";
                }
            }

            const wstring ITypeImpl::GetAddonSubTypeString() const {
                switch (this->GetAddonSubType()) {
                case AddonSubType::GraphicsAddon:
                    return L"G";
                case AddonSubType::NonGraphicsAddon:
                    return L"NG";
                case AddonSubType::PointerReplacingAddon:
                    return L"PR";
                case AddonSubType::VirtualTableReplacingAddon:
                    return L"VTR";
                case AddonSubType::WrapperAddon:
                    return L"WR";
                default:
                    return L"";
                }
            }

        }
    }
}
