#include "main.h"
#include <list>
#include <stdint.h>
#include <imgui.h>
#include <examples/directx9_example/imgui_impl_dx9.h>
#include "addons/addons_manager.h"
#include "addons/Addon.h"
#include "hooks/hooks_manager.h"
#include "hooks/LoaderDirect3D9.h"
#include "hooks/LoaderDirect3DDevice9.h"
#include "gui/gui_manager.h"
#include "gui/SettingsWindow.h"
#include "Config.h"
#include "log.h"
#include "utils.h"

using namespace std;
using namespace loader;


// States
WNDPROC BaseWndProc;
set<uint32_t> PressedKeys;
shared_ptr<gui::SettingsWindow> SettingsWnd = make_shared<gui::SettingsWindow>();
bool prevCaptureMouse = false;
CURSORINFO prevCursor;

// We need this here because of out-of-scope issues
string imGuiConfigFile;

#ifdef _DEBUG
bool imGuiDemoOpen = false;
set<uint32_t> imGuiDemoKeybind { VK_SHIFT, VK_MENU, VK_F1 };
#endif


extern LRESULT ImGui_ImplDX9_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    struct EventKey {
        uint32_t vk: 31;
        bool down: true;
    };

    list<EventKey> eventKeys;

    // Generate our EventKey list for the current message
    {
        bool eventDown = false;
        switch (msg) {
            case WM_SYSKEYDOWN:
            case WM_KEYDOWN:
                eventDown = true;
            case WM_SYSKEYUP:
            case WM_KEYUP:
                if (msg == WM_SYSKEYDOWN || msg == WM_SYSKEYUP)
                {
                    if (((lParam >> 29) & 1) == 1)
                        eventKeys.push_back({ VK_MENU, true });
                    else
                        eventKeys.push_back({ VK_MENU, false });
                }

                eventKeys.push_back({ (uint32_t)wParam, eventDown });
                break;

            case WM_LBUTTONDOWN:
                eventDown = true;
            case WM_LBUTTONUP:
                eventKeys.push_back({ VK_LBUTTON, eventDown });
                break;
            case WM_MBUTTONDOWN:
                eventDown = true;
            case WM_MBUTTONUP:
                eventKeys.push_back({ VK_MBUTTON, eventDown });
                break;
            case WM_RBUTTONDOWN:
                eventDown = true;
            case WM_RBUTTONUP:
                eventKeys.push_back({ VK_RBUTTON, eventDown });
                break;
            case WM_XBUTTONDOWN:
                eventDown = true;
            case WM_XBUTTONUP:
                eventKeys.push_back({ (uint32_t)(GET_XBUTTON_WPARAM(wParam) == XBUTTON1 ? VK_XBUTTON1 : VK_XBUTTON2), eventDown });
                break;
        }
    }

    // Apply key events now
    for (const auto& k : eventKeys) {
        if (k.down) { PressedKeys.insert(k.vk); }
        else { PressedKeys.erase(k.vk); }
    }

    // Only run these for key down/key up (incl. mouse buttons) events
    if (!eventKeys.empty()) {
        if (PressedKeys == AppConfig.GetSettingsKeybind()) {
            gui::IsWindowOpen(SettingsWnd) ? gui::CloseWindow(SettingsWnd) : gui::ShowWindow(SettingsWnd);
            return true;
        }

#ifdef _DEBUG
        if (PressedKeys == imGuiDemoKeybind) {
            imGuiDemoOpen = !imGuiDemoOpen;
            return true;
        }
#endif
    }

    ImGui_ImplDX9_WndProcHandler(hWnd, msg, wParam, lParam);

    const auto& io = ImGui::GetIO();

    // Prevent game from receiving input if ImGui requests capture
    switch (msg) {
        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_RBUTTONDOWN:
        case WM_RBUTTONUP:
        case WM_MBUTTONDOWN:
        case WM_MBUTTONUP:
        case WM_XBUTTONDOWN:
        case WM_XBUTTONUP:
        case WM_MOUSEWHEEL:
        case WM_LBUTTONDBLCLK:
        case WM_RBUTTONDBLCLK:
        case WM_MBUTTONDBLCLK:
        case WM_XBUTTONDBLCLK:
            if (io.WantCaptureMouse) { return true; }
            break;
        case WM_KEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
            if (io.WantCaptureKeyboard) { return true; }
            break;
        case WM_CHAR:
            if (io.WantTextInput) { return true; }
            break;
    }

    // Make sure to show a decent cursor when ImGui has mouse focus
    if (msg == WM_SETCURSOR || msg == WM_MOUSEMOVE) {
        if (io.WantCaptureMouse) {
            HCURSOR systemCursor = LoadCursor(NULL, IDC_ARROW);
            SetCursor(systemCursor);
            return true;
        }
    }

    // Process WndProc to addons
    for (auto it = addons::AddonsList.begin(); it != addons::AddonsList.end(); ++it) {
        if ((*it)->HandleWndProc(hWnd, msg, wParam, lParam)) {
            return true;
        }
    }


    return CallWindowProc(BaseWndProc, hWnd, msg, wParam, lParam);
}


HRESULT PreCreateDevice(IDirect3D9* d3d9, UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DDevice9** ppReturnedDeviceInterface) {
    // Hook WindowProc
    if (!BaseWndProc) {
        BaseWndProc = (WNDPROC)GetWindowLongPtr(hFocusWindow, GWLP_WNDPROC);
        SetWindowLongPtr(hFocusWindow, GWLP_WNDPROC, (LONG_PTR)&WndProc);
    }

    // Initialize addons
    GetLog()->info("Initializing addons");
    for (auto it = addons::AddonsList.begin(); it != addons::AddonsList.end(); ++it) {
        (*it)->SetSdkVersion(hooks::SDKVersion);
        (*it)->SetD3D9(d3d9);
        (*it)->Initialize();
    }

    return D3D_OK;
}

void PostCreateDevice(IDirect3D9* d3d9, IDirect3DDevice9* pDeviceInterface, HWND hFocusWindow) {
    // Set up ImGui
    auto& imio = ImGui::GetIO();
    imGuiConfigFile = AppConfig.GetImGuiConfigPath();
    imio.IniFilename = imGuiConfigFile.c_str();

    ImGui_ImplDX9_Init(hFocusWindow, pDeviceInterface);

    ImGuiStyle* style = &ImGui::GetStyle();
    style->WindowRounding = 2;
    style->ChildWindowRounding = 0;
    style->FrameRounding = 0;
    style->ScrollbarRounding = 0;
    style->Colors[ImGuiCol_FrameBg] = ImVec4(0.2f, 0.2f, 0.2f, style->Colors[ImGuiCol_FrameBg].w);
    style->Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.3f, 0.3f, 0.3f, style->Colors[ImGuiCol_FrameBgHovered].w);
    style->Colors[ImGuiCol_FrameBgActive] = ImVec4(0.4f, 0.4f, 0.4f, style->Colors[ImGuiCol_FrameBgActive].w);
    style->Colors[ImGuiCol_TitleBg] = ImVec4(0.1f, 0.1f, 0.1f, style->Colors[ImGuiCol_TitleBg].w);
    style->Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.1f, 0.1f, 0.1f, style->Colors[ImGuiCol_TitleBgCollapsed].w);
    style->Colors[ImGuiCol_TitleBgActive] = ImVec4(0.15f, 0.15f, 0.15f, style->Colors[ImGuiCol_TitleBgActive].w);
    style->Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.1f, 0.1f, 0.1f, style->Colors[ImGuiCol_ScrollbarBg].w);
    style->Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.3f, 0.3f, 0.2f, style->Colors[ImGuiCol_ScrollbarGrab].w);
    style->Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.5f, 0.5f, 0.35f, style->Colors[ImGuiCol_ScrollbarGrabHovered].w);
    style->Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.6f, 0.6f, 0.43f, style->Colors[ImGuiCol_ScrollbarGrabActive].w);
    style->Colors[ImGuiCol_Button] = ImVec4(0.3f, 0.3f, 0.2f, style->Colors[ImGuiCol_Button].w);
    style->Colors[ImGuiCol_ButtonHovered] = ImVec4(0.5f, 0.5f, 0.35f, style->Colors[ImGuiCol_ButtonHovered].w);
    style->Colors[ImGuiCol_ButtonActive] = ImVec4(0.6f, 0.6f, 0.43f, style->Colors[ImGuiCol_ButtonActive].w);
    style->Colors[ImGuiCol_Header] = ImVec4(0.45f, 0.45f, 0.3f, style->Colors[ImGuiCol_Header].w);
    style->Colors[ImGuiCol_HeaderHovered] = ImVec4(0.5f, 0.5f, 0.35f, style->Colors[ImGuiCol_HeaderHovered].w);
    style->Colors[ImGuiCol_HeaderActive] = ImVec4(0.6f, 0.6f, 0.43f, style->Colors[ImGuiCol_HeaderActive].w);
    style->Colors[ImGuiCol_ResizeGrip] = ImVec4(0.3f, 0.3f, 0.2f, style->Colors[ImGuiCol_ResizeGrip].w);
    style->Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.5f, 0.5f, 0.35f, style->Colors[ImGuiCol_ResizeGripHovered].w);
    style->Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.6f, 0.6f, 0.43f, style->Colors[ImGuiCol_ResizeGripActive].w);
    style->Colors[ImGuiCol_CloseButton] = ImVec4(0.3f, 0.3f, 0.2f, style->Colors[ImGuiCol_CloseButton].w);
    style->Colors[ImGuiCol_CloseButtonHovered] = ImVec4(0.5f, 0.5f, 0.35f, style->Colors[ImGuiCol_CloseButtonHovered].w);
    style->Colors[ImGuiCol_CloseButtonActive] = ImVec4(0.6f, 0.6f, 0.43f, style->Colors[ImGuiCol_CloseButtonActive].w);

    // Load enabled addons
    GetLog()->info("Loading enabled addons");
    for (auto it = addons::AddonsList.begin(); it != addons::AddonsList.end(); ++it) {
        (*it)->SetFocusWindow(hFocusWindow);
        if ((*it)->IsEnabledByConfig()) {
            (*it)->Load();
        }
    }
}

HRESULT PrePresent(IDirect3DDevice9* pDeviceInterface, CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion, bool* done) {
    // Draw ImGui stuff
    pDeviceInterface->BeginScene();
    ImGui_ImplDX9_NewFrame();

    gui::Render();
#ifdef _DEBUG
    if (imGuiDemoOpen) {
        ImGui::ShowTestWindow();
    }
#endif

    ImGui::Render();
    pDeviceInterface->EndScene();

    // Draw addons
    for (auto it = addons::AddonsList.begin(); it != addons::AddonsList.end(); ++it) {
        (*it)->DrawFrame(pDeviceInterface);
    }

    return D3D_OK;
}

HRESULT PreReset(IDirect3DDevice9* pDeviceInterface, D3DPRESENT_PARAMETERS* pPresentationParameters, bool* done) {
    ImGui_ImplDX9_InvalidateDeviceObjects();
    return D3D_OK;
}

void PostReset(IDirect3DDevice9* pDeviceInterface) {
    ImGui_ImplDX9_CreateDeviceObjects();
}


bool WINAPI DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved) {
    switch (fdwReason) {
        case DLL_PROCESS_ATTACH: {
            GetLog()->info("GW2 Addon Loader attached");
            LaunchDebugger();
            
            hooks::PreCreateDeviceHook = &PreCreateDevice;
            hooks::PostCreateDeviceHook = &PostCreateDevice;
            hooks::PrePresentHook = &PrePresent;
            hooks::PreResetHook = &PreReset;
            hooks::PostResetHook = &PostReset;

            // Make ourselves known by setting an environment variable
            // This makes it easy for addon developers to detect early if we are loaded by GW2 or not
            SetEnvironmentVariable(L"_IsGW2AddonLoaderActive", L"1");

            AppConfig.Initialize();
            addons::RefreshAddonList();
        }
        break;
        case DLL_PROCESS_DETACH: {
            ImGui::Shutdown();
            for (auto it = addons::AddonsList.begin(); it != addons::AddonsList.end(); ++it) {
                (*it)->Unload();
                (*it)->Uninitialize();
            }
            hooks::UninitializeHooks();
            GetLog()->info("GW2 Addon Loader detached");
        }
        break;
    }
    return true;
}
