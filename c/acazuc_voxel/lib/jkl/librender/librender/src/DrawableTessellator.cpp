#include "DrawableTessellator.h"

namespace librender
{

	DrawableTessellator::DrawableTessellator(uint32_t shapeType)
	: Drawable(shapeType)
	{
	}

	DrawableTessellator::DrawableTessellator()
	{
	}

	void DrawableTessellator::addPoint(Vec2 position, Vec2 texCoords, Vec4 color)
	{
		this->texCoords.push_back(texCoords);
		this->positions.push_back(position);
		this->colors.push_back(color);
	}

	void DrawableTessellator::clear()
	{
		std::vector<uint32_t> emptyIndices;
		std::vector<Vec2> emptyTexCoords;
		std::vector<Vec2> emptyPositions;
		std::vector<Vec4> emptyColors;
		this->texCoords.swap(emptyTexCoords);
		this->positions.swap(emptyPositions);
		this->indices.swap(emptyIndices);
		this->colors.swap(emptyColors);
	}

}
