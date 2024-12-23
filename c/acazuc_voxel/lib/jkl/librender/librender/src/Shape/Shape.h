#ifndef LIBRENDER_SHAPE_H
# define LIBRENDER_SHAPE_H

# include "../DrawableTessellator.h"
# include "../DrawableBatched.h"
# include "../DrawableBatch.h"
# include "../DrawableBase.h"
# include "../Drawable.h"

namespace librender
{

	class ShapeBase : public virtual DrawableBase
	{
	};

	class Shape : public Drawable
	{

	public:
		Shape(uint32_t shapeType);
		Shape();
		void draw();

	};

	class ShapeBatched : public DrawableBatched
	{

	public:
		void setParent(DrawableBatch *parent);

	};

	class ShapeBatch : public DrawableBatch
	{

	public:
		ShapeBatch(uint32_t shapeType);
		ShapeBatch();
		void draw();

	};

	class ShapeTessellator : public DrawableTessellator
	{

	public:
		ShapeTessellator(uint32_t shapeType);
		ShapeTessellator();

	};

}

#endif
