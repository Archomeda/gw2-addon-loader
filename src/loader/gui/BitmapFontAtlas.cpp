#include "BitmapFontAtlas.h"

using namespace std;

namespace loader {
    namespace gui {

        BitmapFontAtlas::BitmapFontAtlas() {
            this->TexID = nullptr;
            this->TexDesiredWidth = 0;
            this->TexGlyphPadding = 1;
            this->TexPixelsAlpha8 = nullptr;
            this->TexPixelsRGBA32 = nullptr;
            this->TexWidth = 0;
            this->TexHeight = 0;
            this->TexUvWhitePixel = ImVec2(0, 0);
            for (int n = 0; n < IM_ARRAYSIZE(this->CustomRectIds); ++n) {
                this->CustomRectIds[n] = -1;
            }
        }

        BitmapFontAtlas::~BitmapFontAtlas() {
            this->Clear();
        }


        void BitmapFontAtlas::ClearTexData() {

        }

        void BitmapFontAtlas::ClearInputData() {
            for (int i = 0; i < this->ConfigData.Size; ++i) {
                if (this->ConfigData[i].FontData && this->ConfigData[i].FontDataOwnedByAtlas) {
                    ImGui::MemFree(this->ConfigData[i].FontData);
                    this->ConfigData[i].FontData = nullptr;
                }
            }

            for (int i = 0; i < this->Fonts.Size; ++i) {
                if (this->Fonts[i]->ConfigData >= this->ConfigData.Data && this->Fonts[i]->ConfigData < this->ConfigData.Data + this->ConfigData.Size) {
                    this->Fonts[i]->ConfigData = nullptr;
                    this->Fonts[i]->ConfigDataCount = 0;
                }
            }

            this->ConfigData.clear();
        }

        void BitmapFontAtlas::ClearFonts() {
            for (int i = 0; i < this->Fonts.Size; ++i) {
                IM_DELETE(this->Fonts[i]);
            }
            this->Fonts.clear();
        }

        void BitmapFontAtlas::Clear() {
            this->ClearInputData();
            this->ClearTexData();
            this->ClearFonts();
        }


        vector<ImFont*> BitmapFontAtlas::SetBitmapFontFromMemory(const unsigned char* fontData, int fontSize) {
            IM_ASSERT(this->Fonts.Size == 0);

            if (fontSize < 4 || fontData[0] != 'A' || fontData[1] != 'T' || fontData[2] != 'L' || fontData[3] != 0) {
                return {};
            }

            vector<ImFont*> fonts;

            size_t pos = 4;
            const AtlasInfo* atlasInfo = reinterpret_cast<const AtlasInfo*>(fontData + pos);
            void* ownedFontData = ImGui::MemAlloc(fontSize);
            memcpy(ownedFontData, fontData, fontSize);
            this->TextureFile = static_cast<unsigned char*>(ownedFontData) + atlasInfo->textureOffset;
            this->TextureFileSize = fontSize - atlasInfo->textureOffset;
            this->TexWidth = atlasInfo->textureWidth;
            this->TexHeight = atlasInfo->textureHeight;

            for (uint16_t i = 0; i < atlasInfo->fontCount; i++) {
                this->Fonts.push_back(IM_NEW(ImFont));
                ImFontConfig config{};
                config.DstFont = this->Fonts.back();
                config.FontData = ownedFontData;
                config.FontDataSize = fontSize;
                config.FontDataOwnedByAtlas = i == 0; // Just let ImGui only remove it once, since we have allocated it only once
                config.FontNo = i;
                this->ConfigData.push_back(config);
                fonts.push_back(config.DstFont);
            }

            this->ClearTexData();
            return fonts;
        }

        bool BitmapFontAtlas::Build() {
            IM_ASSERT(this->ConfigData.Size > 0);

            this->TexID = nullptr;
            this->TexWidth = 0;
            this->TexHeight = 0;
            this->ClearTexData();

            uint32_t pos = sizeof(AtlasInfo);
            for (int i = 0; i < this->ConfigData.Size; ++i) {
                ImFontConfig& config = this->ConfigData[i];
                unsigned char* data = static_cast<unsigned char*>(config.FontData) + 4;
                const AtlasInfo* atlasInfo = reinterpret_cast<const AtlasInfo*>(data);
                const AtlasFont* atlasFont = reinterpret_cast<const AtlasFont*>(data + pos);
                pos += sizeof(AtlasFont);

                if (this->TexWidth == 0) {
                    this->TexWidth = atlasInfo->textureWidth;
                }
                if (this->TexHeight == 0) {
                    this->TexHeight = atlasInfo->textureHeight;
                }
                if (config.SizePixels <= 0) {
                    // Make sure we set the font size
                    const AtlasGlyph* firstGlyph = reinterpret_cast<const AtlasGlyph*>(data + pos);
                    config.SizePixels = firstGlyph->fontSize;
                }

                ImFontAtlasBuildSetupFont(this, config.DstFont, &config, 0, 0);

                for (int j = 0; j < atlasFont->glyphCount; ++j) {
                    const AtlasGlyph* atlasGlyph = reinterpret_cast<const AtlasGlyph*>(data + pos);
                    pos += sizeof(AtlasGlyph);
                    config.DstFont->AddGlyph(
                        static_cast<ImWchar>(atlasGlyph->id),
                        static_cast<float>(atlasGlyph->xOffset),
                        static_cast<float>(atlasGlyph->yOffset),
                        static_cast<float>(atlasGlyph->xOffset + atlasGlyph->width),
                        static_cast<float>(atlasGlyph->yOffset + atlasGlyph->height),
                        static_cast<float>(atlasGlyph->x) / atlasInfo->textureWidth,
                        static_cast<float>(atlasGlyph->y) / atlasInfo->textureHeight,
                        static_cast<float>(atlasGlyph->x + atlasGlyph->width) / atlasInfo->textureWidth,
                        static_cast<float>(atlasGlyph->y + atlasGlyph->height) / atlasInfo->textureHeight,
                        atlasGlyph->xAdvance);
                }

                pos += atlasFont->kerningCount * sizeof(AtlasKerning);
            }

            for (int i = 0; i < this->Fonts.Size; ++i) {
                this->Fonts[i]->BuildLookupTable();
            }

            // Fix the white pixel that ImGui requires, which should be the pixel in the bottom right corner if packed by our font-atlas-packer
            const ImVec2 tex_uv_scale(1.0f / this->TexWidth, 1.0f / this->TexHeight);
            this->TexUvWhitePixel = ImVec2((this->TexWidth - 0.5f) * tex_uv_scale.x, (this->TexHeight - 0.5f) * tex_uv_scale.y);

            return true;
        }

    }
}
