#include "RenderArea.h"

#include <complex>

RenderLoop::RenderLoop(uint32_t size) : Size(size)
{
    if (size == 0)
    {
        Chunks.insert_or_assign(ChunkOrigin(0,0), nullptr);
    }

    int bounds = int(size);
    for (int x = -bounds; x <= bounds; x++)
    {
        Chunks.insert_or_assign(ChunkOrigin(x, bounds), nullptr);
        Chunks.insert_or_assign(ChunkOrigin(x, -bounds), nullptr);
    }

    for (int y = -bounds +1; y < bounds; y++)
    {
        Chunks.insert_or_assign(ChunkOrigin(bounds, y), nullptr);
        Chunks.insert_or_assign(ChunkOrigin(-bounds, y), nullptr);
    }
}

Chunk* RenderLoop::GetChunk(int32_t x, int32_t y)
{
    int32_t max = std::max(std::abs(x), std::abs(y));

    if (max != Size)
        return nullptr;

    return Chunks[ChunkOrigin(x, y)];
}

void RenderLoop::SetChunk(int32_t x, int32_t y, Chunk* chunk)
{
    int32_t max = std::max(std::abs(x), std::abs(y));

    if (max != Size)
        return;

    Chunks[ChunkOrigin(x, y)] = chunk;
}

RenderArea::RenderArea(uint32_t size)
{
    for (uint32_t i = 0; i <= size; i++)
        Loops.emplace_back(i);
}

Chunk* RenderArea::GetChunk(int32_t x, int32_t y)
{
    int32_t max = std::max(std::abs(x), std::abs(y));

    if (max > Loops.size())
        return nullptr;

    return Loops[max].GetChunk(x, y);
}

void RenderArea::SetChunk(int32_t x, int32_t y, Chunk* chunk)
{
    int32_t max = std::max(std::abs(x), std::abs(y));

    if (max > Loops.size())
        return;

    return Loops[max].SetChunk(x, y, chunk);
}

RenderAreaManager::RenderAreaManager(uint32_t size, int32_t orignX, int32_t originY) : Area(size)
{
    Origin = ChunkOrigin(orignX, originY);

    int RenderTextureSize = 256; 

    for (auto& loop : Area.Loops)
    {
        for (auto& [chunkOrigin, chunkInfo] : loop.Chunks)
        {
            UndefinedChunks[chunkOrigin] = Origin + chunkOrigin;
        }
    }
}

void RenderAreaManager::MoveOrigin(int32_t x, int32_t y)
{
    int32_t size = int32_t(Area.Loops.size()) - 1;

    // origin sifts right (X+), so shift everything left (x-)
    while (x > 0)
    {
        for (int currentY = -size; currentY <= size; currentY++)
        {
            for (int currentX = -size; currentX <= size; currentX++)
            {
                Chunk* chunk = Area.GetChunk(currentX, currentY);

                ChunkOrigin currentOrigin(currentX, currentY);

                // first chunk on the left side is destroyed
                if (currentX == -size)
                {
                    if (chunk != nullptr)
                        DeadChunks.push_back(chunk);

                    Area.SetChunk(currentX, currentY, nullptr);
                    continue;
                }
               
                // move this chunk to the left one slot, and clear it
                Area.SetChunk(currentX - 1, currentY, chunk);
                Area.SetChunk(currentX, currentY, nullptr);

                if (currentX == size)
                {
                    // the last chunk on the right needs to be generated
                    UndefinedChunks[currentOrigin] = Origin + currentOrigin + ChunkOrigin(1,0);
                }
            }
        }
        Origin.X += 1;
        x -= 1;
    }

    // origin sifts left (X-), so shift everything right (x+)
    while (x < 0)
    {
        for (int currentY = -size; currentY <= size; currentY++)
        {
            for (int currentX = size; currentX >= -size; currentX--)
            {
                Chunk* chunk = Area.GetChunk(currentX, currentY);

                ChunkOrigin currentOrigin(currentX, currentY);

                // first chunk on the right side is destroyed
                if (currentX == size)
                {
                    if (chunk != nullptr)
                        DeadChunks.push_back(chunk);

                    Area.SetChunk(currentX, currentY, nullptr);
                    continue;
                }

                // move this chunk to the right one slot, and clear it
                Area.SetChunk(currentX + 1, currentY, chunk);
                Area.SetChunk(currentX, currentY, nullptr);

                if (currentX == -size)
                {
                    // the last chunk on the left needs to be generated
                    UndefinedChunks[currentOrigin] = Origin + currentOrigin + ChunkOrigin(-1, 0);
                }
            }
        }
        Origin.X -= 1;
        x += 1;
    }

    // origin sifts down (Y+), so shift everything up (y-)
    while (y > 0)
    {
        for (int currentX = -size; currentX <= size; currentX++)
        {
            for (int currentY = -size; currentY <= size; currentY++)
            {
                Chunk* chunk = Area.GetChunk(currentX, currentY);

                ChunkOrigin currentOrigin(currentX, currentY);

                // first chunk on the top side is destroyed
                if (currentY == -size)
                {
                    if (chunk != nullptr)
                        DeadChunks.push_back(chunk);

                    Area.SetChunk(currentX, currentY, nullptr);
                    continue;
                }

                // move this chunk to the up one slot, and clear it
                Area.SetChunk(currentX, currentY-1, chunk);
                Area.SetChunk(currentX, currentY, nullptr);

                if (currentY == size)
                {
                    // the last chunk on the bottom needs to be generated
                    UndefinedChunks[currentOrigin] = Origin + currentOrigin + ChunkOrigin(0, 1);
                }
            }
        }
        Origin.Y += 1;
        y -= 1;
    }

    // origin sifts up (Y-), so shift everything down (y+)
    while (y < 0)
    {
        for (int currentX = -size; currentX <= size; currentX++)
        {
            for (int currentY = size; currentY >= -size; currentY--)
            {
                Chunk* chunk = Area.GetChunk(currentX, currentY);

                ChunkOrigin currentOrigin(currentX, currentY);

                // first chunk on the bottom side is destroyed
                if (currentY == size)
                {
                    if (chunk != nullptr)
                        DeadChunks.push_back(chunk);

                    Area.SetChunk(currentX, currentY, nullptr);
                    continue;
                }

                // move this chunk to the right one slot, and clear it
                Area.SetChunk(currentX, currentY+1, chunk);
                Area.SetChunk(currentX, currentY, nullptr);

                if (currentY == -size)
                {
                    // the last chunk on the left needs to be generated
                    UndefinedChunks[currentOrigin] = Origin + currentOrigin + ChunkOrigin(0, -1);
                }
            }
        }
        Origin.Y -= 1;
        y += 1;
    }
}
