#include "Worker.hpp"

#include "Common.hpp"
#include "MeshBuilder.hpp"
#include "parser/MpqManager.hpp"

#include <cassert>
#include <chrono>
#include <exception>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>
#include <thread>
#include <utility>

Worker::Worker(const std::string& dataPath, MeshBuilder* meshBuilder)
    : m_dataPath(dataPath), m_meshBuilder(meshBuilder),
      m_shutdownRequested(false), m_wmo(meshBuilder->IsGlobalWMO()),
      m_isFinished(false), m_thread(&Worker::Work, this)
{
}

Worker::~Worker()
{
    m_shutdownRequested = true;
    m_thread.join();
}

void Worker::Work()
{
    parser::sMpqManager.Initialize(m_dataPath);

    try
    {
        do
        {
            int tileX, tileY;
            if (!m_meshBuilder->GetNextTile(tileX, tileY))
                break;

            if (m_wmo)
            {
                if (!m_meshBuilder->BuildAndSerializeWMOTile(tileX, tileY))
                {
                    std::stringstream error;
                    error << "Thread #" << std::setfill(' ') << std::setw(6)
                          << std::this_thread::get_id() << " Global WMO tile ("
                          << std::setfill(' ') << std::setw(3) << tileX << ", "
                          << std::setfill(' ') << std::setw(3) << tileY
                          << ") FAILED!  cell size = " << MeshSettings::CellSize
                          << " voxels = " << MeshSettings::TileVoxelSize;
                    std::cout << error.str() << std::endl;
                    exit(1);
                }
            }
            else if (!m_meshBuilder->BuildAndSerializeMapTile(tileX, tileY))
            {
                auto const adtX = tileX / MeshSettings::TilesPerADT;
                auto const adtY = tileY / MeshSettings::TilesPerADT;
                auto const x = tileX % MeshSettings::TilesPerADT;
                auto const y = tileY % MeshSettings::TilesPerADT;

                std::stringstream error;
                error << "Thread #" << std::setfill(' ') << std::setw(6)
                      << std::this_thread::get_id() << " ADT ("
                      << std::setfill(' ') << std::setw(2) << adtX << ", "
                      << std::setfill(' ') << std::setw(2) << adtY << ") tile ("
                      << x << ", " << y
                      << ") FAILED!  cell size = " << MeshSettings::CellSize
                      << " voxels = " << MeshSettings::TileVoxelSize;
                std::cout << error.str() << std::endl;
                exit(1);
            }
        } while (!m_shutdownRequested);
    }
    catch (std::exception const& e)
    {
        std::stringstream s;

        s << "\nThread #" << std::setfill(' ') << std::setw(6)
          << std::this_thread::get_id() << ": " << e.what() << "\n";

        std::cerr << s.str();
    }

#ifdef _DEBUG
    std::stringstream str;
    str << "Thread #" << std::this_thread::get_id() << " finished.\n";
    std::cout << str.str();
#endif

    m_isFinished = true;
}

bool Worker::IsFinished() const
{
    return m_isFinished;
}
