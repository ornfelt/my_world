#include "DrawableBase.h"
#include "./DrawableBuffers.h"

namespace librender
{

	DrawableBase::DrawableBase()
	: scale(1)
	, pos(0)
	, verticesNumber(0)
	, indicesNumber(0)
	, updatesRequired(0)
	, buffers(DRAWABLE_BUFFER_POSITIONS | DRAWABLE_BUFFER_COLORS | DRAWABLE_BUFFER_TEX_COORDS | DRAWABLE_BUFFER_INDICES)
	{
	}

	void DrawableBase::resize(uint32_t vertices, uint32_t indices)
	{
		if (this->verticesNumber != vertices)
		{
			this->verticesNumber = vertices;
			this->texCoords.resize(this->verticesNumber);
			this->positions.resize(this->verticesNumber);
			this->colors.resize(this->verticesNumber);
			requireUpdates(DRAWABLE_BUFFER_POSITIONS | DRAWABLE_BUFFER_TEX_COORDS | DRAWABLE_BUFFER_COLORS);
		}
		if (this->indicesNumber != indices)
		{
			this->indicesNumber = indices;
			this->indices.resize(this->indicesNumber);
			requireUpdates(DRAWABLE_BUFFER_INDICES);
		}
	}

	void DrawableBase::requireUpdates(uint8_t update)
	{
		this->updatesRequired |= update;
	}

	void DrawableBase::updateBuffers()
	{
		if (this->updatesRequired & DRAWABLE_BUFFER_TEX_COORDS)
			updateTexCoords();
		if (this->updatesRequired & DRAWABLE_BUFFER_POSITIONS)
			updatePositions();
		if (this->updatesRequired & DRAWABLE_BUFFER_INDICES)
			updateIndices();
		if (this->updatesRequired & DRAWABLE_BUFFER_COLORS)
			updateColors();
		this->updatesRequired = 0;
	}

}
