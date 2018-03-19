#pragma once
#include "Addon.h"

namespace loader {
    namespace addons {

        template<class T>
        class UnknownAddon : public T {
        public:
            UnknownAddon() = delete;
            UnknownAddon(const T& addon) : addon(addon) { }

            virtual bool Initialize() override { return false; }
            virtual bool Uninitialize() override { return false; }
            virtual bool Load() override { return false; }
            virtual bool Unload() override { return false; }

            virtual bool SupportsLoading() const override { return false; }
            virtual bool SupportsHotLoading() const override { return false; }
            virtual bool SupportsSettings() const override { return false; }
            virtual bool SupportsHomepage() const override { return false; }

            virtual AddonType GetType() const override { return AddonType::AddonTypeUnknown; }

        protected:
            virtual const Addon* GetConstBaseAddon() const override { return &addon; }
            virtual Addon* GetBaseAddon() override { return &addon; }

        private:
            T addon;
        };

    }
}
