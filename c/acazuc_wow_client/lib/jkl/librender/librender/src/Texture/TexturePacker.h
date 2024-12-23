#ifndef TEXTURE_PACKER_H
# define TEXTURE_PACKER_H

# include "./Texture.h"
# include <cstdint>
# include <vector>

namespace librender
{

	struct TexturePackerEntry
	{
		Texture **texture;
		uint32_t *x;
		uint32_t *y;
		uint32_t height;
		uint32_t width;
		void *data;
	};

	class TexturePacker
	{

	private:
		std::vector<TexturePackerEntry> entries;
		Texture *packTexture(std::vector<TexturePackerEntry> &datas, int32_t maxSize, std::vector<TexturePackerEntry> &remaining);

	public:
		std::vector<Texture*> pack(std::vector<TexturePackerEntry> &datas, int32_t maxSize);
		inline void addEntry(TexturePackerEntry entry) {this->entries.push_back(entry);};
		inline void clear() {this->entries.clear();};

	};

}

#endif
