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

#include "player.h"
#include "map.h"

Model GunMesh = { 0 };

void PlayerInfo::SetupGraphics()
{
    GunMesh = LoadModel("resources/blasterD.glb");
    for (int i = 0; i < GunMesh.materialCount; i++)
        GunMesh.materials[i].shader = Map::GetLightShader();
}

void PlayerInfo::CleanupGraphics()
{
    if (IsModelReady(GunMesh))
        UnloadModel(GunMesh);
}

void PlayerInfo::Setup()
{
    PlayerNode.SetPosition(0, 0, -5);
    PlayerNode.AddChild(CameraNode);
    CameraNode.MoveV(2);

    CameraNode.AddChild(GunNode);

    GunNode.SetPosition(PlayerInfo::GunDefaultH, PlayerInfo::GunDefaultV, PlayerInfo::GunDefaultD);

    ViewCamera.fovy = 45;
    ViewCamera.up.y = 1;
    ViewCamera.target.y = 2;
    ViewCamera.position.y = 2;
    ViewCamera.position.z = -5;
}

void PlayerInfo::Update(Map& map)
{
    DesiredMovement.x = DesiredMovement.y = DesiredMovement.z = 0;

    if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
    {
        constexpr float mouseSpeedScale = 0.5f;
        constexpr float maxViewAngle = 89.95f;
        constexpr float forwardSpeed = 30;
        constexpr float sideSpeed = 10;

        PlayerNode.RotateV(-GetMouseDelta().x * mouseSpeedScale);

        TitltAngle += -GetMouseDelta().y * mouseSpeedScale;
        if (TitltAngle > maxViewAngle)
            TitltAngle = maxViewAngle;
        else if (TitltAngle < -maxViewAngle)
            TitltAngle = -maxViewAngle;

        CameraNode.SetOrientation(Vector3{ TitltAngle,0,0 });

        Vector2 wadsVector = { 0 };

        if (IsKeyDown(KEY_W))
            wadsVector.y += 1;
        if (IsKeyDown(KEY_S))
            wadsVector.y -= 1;

        if (IsKeyDown(KEY_A))
            wadsVector.x += 1;
        if (IsKeyDown(KEY_D))
            wadsVector.x -= 1;

        DesiredMovement = Vector3Add(DesiredMovement, Vector3Scale(PlayerNode.GetDVector(), forwardSpeed * wadsVector.y * GetFrameTime()));
        DesiredMovement = Vector3Add(DesiredMovement, Vector3Scale(PlayerNode.GetHNegVector(), sideSpeed * wadsVector.x * GetFrameTime()));
    }

    if (Vector3LengthSqr(DesiredMovement) > 0)
    {
        BobbleTime += GetFrameTime();
        GunNode.SeteH(GunDefaultH + sinf(BobbleTime * 4.0f) * 0.05f);
        GunNode.SetV(GunDefaultV + cosf(BobbleTime * 6.0f) * 0.02f);
    }

    Vector3 oldPos = PlayerNode.GetPosition();
    Vector3 newWorldPos = Vector3Add(oldPos, DesiredMovement);

    HitLastFrame = map.CollidePlayer(newWorldPos, oldPos, CollisionRadius, 2);

    PlayerNode.SetPosition(newWorldPos);

    CameraNode.SetCamera(ViewCamera);

    // see what the ray out of the gun would hit

    Ray gunRay = { 0 };
    gunRay.position = Vector3Transform(Vector3Zero(), GunNode.GetWorldMatrix());
    gunRay.direction = Vector3Subtract(Vector3Transform(Vector3{ 0,0,1 }, GunNode.GetWorldMatrix()), gunRay.position);

    RayCollision thisCollision = { 0 };

    LastGunCollision.hit = false;
    LastGunCollision.distance = std::numeric_limits<float>::max();

    map.CollideRay(gunRay, LastGunCollision);

    Reload -= GetFrameTime();

    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) && Reload <= 0)
    {
        Reload = ReloadTime;

        map.AddShotSound();

        if (LastGunCollision.hit)
            map.AddExplosition(LastGunCollision);
    }

    float param = 0;
    if (Reload > 0)
        param = Reload / ReloadTime;

    GunNode.SetD(param * RecoilDistance + GunDefaultD);
}

void PlayerInfo::Draw()
{
    GunNode.PushMatrix();

    DrawModel(GunMesh, Vector3{ 0, 0, 0 }, 1, WHITE);

    DrawLine3D(Vector3Zero(), Vector3{ 0,0,100 }, RED);
    GunNode.PopMatrix();
}
