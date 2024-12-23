#ifndef LIBRENDER_DRAWABLE_TESSELLATOR_H
# define LIBRENDER_DRAWABLE_TESSELLATOR_H

# include "./Drawable.h"

namespace librender
{

	class DrawableTessellator : public Drawable
	{

	protected:
		void addPoint(Vec2 position, Vec2 texCoords, Vec4 color);

	public:
		DrawableTessellator(uint32_t shapeType);
		DrawableTessellator();
		void clear();

	};

}

#endif
