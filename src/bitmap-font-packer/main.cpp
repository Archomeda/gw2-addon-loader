/**
This tool allows packing a .fnt BMF file along with the corresponding textures into one .pbf file.
It was being used before we had a dedicated ATL file that is fed to ImGui directly.
Now it's just here for historical reasons.

Two-way conversion is supported.


USAGE:
./bitmap-font-packer <some-font.fnt>
./bitmap-font-packer <some-packed-font.pbf>
*/

#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdint.h>
#include <vector>

using namespace std;
using namespace std::experimental::filesystem;

#pragma pack(push, 1)
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

struct PackedBitmapFontInfo {
    uint32_t fontFileSize;
    uint32_t fontOffset;
    uint32_t texturesCount;
};

struct PackedBitmapFontTextureInfo {
    uint32_t textureFileSize;
    uint32_t textureOffset;
};
#pragma pack(pop)

bool Pack(ifstream* srcFile, path srcFileName);
bool Unpack(ifstream* srcFile, path srcFileName);

int main(int argc, char* argv[]) {
    if (argc != 2) {
        cout << "Provide a bitmap font filename to pack or unpack" << endl;
        return 1;
    }

    char* file = argv[1];

    ifstream srcFile;
    srcFile.open(file, ios::binary);
    if (srcFile.is_open()) {
        char fourcc[4];
        srcFile.read(fourcc, sizeof(fourcc));
        srcFile.seekg(0, ios::beg);
        if (fourcc[0] == 'B' && fourcc[1] == 'M' && fourcc[2] == 'F' && fourcc[3] == 0x03) {
            // Bitmap Font file v3
            if (!Pack(&srcFile, file)) {
                cout << "An error occured" << endl;
            }
        }
        else if (fourcc[0] == 'P' && fourcc[1] == 'B' && fourcc[2] == 'F' && fourcc[3] == 0) {
            // Packed Bitmap Font file
            if (!Unpack(&srcFile, file)) {
                cout << "An error occured" << endl;
            }
        }
        else {
            cout << "Input file does not have a BMF or PBF header" << endl;
            srcFile.close();
            return 2;
        }
        srcFile.close();
    }

    return 0;
}

bool Pack(ifstream* srcFile, path srcFileName) {
    path folder(srcFileName);
    folder.remove_filename();
    path destFileName(srcFileName);
    destFileName.replace_extension(".pbf");
    vector<string> textureFileNames;

    srcFile->seekg(4);
    while (srcFile->peek() != EOF) {
        unsigned char blockType;
        srcFile->read(reinterpret_cast<char*>(&blockType), sizeof(blockType));
        uint32_t blockSize;
        srcFile->read(reinterpret_cast<char*>(&blockSize), sizeof(blockSize));
        streamoff blockStartOff = srcFile->tellg();

        if (blockType == 3) {
            // Extract file names
            char* blockFileNames = new char[blockSize];
            srcFile->read(blockFileNames, blockSize);
            for (uint32_t i = 0; i < blockSize; ) {
                string fileName = string(blockFileNames + i);
                textureFileNames.push_back(fileName);
                i += static_cast<uint32_t>(fileName.length()) + 1;
            }
            delete blockFileNames;
        }
        else {
            // Nothing to do, skip
        }

        srcFile->seekg(blockStartOff + blockSize);
    }

    ofstream destFile;
    destFile.open(destFileName, ios::trunc | ios::binary);
    if (!destFile.is_open()) {
        return false;
    }
    
    const char fourcc[]{ 'P', 'B', 'F', 0 };
    destFile.write(fourcc, sizeof(fourcc));

    PackedBitmapFontInfo info;
    srcFile->seekg(0, ios::end);
    info.fontFileSize = static_cast<uint32_t>(srcFile->tellg());
    info.texturesCount = static_cast<uint32_t>(textureFileNames.size());
    info.fontOffset = sizeof(fourcc) + sizeof(PackedBitmapFontInfo) + info.texturesCount * sizeof(PackedBitmapFontTextureInfo);
    destFile.write(reinterpret_cast<char*>(&info), sizeof(info));
    srcFile->seekg(0);

    vector<ifstream*> textureFiles;
    uint32_t textureOffset = info.fontOffset + info.fontFileSize;
    for (string textureFileName : textureFileNames) {
        PackedBitmapFontTextureInfo textureInfo;
        ifstream* textureFile = new ifstream;
        textureFile->open((folder / textureFileName).u8string(), ios::binary | ios::ate);
        if (!textureFile->is_open()) {
            return false;
        }
        textureInfo.textureFileSize = static_cast<uint32_t>(textureFile->tellg());
        textureFile->seekg(0);
        textureInfo.textureOffset = textureOffset;
        destFile.write(reinterpret_cast<char*>(&textureInfo), sizeof(textureInfo));
        textureOffset += textureInfo.textureFileSize;
        textureFiles.push_back(textureFile);
    }

    destFile << srcFile->rdbuf();
    for (size_t i = 0; i < textureFiles.size(); ++i) {
        destFile << textureFiles[i]->rdbuf();
        textureFiles[i]->close();
        delete textureFiles[i];
    }
    destFile.close();

    return true;
}

bool Unpack(ifstream* srcFile, path srcFileName) {
    path folder(srcFileName);
    folder.remove_filename();
    path destFileName(srcFileName);
    destFileName.replace_extension(".fnt");
    vector<string> textureFileNames;

    srcFile->seekg(4);
    PackedBitmapFontInfo info;
    srcFile->read(reinterpret_cast<char*>(&info), sizeof(info));
    streampos lastPos = srcFile->tellg();

    srcFile->seekg(info.fontOffset + 4); // + 4 because of FOURCC
    while (srcFile->tellg() < info.fontOffset + info.fontFileSize) {
        unsigned char blockType;
        srcFile->read(reinterpret_cast<char*>(&blockType), sizeof(blockType));
        uint32_t blockSize;
        srcFile->read(reinterpret_cast<char*>(&blockSize), sizeof(blockSize));
        streamoff blockStartOff = srcFile->tellg();

        if (blockType == 3) {
            // Extract file names
            char* blockFileNames = new char[blockSize];
            srcFile->read(blockFileNames, blockSize);
            for (uint32_t i = 0; i < blockSize; ) {
                string fileName = string(blockFileNames + i);
                textureFileNames.push_back(fileName);
                i += static_cast<uint32_t>(fileName.length()) + 1;
            }
            delete blockFileNames;
        }
        else {
            // Nothing to do, skip
        }

        srcFile->seekg(blockStartOff + blockSize);
    }
    
    srcFile->seekg(info.fontOffset);
    ofstream destFile;
    destFile.open(destFileName, ios::trunc | ios::binary);
    if (!destFile.is_open()) {
        return false;
    }
    char* fntBuff = new char[info.fontFileSize];
    srcFile->read(fntBuff, info.fontFileSize);
    destFile.write(fntBuff, info.fontFileSize);
    delete fntBuff;
    destFile.close();

    for (uint32_t i = 0; i < info.texturesCount; ++i) {
        srcFile->seekg(lastPos);
        PackedBitmapFontTextureInfo textureInfo;
        srcFile->read(reinterpret_cast<char*>(&textureInfo), sizeof(textureInfo));
        lastPos = srcFile->tellg();
        srcFile->seekg(textureInfo.textureOffset);
        destFile.open(folder / textureFileNames[i], ios::trunc | ios::binary);
        if (!destFile.is_open()) {
            return false;
        }
        char* texBuff = new char[textureInfo.textureFileSize];
        srcFile->read(texBuff, textureInfo.textureFileSize);
        destFile.write(texBuff, textureInfo.textureFileSize);
        delete texBuff;
        destFile.close();
    }

    return true;
}
