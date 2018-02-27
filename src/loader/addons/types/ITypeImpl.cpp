#include "ITypeImpl.h"
#include "../../Config.h"

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

            const string ITypeImpl::GetAddonSubTypeString() const {
                switch (this->GetAddonSubType()) {
                case AddonSubType::GraphicsAddon:
                    return "G";
                case AddonSubType::NonGraphicsAddon:
                    return "NG";
                case AddonSubType::PointerReplacingAddon:
                    return "PR";
                case AddonSubType::VirtualTableReplacingAddon:
                    return "VTR";
                case AddonSubType::WrapperAddon:
                    return "WR";
                default:
                    return "";
                }
            }

            void ITypeImpl::AddDurationHistory(vector<float>* durationHistory, float value) {
                if (AppConfig.GetShowDebugFeatures()) {
                    if (durationHistory->size() == 4 * 60) {
                        durationHistory->erase(durationHistory->begin());
                    }
                    durationHistory->push_back(value);
                }
            }

            void ITypeImpl::StartTimeMeasure() {
                if (AppConfig.GetShowDebugFeatures()) {
                    this->timeMeasureStart = chrono::steady_clock::now();
                }
            }

            float ITypeImpl::EndTimeMeasure() {
                if (AppConfig.GetShowDebugFeatures()) {
                    auto time = (chrono::steady_clock::now() - this->timeMeasureStart).count();
                    this->timeMeasureStart = {};
                    return (time / 10000) / 100.0f;
                }
                return 0;
            }

        }
    }
}
