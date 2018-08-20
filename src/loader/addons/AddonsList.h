#pragma once
#include "../stdafx.h"
#include "Addon.h"
#include "LegacyAddon.h"

namespace loader::addons {

    class AddonsList {
        class iterator {
        public:
            // Iterator traits
            using value_type = std::shared_ptr<Addon>;
            using iterator_category = std::input_iterator_tag;
            using difference_type = value_type;
            using pointer = const value_type*;
            using reference = const value_type&;

            typedef std::vector<std::shared_ptr<LegacyAddon>>::iterator legacy_iterator;
            typedef std::vector<std::shared_ptr<Addon>>::iterator addon_iterator;

            iterator(AddonsList* addonsList, legacy_iterator it) : addonsList(addonsList) {
                this->legacyIt = it;
                this->addonIt = static_cast<addon_iterator>(addonsList->addons.begin());
            }
            iterator(AddonsList* addonsList, addon_iterator it) : addonsList(addonsList) {
                this->addonIt = it;
                this->legacyIt = addonsList->legacyAddons.end();
            }

            iterator operator++() {
                if (this->addonsList->legacyAddons.end() == this->legacyIt) {
                    ++this->addonIt;
                }
                else {
                    ++this->legacyIt;
                }
                return *this;
            }

            bool operator==(const iterator& rhs) const {
                return this->legacyIt == rhs.legacyIt && this->addonIt == rhs.addonIt;
            }

            bool operator!=(const iterator& rhs) const {
                return !(*this == rhs);
            }

            reference operator*() const {
                if (this->addonsList->legacyAddons.end() == this->legacyIt) {
                    return *this->addonIt;
                }
                else {
                    return *reinterpret_cast<std::shared_ptr<Addon>*>(this->legacyIt._Ptr);
                }
            }

        private:
            legacy_iterator legacyIt;
            addon_iterator addonIt;
            const AddonsList* addonsList;
        };

    public:
        void Add(const std::shared_ptr<Addon>& addon);
        void Add(const std::shared_ptr<LegacyAddon>& addon);

        bool CanSwap(const Addon* const a, const Addon* const b) const;
        void Swap(const Addon* const a, const Addon* const b);

        void Sort(std::function<bool(const std::shared_ptr<Addon>&, const std::shared_ptr<Addon>&)> func);

        const std::vector<std::shared_ptr<Addon>> GetAddons() const { return this->addons; }
        const std::vector<std::shared_ptr<LegacyAddon>> GetLegacyAddons() const { return this->legacyAddons; }

        iterator begin() { return iterator(this, this->legacyAddons.begin()); }
        iterator end() { return iterator(this, this->addons.end()); }

    private:
        template<typename T>
        size_t GetIndexOf(const T* const addon, const std::vector<std::shared_ptr<T>>& list) const {
            for (auto it = list.begin(); it != list.end(); ++it) {
                if ((*it)->GetID() == addon->GetID()) {
                    return static_cast<size_t>(it - list.begin());
                }
            }
            return -1;
        }

        template<typename T>
        void Swap(const T* const a, const T* const b, std::vector<std::shared_ptr<T>>& list) {
            int iA = -1;
            int iB = -1;
            for (auto it = list.begin(); it != list.end(); ++it) {
                if ((*it)->GetID() == a->GetID()) {
                    iA = static_cast<int>(it - list.begin());
                }
                else if ((*it)->GetID() == b->GetID()) {
                    iB = static_cast<int>(it - list.begin());
                }
                if (iA > -1 && iB > -1) {
                    break;
                }
            }
            if (iA > -1 && iB > -1) {
                iter_swap(list.begin() + iA, list.begin() + iB);
            }
        }

        std::vector<std::shared_ptr<Addon>> addons;
        std::vector<std::shared_ptr<LegacyAddon>> legacyAddons;

    };

}
