/**********************************************************************************************
*
*   raylib-extras, examples-cpp * examples for Raylib in C++
*
*   LICENSE: MIT
*
*   Copyright (c) 2023 Jeffery Myers
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

#include "raylib.h"
#include "raymath.h"

#include "object_transform.h"

#include <vector>
#include <list>

ObjectTransform PlayerBase;
ObjectTransform CameraNode;
ObjectTransform GunNode;

Model GunModel = { 0 };

void GameInit()
{
	// build up a Hierarchy of transform nodes in a parent child relationship
	// 
	// Camera
	//   |\
	//   | \Gun
	//	 |	  	
	//   + Base
	// 
	// player base is where the players 'feet' are
	// its child is the camera node at an offset for the 'head'
	// the gun is a child of the camera so it moves with it

	PlayerBase.AddChild(CameraNode);
	CameraNode.MoveV(1);
	CameraNode.AddChild(GunNode);

	GunNode.MoveV(-0.5f);
	GunNode.MoveH(-0.5f);

	// load a gun model
	GunModel = LoadModel("resources/blasterD.glb");
}

bool GameUpdate()
{
	//rotate the player base left and right
	if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
		PlayerBase.RotateV(GetMouseDelta().x * -0.5f);

	// rotate the camera up and down
	if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
		CameraNode.RotateH(GetMouseDelta().y * -0.5f);

	// move the player based on it's local transform
	if (IsKeyDown(KEY_W))
		PlayerBase.MoveD(GetFrameTime() * 10);

    if (IsKeyDown(KEY_S))
        PlayerBase.MoveD(GetFrameTime() * -10);

    if (IsKeyDown(KEY_A))
        PlayerBase.MoveH(GetFrameTime() * 10);

    if (IsKeyDown(KEY_D))
        PlayerBase.MoveH(GetFrameTime() * -10);

	return true;
}

void GameDraw()
{
	BeginDrawing();
	ClearBackground(DARKGRAY);

	// the camera is not our controller, so we can always recreate it from the camera node
	Camera3D camera = { 0 };
	camera.projection = CAMERA_PERSPECTIVE;
	camera.fovy = 45;
	camera.up.y = 1;

	// where the camera node is in the world
	camera.position = CameraNode.GetWorldPosition();

	// where the depth vector of the camera node is in world space
	camera.target = Vector3Transform({ 0,0,1 }, CameraNode.GetWorldMatrix());

	BeginMode3D(camera);

	DrawGrid(10, 10);
	DrawCube(Vector3{ 0,0.5f,10 }, 1, 1, 1, RED);

	// push the OpenGL matrix for the gun node in world space
	// so we can draw the gun there
	GunNode.PushMatrix();
	// we are now drawing local to the gun node, this model needs to stick out a little, so we shift it by 1.5 in z
	DrawModel(GunModel, Vector3{0,0,1.5f}, 1, WHITE);
	GunNode.PopMatrix();

	EndMode3D();
	EndDrawing();
}

int main()
{
	SetConfigFlags(FLAG_VSYNC_HINT);
	InitWindow(1280, 800, "Transform Example");
	SetTargetFPS(144);

	GameInit();

	while (!WindowShouldClose())
	{
		if (!GameUpdate())
			break;
		
		GameDraw();
	}

	UnloadModel(GunModel);

	CloseWindow();
	return 0;
}