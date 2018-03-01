#pragma once
#include "ITypeImpl.h"

namespace loader {
    namespace addons {
        namespace types {

            class DummyAddonImpl : public ITypeImpl {
            public:
                DummyAddonImpl(const std::string& filePath);

                virtual const std::string GetID() const override { return this->fileName; }
                virtual const std::string GetName() const override { return this->fileName; }
                
                virtual bool SupportsHotLoading() const override { return false; };
                virtual bool SupportsSettings() const override { return false; }

                virtual void Initialize() override;
                virtual void Uninitialize() override;
                virtual void Load() override;
                virtual void Unload() override;

            private:
                std::string fileName;

            };

        }
    }
}
