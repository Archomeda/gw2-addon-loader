#pragma once
#include "ITypeImpl.h"

namespace loader {
    namespace addons {
        namespace types {

            class DummyAddonImpl : public ITypeImpl {
            public:
                DummyAddonImpl(const std::wstring& filePath);

                virtual bool SupportsHotLoading() const override { return false; };
                virtual void Initialize() override;
                virtual void Uninitialize() override;
                virtual void Load() override;
                virtual void Unload() override;
            };

        }
    }
}
