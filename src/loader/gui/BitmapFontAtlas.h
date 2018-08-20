#pragma once
#include "../stdafx.h"

namespace loader::gui {

#pragma pack(push, 1)
    struct AtlasInfo {
        uint16_t fontCount;
        uint16_t textureWidth;
        uint16_t textureHeight;
        uint32_t textureOffset;
    };

    struct AtlasFont {
        uint16_t glyphCount;
        uint16_t kerningCount;
    };

    struct AtlasGlyph {
        uint32_t id;
        int16_t fontSize;
        uint16_t lineHeight;
        uint16_t baseLine;
        uint16_t x;
        uint16_t y;
        uint16_t width;
        uint16_t height;
        int16_t xOffset;
        int16_t yOffset;
        int16_t xAdvance;
    };

    struct AtlasKerning {
        uint32_t first;
        uint32_t second;
        int16_t amount;
    };
#pragma pack(pop)

    struct BitmapFontAtlas : ImFontAtlas {

        BitmapFontAtlas();
        ~BitmapFontAtlas();

        void ClearTexData();
        void ClearInputData();
        void ClearFonts();
        void Clear();

        std::vector<ImFont*> SetBitmapFontFromMemory(const unsigned char* fontData, int fontSize);

        bool Build();
        using ImFontAtlas::SetTexID;

        unsigned char* TextureFile;
        int TextureFileSize;

        // Ideally the functions below shouldn't be called ever
        ImFont* AddFont(const ImFontConfig* font_cfg) { throw "not implemented"; }
        ImFont* AddFontDefault(const ImFontConfig* font_cfg) { throw "not implemented"; }
        ImFont* AddFontFromFileTTF(const char* filename, float size_pixels, const ImFontConfig* font_cfg, const ImWchar* glyph_ranges) { throw "not implemented"; }
        ImFont* AddFontFromMemoryTTF(void* font_data, int font_size, float size_pixels, const ImFontConfig* font_cfg, const ImWchar* glyph_ranges) { throw "not implemented"; }
        ImFont* AddFontFromMemoryCompressedTTF(const void* compressed_font_data, int compressed_font_size, float size_pixels, const ImFontConfig* font_cfg, const ImWchar* glyph_ranges) { throw "not implemented"; }
        ImFont* AddFontFromMemoryCompressedBase85TTF(const char* compressed_font_data_base85, float size_pixels, const ImFontConfig* font_cfg, const ImWchar* glyph_ranges) { throw "not implemented"; }
        void GetTexDataAsAlpha8(unsigned char** out_pixels, int* out_width, int* out_height, int* out_bytes_per_pixel) { throw "not implemented"; }
        void GetTexDataAsRGBA32(unsigned char** out_pixels, int* out_width, int* out_height, int* out_bytes_per_pixel) { throw "not implemented"; }
        const ImWchar* GetGlyphRangesDefault() { throw "not implemented"; }
        const ImWchar* GetGlyphRangesKorean() { throw "not implemented"; }
        const ImWchar* GetGlyphRangesJapanese() { throw "not implemented"; }
        const ImWchar* GetGlyphRangesChinese() { throw "not implemented"; }
        const ImWchar* GetGlyphRangesCyrillic() { throw "not implemented"; }
        const ImWchar* GetGlyphRangesThai() { throw "not implemented"; }
        const int AddCustomRectRegular(unsigned int id, int width, int height) { throw "not implemented"; }
        const int AddCustomRectFontGlyph(ImFont* font, ImWchar id, int width, int height, float advance_x, const ImVec2& offset) { throw "not implemented"; }
        void CalcCustomRectUV(const CustomRect* rect, ImVec2* out_uv_min, ImVec2* out_uv_max) { throw "not implemented"; }
        const CustomRect* GetCustomRectByIndex(int index) const { throw "not implemented"; }
    };

}
