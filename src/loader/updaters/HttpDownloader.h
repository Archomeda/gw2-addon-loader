#pragma once
#include "../stdafx.h"
#include "Downloader.h"

namespace loader::updaters {

    class HttpDownloader : public Downloader {
    public:
        HttpDownloader() = default;
        HttpDownloader(const std::string& url) : url(url) { }

        const std::string GetUrl() const { return this->url; }

        virtual bool IsValid() const override { return !this->url.empty(); }

    protected:
        virtual void DownloadUpdate() override;

    private:
        std::string url;
    };

}
