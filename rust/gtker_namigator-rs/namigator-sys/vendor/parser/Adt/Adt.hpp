#pragma once

#include "parser/Doodad/DoodadInstance.hpp"
#include "parser/Wmo/WmoInstance.hpp"
#include "utility/BoundingBox.hpp"

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace parser
{
struct AdtChunk
{
    bool m_holeMap[8][8];

    float m_heights[9 * 9 + 8 * 8];

    std::vector<math::Vertex> m_terrainVertices;
    std::vector<int> m_terrainIndices;

    std::vector<math::Vertex> m_liquidVertices;
    std::vector<int> m_liquidIndices;

    std::vector<std::uint32_t> m_wmoInstances;
    std::vector<std::uint32_t> m_doodadInstances;

    std::uint32_t m_areaId;
    std::uint32_t m_zoneId;

    float m_minZ;
    float m_maxZ;
};

class Map;

class Adt
{
private:
    std::unique_ptr<AdtChunk> m_chunks[16][16];
    Map* const m_map;

public:
    const int X;
    const int Y;

    math::BoundingBox Bounds;

    Adt(Map* map, int x, int y);
    const AdtChunk* GetChunk(int chunkX, int chunkY) const;
};
} // namespace parser