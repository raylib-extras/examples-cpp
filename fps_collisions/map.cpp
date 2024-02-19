
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

#include "map.h"
#include "collisions.h"

#define RLIGHTS_IMPLEMENTATION
#include "rlights.h"

// obstacle
Obstacle::Obstacle(float x, float z, float width, float height, float depth, float angle)
{
    Transform.SetPosition(x, height * 0.5f, z);
    Transform.RotateV(angle);
    Scale.x = width;
    Scale.y = height;
    Scale.z = depth;

    Bounds.min = Vector3{ width * -0.5f, height * -0.5f, depth * -0.5f };
    Bounds.max = Vector3{ width * 0.5f, height * 0.5f, depth * 0.5f };
}

bool Obstacle::CollideWithPlayer(Vector3& newPosition, Vector3 oldPosition, float radius, float height)
{
    // transform the input into the rotated space of the obstacle
    Matrix objectMatrix = Transform.GetWorldMatrix();
    Vector3 localPos = Vector3Transform(newPosition, MatrixInvert(objectMatrix));
    Vector3 locaOldPos = Vector3Transform(oldPosition, MatrixInvert(objectMatrix));

    Vector3 hitNormal = { 0 };

    // see if the player cylinder (in local space) hits our bounding box, and clamp the position to be outside of it
    bool hit = IntersectBBoxCylinder(Bounds, localPos, locaOldPos, radius, height, LastNearestPoint, hitNormal);

    // transform the local position back into worldspace
    newPosition = Vector3Transform(localPos, objectMatrix);

    return hit;
}

bool Obstacle::CheckRaycast(Ray worldRay, RayCollision & collision)
{
    // transform the ray into local space
    Matrix objectMatrix = Transform.GetWorldMatrix();
    Ray localRay = { 0 };
    localRay.position = Vector3Transform(worldRay.position, MatrixInvert(objectMatrix));
    localRay.direction = Vector3Transform(Vector3Add(worldRay.position, worldRay.direction), MatrixInvert(objectMatrix));
    localRay.direction = Vector3Subtract(localRay.direction, localRay.position);

    // see if the local space ray hits our bounding box
    collision = GetRayCollisionBox(localRay, Bounds);


    // transform the hit point and normal back into world space
    if (collision.hit)
    {
        collision.point = Vector3Transform(collision.point, objectMatrix);
        collision.normal = Vector3RotateByQuaternion(collision.normal, Transform.GetOrientation());
    }

    return collision.hit;
}

// map graphics
static Mesh WallMesh = { 0 };
static Mesh PlaneMesh = { 0 };

static Material WallMaterial = { 0 };
static Material PlaneMaterial = { 0 };

void BuildDemoMap(Map& map)
{  
    map.Walls.emplace_back(10.0f, 10.0f, 3.5f, 3.0f, 14.0f, 0.0f);
    map.Walls.emplace_back(20.0f, 10.0f, 0.5f, 3.0f, 15.0f, 90.0f);
    map.Walls.emplace_back(-20.0f, 10.0f, 0.5f, 3.0f, 15.0f, 45.0f);
    map.Walls.emplace_back(-20.0f, 10.0f, 0.5f, 3.0f, 15.0f, -45.0f);

    map.Walls.emplace_back(20.0f, -10.0f, 2.5f, 3.0f, 25.0f, 30.0f);

    map.Walls.emplace_back(0.0f, -20.0f, 1.5f, 3.0f, 35.0f, 90.0f);
}


// map
void Map::SetupGraphics()
{
    CleanupGraphics();

    WallMesh = GenMeshCube(1, 1, 1);
    WallMaterial = LoadMaterialDefault();
    WallMaterial.shader = LoadShader("resources/shaders/lighting.vs", "resources/shaders/lighting.fs");
    WallMaterial.shader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(WallMaterial.shader, "viewPos");

    CreateLight(LIGHT_POINT, Vector3{ -200, 100, -200 }, Vector3Zero(), WHITE, WallMaterial.shader);

    // Ambient light level (some basic lighting)
    int ambientLoc = GetShaderLocation(WallMaterial.shader, "ambient");
    float ambient[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
    SetShaderValue(WallMaterial.shader, ambientLoc, ambient, SHADER_UNIFORM_VEC4);

    WallMaterial.maps[MATERIAL_MAP_ALBEDO].texture = LoadTexture("resources/texture_12.png");

    PlaneMesh = GenMeshPlane(100, 100, 50, 50);
    PlaneMaterial = LoadMaterialDefault();
    PlaneMaterial.shader = WallMaterial.shader;
    PlaneMaterial.maps[MATERIAL_MAP_ALBEDO].texture = LoadTexture("resources/texture_07.png");
}

void Map::CleanupGraphics()
{
    if (!IsShaderReady(WallMaterial.shader))
        return;

    UnloadShader(WallMaterial.shader);
    WallMaterial.shader.id = 0;

    UnloadTexture(WallMaterial.maps[MATERIAL_MAP_ALBEDO].texture);
    WallMaterial.maps[MATERIAL_MAP_ALBEDO].texture.id = 0;

    UnloadTexture(PlaneMaterial.maps[MATERIAL_MAP_ALBEDO].texture);
    PlaneMaterial.maps[MATERIAL_MAP_ALBEDO].texture.id = 0;

    UnloadMesh(WallMesh);
    WallMesh.vertexCount = 0;
}

Shader Map::GetLightShader()
{
    return WallMaterial.shader;
}

void Map::Setup()
{
    ShotSound = LoadSound("resources/laserLarge_004.ogg");
    for (int i = 0; i < MaxSounds; i++)
        ShotLoop[i] = LoadSoundAlias(ShotSound);
}

void Map::Cleanup()
{
    for (int i = 0; i < MaxSounds; i++)
        UnloadSoundAlias(ShotLoop[i]);

    UnloadSound(ShotSound);
}

bool Map::CollidePlayer(Vector3& newPosition, Vector3 oldPosition, float radius, float height)
{
    bool hitSomething = false;
    for (auto& wall : Walls)
    {
        if (wall.CollideWithPlayer(newPosition, oldPosition, radius, height))
            hitSomething = true;
    }

    return hitSomething;
}

bool Map::CollideRay(Ray worldspaceRay, RayCollision& outputCollision, Obstacle* hitObstacle)
{
    RayCollision collision = { 0 };

    outputCollision.hit = false;
    outputCollision.distance = std::numeric_limits<float>::max();

    for (auto& wall : Walls)
    {
        if (wall.CheckRaycast(worldspaceRay, collision))
        {
            if (collision.distance < outputCollision.distance)
            {
                outputCollision = collision;
                if (hitObstacle != nullptr)
                    hitObstacle = &wall;
            }
        }
    }

    return outputCollision.hit;
}

void Map::Draw(Camera3D &view)
{
    if (!IsTextureReady(PlaneMaterial.maps[MATERIAL_MAP_ALBEDO].texture))
        return;

    float cameraPos[3] = { view.position.x, view.position.y, view.position.z };
    SetShaderValue(WallMaterial.shader, WallMaterial.shader.locs[SHADER_LOC_VECTOR_VIEW], cameraPos, SHADER_UNIFORM_VEC3);

    DrawMesh(PlaneMesh, PlaneMaterial, MatrixIdentity());

    DrawWalls(view);

    // axis markers
    DrawCube(Vector3{ 1,0,0 }, 1.25f, 0.25f, 0.25f, RED);
    DrawCube(Vector3{ 0,0,1 }, 0.25f, 0.25f, 1.25f, BLUE);

    // explosions
    rlDisableDepthMask();
    for (auto& explosion : Explosions)
    {
        explosion.Lifetime -= GetFrameTime();
        if (explosion.Lifetime > 0)
        {
            float param = explosion.Lifetime / Explosion::MaxLife;
            DrawSphere(explosion.Postion, 0.125f + ((1.0f - param) * 0.5f), ColorAlpha(ORANGE, explosion.Lifetime / Explosion::MaxLife));
        }
    }
    rlDrawRenderBatchActive();
    rlEnableDepthMask();

    while (!Explosions.empty() && Explosions.front().Lifetime < 0)
        Explosions.pop_front();
}

void Map::DrawWalls(Camera3D& view)
{
    if (!IsTextureReady(WallMaterial.maps[MATERIAL_MAP_ALBEDO].texture))
        return;

    float cameraPos[3] = { view.position.x, view.position.y, view.position.z };
    SetShaderValue(WallMaterial.shader, WallMaterial.shader.locs[SHADER_LOC_VECTOR_VIEW], cameraPos, SHADER_UNIFORM_VEC3);

    for (auto& wall : Walls)
    {
        wall.Transform.PushMatrix();
        DrawMesh(WallMesh, WallMaterial, MatrixScale(wall.Scale.x, wall.Scale.y, wall.Scale.z));

        wall.Transform.PopMatrix();
    }
}

void Map::AddExplosition(RayCollision& collision)
{
    Explosions.emplace_back();
    Explosions.back().Lifetime = Explosion::MaxLife;
    Explosions.back().Postion = collision.point;
    Explosions.back().Normal = collision.normal;
}

void Map::AddShotSound()
{
    CurrentShotSound++;
    if (CurrentShotSound >= MaxSounds)
        CurrentShotSound = 0;
    PlaySound(ShotLoop[CurrentShotSound]);
}
