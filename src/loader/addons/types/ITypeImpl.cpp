#include "ITypeImpl.h"

using namespace std;

namespace loader {
    namespace addons {
        namespace types {

            ITypeImpl::ITypeImpl() {
                const int frames = 4 * 60;
                this->durationHistoryDrawFrameBeforeGui.resize(frames);
                this->durationHistoryDrawFrameBeforeGui.reserve(2 * frames);
                this->durationHistoryDrawFrameBeforePostProcessing.resize(frames);
                this->durationHistoryDrawFrameBeforePostProcessing.reserve(2 * frames);
                this->durationHistoryDrawFrame.resize(frames);
                this->durationHistoryDrawFrame.reserve(2 * frames);
            }

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

            void ITypeImpl::AddDurationHistory(vector<float>* durationHistory, float value) {
                if (durationHistory->size() == 4 * 60) {
                    durationHistory->erase(durationHistory->begin());
                }
                durationHistory->push_back(value);
            }

            void ITypeImpl::StartTimeMeasure() {
                this->timeMeasureStart = chrono::steady_clock::now();
            }

            float ITypeImpl::EndTimeMeasure() {
                auto time = (chrono::steady_clock::now() - this->timeMeasureStart).count();
                this->timeMeasureStart = {};
                return (time / 10000) / 100.0f;
            }

        }
    }
}
