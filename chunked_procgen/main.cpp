/*
Raylib example file.
This is an example main file for a simple raylib project.
Use this as a starting point or replace it with your code.

by Jeffery Myers is marked with CC0 1.0. To view a copy of this license, visit https://creativecommons.org/publicdomain/zero/1.0/

*/

#include "raylib.h"
#include "raymath.h"

#include "resource_dir.h"	// utility header for SearchAndSetResourceDir

#include "RenderArea.h"

#include "external/stb_perlin.h"

int ScaleToDPI(int x)
{
	return int(x * GetWindowScaleDPI().x);
}

float ScaleToDPI(float x)
{
	return float(x * GetWindowScaleDPI().x);
}

Vector2 ScaleToDPI(const Vector2& x)
{
	return x * GetWindowScaleDPI();
}

std::vector<ChunkRenderInfo> ChunkTextureCache;
std::vector<ChunkRenderInfo*> AvailableChunkTextures;

Vector2 PlayerPos = { 0,0 };

ChunkOrigin CurrentChunk(0, 0);

int main()
{
	// Tell the window to use vsync and work on high DPI displays
	SetConfigFlags(FLAG_VSYNC_HINT);

	// Create the window and OpenGL context
	InitWindow(1280, 800, "Hello Chunks");

	// Utility function from resource_dir.h to find the resources folder and set it as the current working directory so we can load from it
	SearchAndSetResourceDir("resources");

	// Load a texture from the resources directory
	Texture wabbit = LoadTexture("wabbit_alpha.png");

	constexpr int renderDistance = 3;

	RenderAreaManager renderArea(renderDistance);

    for (auto& loop : renderArea.Area.Loops)
    {
        for (auto& [chunkOrigin, chunkInfo] : loop.Chunks)
        {
			auto& cachedChunkTexture = ChunkTextureCache.emplace_back();
			cachedChunkTexture.BaseLayer = LoadRenderTexture(256, 256);
			SetTextureWrap(cachedChunkTexture.BaseLayer.texture, TEXTURE_WRAP_CLAMP);
        }
    }

    for (auto& chunkInfo : ChunkTextureCache)
    {
		AvailableChunkTextures.push_back(&chunkInfo);
    }

	bool forceGenerate = true;
	constexpr float chunkSize = 256.0f;

	constexpr float chunkTileCount = 16;

	Camera2D camera = { 0 };
	camera.zoom = 1.0f;
	camera.offset = { float(GetScreenWidth() / 2), float(GetScreenHeight() / 2) };

	Color ringColors[6] = { RED, DARKGRAY, DARKBLUE, ORANGE, DARKPURPLE, DARKBROWN };

	// game loop
	while (!WindowShouldClose())		// run the loop until the user presses ESCAPE or presses the Close button on the window
	{
		float speed = 200.0f * GetFrameTime();
		if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT))
			speed *= 10;

		if (IsKeyDown(KEY_D))
            PlayerPos.x += speed;
        if (IsKeyDown(KEY_A))
            PlayerPos.x -= speed;

        if (IsKeyDown(KEY_S))
            PlayerPos.y += speed;
        if (IsKeyDown(KEY_W))
            PlayerPos.y -= speed;

        camera.zoom += (GetMouseWheelMove() * 0.1f);
		if (camera.zoom < 0.1f)
            camera.zoom = 0.1f;

        float halfChunkSize = chunkSize * 0.5f;

		if (PlayerPos.x > halfChunkSize)
		{
			renderArea.MoveOrigin(1, 0);
			PlayerPos.x -= chunkSize;
			CurrentChunk.X += 1;
        }
        else if (PlayerPos.x < -halfChunkSize)
        {
            renderArea.MoveOrigin(-1, 0);
            PlayerPos.x += chunkSize;
			CurrentChunk.X += -1;
        }

        if (PlayerPos.y > halfChunkSize)
        {
            renderArea.MoveOrigin(0, 1);
            PlayerPos.y -= chunkSize;
			CurrentChunk.Y += 1;
        }
        else if (PlayerPos.y < -halfChunkSize)
        {
            renderArea.MoveOrigin(0, -1);
            PlayerPos.y += chunkSize;
			CurrentChunk.Y += -1;
        }

		if (!renderArea.UndefinedChunks.empty())
		{
			forceGenerate = false;

            for (auto dead : renderArea.DeadChunks)
            {
				AvailableChunkTextures.push_back(dead->RenderInfo);
                delete dead;
            }
            renderArea.DeadChunks.clear();

			for (const auto& [relative, global] : renderArea.UndefinedChunks)
			{
                auto* chunk = new Chunk(global);
				renderArea.Area.SetChunk(relative.X,relative.Y, chunk);

				for (int y = 0; y < 16; y++)
				{
                    for (int x = 0; x < 16; x++)
                    {
						float noiseX = global.X + (1.0f/16 * x);
                        float noiseY = global.Y + (1.0f / 16 * y);

						float value = (stb_perlin_fbm_noise3(noiseX, noiseY, 1.0f, 2.0f, 0.5f, 6) + 1) * 0.49f;

						int index = int(std::floor(value * 3));

						chunk->Tiles[y * 16 + x] = index;
                    }
				}
               
                chunk->RenderInfo = AvailableChunkTextures.back();
                AvailableChunkTextures.pop_back();

				BeginTextureMode(chunk->RenderInfo->BaseLayer);
				ClearBackground(MAGENTA);
                for (int y = 0; y < 16; y++)
                {
                    for (int x = 0; x < 16; x++)
                    {
						int index = chunk->Tiles[y * 16 + x];

						Color color = BLANK;
						switch (index)
						{
						default:
							color = PURPLE;
							break;
                        case 0:
							color = BLUE;
							break;

                        case 1:
                            color = DARKBROWN;
                            break;

                        case 2:
                            color = DARKGREEN;
                            break;
						}
						DrawRectangle(x * 16, y * 16, 16, 16, color);
                    }
                }
				EndTextureMode();
			}
			renderArea.UndefinedChunks.clear();
		}
		// drawing
		BeginDrawing();

		// Setup the back buffer for drawing (clear color and depth buffers)
		ClearBackground(BLACK);
		
		camera.target = PlayerPos;

		BeginMode2D(camera);
		int loopIndex = 0;
        for (auto& loop : renderArea.Area.Loops)
        {
            for (const auto& [relativeOrigin, chunk] : loop.Chunks)
            {
				Vector2 pos = { 0,0 };
				pos.x = relativeOrigin.X * chunkSize;
				pos.y = relativeOrigin.Y * chunkSize;

                Rectangle rec = { pos.x-chunkSize*0.5f, pos.y - chunkSize * 0.5f, chunkSize, chunkSize };

				if (chunk != nullptr)
				{
					DrawTexturePro(chunk->RenderInfo->BaseLayer.texture, Rectangle{ 0,0,256,-256 }, rec, Vector2Zeros, 0, WHITE);
				}
				else
				{
					DrawRectangleRec(rec, ringColors[loopIndex]);
				}
		
                DrawRectangleLinesEx(rec, 3.0f, ringColors[loopIndex]);

                DrawText(TextFormat("R(%i,%i)", relativeOrigin.X, relativeOrigin.Y), int(pos.x) - 20, int(pos.y) - 10, 10, WHITE);

				if (chunk != nullptr)
				{
                    DrawText(TextFormat("O[%i,%i]", chunk->Origin.X, chunk->Origin.Y), int(pos.x) - 20, int(pos.y + 20) - 10, 10, SKYBLUE);
				}

                DrawTexturePro(wabbit, 
					Rectangle{ 0,0,float(wabbit.width),float(wabbit.height) },
					Rectangle{ PlayerPos.x, PlayerPos.y, float(wabbit.width), float(wabbit.height) },
					Vector2{ wabbit.width *0.5f, wabbit.height *0.5f},
					0, 
					WHITE);

            }
			loopIndex++;
        }
		EndMode2D();

		DrawFPS(10, 10);

		DrawRectangle(10, 30, 300, 80, ColorAlpha(WHITE, 0.5f));
		DrawText(TextFormat("Current Chunk[%i,%i]", CurrentChunk.X, CurrentChunk.Y), 20, 40, 20, BLACK);

        double realX = double(CurrentChunk.X) * chunkSize + PlayerPos.x;
        double realY = double(CurrentChunk.Y) * chunkSize + PlayerPos.y;
        DrawText(TextFormat("Real Pos[%0.2lf,%0.2lf]", realX, realY), 20, 60, 20, BLACK);

		DrawText(TextFormat("Local Pos[%0.2f,%0.2f]", PlayerPos.x, PlayerPos.x), 20, 80, 20, BLACK);
		EndDrawing();
	}

	// cleanup
	// unload our texture so it can be cleaned up
	UnloadTexture(wabbit);

	// destroy the window and cleanup the OpenGL context
	CloseWindow();
	return 0;
}
