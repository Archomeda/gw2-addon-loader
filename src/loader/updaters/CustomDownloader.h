#pragma once
#include "../stdafx.h"
#include "Downloader.h"
#include "../addons/NativeAddon.h"

namespace loader::updaters {

    class CustomDownloader : public Downloader {
    public:
        CustomDownloader(GW2AddonDownloadUpdate_t* downloadFunc) : downloadFunc(downloadFunc) { }

        virtual bool IsValid() const override { return this->downloadFunc; }

    protected:
        virtual void DownloadUpdate() override;
        static void GW2ADDON_CALL WriteBuffer(void* downloader, unsigned int progress, unsigned int totalSize);

    private:
        GW2AddonDownloadUpdate_t* downloadFunc;
        char* downloadBuffer;
    };

}
