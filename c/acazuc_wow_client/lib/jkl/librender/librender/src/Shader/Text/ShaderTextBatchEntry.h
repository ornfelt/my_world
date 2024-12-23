#ifndef LIBRENDER_SHADER_TEXT_BATCH_ENTRY_H
# define LIBRENDER_SHADER_TEXT_BATCH_ENTRY_H

# include "./ShaderTextEntry.h"

namespace librender
{

	class ShaderTextBatch;

	class ShaderTextBatchEntry : public ShaderTextEntry
	{

	private:
		ShaderTextBatch *parent;
		uint8_t changes;
		void resize(uint32_t len);

	public:
		ShaderTextBatchEntry();
		~ShaderTextBatchEntry();
		void requireUpdates(uint8_t updates);
		void update();
		Font *getFont();
		void setParent(ShaderTextBatch *textBatch);
		void setX(float x);
		void setY(float y);
		void setScaleX(float scaleX);
		void setScaleY(float scaleY);
		inline uint8_t getChanges() {return this->changes;};
		inline void removeChanges(uint8_t changes) {this->changes &= ~changes;};
		inline void addChanges(uint8_t changes) {this->changes |= changes;};

	};

}

#endif
