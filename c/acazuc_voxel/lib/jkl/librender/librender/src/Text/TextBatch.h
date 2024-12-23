#ifndef LIBRENDER_TEXT_BATCH_H
# define LIBRENDER_TEXT_BATCH_H

# include "../DrawableBatch.h"
# include "../Font/Font.h"

namespace librender
{

	class TextBatch : public DrawableBatch
	{

	private:
		Font *font;

	public:
		TextBatch();
		void draw();
		int32_t getLineHeight();
		void setFont(Font *font);
		inline Font *getFont() {return this->font;};

	};

}

#endif
