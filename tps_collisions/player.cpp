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
#include "raymath.h"

Model GunMesh = { 0 };

void PlayerInfo::SetupGraphics()
{
    GunMesh = LoadModel("resources/blasterD.glb");
    for (int i = 0; i < GunMesh.materialCount; i++)
        GunMesh.materials[i].shader = Map::GetLightShader();
}

void PlayerInfo::CleanupGraphics()
{
    if (IsModelValid(GunMesh))
        UnloadModel(GunMesh);
}

void PlayerInfo::Setup()
{
    PlayerNode.SetPosition(0, 0, -5);
    
    PlayerNode.AddChild(PivotNode);
    PivotNode.MoveV(2);

    PivotNode.AddChild(CameraNode);
    CameraNode.MoveD(-10);

    PlayerNode.AddChild(ShoulderNode);

    // move the pivot of the gun down a little so it's not attached to the center of our head
    ShoulderNode.MoveV(1.85f);
	ShoulderNode.MoveD(0.85f);


    ShoulderNode.AddChild(GunNode);

    GunNode.SetPosition(PlayerInfo::GunDefaultH, PlayerInfo::GunDefaultV, PlayerInfo::GunDefaultD);

    ViewCamera.fovy = 45;
    ViewCamera.up.y = 1;
    ViewCamera.target.y = 2;
    ViewCamera.position.y = 2;
    ViewCamera.position.z = -5;

#ifndef _DEBUG
    DisableCursor();
#endif // _DEBUG
}

void PlayerInfo::Update(Map& map)
{
    DesiredMovement.x = DesiredMovement.y = DesiredMovement.z = 0;

#ifdef _DEBUG
    if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
#endif
    {
        constexpr float mouseSpeedScale = 0.5f;
        constexpr float maxViewAngle = 89.95f;
        constexpr float forwardSpeed = 30;
        constexpr float sideSpeed = 10;

        // rotate the player by the horizontal delta
        PlayerNode.RotateV(-GetMouseDelta().x * mouseSpeedScale);

        // rotate the camera node (head) by the tilt angle (clamped)
        TitltAngle += -GetMouseDelta().y * mouseSpeedScale;
        if (TitltAngle > maxViewAngle)
            TitltAngle = maxViewAngle;
        else if (TitltAngle < -maxViewAngle)
            TitltAngle = -maxViewAngle;

        PivotNode.SetOrientation(Vector3{ TitltAngle,0,0 });
        ShoulderNode.SetOrientation(Vector3{ TitltAngle,0,0 });

        // get the input movement
        Vector2 wadsVector = { 0 };

        if (IsKeyDown(KEY_W))
            wadsVector.y += 1;
        if (IsKeyDown(KEY_S))
            wadsVector.y -= 1;

        if (IsKeyDown(KEY_A))
            wadsVector.x += 1;
        if (IsKeyDown(KEY_D))
            wadsVector.x -= 1;

        // handle forward and sidestep motion
        DesiredMovement = Vector3Add(DesiredMovement, Vector3Scale(PlayerNode.GetDVector(), forwardSpeed * wadsVector.y * GetFrameTime()));
        DesiredMovement = Vector3Add(DesiredMovement, Vector3Scale(PlayerNode.GetHNegVector(), sideSpeed * wadsVector.x * GetFrameTime()));

		if (IsKeyPressed(KEY_LEFT))
			GunNode.MoveH(-0.1f);
		if (IsKeyPressed(KEY_RIGHT))
			GunNode.MoveH(0.1f);


		if (IsKeyPressed(KEY_UP))
			GunNode.MoveV(0.1f);
		if (IsKeyPressed(KEY_DOWN))
			GunNode.MoveV(-0.1f);


        DesiredPullback -= GetMouseWheelMove();
        if (DesiredPullback < 0)
            DesiredPullback = 0;
    }

    // if we are moving, bobble the gun a little
    if (Vector3LengthSqr(DesiredMovement) > 0)
    {
        BobbleTime += GetFrameTime();
        GunNode.SeteH(GunDefaultH + sinf(BobbleTime * 4.0f) * 0.05f);
        GunNode.SetV(GunDefaultV + cosf(BobbleTime * 6.0f) * 0.02f);
    }

    // try and move the player in the world and see where we end up after we hit all the things.
    Vector3 oldPos = PlayerNode.GetPosition();
    Vector3 newWorldPos = Vector3Add(oldPos, DesiredMovement);

    HitLastFrame = map.CollidePlayer(newWorldPos, oldPos, CollisionRadius, 2);

    // set the player to where they can be
    PlayerNode.SetPosition(newWorldPos);

    // see if the camera collides with anything
	Vector3 cameraRoot = PivotNode.GetWorldPosition();
    CameraNode.SetD(-DesiredPullback);

	Vector3 cameraPos = CameraNode.GetWorldPosition();

    Ray camRay = { 0 };
    camRay.position = cameraRoot;
    camRay.direction = Vector3Normalize(cameraPos - cameraRoot);

    LastCameraCollision = { 0 };

    map.CollideRay(camRay, LastCameraCollision, nullptr, 0.25f);

    if (LastCameraCollision.hit)
    {
		if (LastCameraCollision.distance < DesiredPullback)
		{
			CameraNode.SetD(-(LastCameraCollision.distance - 0.25f));
		}
    }

    // update the camera with the new view.
    CameraNode.SetCamera(ViewCamera);

    // raycast from the view into the world to see what it would hit
    Ray gunRay = { 0 };
    gunRay.position = Vector3Transform(Vector3Zero(), CameraNode.GetWorldMatrix());
    gunRay.direction = Vector3Subtract(Vector3Transform(Vector3{ 0,0,1 }, CameraNode.GetWorldMatrix()), gunRay.position);

    map.CollideRay(gunRay, LastGunCollision);   // optional, if you need to know what you hit, you can pass a pointer in here that will be set with the wall that is hit

    // handle shooting
    Reload -= GetFrameTime();  // decrement reload wait time

    // if we can shoot, and they want to shoot
    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) && Reload <= 0)
    {
        // make them wait for another shot
        Reload = ReloadTime;

        // make the gun sound
        map.AddShotSound();

        // if they hit something, then add an explosion there
        if (LastGunCollision.hit)
            map.AddExplosition(LastGunCollision);
    }

    // gun recoil 'animation'
    float param = 0;
    if (Reload > 0)
        param = Reload / ReloadTime;

    GunNode.SetD(param * RecoilDistance + GunDefaultD);
}

void PlayerInfo::Draw()
{
    PlayerNode.PushMatrix();
    float alpha = 1;

    float dist = fabsf(CameraNode.GetPosition().z);

    if (dist < 4)
    {
		alpha = dist / 4.0f;
    }

    DrawCylinder(Vector3Zeros, 0.25f, 0.30f, 1, 8, ColorAlpha(GREEN, alpha));
    DrawSphere(Vector3UnitY * 1.25f, 0.25f, ColorAlpha(HitCameraLastFrame ? RED : DARKGREEN, alpha));
    PlayerNode.PopMatrix();

    GunNode.PushMatrix();
    DrawModel(GunMesh, Vector3Zero(), 1, WHITE);

	rlBegin(RL_LINES);
	rlColor4f(1,0,0,1);
	rlVertex3f(Vector3Zeros.x, Vector3Zeros.y, Vector3Zeros.z);
    rlColor4f(1, 0, 0, 0);
	rlVertex3f(0,0, 3);
	rlEnd();

    GunNode.PopMatrix();
}
