/**
This tool will merge multiple .fnt BMF files into one .atl ATL file, which serves as an atlas for our Addon Loader font.
The ATL file is a combination of an RGBA PNG file with some additional information before the image data as header.

It assumes the input texture files are RGBA. Anything else than that causes undefined behavior.

Keep in mind, converting BMF files to an ATL file is one-way. You'll lose all redundant BMF metadata.


USAGE:
./font-atlas-packer <fntA> [[-m] <fntN>]...

-m indicates that the next given font should be merged into the previous one. Glyph ids need to be unique!
If -m is not given, the next font will be a new font entry in the atlas.
*/

#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <stdint.h>
#include <string>
#include <vector>
#include "zlib/zlib.h"
#define STB_RECT_PACK_IMPLEMENTATION
#include "stb_rect_pack.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

static unsigned char* zlib_compress(unsigned char* data, int data_len, int* out_len, int quality) {
    uLongf dest_len = compressBound(data_len);
    unsigned char* dest = (unsigned char*)malloc(data_len);
    compress2(dest, &dest_len, data, data_len, quality);
    *out_len = static_cast<int>(dest_len);
    return dest;
}

#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBI_MSC_SECURE_CRT
#define STBIW_ZLIB_COMPRESS zlib_compress
#include "stb_image_write.h"

using namespace std;
using namespace std::experimental::filesystem;

#pragma pack(push, 1)
struct AtlasInfo {
    uint16_t fontCount;
    uint16_t textureWidth;
    uint16_t textureHeight;
    uint32_t textureOffset;
    // AtlasFont fonts[];
    // Texture;
};

struct AtlasFont {
    uint16_t glyphCount;
    uint16_t kerningCount;
    // AtlasGlyph glyphs[];
    // AtlasKerning kernings[];
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
    // char fontName[];
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

struct BitmapFontFile {
    BitmapFontInfo info;
    BitmapFontCommon common;
    vector<string> textureFiles;
    vector<BitmapFontGlyph> glyphs;
    vector<BitmapFontKerning> kernings;
};

struct BitmapFontGlyphMap {
    const BitmapFontGlyph* glyph;
    const BitmapFontFile* file;
    uint16_t newX;
    uint16_t newY;
};

struct Image {
    int x;
    int y;
    int n;
    unsigned char* data;
};

// ImGui's function
static inline int UpperPowerOfTwo(int v) { v--; v |= v >> 1; v |= v >> 2; v |= v >> 4; v |= v >> 8; v |= v >> 16; v++; return v; }

static inline string u8(const wstring& wstr) {
    return wstring_convert<codecvt_utf8<wchar_t>>().to_bytes(wstr);
}

BitmapFontFile ReadBMF(ifstream* srcFile, string srcFileName);
vector<vector<BitmapFontGlyphMap>> GetGlyphMappings(const vector<vector<BitmapFontFile>>& fontFiles, int* textureWidth, int* textureHeight);
vector<vector<BitmapFontKerning>> GetKernings(const vector<vector<BitmapFontFile>>& fontFiles);
bool ExportAtlas(const vector<vector<BitmapFontGlyphMap>>& glyphMap, const vector<vector<BitmapFontKerning>>& kernings, const string& destFileName, int textureWidth, int textureHeight);
bool CopyImageRect(const Image& srcImage, int srcX, int srcY, Image& destImage, int destX, int destY, int width, int height);


int main(int argc, char* argv[]) {
    if (argc < 2) {
        cout << "Provide one more more bitmap fonts to pack into an atlas" << endl;
        return 1;
    }

    // Parse command line
    vector<bool> srcMerged;
    vector<string> args(argv + 1, argv + argc);
    vector<string> srcFileNames;
    for (auto it = args.begin(); it != args.end(); ++it) {
        if (*it == "-m") {
            srcMerged.push_back(true);
            ++it;
        }
        else {
            srcMerged.push_back(false);
        }
        srcFileNames.push_back(*it);
    }
    vector<ifstream*> srcStreams;
    vector<BitmapFontFile> srcFiles;

    path destFileName(srcFileNames[0]);
    destFileName.replace_extension(".atl");

    // Read all BMF files
    for (const auto& srcFileName : srcFileNames) {
        ifstream* srcStream = new ifstream;
        srcStream->open(srcFileName, ios::binary);
        if (srcStream->is_open()) {
            char fourcc[4];
            srcStream->read(fourcc, sizeof(fourcc));
            srcStream->seekg(0);
            if (fourcc[0] == 'B' && fourcc[1] == 'M' && fourcc[2] == 'F' && fourcc[3] == 0x03) {
                // Bitmap Font v3 file
                srcFiles.push_back(ReadBMF(srcStream, srcFileName));
            }
            else {
                cout << "A file was given that is not a BMF v3 file" << endl;
                return 2;
            }
        }
    }

    // Combine fonts
    vector<vector<BitmapFontFile>> srcFonts;
    vector<BitmapFontFile>* lastMerge;
    for (size_t i = 0; i < srcFiles.size(); ++i) {
        if (!srcMerged[i]) {
            srcFonts.push_back(vector<BitmapFontFile>());
            lastMerge = &srcFonts.back();
        }
        lastMerge->push_back(srcFiles.at(i));
    }
    lastMerge = nullptr;

    // Get the packed glyph mapping
    int textureWidth;
    int textureHeight;
    vector<vector<BitmapFontGlyphMap>> glyphMap = GetGlyphMappings(srcFonts, &textureWidth, &textureHeight);
    for (const auto& innerGlyphMap : glyphMap) {
        if (innerGlyphMap.size() != innerGlyphMap.capacity()) {
            cout << "Not all glyphs were packed" << endl;
            return 3;
        }
    }
    textureHeight = UpperPowerOfTwo(textureHeight);

    // Merge the kernings
    vector<vector<BitmapFontKerning>> kernings = GetKernings(srcFonts);

    // Export
    if (!ExportAtlas(glyphMap, kernings, destFileName.string(), textureWidth, textureHeight)) {
        cout << "Could not export atlas" << endl;
        return 4;
    }

    // Clean up
    for (ifstream* srcStream : srcStreams) {
        srcStream->close();
        delete srcStream;
    }
    srcStreams.clear();

    return 0;
}

BitmapFontFile ReadBMF(ifstream* srcFile, string srcFileName) {
    path srcFolder(srcFileName);
    srcFolder.remove_filename();
    BitmapFontFile file;

    srcFile->seekg(4);
    while (srcFile->peek() != EOF) {
        unsigned char blockType;
        srcFile->read(reinterpret_cast<char*>(&blockType), sizeof(blockType));
        uint32_t blockSize;
        srcFile->read(reinterpret_cast<char*>(&blockSize), sizeof(blockSize));
        streamoff blockStartOff = srcFile->tellg();

        switch (blockType) {
        case 1: {
            // Extract info
            char* blockInfo = new char[blockSize];
            srcFile->read(blockInfo, blockSize);
            file.info = BitmapFontInfo(*reinterpret_cast<const BitmapFontInfo*>(blockInfo));
            break;
        }
        case 2: {
            // Extract common
            char* blockCommon = new char[blockSize];
            srcFile->read(blockCommon, blockSize);
            file.common = BitmapFontCommon(*reinterpret_cast<const BitmapFontCommon*>(blockCommon));
            break;
        }
        case 3: {
            // Extract file names
            char* blockFileNames = new char[blockSize];
            srcFile->read(blockFileNames, blockSize);
            for (uint32_t i = 0; i < blockSize; ) {
                string fileName = string(blockFileNames + i);
                file.textureFiles.push_back((srcFolder / fileName).string());
                i += static_cast<uint32_t>(fileName.length()) + 1;
            }
            delete blockFileNames;
            break;
        }
        case 4: {
            // Extract glyphs
            char* blockGlyphs = new char[blockSize];
            srcFile->read(blockGlyphs, blockSize);
            file.glyphs.reserve(blockSize / sizeof(BitmapFontGlyph));
            for (const char* g = blockGlyphs; g < blockGlyphs + blockSize; g += sizeof(BitmapFontGlyph)) {
                file.glyphs.push_back(BitmapFontGlyph(*reinterpret_cast<const BitmapFontGlyph*>(g)));
            }
            break;
        }
        case 5: {
            // Extract kernings
            char* blockKernings = new char[blockSize];
            srcFile->read(blockKernings, blockSize);
            file.kernings.reserve(blockSize / sizeof(BitmapFontKerning));
            for (const char* k = blockKernings; k < blockKernings + blockSize; k += sizeof(BitmapFontKerning)) {
                file.kernings.push_back(BitmapFontKerning(*reinterpret_cast<const BitmapFontKerning*>(k)));
            }
            break;
        }
        }

        srcFile->seekg(blockStartOff + blockSize);
    }

    return file;
}

vector<vector<BitmapFontGlyphMap>> GetGlyphMappings(const vector<vector<BitmapFontFile>>& fontFiles, int* textureWidth, int* textureHeight) {
    // This algorithm based on ImGui's font atlas generator, with a few changes to support our goal
    vector<vector<BitmapFontGlyphMap>> glyphMap;

    size_t glyphCount = 0;
    for (const auto& innerFontFiles : fontFiles) {
        for (const auto& innerFontFile : innerFontFiles) {
            glyphCount += innerFontFile.glyphs.size();
        }
    }
    
    // We need a width for the skyline algorithm. Using a dumb heuristic here to decide the width.
    // Width doesn't really matter much, but some API/GPU have texture size limitations and increasing width can decrease height.
    *textureWidth = glyphCount > 4000 ? 4096 : glyphCount > 2000 ? 2048 : glyphCount > 1000 ? 1024 : 512;
    *textureHeight = 0;

    // Init packing
    const uint16_t textureHeightMax = 1024 * 32;
    stbrp_context packContext = {};
    stbrp_node* tempNodes = new stbrp_node[glyphCount];
    stbrp_init_target(&packContext, *textureWidth, textureHeightMax, tempNodes, static_cast<int>(glyphCount));
    vector<stbrp_rect> packRects;
    packRects.reserve(glyphCount);

    // Start packing
    for (const auto& innerFontFiles : fontFiles) {
        for (const auto& innerFontFile : innerFontFiles) {
            for (BitmapFontGlyph glyph : innerFontFile.glyphs) {
                stbrp_rect rect = {};
                rect.w = glyph.width;
                rect.h = glyph.height;
                packRects.push_back(rect);
            }
        }
    }
    stbrp_pack_rects(&packContext, packRects.data(), static_cast<int>(glyphCount));

    size_t r = 0;
    for (const auto& innerFontFiles : fontFiles) {
        size_t innerGlyphCount = 0;
        for (const auto& innerFontFile : innerFontFiles) {
            innerGlyphCount += innerFontFile.glyphs.size();
        }

        vector<BitmapFontGlyphMap> innerGlyphMap;
        innerGlyphMap.reserve(innerGlyphCount);

        for (const auto& innerFontFile : innerFontFiles) {
            for (const auto& glyph : innerFontFile.glyphs) {
                stbrp_rect rect = packRects.at(r);
                if (rect.was_packed) {
                    BitmapFontGlyphMap map;
                    map.file = &innerFontFile;
                    map.glyph = &glyph;
                    map.newX = rect.x;
                    map.newY = rect.y;
                    innerGlyphMap.push_back(map);
                    *textureHeight = max(*textureHeight, rect.y + rect.h);
                }
                ++r;
            }
        }

        glyphMap.push_back(innerGlyphMap);
    }
    
    delete[] tempNodes;
    return glyphMap;
}

vector<vector<BitmapFontKerning>> GetKernings(const vector<vector<BitmapFontFile>>& fontFiles) {
    vector<vector<BitmapFontKerning>> kernings;
    kernings.reserve(fontFiles.size());

    for (const auto& innerFontFiles : fontFiles) {
        vector<BitmapFontKerning> innerKernings;
        for (const auto& innerFontFile : innerFontFiles) {
            innerKernings.reserve(innerKernings.size() + innerFontFile.kernings.size());
            innerKernings.insert(innerKernings.end(), innerFontFile.kernings.begin(), innerFontFile.kernings.end());
        }
        kernings.push_back(innerKernings);
    }

    return kernings;
}

bool ExportAtlas(const vector<vector<BitmapFontGlyphMap>>& glyphMap, const vector<vector<BitmapFontKerning>>& kernings, const string& destFileName, int textureWidth, int textureHeight) {
    map<string, Image> textureImages;
    Image atlasImage;
    atlasImage.x = textureWidth;
    atlasImage.y = textureHeight;
    atlasImage.n = 4;
    atlasImage.data = new unsigned char[atlasImage.x * atlasImage.y * atlasImage.n]{ 0 };

    // Do stuff with glyphs
    for (const auto& innerGlyphMap : glyphMap) {
        for (const auto& map : innerGlyphMap) {
            string textureFileName = map.file->textureFiles.at(map.glyph->page);

            // Load the texture if it's not loaded yet
            if (textureImages.find(textureFileName) == textureImages.end()) {
                Image texture;
                texture.data = stbi_load(textureFileName.c_str(), &texture.x, &texture.y, &texture.n, 4);
                if (texture.data == nullptr) {
                    return false;
                }
                textureImages[textureFileName] = texture;
            }

            // Copy the glyph to our new texture
            Image textureImage = textureImages[textureFileName];
            if (!CopyImageRect(textureImage, map.glyph->x, map.glyph->y, atlasImage, map.newX, map.newY, map.glyph->width, map.glyph->height)) {
                return false;
            }
        }
    }

    // ImGui expects a white pixel somewhere for reference, we create our white pixel at the bottom right corner, which is unlikely to be used by something
    memset(atlasImage.data + (atlasImage.x * atlasImage.y - 1) * atlasImage.n, 0xFF, 4);

    int textureImageDataLength;
    unsigned char* textureImageData = stbi_write_png_to_mem(atlasImage.data, atlasImage.x * atlasImage.n, atlasImage.x, atlasImage.y, atlasImage.n, &textureImageDataLength);

    // Export
    ofstream destFile;
    destFile.open(destFileName, ios::trunc | ios::binary);
    if (!destFile.is_open()) {
        return false;
    }
    const char fourcc[]{ 'A', 'T', 'L', 0 };
    destFile.write(fourcc, sizeof(fourcc));

    uint32_t headerSize = sizeof(AtlasInfo);
    for (size_t i = 0; i < glyphMap.size(); ++i) {
        headerSize += static_cast<uint32_t>(sizeof(AtlasFont) + glyphMap[i].size() * sizeof(AtlasGlyph) + kernings[i].size() * sizeof(AtlasKerning));
    }
    AtlasInfo info;
    info.fontCount = static_cast<uint16_t>(glyphMap.size());
    info.textureWidth = textureWidth;
    info.textureHeight = textureHeight;
    info.textureOffset = headerSize + static_cast<uint32_t>(destFile.tellp());
    destFile.write(reinterpret_cast<char*>(&info), sizeof(info));

    for (size_t i = 0; i < glyphMap.size(); ++i) {
        const auto& innerGlyphMap = glyphMap[i];
        const auto& innerKernings = kernings[i];

        AtlasFont font;
        font.glyphCount = static_cast<uint16_t>(innerGlyphMap.size());
        font.kerningCount = static_cast<uint16_t>(innerKernings.size());
        destFile.write(reinterpret_cast<char*>(&font), sizeof(font));

        vector<AtlasGlyph> glyphs;
        for (const auto& innerGlyphMap : innerGlyphMap) {
            AtlasGlyph glyph;
            glyph.id = innerGlyphMap.glyph->id;
            glyph.fontSize = innerGlyphMap.file->info.fontSize;
            glyph.lineHeight = innerGlyphMap.file->common.lineHeight;
            glyph.baseLine = innerGlyphMap.file->common.base;
            glyph.x = innerGlyphMap.newX;
            glyph.y = innerGlyphMap.newY;
            glyph.width = innerGlyphMap.glyph->width;
            glyph.height = innerGlyphMap.glyph->height;
            glyph.xOffset = innerGlyphMap.glyph->xOffset;
            glyph.yOffset = innerGlyphMap.glyph->yOffset;
            glyph.xAdvance = innerGlyphMap.glyph->xAdvance;
            glyphs.push_back(glyph);
        }
        destFile.write(reinterpret_cast<char*>(glyphs.data()), glyphs.size() * sizeof(AtlasGlyph));

        vector<AtlasKerning> kernings;
        for (const auto& innerKerning : innerKernings) {
            AtlasKerning kerning;
            kerning.first = innerKerning.first;
            kerning.second = innerKerning.second;
            kerning.amount = innerKerning.amount;
            kernings.push_back(kerning);
        }
        destFile.write(reinterpret_cast<char*>(kernings.data()), kernings.size() * sizeof(AtlasKerning));
    }

    destFile.write(reinterpret_cast<char*>(textureImageData), textureImageDataLength);
    destFile.close();

    // Clear
    for (auto texture : textureImages) {
        stbi_image_free(texture.second.data);
    }
    textureImages.clear();
    delete[] atlasImage.data;
    delete[] textureImageData;

    return true;
}

bool CopyImageRect(const Image& srcImage, int srcX, int srcY, Image& destImage, int destX, int destY, int width, int height) {
    if (srcImage.n != destImage.n) {
        return false;
    }
    if (srcImage.x < srcX + width || srcImage.y < srcY + height) {
        return false;
    }
    if (destImage.x < destX + width || destImage.y < destY + height) {
        return false;
    }

    for (int y = 0; y < height; ++y) {
        memcpy(destImage.data + (destY + y) * destImage.x * destImage.n + destX * destImage.n, srcImage.data + (srcY + y) * srcImage.y * destImage.n + srcX * srcImage.n, width * srcImage.n);
    }
    return true;
}
