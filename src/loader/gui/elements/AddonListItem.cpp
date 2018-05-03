#include "AddonListItem.h"
#include "../imgui.h"

using namespace std;
using namespace loader::addons;

namespace loader {
    namespace gui {
        namespace elements {

            bool AddonListItem(const addons::Addon* addon, bool selected, const ImVec2& size) {
                const ImGuiStyle& style = ImGui::GetStyle();
                ImDrawList* draw = ImGui::GetWindowDrawList();
                if (ImGui::Selectable("", selected, 0, size)) {
                    selected = true;
                }
                else {
                    selected = false;
                }

                ImVec2 pos = ImGui::GetCursorPos();
                ImGui::SetCursorPosY(pos.y - size.y - style.ItemSpacing.y);

                // Status line
                ImGuiCol statusColor = IM_COL32(120, 120, 120, 255);
                switch (addon->GetState()) {
                case AddonState::LoadingState:
                    statusColor = IM_COL32(255, 255, 0, 255);
                    break;
                case AddonState::ActivatedOnRestartState:
                case AddonState::DeactivatedOnRestartState:
                    statusColor = IM_COL32(0, 255, 255, 255);
                    break;
                case AddonState::LoadedState:
                    statusColor = IM_COL32(0, 255, 0, 255);
                    break;
                case AddonState::ErroredState:
                    statusColor = IM_COL32(255, 0, 0, 255);
                    break;
                }
                ImVec2 screenPos = ImGui::GetCursorScreenPos();
                draw->AddRectFilled(screenPos, screenPos + ImVec2(3, 32), statusColor);

                // Icon
                IDirect3DTexture9* icon = addon->GetIcon();
                ImGui::SetCursorPosX(pos.x + 4);
                if (icon) {
                    // Dedicated
                    ImGui::Image(icon, ImVec2(32, 32));
                }
                else {
                    // Fallback generic
                    ImGui::PushFont(imgui::FontIconButtons);
                    if (addon->SupportsLoading() && !addon->IsForced()) {
                        ImGui::TextUnformatted(ICON_MD_EXTENSION);
                    }
                    else if (addon->IsForced()) {
                        ImGui::TextDisabled(ICON_MD_EXTENSION);
                    }
                    ImGui::PopFont();
                }

                // Update icon
                if (addon->HasUpdate()) {
                    ImGui::SetCursorPos(pos + ImVec2(28, -size.y - style.ItemSpacing.y + 18));
                    ImGui::TextUnformatted(ICON_MD_FILE_DOWNLOAD);
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip("There's an update available for this addon");
                    }
                }
                
                // Name
                string name = addon->GetName();
                const char* text = name.c_str();
                ImVec2 textSize = ImGui::CalcTextSize(text);
                ImGui::SetCursorPos(pos + ImVec2(42, (32 - textSize.y) / 2 - size.y - style.ItemSpacing.y - 2));
                if (!addon->IsForced() && addon->SupportsLoading()) {
                    ImGui::TextUnformatted(text);
                }
                else {
                    ImGui::TextDisabled(text);
                }

                // Reset position
                ImGui::SetCursorPos(pos);

                return selected;
            }

            bool AddonListItem(const Addon* addon, bool* selected, const ImVec2& size) {
                if (AddonListItem(addon, *selected, size)) {
                    *selected = !*selected;
                    return true;
                }
                return false;
            }
        }
    }
}
