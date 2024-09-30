#include "Doodad/Doodad.hpp"

#include "MpqManager.hpp"
#include "utility/BinaryStream.hpp"
#include "utility/Exception.hpp"
#include "utility/String.hpp"
#include "utility/Vector.hpp"

#include <cstdint>
#include <memory>
#include <string>

namespace parser
{
Doodad::Doodad(const std::string& path) : MpqPath(utility::lower(path))
{
    auto reader = sMpqManager.OpenFile(MpqPath);

    if (!reader)
    {
        // THROW_MSG("Doodad " + path + " not found", ResultCode::DOODAD_PATH_NOT_FOUND);
        std::cerr << "Doodad " << path << " not found" << std::endl;
        return;
    }

    auto const magic = reader->Read<std::uint32_t>();

    std::uint32_t vertexCount, indexCount;
    size_t verticesPosition, indicesPosition;

    if (magic == AlphaMagic)
    {
        auto const versionMagic = reader->Read<std::uint32_t>();
        auto const versionSize = reader->Read<std::uint32_t>();
        auto const version = reader->Read<std::uint32_t>();

        if (versionMagic != 'SREV')
            THROW(Result::UNEXPECTED_VERSION_MAGIC_IN_ALPHA_MODEL);
        if (versionSize != 4)
            THROW(Result::UNEXPECTED_VERSION_SIZE_IN_ALPHA_MODEL);
        if (version != 1300)
            THROW(Result::UNSUPPORTED_ALPHA_MODEL_VERSION);

        // offset to collision data pointer
        reader->rpos(0x5C);
        auto const collisionOffset = reader->Read<std::uint32_t>();

        // not all models are collideable
        size_t clidLocation;
        if (!reader->GetChunkLocation("DILC", 4, clidLocation))
            return;

        reader->rpos(clidLocation + 8);

        auto const vertexMagic = reader->Read<std::uint32_t>();
        if (vertexMagic != 'XTRV')
            THROW(Result::UNEXPECTED_VERTEX_MAGIC_IN_ALPHA_MODEL);

        vertexCount = reader->Read<std::uint32_t>();
        verticesPosition = reader->rpos();

        reader->rpos(verticesPosition + vertexCount * sizeof(math::Vertex));

        auto const indexMagic = reader->Read<std::uint32_t>();
        if (indexMagic != ' IRT')
            THROW(Result::UNEXPECTED_TRIANGLE_MAGIC_IN_ALPHA_MODEL);

        indexCount = reader->Read<std::uint32_t>();
        indicesPosition = reader->rpos();
    }
    else
    {
        if (magic != Magic)
            THROW(Result::INVALID_DOODAD_FILE);

        auto const version = reader->Read<std::uint32_t>();

        switch (version)
        {
            case 256: // Classic
            case 257: // Classic
            case 260: // TBC
            case 261: // TBC
            case 262: // TBC
            case 263: // TBC
                reader->rpos(0xEC);
                break;

            case 272: // TBC
            case 264: // WOTLK
                reader->rpos(0xD8);
                break;

            default:
                THROW_MSG("Unsupported doodad format: " + version, Result::UNSUPPORTED_DOOAD_FORMAT);
        }

        indexCount = reader->Read<std::uint32_t>();
        indicesPosition = reader->Read<std::uint32_t>();
        vertexCount = reader->Read<std::uint32_t>();
        verticesPosition = reader->Read<std::uint32_t>();
    }

    if (!indexCount || !vertexCount)
        return;

    Vertices.resize(vertexCount);

    // read bounding vertices
    reader->rpos(verticesPosition);
    reader->ReadBytes(&Vertices[0], vertexCount * sizeof(math::Vertex));

    // read bounding indices
    Indices.reserve(indexCount);

    reader->rpos(indicesPosition);
    for (auto i = 0u; i < indexCount; ++i)
        Indices.push_back(reader->Read<std::uint16_t>());
}
} // namespace parser