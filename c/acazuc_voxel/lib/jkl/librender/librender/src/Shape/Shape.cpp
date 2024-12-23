#include "Shape.h"
#include "../DrawableBuffers.h"
#include <iostream>

namespace librender
{

	Shape::Shape(uint32_t shapeType)
	: Drawable(shapeType)
	{
	}

	Shape::Shape()
	{
	}

	void Shape::draw()
	{
		updateBuffers();
		Drawable::draw();
	}

	void ShapeBatched::setParent(DrawableBatch *parent)
	{
		if (this->parent)
			requireUpdates(DRAWABLE_BUFFER_POSITIONS | DRAWABLE_BUFFER_COLORS);
		DrawableBatched::setParent(parent);
	}

	ShapeBatch::ShapeBatch(uint32_t shapeType)
	: DrawableBatch(shapeType)
	{
	}

	ShapeBatch::ShapeBatch()
	{
	}

	void ShapeBatch::draw()
	{
		if (!update())
			return;
		DrawableBatch::draw();
	}

	ShapeTessellator::ShapeTessellator(uint32_t shapeType)
	: DrawableTessellator(shapeType)
	{
	}

	ShapeTessellator::ShapeTessellator()
	{
	}

}
