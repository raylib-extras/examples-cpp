/**********************************************************************************************
*
*   raylib-extras, examples-cpp * examples for Raylib in C++
*
*   stencil_reflection * an example of using the stencil buffer for reflections
*
*   LICENSE: MIT
*
*   Copyright (c) 2021 Jeffery Myers
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
#include "rlgl.h"

#include "external/glad.h"

// stencil mask stuff


int main()
{
	SetConfigFlags(FLAG_VSYNC_HINT);
	InitWindow(1280, 800, "Reflections");
	SetTargetFPS(144);

	float spinAngle = 0;
	Camera3D camera = { 0 };

	camera.fovy = 60;
	camera.up.y = 1;
	camera.position.z = -5;
	camera.position.y = 2;

	while (!WindowShouldClose())
	{
		spinAngle += GetFrameTime() * 45;

		BeginDrawing();
		ClearBackground(GRAY);

		BeginMode3D(camera);

		rlPushMatrix();
		rlRotatef(spinAngle, 0, 1, 0);
		DrawCube(Vector3{ 0,0.5f,0 }, 1, 1, 1, DARKBLUE);
		rlDrawRenderBatchActive();

		glEnable(GL_STENCIL_TEST);

		glStencilFunc(GL_ALWAYS, 1, 0xFF); // Set any stencil to 1
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
		glStencilMask(0xFF); // Write to stencil buffer
		rlDisableDepthMask();
		glClear(GL_STENCIL_BUFFER_BIT); // Clear stencil buffer (0 by default)

		DrawPlane(Vector3{ 0, 0, 0 }, Vector2{ 3, 3 }, BLACK);
		rlDrawRenderBatchActive();


		glStencilFunc(GL_EQUAL, 1, 0xFF); // Pass test if stencil value is 1
		glStencilMask(0x00); // Don't write anything to stencil buffer
		rlEnableDepthMask();

		DrawCube(Vector3{ 0,-0.5f,0 }, 1, 1, 1, ColorAlpha(DARKBLUE,0.5f));
		rlDrawRenderBatchActive();

		glDisable(GL_STENCIL_TEST);

		rlPopMatrix();
		EndMode3D();

		EndDrawing();
	}

	CloseWindow();
	return 0;
}