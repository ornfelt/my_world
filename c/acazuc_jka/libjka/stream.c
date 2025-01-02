#include "libjka.h"

#include <stdlib.h>

void jka_stream_delete(struct jka_stream *stream)
{
	if (!stream)
		return;
	if (stream->op->dtr)
		stream->op->dtr(stream);
	free(stream);
}
