#include "Adt/Chunks/MHDR.hpp"

#include "Adt/AdtChunk.hpp"
#include "utility/BinaryStream.hpp"
#include "utility/Exception.hpp"

#include <cassert>

namespace parser
{
namespace input
{
MHDR::MHDR(size_t position, utility::BinaryStream* reader, bool alpha)
    : AdtChunk(position, reader)
{
    assert(Type == AdtChunkType::MHDR);
    assert(Size == 64);

    if (alpha)
    {
        Mh2oOffset = 0;

        DoodadNamesOffset = 0;
        WmoNamesOffset = 0;

        reader->rpos(position + 8 + 0x0C);
        DoodadPlacementOffset = reader->Read<std::uint32_t>();

        reader->rpos(position + 8 + 0x14);
        WmoPlacementOffset = reader->Read<std::uint32_t>();
    }
    else
    {
        reader->rpos(position + 8 + 0x0C);
        DoodadNamesOffset = reader->Read<std::uint32_t>();

        reader->rpos(position + 8 + 0x14);
        WmoNamesOffset = reader->Read<std::uint32_t>();

        reader->rpos(position + 8 + 0x1C);
        DoodadPlacementOffset = reader->Read<std::uint32_t>();

        reader->rpos(position + 8 + 0x20);
        WmoPlacementOffset = reader->Read<std::uint32_t>();

        reader->rpos(position + 8 + 0x28);
        Mh2oOffset = reader->Read<std::uint32_t>();
    }

    auto const offset = 8 + static_cast<std::uint32_t>(position);

    // if these offsets have values, adjust them to correct absolute position
    if (DoodadNamesOffset > 0)
        DoodadNamesOffset += offset;
    if (WmoNamesOffset > 0)
        WmoNamesOffset += offset;
    if (DoodadPlacementOffset > 0)
        DoodadPlacementOffset += offset;
    if (WmoPlacementOffset > 0)
        WmoPlacementOffset += offset;
    if (Mh2oOffset > 0)
        Mh2oOffset += offset;
}
} // namespace input
} // namespace parser