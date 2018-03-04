#pragma once
#include <d3d9.h>
#include <stdint.h>
#include <vector>
#include <imgui.h>

namespace loader {
    namespace gui {

#pragma pack(push, 1)
        struct PackedBitmapFontInfo {
            uint32_t fontFileSize;
            uint32_t fontOffset;
            uint32_t texturesCount;
        };

        struct PackedBitmapFontTextureInfo {
            uint32_t textureFileSize;
            uint32_t textureOffset;
        };

        struct BitmapFontInfo {
            int16_t fontSize;
            uint8_t bitField; // bit 0: smooth, bit 1: unicode, bit 2: italic, bit 3: bold, bit 4: fixedHeight, bits 5-7: reserved
            uint8_t charSet;
            uint16_t stretchH;
            uint8_t aa;
            uint8_t paddingUp;
            uint8_t paddingRight;
            uint8_t paddingDown;
            uint8_t paddingLeft;
            uint8_t spacingHoriz;
            uint8_t spacingVert;
            uint8_t outline;
            //char fontName[];
        };

        struct BitmapFontCommon {
            uint16_t lineHeight;
            uint16_t base;
            uint16_t scaleW;
            uint16_t scaleH;
            uint16_t pages;
            uint8_t bitField; // bits 0-6: reserved, bit 7: packed
            uint8_t channels[4];
        };

        struct BitmapFontGlyph {
            uint32_t id;
            uint16_t x;
            uint16_t y;
            uint16_t width;
            uint16_t height;
            int16_t xOffset;
            int16_t yOffset;
            int16_t xAdvance;
            uint8_t page;
            uint8_t channel;
        };

        struct BitmapFontKerning {
            uint32_t first;
            uint32_t second;
            int16_t amount;
        };
#pragma pack(pop)

        class BitmapFont {
        public:
            bool ReadFromMemory(const uint8_t* data, size_t size);
            const BitmapFontGlyph* FindGlyph(uint16_t c) const;
            const std::vector<uint8_t> GetTextureFile(int page) const;
            IDirect3DTexture9* GetTexture(int page, IDirect3DDevice9* device);
            void ClearTextures();

            const BitmapFontInfo& GetInfo() const { return this->info; }
            const BitmapFontCommon& GetCommon() const { return this->common; }
            std::vector<BitmapFontGlyph> GetGlyphs() const { return this->glyphs; }
            std::vector<BitmapFontKerning> GetKerning() const { return this->kerning; }

        private:
            BitmapFontInfo info;
            BitmapFontCommon common;
            std::vector<BitmapFontGlyph> glyphs;
            std::vector<BitmapFontKerning> kerning;
            std::vector<int> indexLookup;
            std::vector<std::vector<uint8_t>> textureFiles;
            std::vector<IDirect3DTexture9*> textures;
        };

    }
}
