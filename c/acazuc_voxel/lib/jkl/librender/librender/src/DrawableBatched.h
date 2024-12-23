#ifndef LIBRENDER_DRAWABLE_BATCHED_H
# define LIBRENDER_DRAWABLE_BATCHED_H

# include "./DrawableBase.h"

namespace librender
{

	class DrawableBatch;

	class DrawableBatched : public virtual DrawableBase
	{

	protected:
		DrawableBatch *parent;
		uint8_t changes;

	public:
		DrawableBatched();
		~DrawableBatched();
		void requireUpdates(uint8_t updates);
		void resize(uint32_t vertices, uint32_t indices);
		void update();
		virtual void setParent(DrawableBatch *parent);
		void setX(float x);
		float getX() const;
		void setY(float y);
		float getY() const;
		void setScaleX(float scaleX);
		void setScaleY(float scaleY);
		inline uint8_t getChanges() {return this->changes;};
		inline void removeChanges(uint8_t changes) {this->changes &= ~changes;};
		void addChanges(uint8_t changes);

	};

}

#endif
