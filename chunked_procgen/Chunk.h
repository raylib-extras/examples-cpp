#pragma once

#include "raylib.h"

#include <vector>
#include <mutex>

struct ChunkOrigin
{
    int32_t X;
    int32_t Y;

    uint64_t GetId() const
    {
        return (uint64_t(X) << 32) | uint64_t(Y);
    }

    bool operator<(const ChunkOrigin& o)  const
    {
        return GetId() < o.GetId();
    }

    ChunkOrigin() = default;

    ChunkOrigin(int32_t x, int32_t y)
    {
        X = x;
        Y = y;
    }

    // Arithmetic operators for ChunkOrigin
    ChunkOrigin operator+(const ChunkOrigin& other) const
    {
        return ChunkOrigin(X + other.X, Y + other.Y);
    }

    ChunkOrigin operator-(const ChunkOrigin& other) const
    {
        return ChunkOrigin(X - other.X, Y - other.Y);
    }

    ChunkOrigin operator*(int32_t scalar) const
    {
        return ChunkOrigin(X * scalar, Y * scalar);
    }

    ChunkOrigin operator/(int32_t scalar) const
    {
        return ChunkOrigin(X / scalar, Y / scalar);
    }

    ChunkOrigin& operator+=(const ChunkOrigin& other)
    {
        X += other.X;
        Y += other.Y;
        return *this;
    }

    ChunkOrigin& operator-=(const ChunkOrigin& other)
    {
        X -= other.X;
        Y -= other.Y;
        return *this;
    }

    ChunkOrigin& operator*=(int32_t scalar)
    {
        X *= scalar;
        Y *= scalar;
        return *this;
    }

    ChunkOrigin& operator/=(int32_t scalar)
    {
        X /= scalar;
        Y /= scalar;
        return *this;
    }

    bool operator==(const ChunkOrigin& other) const
    {
        if (X == other.X && Y == other.Y) 
            return true;
        else 
            return false;
    }

    struct Hasher
    {
        size_t operator()(const ChunkOrigin& k) const
        {
            return size_t(k.GetId());
        }
    };
};

template <>
struct std::hash<ChunkOrigin>
{
    std::size_t operator()(const ChunkOrigin& k) const
    {
        return size_t(k.GetId());
    }
};

enum class ChunkState
{
    Ungenerated,
    Generated,
    Cached,
};

struct ChunkRenderInfo
{
    RenderTexture2D BaseLayer;
    RenderTexture2D OverlayLayer;
};


struct Chunk
{
    ChunkOrigin Origin;
    
    ChunkRenderInfo* RenderInfo = nullptr;
    std::vector<int> Tiles;

    std::mutex Mutex;

    ChunkState State = ChunkState::Ungenerated;

    Chunk(int32_t x, int32_t y)
    {
        Origin.X = x;
        Origin.Y = y;
        Tiles.resize(16 * 16);
    }

    Chunk(ChunkOrigin origin)
    {
        Origin = origin;
        Tiles.resize(16 * 16);
    }

    ChunkState GetState()
    {
        std::lock_guard<std::mutex> lock(Mutex);
        return State;
    }
};