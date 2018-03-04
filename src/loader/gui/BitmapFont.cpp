#include "BitmapFont.h"
#include <d3dx9tex.h>

using namespace std;

namespace loader {
    namespace gui {

        bool BitmapFont::ReadFromMemory(const uint8_t* data, size_t size) {
            // Based on an older ImGui build: https://github.com/ocornut/imgui/blob/dd8a7655f5f6069147c6312263080b88d3215618/imgui.cpp
            if (size < 4 || data[0] != 'P' || data[1] != 'B' || data[2] != 'F' || data[3] != 0) {
                return false;
            }

            // Read PBF
            size_t pos = 4;
            const PackedBitmapFontInfo* packedInfo = reinterpret_cast<const PackedBitmapFontInfo*>(data + pos);
            const uint8_t* fntData = data + packedInfo->fontOffset;
            const uint32_t fntSize = packedInfo->fontFileSize;

            // Read textures
            pos += sizeof(PackedBitmapFontInfo);
            for (uint32_t i = 0; i < packedInfo->texturesCount; ++i) {
                const PackedBitmapFontTextureInfo* textureInfo = reinterpret_cast<const PackedBitmapFontTextureInfo*>(data + pos);
                vector<uint8_t> textureBytes;
                textureBytes.assign(data + textureInfo->textureOffset, data + textureInfo->textureOffset + textureInfo->textureFileSize);
                this->textureFiles.push_back(textureBytes);
                pos += sizeof(PackedBitmapFontTextureInfo);
            }
            
            // Read FNT
            for (const uint8_t* p = fntData + 4; p < fntData + fntSize; ) {
                const uint8_t blockType = *p;
                ++p;
                uint32_t blockSize; // Use memcpy to read 4-byte because they may be misaligned. This seems to break when compiling for Emscripten.
                memcpy(&blockSize, p, sizeof(uint32_t));
                p += sizeof(uint32_t);

                switch (blockType) {
                case 1:
                    this->info = BitmapFontInfo(*reinterpret_cast<const BitmapFontInfo*>(p));
                    break;
                case 2:
                    this->common = BitmapFontCommon(*reinterpret_cast<const BitmapFontCommon*>(p));
                    break;
                case 4:
                    this->glyphs.reserve(blockSize / sizeof(BitmapFontGlyph));
                    for (const uint8_t* g = p; g < p + blockSize; g += sizeof(BitmapFontGlyph)) {
                        this->glyphs.push_back(BitmapFontGlyph(*reinterpret_cast<const BitmapFontGlyph*>(g)));
                    }
                    break;
                case 5:
                    this->kerning.reserve(blockSize / sizeof(BitmapFontKerning));
                    for (const uint8_t* k = p; k < p + blockSize; k += sizeof(BitmapFontKerning)) {
                        this->kerning.push_back(BitmapFontKerning(*reinterpret_cast<const BitmapFontKerning*>(k)));
                    }
                }

                p += blockSize;
            }

            ImU32 maxC = 0;
            for (size_t i = 0; i < this->glyphs.size(); ++i) {
                if (maxC < this->glyphs[i].id) {
                    maxC = this->glyphs[i].id;
                }
            }

            this->indexLookup.resize(maxC + 1);
            for (size_t i = 0; i < this->indexLookup.size(); ++i) {
                this->indexLookup[i] = -1;
            }
            for (size_t i = 0; i < this->glyphs.size(); ++i) {
                this->indexLookup[this->glyphs[i].id] = static_cast<int>(i);
            }

            return true;
        }
        
        const BitmapFontGlyph* BitmapFont::FindGlyph(uint16_t c) const {
            if (c < static_cast<int>(this->indexLookup.size())) {
                const int i = this->indexLookup[c];
                if (i >= 0 && i < static_cast<int>(this->glyphs.size())) {
                    return &glyphs[i];
                }
            }
            return nullptr;
        }

        const vector<uint8_t> BitmapFont::GetTextureFile(int page) const {
            if (static_cast<int>(this->textureFiles.size()) > page) {
                return this->textureFiles[page];
            }
            return {};
        }

        IDirect3DTexture9* BitmapFont::GetTexture(int page, IDirect3DDevice9* device) {
            while (static_cast<int>(this->textures.size()) <= page) {
                this->textures.push_back(nullptr);
            }
            if (!this->textures[page]) {
                IDirect3DTexture9* texture;
                D3DXCreateTextureFromFileInMemory(device, this->textureFiles[page].data(), static_cast<UINT>(this->textureFiles[page].size()), &texture);
                this->textures[page] = texture;
            }
            return this->textures[page];
        }

        void BitmapFont::ClearTextures() {
            for (size_t i = 0; i < this->textures.size(); ++i) {
                if (this->textures[i]) {
                    this->textures[i]->Release();
                    this->textures[i] = nullptr;
                }
            }
        }

    }
}
