#pragma once

#include "Chunk.h"

#include <unordered_map>

struct RenderLoop
{
    uint32_t Size = 0;
    std::unordered_map<ChunkOrigin, Chunk*> Chunks;

    RenderLoop(uint32_t size);

    Chunk* GetChunk(int32_t x, int32_t y);

    void SetChunk(int32_t x, int32_t y, Chunk* chunk);
};

struct RenderArea
{
    std::vector<RenderLoop> Loops;

    Chunk* GetChunk(int32_t x, int32_t y);

    void SetChunk(int32_t x, int32_t y, Chunk* chunk);

    RenderArea(uint32_t size);
};

class RenderAreaManager
{
public:
    RenderAreaManager(uint32_t size, int32_t originX = 0, int32_t originY = 0);

    std::unordered_map<ChunkOrigin,ChunkOrigin, ChunkOrigin::Hasher> UndefinedChunks;
    std::vector<Chunk*> DeadChunks;

    RenderArea Area;
    ChunkOrigin Origin;

    void MoveOrigin(int32_t x, int32_t y);
};