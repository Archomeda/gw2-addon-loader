#include "AddonsList.h"

using namespace std;

namespace loader::addons {

    void AddonsList::Add(const std::shared_ptr<Addon>& addon) {
        if (addon->GetType() == AddonType::AddonTypeLegacy) {
            this->Add(static_pointer_cast<LegacyAddon>(addon));
        }
        else {
            this->addons.push_back(addon);
        }
    }

    void AddonsList::Add(const std::shared_ptr<LegacyAddon>& addon) {
        this->legacyAddons.push_back(addon);
    }


    bool AddonsList::CanSwap(const Addon* const a, const Addon* const b) const {
        return a->GetType() == b->GetType();
    }


    void AddonsList::Swap(const Addon* const a, const Addon* const b) {
        if (!this->CanSwap(a, b)) {
            return;
        }
        if (a->GetType() == AddonType::AddonTypeLegacy) {
            return this->Swap(static_cast<const LegacyAddon* const>(a), static_cast<const LegacyAddon* const>(b), this->legacyAddons);
        }
        else {
            return this->Swap(a, b, this->addons);
        }
    }


    void AddonsList::Sort(function<bool(const shared_ptr<Addon>&, const shared_ptr<Addon>&)> func) {
        sort(this->legacyAddons.begin(), this->legacyAddons.end(), func);
        sort(this->addons.begin(), this->addons.end(), func);
    }

}
