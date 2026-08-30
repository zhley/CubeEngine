#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "glm/vec2.hpp"

#include "TextureRegion.h"
#include "Cube/Core/Path.h"
#include "Cube/Resource/Resource.h"

namespace Cube {
    class Texture2D;

    struct Glyph {
        glm::vec2 size;
		glm::vec2 bearing; // offset of the top-left corner relative to the origin
		TextureRegion texRegion; // top-left and bottom-right
		int advance;
		Texture2D* texture;
    };

	// Font class. based on FreeType
	class Font : public ResourceBase{
	public:
		Font(const Cube::Path& fontFilePath, int fontSize);
		~Font() = default;

		Glyph* getGlyph(uint32_t c);

		int getDescender() const;
		int getAscender() const;
		glm::vec2 calcTextSize(const std::string& text);
	private:
		Cube::Path fontFilePath;
        int fontSize;
		std::unordered_map<uint32_t, Glyph> glyphs;
		std::vector<std::unique_ptr<Texture2D>> atlasPages;

		struct FreeTypeData;
		std::unique_ptr<FreeTypeData> ftData;

		Glyph* loadGlyph(uint32_t c);
		// for function loadGlyph
		int texSize = 512;
		int x = 0;
		int y = 0;
		int maxH = 0;

		void loadFontFile();
	};
}