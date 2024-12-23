#include "DrawableBatched.h"
#include "./DrawableBuffers.h"
#include "./DrawableBatch.h"

namespace librender
{

	DrawableBatched::DrawableBatched()
	: parent(nullptr)
	, changes(0)
	{
	}

	DrawableBatched::~DrawableBatched()
	{
		if (this->parent)
			this->parent->removeChild(this);
	}

	void DrawableBatched::requireUpdates(uint8_t updates)
	{
		DrawableBase::requireUpdates(updates);
		addChanges(updates);
	}

	void DrawableBatched::addChanges(uint8_t changes)
	{
		this->changes |= changes;
		if (this->parent)
			this->parent->requireUpdates(changes);
	}

	void DrawableBatched::resize(uint32_t vertices, uint32_t indices)
	{
		if (this->parent)
			this->parent->setMustResize(true);
		DrawableBase::resize(vertices, indices);
	}

	void DrawableBatched::update()
	{
		updateBuffers();
	}

	void DrawableBatched::setParent(DrawableBatch *parent)
	{
		this->parent = parent;
	}

	void DrawableBatched::setX(float x)
	{
		if (this->pos.x == x)
			return;
		this->pos.x = x;
		addChanges(DRAWABLE_BUFFER_POSITIONS);
	}

	float DrawableBatched::getX() const
	{
		if (!this->parent)
			return 0;
		return this->parent->getX() + this->pos.x;
	}

	void DrawableBatched::setY(float y)
	{
		if (this->pos.y == y)
			return;
		this->pos.y = y;
		addChanges(DRAWABLE_BUFFER_POSITIONS);
	}

	float DrawableBatched::getY() const
	{
		if (!this->parent)
			return 0;
		return this->parent->getY() + this->pos.y;
	}
	
	void DrawableBatched::setScaleX(float scaleX)
	{
		if (this->scale.x == scaleX)
			return;
		this->scale.x = scaleX;
		addChanges(DRAWABLE_BUFFER_POSITIONS);
	}

	void DrawableBatched::setScaleY(float scaleY)
	{
		if (this->scale.y == scaleY)
			return;
		this->scale.y = scaleY;
		addChanges(DRAWABLE_BUFFER_POSITIONS);
	}

}
