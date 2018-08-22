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
        static void WriteBuffer(void* downloader, size_t progress, size_t totalSize);

    private:
        GW2AddonDownloadUpdate_t* downloadFunc;
        char* downloadBuffer;
    };

}
