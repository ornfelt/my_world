#ifndef LIBRENDER_DRAWABLE_BATCH_H
# define LIBRENDER_DRAWABLE_BATCH_H

# include "./DrawableBatched.h"
# include "./Drawable.h"

namespace librender
{

	class DrawableBatch : public Drawable
	{

	protected:
		std::vector<DrawableBatched*> childs;
		bool mustResize;
		void updateVerticesNumber();
		void updateIndicesNumber();
		void updateTexCoords();
		void updatePositions();
		void updateIndices();
		void updateColors();
		void resize();
		bool update();

	public:
		DrawableBatch(uint32_t shapeType);
		DrawableBatch();
		~DrawableBatch();
		virtual void addChild(DrawableBatched *child);
		virtual void removeChild(DrawableBatched *child);
		virtual void clearChilds();
		inline std::vector<DrawableBatched*> &getChilds() {return this->childs;};
		inline void setMustResize(bool mustResize) {this->mustResize = mustResize;};

	};

}

#endif
