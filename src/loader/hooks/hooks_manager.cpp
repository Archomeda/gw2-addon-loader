#include "hooks_manager.h"
#include "LoaderDirect3D9.h"
#include "../d3d9types.h"
#include "../log.h"
#include "../utils.h"

using namespace std;

namespace loader {
    namespace hooks {

        UINT SDKVersion;
        HMODULE SystemD3D9 = nullptr;
        
        void UninitializeHooks() {
            GetLog()->debug("loader::hooks::UnintializeHooks()");

            if (SystemD3D9)
            {
                if (!FreeLibrary(SystemD3D9)) {
                    GetLog()->error("Failed to free system d3d9 library: {0}", LastErrorToString(GetLastError()));
                }
                SystemD3D9 = nullptr;
            }
        }


        bool BeforeDirect3DCreate9(UINT sdkVersion) {
            SDKVersion = sdkVersion;

            if (!SystemD3D9) {
                SystemD3D9 = LoadSystemD3D9();
            }
            if (!SystemD3D9) {
                GetLog()->error("Failed to load the system d3d9.dll: {0}", LastErrorToString(GetLastError()));
                return false;
            }
            return true;
        }

        IDirect3D9* WINAPI Direct3DCreate9(UINT sdkVersion) {
            GetLog()->debug("loader::hooks::Direct3DCreate9()");
            if (!BeforeDirect3DCreate9(sdkVersion)) {
                return nullptr;
            }

            // Create our stuff
            GetLog()->info("Proxying D3D9");
            auto systemDirect3DCreate9 = reinterpret_cast<Direct3DCreate9_t>(GetProcAddress(SystemD3D9, "Direct3DCreate9"));
            if (!systemDirect3DCreate9) {
                GetLog()->error("System Direct3DCreate GetProcAddress failed");
                return nullptr;
            }
            return new LoaderDirect3D9(systemDirect3DCreate9(sdkVersion));
        }

        IDirect3D9Ex* WINAPI Direct3DCreate9Ex(UINT sdkVersion) {
            GetLog()->debug("loader::hooks::Direct3DCreate9Ex()");
            if (!BeforeDirect3DCreate9(sdkVersion)) {
                return nullptr;
            }

            // Create our stuff
            GetLog()->info("Proxying D3D9Ex");
            auto systemDirect3DCreate9Ex = reinterpret_cast<Direct3DCreate9Ex_t>(GetProcAddress(SystemD3D9, "Direct3DCreate9Ex"));
            if (!systemDirect3DCreate9Ex) {
                GetLog()->error("System Direct3DCreate9Ex GetProcAddress failed");
                return nullptr;
            }
            return new LoaderDirect3D9Ex(systemDirect3DCreate9Ex(sdkVersion));
        }

    }
}
