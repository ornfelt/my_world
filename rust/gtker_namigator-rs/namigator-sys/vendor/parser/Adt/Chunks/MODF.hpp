#include "Adt/AdtChunk.hpp"
#include "Wmo/WmoPlacement.hpp"
#include "utility/BinaryStream.hpp"

#include <vector>

namespace parser
{
namespace input
{
class MODF : AdtChunk
{
public:
    std::vector<WmoPlacement> Wmos;

    MODF(size_t position, utility::BinaryStream* reader);
};
} // namespace input
} // namespace parser