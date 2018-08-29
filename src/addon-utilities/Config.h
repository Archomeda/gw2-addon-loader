#pragma once
#include "stdafx.h"

namespace addon {

    enum CoherentPriorityLevel {
        CoherentPriorityDontChange = 0,
        CoherentPriorityNormal,
        CoherentPriorityAboveNormal,
        CoherentPriorityHigh
    };

    class Config {
    public:
        Config() { }

        void Initialize();

        const std::string GetConfigPath() const { return this->configPath; }

        bool GetCursorMovementFix() const { return this->cursorMovementFix; }
        CoherentPriorityLevel GetCoherentPriority() const { return this->coherentPriority; }
        bool GetConfineCursor() const { return this->confineCursor; }

        void SetCursorMovementFix(bool cursorMovementFix);
        void SetCoherentPriority(CoherentPriorityLevel coherentPriority);
        void SetConfineCursor(bool confineCursor);

    private:
        const std::string configName = "utilities.ini";

        std::string configPath;

        bool cursorMovementFix = false;
        CoherentPriorityLevel coherentPriority = CoherentPriorityLevel::CoherentPriorityDontChange;
        bool confineCursor = false;

        CSimpleIni ini;
    };

    extern Config AppConfig;

}
