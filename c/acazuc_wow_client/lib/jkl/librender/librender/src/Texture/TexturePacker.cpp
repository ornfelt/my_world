#include "TexturePacker.h"
#include "TexturePackerInternal.h"
#include <algorithm>
#include <cstring>

#define DISCARD_STEP 1

namespace librender
{

	static bool heuristicArea(TexturePackerEntry &a, TexturePackerEntry &b)
	{
		return a.width * a.height > b.width * b.height;
	}

	static bool heuristicPerimeter(TexturePackerEntry &a, TexturePackerEntry &b)
	{
		return (a.width + a.height) * 2 > (b.width + b.height) * 2;
	}

	static bool heuristicMaxSide(TexturePackerEntry &a, TexturePackerEntry &b)
	{
		return std::max(a.width, a.height) > std::max(b.width, b.height);
	}

	static bool heuristicMaxWidth(TexturePackerEntry &a, TexturePackerEntry &b)
	{
		return a.width > b.width;
	}

	static bool heuristicMaxHeight(TexturePackerEntry &a, TexturePackerEntry &b)
	{
		return a.height > b.height;
	}

	static bool (*heuristics[])(TexturePackerEntry &a, TexturePackerEntry &b) =
	{
		heuristicArea,
		heuristicPerimeter,
		heuristicMaxSide,
		heuristicMaxWidth,
		heuristicMaxHeight
	};

	Texture *TexturePacker::packTexture(std::vector<TexturePackerEntry> &datas, int32_t maxSize, std::vector<TexturePackerEntry> &remaining)
	{
		Node root;
		int32_t heuristicsNb = sizeof(heuristics) / sizeof(*heuristics);
		std::vector<TexturePackerEntry> ordered[heuristicsNb];
		for (int32_t i = 0; i < heuristicsNb; ++i)
		{
			ordered[i] = datas;
			std::sort(ordered[i].begin(), ordered[i].end(), heuristics[i]);
		}
		WHRect minSize = WHRect(maxSize, maxSize);
		int32_t minHeuristic = -1;
		int32_t bestHeuristic = 0;
		int32_t bestArea = 0;
		int32_t area = 0;
		for (int32_t h = 0; h < heuristicsNb; ++h)
		{
			bool fail = false;
			std::vector<TexturePackerEntry> &rects = ordered[h];
			int32_t step = minSize.width / 2;
			root.reset(minSize);
			while (true)
			{
				if (root.rect.width > minSize.width)
				{
					if (minHeuristic > -1)
						break;
					area = 0;
					root.reset(minSize);
					for (int32_t i = 0; i < rects.size(); ++i)
					{
						if (root.insert(WHRect(rects[i].width, rects[i].height)))
							area += rects[i].width * rects[i].height;
					}
					fail = true;
					break;
				}
				int32_t grow = -1;
				for (int32_t i = 0; i < rects.size(); ++i)
				{
					if (!root.insert(WHRect(rects[i].width, rects[i].height)))
					{
						grow = 1;
						break;
					}
				}
				if (grow == -1 && step <= DISCARD_STEP)
					break;
				root.reset(WHRect(root.rect.width + grow * step, root.rect.height + grow * step));
				step /= 2;
				if (!step)
					step = 1;
			}
			if (!fail && root.rect.width * root.rect.height < minSize.width * minSize.height)
			{
				minSize = WHRect(root.rect.width, root.rect.height);
				minHeuristic = h;
			}
			else if (fail && area > bestArea)
			{
				bestArea = area;
				bestHeuristic = h;
			}
		}
		std::vector<TexturePackerEntry> &rects = ordered[minHeuristic == -1 ? bestHeuristic : minHeuristic];
		root.reset(minSize);
		Texture *texture = new Texture();
		std::vector<TexturePackerEntry> placed;
		int32_t maxWidth = 0;
		int32_t maxHeight = 0;
		for (int32_t i = 0; i < rects.size(); ++i)
		{
			Node *ret = root.insert(WHRect(rects[i].width, rects[i].height));
			if (!ret)
			{
				remaining.push_back(rects[i]);
				continue;
			}
			*rects[i].x = ret->rect.x;
			*rects[i].y = ret->rect.y;
			*rects[i].texture = texture;
			if (ret->rect.x + ret->rect.width > maxWidth)
				maxWidth = ret->rect.x + ret->rect.width;
			if (ret->rect.y + ret->rect.height > maxHeight)
				maxHeight = ret->rect.y + ret->rect.height;
			placed.push_back(rects[i]);
		}
		char *data = new char[maxWidth * maxHeight * 4];
		for (uint32_t i = 0; i < placed.size(); ++i)
		{
			for (uint32_t y = 0; y < placed[i].height; ++y)
				std::memcpy(&data[(*placed[i].x + *placed[i].y * placed[i].width) * 4], &((char*)placed[i].data)[y * placed[i].width * 4], placed[i].width * 4);
		}
		texture->setData(data, maxWidth, maxHeight);
		delete[] (data);
		return texture;
	}


	std::vector<Texture*> TexturePacker::pack(std::vector<TexturePackerEntry> &datas, int32_t maxSize)
	{
		std::vector<Texture*> textures;
		std::vector<TexturePackerEntry> vec[2];
		vec[0] = datas;
		vec[1].reserve(datas.size());
		int32_t i = 0;
		while (true)
		{
			textures.push_back(packTexture(vec[0], maxSize, vec[1]));
			++i;
			if (!vec[1].size())
				return textures;
			vec[0].clear();
			std::swap(vec[0], vec[1]);
		}
		return textures;
	}

}
