#ifndef LIBRENDER_DRAWABLE_H
# define LIBRENDER_DRAWABLE_H

# include "./DrawableBase.h"

namespace librender
{

	class Drawable : public virtual DrawableBase
	{

	protected:
		uint32_t shapeType;

	public:
		Drawable(uint32_t shapeType);
		Drawable();
		virtual void draw();

	};

}

#endif
