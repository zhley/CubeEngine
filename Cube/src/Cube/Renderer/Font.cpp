#include "Font.h"

#include <algorithm>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "Cube/Renderer/Texture.h"
#include "Cube/Utils/Utils.h"
#include "Cube/Core/Log.h"


namespace Cube {

    struct Font::FreeTypeData {
        FT_Library lib;
        FT_Face face;
        ~FreeTypeData() {
            if(face) FT_Done_Face(face);
            if(lib) FT_Done_FreeType(lib);
        }
    };

    Font::Font(const Cube::Path& fontFilePath, int fontSize) : fontFilePath(fontFilePath), fontSize(fontSize){
        loadFontFile();
    }

    Glyph* Font::getGlyph(uint32_t c) {
        auto it = glyphs.find(c);
        if(it != glyphs.end()) {
            return &it->second;
        }
        return loadGlyph(c);
    }

    int Font::getDescender() const { return ftData->face->descender * fontSize / ftData->face->units_per_EM; }
    int Font::getAscender() const { return ftData->face->ascender * fontSize / ftData->face->units_per_EM; }

    glm::vec2 Font::calcTextSize(const std::string& text){
        glm::vec2 res;
        res.y = (ftData->face->ascender - ftData->face->descender) * fontSize / ftData->face->units_per_EM;
        res.x = 0;
        for(auto c : Utils::utf8To32(text)) {
            res.x += getGlyph(c)->advance;
        }
        return res;
    }

    Glyph* Font::loadGlyph(uint32_t c) {
        if(FT_Error err = FT_Load_Char(ftData->face, c, FT_LOAD_RENDER)) {
            CB_CORE_ERROR("FreeType: Failed to load character '{}'. Error Code = {}", c, err);
            return nullptr;
        }
        Glyph glyph;
        glyph.size = {ftData->face->glyph->bitmap.width, ftData->face->glyph->bitmap.rows};
        glyph.bearing = {
            ftData->face->glyph->bitmap_left,
            ftData->face->glyph->bitmap_top,
        };
        glyph.advance = ftData->face->glyph->advance.x >> 6;

        // add glyph to atlas
        uint8_t* rawData = ftData->face->glyph->bitmap.buffer;
        int width = ftData->face->glyph->bitmap.width;
        int height = ftData->face->glyph->bitmap.rows;
        if(y + height > texSize) {
            x = 0;
            y = 0;
            maxH = 0;
            atlasPages.push_back(nullptr);
        }
        if(!atlasPages.back()) {
            uint8_t* d = new uint8_t[texSize * texSize * 4];
            atlasPages.back().reset(new Texture2D(texSize, texSize, d));
            delete[] d;
        }
        uint8_t* data = new uint8_t[width * height * 4];
        for(int i = 0; i < height; i++) {
            for(int j = 0; j < width; j++) {
                data[i * width * 4 + j * 4] = 255;
                data[i * width * 4 + j * 4 + 1] = 255;
                data[i * width * 4 + j * 4 + 2] = 255;
                data[i * width * 4 + j * 4 + 3] = rawData[(height - i - 1) * width + j];
            }
        }
        atlasPages.back()->updateData(x, y, width, height, data);
        delete[] data;

        glyph.texture = atlasPages.back().get();
        glyph.texRegion.uvMin = glm::vec2(x, y + height) / (float)texSize;
        glyph.texRegion.uvMax = glm::vec2(x + width, y) / (float)texSize;
        glyphs[c] = glyph;

        x += width;
        maxH = std::max(height, maxH);
        if(x + width > texSize) {
            y += maxH;
            x = 0;
        }

        return &glyphs[c];
    }

    void Font::loadFontFile() {
        if(fontSize < 16) {
            texSize = 512;
        }else if(fontSize < 32) {
            texSize = 1024;
        }else if(fontSize < 64) {
            texSize = 2048;
        }else {
            texSize = 4096;
        }
        atlasPages.push_back(nullptr);
        ftData = std::make_unique<FreeTypeData>();
        if(FT_Error err = FT_Init_FreeType(&ftData->lib)) {
            CB_CORE_ERROR("FreeType: Failed to initialize library. Error Code = {}", err);
            return;
        }

        if(FT_Error err = FT_New_Face(ftData->lib, fontFilePath.c_str(), 0, &ftData->face)) {
            CB_CORE_ERROR("FreeType: Failed to load font file \"{}\". Error Code = {}", fontFilePath, err);
            return;
        }

        FT_Set_Pixel_Sizes(ftData->face, 0, fontSize);
        for(unsigned char c = 32; c < 128; c++) {
            loadGlyph(c);
        }
    }

}  // namespace Cube