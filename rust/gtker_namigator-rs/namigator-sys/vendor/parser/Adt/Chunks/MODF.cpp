#include "Adt/Chunks/MODF.hpp"

#include <cassert>

namespace parser
{
namespace input
{
MODF::MODF(size_t position, utility::BinaryStream* reader)
    : AdtChunk(position, reader)
{
    assert(Type == AdtChunkType::MODF);

    if (!Size)
        return;

    Wmos.resize(Size / sizeof(WmoPlacement));
    reader->ReadBytes(&Wmos[0], Size);
}
} // namespace input
} // namespace parser