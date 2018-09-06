#pragma once
#include "stdafx.h"

namespace addon {

    enum CoherentPriorityLevel {
        CoherentPriorityDontChange = 0,
        CoherentPriorityNormal,
        CoherentPriorityAboveNormal,
        CoherentPriorityHigh
    };

    enum CursorHighlightType {
        CursorNoHighlight = 0,
        CursorArrowsHighlight,
        CursorCirclesInHighlight,
        CursorCirclesOutHighlight
    };

    class Config {
    public:
        Config() { }

        void Initialize();

        const std::string GetConfigPath() const { return this->configPath; }

        bool GetCursorMovementFix() const { return this->cursorMovementFix; }
        CoherentPriorityLevel GetCoherentPriority() const { return this->coherentPriority; }
        bool GetConfineCursor() const { return this->confineCursor; }
        CursorHighlightType GetCursorHighlightType() const { return this->cursorHighlightType; }
        int GetCursorHighlightKey() const { return this->cursorHighlightKey; }

        void SetCursorMovementFix(bool cursorMovementFix);
        void SetCoherentPriority(CoherentPriorityLevel coherentPriority);
        void SetConfineCursor(bool confineCursor);
        void SetCursorHighlightType(CursorHighlightType cursorHighlightType);
        void SetCursorHighlightKey(int cursorHighlightKey);

    private:
        const std::string configName = "utilities.ini";

        std::string configPath;

        bool cursorMovementFix = false;
        CoherentPriorityLevel coherentPriority = CoherentPriorityLevel::CoherentPriorityDontChange;
        bool confineCursor = false;
        CursorHighlightType cursorHighlightType = CursorHighlightType::CursorNoHighlight;
        int cursorHighlightKey = VK_CONTROL;

        CSimpleIni ini;
    };

    extern Config AppConfig;

}
