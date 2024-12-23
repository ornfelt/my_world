#include "DrawableBatch.h"
#include "./DrawableBuffers.h"

namespace librender
{

	DrawableBatch::DrawableBatch(uint32_t shapeType)
	: Drawable(shapeType)
	, mustResize(true)
	{
	}

	DrawableBatch::DrawableBatch()
	: mustResize(true)
	{
	}

	DrawableBatch::~DrawableBatch()
	{
		for (size_t i = 0; i < this->childs.size(); ++i)
			this->childs[i]->setParent(nullptr);
	}

	void DrawableBatch::updateVerticesNumber()
	{
		this->verticesNumber = 0;
		for (size_t i = 0; i < this->childs.size(); ++i)
			this->verticesNumber += this->childs[i]->getVerticesNumber();
	}

	void DrawableBatch::updateIndicesNumber()
	{
		this->indicesNumber = 0;
		for (size_t i = 0; i < this->childs.size(); ++i)
			this->indicesNumber += this->childs[i]->getIndicesNumber();
	}

	void DrawableBatch::updateTexCoords()
	{
		size_t count = 0;
		for (size_t i = 0; i < this->childs.size(); ++i)
		{
			DrawableBatched *child = this->childs[i];
			if (this->mustResize || child->getChanges() & DRAWABLE_BUFFER_TEX_COORDS)
			{
				std::copy(child->getTexCoords().begin(), child->getTexCoords().end(), this->texCoords.begin() + count);
				child->removeChanges(DRAWABLE_BUFFER_TEX_COORDS);
			}
			count += child->getTexCoords().size();
		}
	}

	void DrawableBatch::updatePositions()
	{
		size_t count = 0;
		for (size_t i = 0; i < this->childs.size(); ++i)
		{
			DrawableBatched *child = this->childs[i];
			if (this->mustResize || child->getChanges() & DRAWABLE_BUFFER_POSITIONS)
			{
				std::copy(child->getPositions().begin(), child->getPositions().end(), this->positions.begin() + count);
				child->removeChanges(DRAWABLE_BUFFER_POSITIONS);
				float x = child->getX() - this->getX();
				float y = child->getY() - this->getY();
				Vec2 pos(child->getX() - getX(), child->getY() - getY());
				Vec2 scale(child->getScale());
				if (pos.x || pos.y)
				{
					if (scale.x != 1 || scale.y != 1)
					{
						for (size_t i = count; i < count + child->getPositions().size(); ++i)
							this->positions[i] = this->positions[i] * scale + pos;
					}
					else
					{
						for (size_t i = count; i < count + child->getPositions().size(); ++i)
							this->positions[i] = this->positions[i] + pos;
					}
				}
				else if (scale.x != 1 || scale.y != 1)
				{
					for (size_t i = count; i < count + child->getPositions().size(); ++i)
						this->positions[i] = this->positions[i] * scale;
				}
			}
			count += child->getPositions().size();
		}
	}

	void DrawableBatch::updateIndices()
	{
		size_t count = 0;
		size_t offset = 0;
		for (size_t i = 0; i < this->childs.size(); ++i)
		{
			DrawableBatched *child = this->childs[i];
			if (this->mustResize || child->getChanges() & DRAWABLE_BUFFER_INDICES)
			{
				std::copy(child->getIndices().begin(), child->getIndices().end(), this->indices.begin() + count);
				for (size_t i = count; i < count + child->getIndices().size(); ++i)
					this->indices[i] += offset;
				child->removeChanges(DRAWABLE_BUFFER_INDICES);
			}
			offset += child->getVerticesNumber();
			count += child->getIndices().size();
		}
	}

	void DrawableBatch::updateColors()
	{
		size_t count = 0;
		for (size_t i = 0; i < this->childs.size(); ++i)
		{
			DrawableBatched *child = this->childs[i];
			if (this->mustResize || child->getChanges() & DRAWABLE_BUFFER_COLORS)
			{
				std::copy(child->getColors().begin(), child->getColors().end(), this->colors.begin() + count);
				child->removeChanges(DRAWABLE_BUFFER_COLORS);
			}
			count += child->getColors().size();
		}
	}

	void DrawableBatch::resize()
	{
		updateVerticesNumber();
		updateIndicesNumber();
		this->texCoords.resize(this->verticesNumber);
		this->positions.resize(this->verticesNumber);
		this->indices.resize(this->indicesNumber);
		this->colors.resize(this->verticesNumber);
	}

	bool DrawableBatch::update()
	{
		for (size_t i = 0; i < this->childs.size(); ++i)
			this->childs[i]->update();
		if (this->mustResize)
			resize();
		if (!this->verticesNumber)
			return false;
		if (this->mustResize)
			requireUpdates(DRAWABLE_BUFFER_INDICES | DRAWABLE_BUFFER_TEX_COORDS | DRAWABLE_BUFFER_POSITIONS | DRAWABLE_BUFFER_COLORS);
		if (this->updatesRequired & DRAWABLE_BUFFER_TEX_COORDS)
			updateTexCoords();
		if (this->updatesRequired & DRAWABLE_BUFFER_POSITIONS)
			updatePositions();
		if (this->updatesRequired & DRAWABLE_BUFFER_INDICES)
			updateIndices();
		if (this->updatesRequired & DRAWABLE_BUFFER_COLORS)
			updateColors();
		if (this->mustResize)
			setMustResize(false);
		this->updatesRequired = 0;
		return true;
	}

	void DrawableBatch::addChild(DrawableBatched *child)
	{
		child->setParent(this);
		this->childs.push_back(child);
		this->mustResize = true;
		requireUpdates(DRAWABLE_BUFFER_INDICES | DRAWABLE_BUFFER_TEX_COORDS | DRAWABLE_BUFFER_POSITIONS | DRAWABLE_BUFFER_COLORS);
	}

	void DrawableBatch::removeChild(DrawableBatched *child)
	{
		for (size_t i = 0; i < this->childs.size(); ++i)
		{
			if (this->childs[i] == child)
			{
				this->childs.erase(this->childs.begin() + i);
				child->setParent(nullptr);
				setMustResize(true);
				requireUpdates(DRAWABLE_BUFFER_INDICES | DRAWABLE_BUFFER_TEX_COORDS | DRAWABLE_BUFFER_POSITIONS | DRAWABLE_BUFFER_COLORS);
				return;
			}
		}
	}

	void DrawableBatch::clearChilds()
	{
		for (size_t i = 0; i < this->childs.size(); ++i)
			this->childs[i]->setParent(nullptr);
		this->childs.clear();
		setMustResize(true);
		requireUpdates(DRAWABLE_BUFFER_INDICES | DRAWABLE_BUFFER_TEX_COORDS | DRAWABLE_BUFFER_POSITIONS | DRAWABLE_BUFFER_COLORS);
	}

}
