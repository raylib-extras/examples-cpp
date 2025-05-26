/*
Simple skill tree using a camera2d

-- Copyright (c) 2020-2024 Jeffery Myers
--
--This software is provided "as-is", without any express or implied warranty. In no event 
--will the authors be held liable for any damages arising from the use of this software.

--Permission is granted to anyone to use this software for any purpose, including commercial 
--applications, and to alter it and redistribute it freely, subject to the following restrictions:

--  1. The origin of this software must not be misrepresented; you must not claim that you 
--  wrote the original software. If you use this software in a product, an acknowledgment 
--  in the product documentation would be appreciated but is not required.
--
--  2. Altered source versions must be plainly marked as such, and must not be misrepresented
--  as being the original software.
--
--  3. This notice may not be removed or altered from any source distribution.

*/

#include "raylib.h"
#include "raymath.h"

#include <vector>
#include <string>

Camera2D ViewCamera = { 0 };

struct SkillItem
{
	Vector2 Postion = Vector2Zeros;

	std::string Icon;
	std::string Name;
	std::string Description;

	Color Tint = DARKGRAY;

	int ParentIndex = -1;
	std::vector<int> ChildrenIndexes;
};

std::vector<SkillItem> Skills;

int SelectedSkill = -1;

Vector2 DesiredTarget = { -1,1 };
bool MovingToTarget = false;

void Init()
{
	ViewCamera.zoom = 1;
	ViewCamera.offset = Vector2{ GetScreenWidth() * 0.5f, GetScreenHeight() * 0.5f };

	// inner ring
	float angleDelta = 360.0f/16.0f;

	float innerRadius = 200;
	for (int i = 0; i < 16; i++)
	{
		float angle = i * angleDelta;
		Vector2 pos = Vector2{ cosf(angle * DEG2RAD), sinf(angle * DEG2RAD) };

		pos *= innerRadius;

		Skills.emplace_back();
		Skills.back().Postion = pos;
		Skills.back().Icon = TextFormat("%d", i);
		Skills.back().Name = "Base Skill " + Skills.back().Icon;
		Skills.back().Description = "A simple inner ring skill";
		Skills.back().Tint = DARKGREEN;
	}

	// outer ring
	float outerRadius = 500;

	angleDelta = 360.0f / 32;
	for (int i = 0; i < 32; i++)
	{
		float angle = (i * angleDelta) - (angleDelta/2);
		Vector2 pos = Vector2{ cosf(angle * DEG2RAD), sinf(angle * DEG2RAD) };

		int parent = i / 2;

		pos *= outerRadius;

		Skills.emplace_back();
		Skills.back().Postion = pos;
		Skills.back().Icon = TextFormat("%d", i + 16);
		Skills.back().Name = "Mid Skill " + Skills.back().Icon;
		Skills.back().Description = "A simple mid ring skill";
		Skills.back().Tint = DARKBLUE;

		Skills.back().ParentIndex = parent;

		Skills[parent].ChildrenIndexes.push_back(i + 16);
	}
}

void RecenterOffset()
{
	Vector2 offsetInWorldSpce = GetScreenToWorld2D(ViewCamera.offset, ViewCamera);
	Vector2 centerInWorldSpace = GetScreenToWorld2D(Vector2{ GetScreenWidth() * 0.5f, GetScreenHeight() * 0.5f }, ViewCamera);

	ViewCamera.target += centerInWorldSpace - offsetInWorldSpce;

	ViewCamera.offset = Vector2{ GetScreenWidth() * 0.5f, GetScreenHeight() * 0.5f };
}

void DrawTree()
{
	Vector2 mouseInWorld = GetScreenToWorld2D(GetMousePosition(), ViewCamera);
	for (int i = int(Skills.size())-1; i >= 0; i--)
	{
		auto& skill = Skills[i];
		if (skill.ParentIndex >= 0)
			DrawLineV(skill.Postion, Skills[skill.ParentIndex].Postion, WHITE);
		else
			DrawLineV(skill.Postion, Vector2Zeros, GRAY);

		if (CheckCollisionPointCircle(mouseInWorld, skill.Postion, 20))
		{
			DrawCircleV(skill.Postion, 28, YELLOW);

			if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
			{
				RecenterOffset();

				DesiredTarget = skill.Postion;
				MovingToTarget = true;
				SelectedSkill = i;
			}

			DrawText(skill.Description.c_str(), int(skill.Postion.x) - 50, int(skill.Postion.y) + 30, 10, SKYBLUE);
		}

// 		if (SelectedSkill == i)
// 			DrawCircleV(skill.Postion, 23, PURPLE);

		DrawCircleV(skill.Postion, 20, skill.Tint);
		DrawCircleLinesV(skill.Postion, 20, DARKGRAY);
		DrawTextEx(GetFontDefault(), skill.Icon.c_str(), skill.Postion - (Vector2Ones * 5), 20, 1, WHITE);
		DrawText(skill.Name.c_str(), int(skill.Postion.x) - 50, int(skill.Postion.y) - 45, 20, WHITE);
	}

// 	if (!MovingToTarget && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
// 		SelectedSkill = -1;
}

int main ()
{
	// Tell the window to use vsync and work on high DPI displays
	SetConfigFlags(FLAG_VSYNC_HINT);

	// Create the window and OpenGL context
	InitWindow(1280, 800, "Hello Skilltree");

	Init();
	
	// game loop
	while (!WindowShouldClose())		// run the loop until the user presses ESCAPE or presses the Close button on the window
	{
		// see if the user dragged with the right button
		if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
		{
			RecenterOffset();

			Vector2 worldMouseDrag = GetMouseDelta() / ViewCamera.zoom;
			ViewCamera.target -= worldMouseDrag;
			MovingToTarget = false;
		}

		if (GetMouseWheelMove() != 0)
		{
			MovingToTarget = false;

			// get the world point that is under the mouse
			Vector2 mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), ViewCamera);

			// set the offset to where the mouse is
			ViewCamera.offset = GetMousePosition();

			// set the target to match, so that the camera maps the world space point under the cursor to the screen space point under the cursor at any zoom
			ViewCamera.target = mouseWorldPos;

			// zoom on the mouse point since that's where our origin is
			ViewCamera.zoom += (GetMouseWheelMove() / fabsf(GetMouseWheelMove()))  * 0.1f;

			if (ViewCamera.zoom < 0.1f)
				ViewCamera.zoom = 0.1f;
		}

		// do we want to animate to some location?
		if (MovingToTarget)
		{
			float maxMove = 800 * GetFrameTime();

			Vector2 vecToTarget = DesiredTarget - ViewCamera.target;
			float len = Vector2Length(vecToTarget);

			if (len < maxMove)
			{
				ViewCamera.target = DesiredTarget;
				MovingToTarget = false;
			}
			else
			{
				vecToTarget /= len;
				vecToTarget *= maxMove;
				ViewCamera.target += vecToTarget;
			}
		}

		// drawing
		BeginDrawing();

		// Setup the back buffer for drawing (clear color and depth buffers)
		ClearBackground(BLACK);

		BeginMode2D(ViewCamera);
		DrawTree();
		DrawCircle(0, 0, 10, RED);
		EndMode2D();

		if (SelectedSkill >= 0)
		{
			DrawText(Skills[SelectedSkill].Name.c_str(), 5, 5, 20, YELLOW);
			DrawText(Skills[SelectedSkill].Description.c_str(), 5, 25, 20, YELLOW);
		}

		DrawFPS(0, GetScreenHeight() - 20);
		
		// end the frame and get ready for the next one  (display frame, poll input, etc...)
		EndDrawing();
	}

	// destroy the window and cleanup the OpenGL context
	CloseWindow();
	return 0;
}
