/**********************************************************************************************
*
*   raylib-extras, examples-cpp * examples for Raylib in C++
*
*   Editable Terrain 2d * an example of using an image for editable terrain with collison
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

#include <vector>
#include <list>

Image Terrain = { 0 };
Texture TerrainTexture = { 0 };

struct Particle
{
	Vector2 Position = Vector2Zeros;
	Vector2 Direction = Vector2Zeros;
	float Lifetime = 0;
};
std::vector<Particle> Particles;


constexpr int TerrainScale = 8;

Color TerainColor = DARKGREEN;

Vector2 PlayerPos = Vector2Zeros;
float PlayerRadius = 20.0f;

Vector2 ParticleSize = { 5.0f, 5.0f };

float LastParticleTime = 0;

void GameInit()
{
	Terrain = GenImageColor(GetScreenWidth() / TerrainScale, GetScreenHeight() / TerrainScale, BLANK);

	constexpr size_t count = 100;
	Particles.reserve(count);

	ImageDrawRectangle(&Terrain, 0, Terrain.height / 2, Terrain.width, Terrain.height / 2, TerainColor);
	ImageDrawCircle(&Terrain, 100, 60, 30, TerainColor);

	TerrainTexture = LoadTextureFromImage(Terrain);
	SetTextureFilter(TerrainTexture, TEXTURE_FILTER_POINT);

	PlayerPos.x = 100;
	PlayerPos.y = 30;
}

bool IsTerrainSolid(const Vector2& min, const Vector2& max)
{
	Vector2 minPos = min / TerrainScale;
	Vector2 maxPos = max / TerrainScale;

	for (float y = minPos.y; y <= maxPos.y; y += 1)
	{
		if (y < 0 || y >= Terrain.height)
			continue;

		for (float x = minPos.x; x <= maxPos.x; x += 1)
		{
			if (x < 0 || x >= Terrain.width)
				continue;

			if (GetImageColor(Terrain, int(x), int(y)).a != 0)
				return true;
		}
	}

	return false;
}

void AddParticle(const Vector2 pos, const Vector2 dir)
{
	size_t i = 0;
	for (; i < Particles.size(); i++)
	{
		if (Particles[i].Lifetime <= 0)
			break;
	}

	if (i >= Particles.size())
	{
		Particles.push_back(Particle());
	}

	Particles[i].Position = pos;
	Particles[i].Direction = dir;
	Particles[i].Lifetime = 2.0f;
}

bool GameUpdate()
{
	Vector2 motion = Vector2Zeros;
	if (IsKeyDown(KEY_S))
		motion.y += 1;
	if (IsKeyDown(KEY_W))
		motion.y -= 1;
	if (IsKeyDown(KEY_A))
		motion.x -= 1;
	if (IsKeyDown(KEY_D))
		motion.x += 1;

	motion *= GetFrameTime() * 300;

	auto newPos = PlayerPos + motion;

	Vector2 halfBounds = { PlayerRadius, PlayerRadius };
	if (!IsTerrainSolid(newPos - halfBounds, newPos + halfBounds))
		PlayerPos = newPos;

	bool imageDirty = false;

	Vector2 player = PlayerPos / TerrainScale;
	Vector2 mouse = GetMousePosition() / TerrainScale;

	LastParticleTime -= GetFrameTime();

	if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
	{
		ImageDrawLine(&Terrain, int(player.x), int(player.y), int(mouse.x), int(mouse.y), BLANK);
		imageDirty = true;

		if (LastParticleTime <= 0)
		{
			LastParticleTime = 0.125f;
			AddParticle(PlayerPos - (Vector2UnitY * PlayerRadius * 2), Vector2{ float(GetRandomValue(-100,100)),float(GetRandomValue(-50,-100)) });
		}
	}

	if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
	{
		Vector2 dir = Vector2Subtract(mouse, player);
		if (Vector2Length(dir) > PlayerRadius)
		{
			dir = Vector2Normalize(dir);
			player += dir * (PlayerRadius);
			ImageDrawLine(&Terrain, int(player.x), int(player.y), int(mouse.x), int(mouse.y), TerainColor);
			imageDirty = true;
		}
	}

	if (imageDirty)
		UpdateTexture(TerrainTexture, Terrain.data);

	for (Particle& particle : Particles)
	{
		if (particle.Lifetime <= 0)
			continue;

		particle.Lifetime -= GetFrameTime();
		particle.Direction.y += 400.0f * GetFrameTime();

		auto newPos = particle.Position + particle.Direction * GetFrameTime();

		if (IsTerrainSolid(newPos - ParticleSize, newPos + ParticleSize))
		{
			particle.Direction *= -1.0f;
		}
		else
		{
			particle.Position = newPos;
		}
	}
	return true;
}

void GameDraw()
{
	BeginDrawing();
	ClearBackground(SKYBLUE);

	DrawTexturePro(TerrainTexture,
		Rectangle{ 0,0, float(TerrainTexture.width),float(TerrainTexture.height) },
		Rectangle{ 0,0, float(GetScreenWidth()), float(GetScreenHeight()) },
		Vector2Zeros,
		0,
		WHITE);

	DrawCircleV(PlayerPos, PlayerRadius, BLUE);

	DrawLineV(PlayerPos, GetMousePosition(), BLACK);

	for (Particle& particle : Particles)
	{
		if (particle.Lifetime <= 0)
			continue;
		DrawCircleV(particle.Position, 4, DARKBROWN);
	}
	DrawFPS(0, 0);
	EndDrawing();
}

int main()
{
	//	SetConfigFlags(FLAG_VSYNC_HINT);
	InitWindow(1280, 800, "Example");
	//SetTargetFPS(144);

	GameInit();

	while (!WindowShouldClose())
	{
		if (!GameUpdate())
			break;

		GameDraw();
	}

	CloseWindow();
	return 0;
}