#include "Columns.h"

using namespace std;

namespace loader::gui::elements {

    set<pair<ImGuiID, int>> appliedWidths;

    void SetColumnWidth(int columnIndex, float width) {
        pair<ImGuiID, int> id(ImGui::GetItemID(), columnIndex);
        if (appliedWidths.find(id) == appliedWidths.end()) {
            ImGui::SetColumnWidth(columnIndex, width);
            appliedWidths.insert(id);
        }
    }

}
