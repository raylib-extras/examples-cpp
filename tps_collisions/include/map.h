
/*********************************************************************************************
*
*   raylib-extras, FPS collision example
*
*   LICENSE: MIT
*
*   Copyright (c) 2024 Jeffery Myers
*
*   Permission is hereby granted, free of charge, to any person obtaining a copy
*   of this software and associated documentation files (the "Software"), to deal
*   in the Software without restriction, including without limitation the rights
*   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*   copies of the Software, and to permit persons to whom the Software is
*   furnished to do so, subject to the following conditions:
*
*   The above copyright notice and this permission notice shall be included in all
*   copies or substantial portions of the Software.
*
*   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
*   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
*   SOFTWARE.
*
**********************************************************************************************/

#pragma once

#include "object_transform.h"
#include <vector>
#include <deque>

inline BoundingBox operator + (const BoundingBox& lhs, const Vector3& rhs)
{
    return BoundingBox{ lhs.min - rhs, lhs.max + rhs };
}

inline BoundingBox operator + (const BoundingBox& lhs, const float& rhs)
{
    return BoundingBox{ lhs.min - Vector3{rhs,rhs,rhs}, lhs.max + Vector3{rhs, rhs,rhs} };
}

class Obstacle
{
public:
    ObjectTransform Transform;
    Vector3 Scale = { 1, 1, 1 };
    Color Tint = DARKGREEN;

    Vector3 thing{ 1,2,2 };

    Vector3 LastNearestPoint = { 0,0 };

    BoundingBox Bounds = { 0 };

    Obstacle() = default;
    Obstacle(float x, float z, float width, float height, float depth, float angle);
    bool CollideWithPlayer(Vector3& newPosition, Vector3 oldPosition, float radius, float height);

    bool CheckRaycast(Ray worldRay, RayCollision& collision, float radius = 0);
};

struct Explosion
{
    Vector3 Postion = { 0 };
    Vector3 Normal = { 0 };
    float Lifetime = 0;

    static constexpr float MaxLife = 0.25f;
};

class Map
{
public:
    std::vector<Obstacle> Walls;
    std::deque<Explosion> Explosions;

    static void SetupGraphics();
    static void CleanupGraphics();
    static Shader GetLightShader();

    void Setup();
    void Cleanup();

    bool CollidePlayer(Vector3& newPosition, Vector3 oldPosition, float radius, float height);
    bool CollideRay(Ray worldspaceRay, RayCollision& outputCollision, Obstacle* hitObstacle = nullptr, float radius = 0);

    void Draw(Camera3D& view);
    void DrawWalls(Camera3D& view);

    void AddExplosition(RayCollision& collision);

    void AddShotSound();

private:
    static constexpr int MaxSounds = 32;

    Sound ShotSound = { 0 };
    Sound ShotLoop[MaxSounds] = { 0 };
    int CurrentShotSound = 0;
};


void BuildDemoMap(Map& map);