#include "ProxyAddon.h"
#include "addons_manager.h"
#include "../hooks/LoaderDirect3D9.h"
#include "../log.h"

using namespace std;

namespace loader {
    namespace addons {

        IDirect3DDevice9* GW2PROXY_CALL CreateProxyDirect3DDevice9(IDirect3DDevice9* sourceDev) {
            ProxyDirect3DDevice9* proxyDev = new ProxyDirect3DDevice9(sourceDev);
            ProxyAddon::Instance->LastProxiedDevice = proxyDev;
            return proxyDev;
        }

        IDirect3DDevice9Ex* GW2PROXY_CALL CreateProxyDirect3DDevice9Ex(IDirect3DDevice9Ex* sourceDev) {
            ProxyDirect3DDevice9Ex* proxyDev = new ProxyDirect3DDevice9Ex(sourceDev);
            ProxyAddon::Instance->LastProxiedDevice = dynamic_cast<ProxyDirect3DDevice9*>(proxyDev);
            return proxyDev;
        }


        ProxyAddon* ProxyAddon::Instance = nullptr;

        bool ProxyAddon::Initialize() {
            HMODULE h = LoadLibrary(this->GetFilePath().c_str());
            if (h == NULL) {
                this->ChangeState(AddonState::ErroredState);
                ADDONS_LOG()->error("Could not initialize proxy add-on {0}: Library handle is empty", this->GetFileName());
                return false;
            }
            
            this->addonHandle = h;

            this->ProxyInitialize = reinterpret_cast<ProxyInitialize_t*>(GetProcAddress(h, GW2PROXY_DLL_Initialize));
            this->ProxyRelease = reinterpret_cast<ProxyRelease_t*>(GetProcAddress(h, GW2PROXY_DLL_Release));

            if (this->ProxyInitialize == nullptr) {
                this->ChangeState(AddonState::ErroredState);
                ADDONS_LOG()->error("Could not initialize proxy add-on {0}: Addon doesn't contain a valid ProxyInitialize export", this->GetFileName());
                return false;
            }
            if (this->ProxyRelease == nullptr) {
                this->ChangeState(AddonState::ErroredState);
                ADDONS_LOG()->error("Could not initialize proxy add-on {0}: Addon doesn't contain a valid ProxyRelease export", this->GetFileName());
                return false;
            }

            this->proxyApi.GetDirect3DDevice = &hooks::GetCreatedDevice;
            this->proxyApi.CreateProxyDirect3DDevice9 = &CreateProxyDirect3DDevice9;
            this->proxyApi.CreateProxyDirect3DDevice9Ex = &CreateProxyDirect3DDevice9Ex;

            return true;
        }

        bool ProxyAddon::Uninitialize() {
            this->Unload();

            this->ProxyInitialize = nullptr;
            this->ProxyRelease = nullptr;

            this->proxyApi.GetDirect3DDevice = nullptr;
            this->proxyApi.CreateProxyDirect3DDevice9 = nullptr;
            this->proxyApi.CreateProxyDirect3DDevice9Ex = nullptr;

            FreeLibrary(this->addonHandle);
            this->addonHandle = NULL;

            return true;
        }

        bool ProxyAddon::Load() {
            if (this->ProxyInitialize == nullptr) {
                return false;
            }
            if (this->GetState() != AddonState::UnloadedState) {
                return false;
            }
            this->ChangeState(AddonState::LoadingState);

            this->proxyMetadata = this->ProxyInitialize(&this->proxyApi);
            if (!this->proxyMetadata.name) {
                this->ChangeState(AddonState::ErroredState);
                ADDONS_LOG()->error("Could not load proxy add-on {0}", this->GetFileName());
                return false;
            }

            this->ChangeState(AddonState::LoadedState);
            return true;
        }

        bool ProxyAddon::Unload() {
            if (this->GetState() != AddonState::LoadedState) {
                return false;
            }
            this->ChangeState(AddonState::UnloadingState);

            bool result = Addon::Unload();
            this->ProxyRelease();

            this->ChangeState(AddonState::UnloadedState);
            return result;
        }

    }
}
