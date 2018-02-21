#pragma once
#include <set>
#include <string>
#include "ITypeImpl.h"
#include "../../d3d9types.h"

namespace loader {
    namespace addons {
        namespace types {

            class LegacyAddonImpl : public ITypeImpl {
            public:
                LegacyAddonImpl(const std::wstring& filePath);

                virtual bool SupportsHotLoading() const override { return false; };
                virtual void Initialize() override;
                virtual void Uninitialize() override;
                virtual void Load() override;
                virtual void Unload() override;

            private:
                Direct3DCreate9_t Direct3DCreate9 = nullptr;
                Direct3DCreate9Ex_t Direct3DCreate9Ex = nullptr;
                std::wstring filePath;
                std::wstring fileName;
            };

        }
    }
}
