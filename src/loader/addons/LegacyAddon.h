#pragma once
#include "../stdafx.h"
#include "Addon.h"
#include "ProxyDirect3DDevice9.h"
#include "../disasm/d3d9Vtbl.h"
#include "../disasm/opcodes.h"

#define INSTRUCTION_BACKUP_SIZE 5

namespace loader {
    namespace addons {

        typedef IDirect3D9*(WINAPI Direct3DCreate9_t)(UINT sdkVersion);

        class LegacyAddon : public Addon {
        public:
            LegacyAddon() : Addon() { }
            LegacyAddon(const std::string& filePath) : Addon(filePath) { }
            LegacyAddon(const std::experimental::filesystem::path& filePath) : Addon(filePath) { }

            bool ApplySafeEnv();
            bool RevertSafeEnv();
            void SetNextAddonChain(LegacyAddon* addon);

            virtual bool Initialize() override;
            virtual bool Uninitialize() override;
            virtual bool Load() override;
            virtual bool Unload() override;

            virtual bool SupportsLoading() const override { return true; }

            virtual AddonType GetType() const override { return AddonType::AddonTypeLegacy; }

            IDirect3D9* AddonD3D9 = nullptr;
            IDirect3DDevice9* AddonD3DDevice9 = nullptr;
            ProxyDirect3DDevice9* ProxyD3DDevice9 = nullptr;

        private:
            template<typename T>
            void CopyPointerIfNotEqual(T* target, LPVOID p1, LPVOID p2) {
                if (p1 != p2) {
                    *target = reinterpret_cast<T>(p1);
                }
            }

            template<typename T>
            void CopyAndRestorePointerIfHooked(T* target, uint8_t* orig, LPVOID comp, size_t size) {
                uint8_t* c = reinterpret_cast<uint8_t*>(comp);
                if (!std::equal(orig, orig + size, c, c + size)) {
                    if (c[0] == 0xE9) {
                        // JMP
                        disasm::JMP_REL jmpRel = *reinterpret_cast<disasm::JMP_REL*>(c);
                        void* pJmpFunction = c + sizeof(disasm::JMP_REL) + jmpRel.operand;
                        *target = reinterpret_cast<T>(pJmpFunction);
                    }
                    // If the only JMP instruction above doesn't cover this, just restore it for safety reasons
                    // In that case it "just doesn't work", but at least we don't crash, hopefully
                    DWORD protection = PAGE_READWRITE;
                    if (VirtualProtect(c, size, protection, &protection)) {
                        memcpy(c, orig, size);
                        VirtualProtect(c, size, protection, &protection);
                    }
                }
            }

            HMODULE addonHandle = NULL;
            DWORD proxyAddonNumberOfExports = 0;
            D3DDevice9Vtbl proxyVtbl = { 0 };
            D3DDevice9Functions<uint8_t[INSTRUCTION_BACKUP_SIZE]> proxyFunctionInstructions;

            Direct3DCreate9_t* AddonCreate = nullptr;
        };

    }
}
