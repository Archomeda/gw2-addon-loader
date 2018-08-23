#pragma once
#include "Updater.h"

namespace loader::updaters {

    class CustomUpdater : public Updater {
    public:
        CustomUpdater(GW2AddonCheckUpdate_t* const checkUpdateFunc) : checkUpdateFunc(checkUpdateFunc) { }

    protected:
        virtual VersionInfo CheckLatestVersion() override;

    private:
        GW2AddonCheckUpdate_t* checkUpdateFunc;
    };

}
