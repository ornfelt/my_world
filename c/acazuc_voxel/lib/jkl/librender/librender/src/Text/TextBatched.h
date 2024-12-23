#ifndef LIBRENDER_TEXT_BATCHED_H
# define LIBRENDER_TEXT_BATCHED_H

# include "../DrawableBatched.h"
# include "./TextBase.h"

namespace librender
{

	class TextBatch;

	class TextBatched : public DrawableBatched, public TextBase
	{

	public:
		void setParent(DrawableBatch *parent);
		Font *getFont();

	};

}

#endif
