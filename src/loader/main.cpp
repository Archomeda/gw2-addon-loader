#include "main.h"
#include <list>
#include <stdint.h>
#include <imgui.h>
#include "addons/addons_manager.h"
#include "addons/Addon.h"
#include "hooks/hooks_manager.h"
#include "hooks/LoaderDirect3D9.h"
#include "hooks/LoaderDirect3DDevice9.h"
#include "gui/gui_manager.h"
#include "gui/SettingsWindow.h"
#include "Config.h"
#include "imgui_impl_dx9.h"
#include "log.h"
#include "utils.h"

using namespace std;
using namespace loader;


IMGUI_API LRESULT ImGui_ImplDX9_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

HMODULE dllModule;

// States
WNDPROC BaseWndProc;
set<uint32_t> PressedKeys;
bool prevCaptureMouse = false;
CURSORINFO prevCursor;

// We need this here because of out-of-scope issues
string imGuiConfigFile;

#ifdef _DEBUG
bool imGuiDemoOpen = false;
set<uint32_t> imGuiDemoKeybind { VK_SHIFT, VK_MENU, VK_F1 };
#endif


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
            gui::IsWindowOpen(gui::SettingsWnd) ? gui::CloseWindow(gui::SettingsWnd) : gui::ShowWindow(gui::SettingsWnd);
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
    addons::InitializeAddons(hooks::SDKVersion, d3d9);

    return D3D_OK;
}

void PostCreateDevice(IDirect3D9* d3d9, IDirect3DDevice9* pDeviceInterface, HWND hFocusWindow) {
    // Create textures
    gui::LoadTextures(dllModule, pDeviceInterface);

    // Set up ImGui
    ImGuiIO imio = ImGui::GetIO();
    imGuiConfigFile = AppConfig.GetImGuiConfigPath();
    imio.IniFilename = imGuiConfigFile.c_str();

    gui::LoadFonts(dllModule);
    ImGui_ImplDX9_Init(hFocusWindow, pDeviceInterface);

    ImGuiStyle* style = &ImGui::GetStyle();
    style->WindowRounding = 2;
    style->ChildRounding = 0;
    style->FrameRounding = 0;
    style->ScrollbarRounding = 0;
    ImVec4* colors = ImGui::GetStyle().Colors;
    colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 0.94f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.06f, 0.06f, 0.94f);
    colors[ImGuiCol_ChildBg] = ImVec4(1.00f, 1.00f, 1.00f, 0.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
    colors[ImGuiCol_Border] = ImVec4(0.47f, 0.47f, 0.31f, 0.50f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.16f, 0.16f, 0.11f, 0.54f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.39f, 0.39f, 0.26f, 0.40f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.39f, 0.39f, 0.26f, 0.67f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.09f, 0.09f, 0.06f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.24f, 0.24f, 0.16f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.09f, 0.09f, 0.06f, 0.51f);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.16f, 0.16f, 0.11f, 0.54f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.39f, 0.39f, 0.26f, 0.69f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.39f, 0.39f, 0.26f, 0.82f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.39f, 0.39f, 0.26f, 1.00f);
    colors[ImGuiCol_CheckMark] = ImVec4(1.00f, 1.00f, 0.94f, 1.00f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.39f, 0.39f, 0.26f, 0.69f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.39f, 0.39f, 0.26f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.39f, 0.39f, 0.26f, 0.69f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.39f, 0.39f, 0.26f, 0.82f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.39f, 0.39f, 0.26f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(0.39f, 0.39f, 0.26f, 0.69f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.39f, 0.39f, 0.26f, 0.82f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.39f, 0.39f, 0.26f, 1.00f);
    colors[ImGuiCol_Separator] = ImVec4(0.47f, 0.47f, 0.31f, 0.50f);
    colors[ImGuiCol_SeparatorHovered] = ImVec4(0.47f, 0.47f, 0.31f, 0.75f);
    colors[ImGuiCol_SeparatorActive] = ImVec4(0.47f, 0.47f, 0.31f, 1.00f);
    colors[ImGuiCol_ResizeGrip] = ImVec4(0.39f, 0.39f, 0.26f, 0.69f);
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.39f, 0.39f, 0.26f, 0.82f);
    colors[ImGuiCol_ResizeGripActive] = ImVec4(0.39f, 0.39f, 0.26f, 1.00f);
    colors[ImGuiCol_CloseButton] = ImVec4(0.39f, 0.39f, 0.26f, 0.69f);
    colors[ImGuiCol_CloseButtonHovered] = ImVec4(0.39f, 0.39f, 0.26f, 0.82f);
    colors[ImGuiCol_CloseButtonActive] = ImVec4(0.39f, 0.39f, 0.26f, 1.00f);
    colors[ImGuiCol_PlotLines] = ImVec4(1.00f, 1.00f, 0.94f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.82f, 0.82f, 0.55f, 1.00f);
    colors[ImGuiCol_PlotHistogram] = ImVec4(1.00f, 1.00f, 0.94f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.82f, 0.82f, 0.55f, 1.00f);
    colors[ImGuiCol_TextSelectedBg] = ImVec4(0.47f, 0.47f, 0.31f, 1.00f);
    colors[ImGuiCol_ModalWindowDarkening] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
    colors[ImGuiCol_DragDropTarget] = ImVec4(0.82f, 0.82f, 0.55f, 1.00f);

    // Load enabled addons
    GetLog()->info("Loading enabled addons");
    addons::LoadAddons(hFocusWindow);
}


void PreReset(IDirect3DDevice9* pDeviceInterface, D3DPRESENT_PARAMETERS* pPresentationParameters) {
    ImGui_ImplDX9_InvalidateDeviceObjects();
    gui::UnloadTextures();
}

void PostReset(IDirect3DDevice9* pDeviceInterface, D3DPRESENT_PARAMETERS* pPresentationParameters) {
    ImGui_ImplDX9_CreateDeviceObjects();
    gui::LoadTextures(dllModule, pDeviceInterface);
}

void PrePresent(IDirect3DDevice9* pDeviceInterface, CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion) {
    pDeviceInterface->BeginScene();

    addons::DrawFrame(pDeviceInterface);

    // Draw ImGui stuff
    ImGui_ImplDX9_NewFrame();

    gui::Render();
#ifdef _DEBUG
    if (imGuiDemoOpen) {
        ImGui::ShowTestWindow();
    }
#endif

    ImGui::Render();
    pDeviceInterface->EndScene();
}


bool WINAPI DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved) {
    switch (fdwReason) {
        case DLL_PROCESS_ATTACH: {
            GetLog()->info("GW2 Addon Loader attached");
            LaunchDebugger();

            dllModule = hModule;
            hooks::InitializeHooks();
            
            hooks::PreCreateDeviceHook = &PreCreateDevice;
            hooks::PostCreateDeviceHook = &PostCreateDevice;

            hooks::PreResetHook = &PreReset;
            hooks::PostResetHook = &PostReset;
            hooks::PrePresentHook = &PrePresent;

            // Make ourselves known by setting an environment variable
            // This makes it easy for addon developers to detect early if we are loaded by GW2 or not
            SetEnvironmentVariable(L"_IsGW2AddonLoaderActive", L"1");

            AppConfig.Initialize();
            addons::RefreshAddonList();
        }
        break;
        case DLL_PROCESS_DETACH: {
            ImGui::Shutdown();
            GetLog()->info("Unloading and uninitializing addons");
            addons::UnloadAddons();
            addons::UninitializeAddons();
            GetLog()->info("Uninitializing hooks");
            hooks::UninitializeHooks();
            GetLog()->info("GW2 Addon Loader detached");
        }
        break;
    }
    return true;
}
