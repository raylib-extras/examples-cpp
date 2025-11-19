/*
Raylib example file.
This is an example main file for a simple raylib project.
Use this as a starting point or replace it with your code.

by Jeffery Myers is marked with CC0 1.0. To view a copy of this license, visit https://creativecommons.org/publicdomain/zero/1.0/

*/

#include "raylib.h"
#include "rlgl.h"

#include <chrono>
#include <thread>
#include <mutex>
#include <atomic>
#include <vector>

#include "GLFW/glfw3.h"


std::mutex TextureLocker;
std::atomic<bool> LoadComplete;
std::atomic<bool> Abort;
GLFWwindow* LoaderWindow = nullptr;

std::vector<Texture2D> Textures;

int TextureCount = 40;

void LoadTextureInThread()
{
	if (!LoaderWindow)
		return;

	glfwMakeContextCurrent(LoaderWindow);
	for (int i = 0; i < TextureCount; i++)
	{
		if (Abort)
			break;

		// load perlin noise images and rescale them so they take some time
		Image perlin = GenImagePerlinNoise(1024, 1024, i*16, i*34, i/(TextureCount /3.0f));
		ImageResize(&perlin, 128, 128);

		{
			std::lock_guard<std::mutex> guard(TextureLocker);
			Textures.push_back(LoadTextureFromImage(perlin));
		}

		UnloadImage(perlin);
	}
	glfwDestroyWindow(LoaderWindow);

	LoaderWindow = nullptr;
	
	LoadComplete = true;
}

std::thread LoaderThread;

void StartLoad()
{
	Abort = false;
	LoadComplete = false;

	glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);     // Window initially hidden
	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);     // Window initially hidden
	LoaderWindow = glfwCreateWindow(1, 1, "TempWindow", NULL, glfwGetCurrentContext());
	glfwHideWindow(LoaderWindow);
	LoaderThread = std::thread(LoadTextureInThread);
}

void CleanupLoad()
{
	Abort = true;
	if (LoaderThread.joinable())
		LoaderThread.join();

	for (auto& texture : Textures)
	{
		UnloadTexture(texture);
	}

	Textures.clear();
}

int main ()
{
	// Create the window and OpenGL context
	InitWindow(1280, 800, "Hello Raylib");
	SetTargetFPS(60);
	StartLoad();


	float throberPos = 0;
	float throberDir = 1;

	// game loop
	while (!WindowShouldClose())		// run the loop untill the user presses ESCAPE or presses the Close button on the window
	{
		// drawing
		BeginDrawing();
		ClearBackground(DARKGRAY);

		{
			std::lock_guard<std::mutex> guard(TextureLocker);
			if (!LoadComplete)
			{
				// show a progress bar
				DrawText(TextFormat("Loading %d / %d", Textures.size() + 1, TextureCount), 10, 30, 20, WHITE);

				throberPos += GetFrameTime() * 800 * throberDir;
				if (throberPos > GetScreenWidth())
				{
					throberPos = GetScreenWidth();
					throberDir *= -1;
				}
				else if (throberPos < 0)
				{
					throberPos = 0;
					throberDir *= -1;
				}

				DrawCircle(throberPos, 100, 20, RED);
			}
			else
			{
				int x = 0;
				int y = 40;
				for (auto& tx : Textures)
				{
					DrawTexture(tx, x, y, WHITE);
					x += tx.width + 1;
					if (x > GetScreenWidth() - tx.width)
					{
						x = 0;
						y += tx.height + 1;
					}
				}
			}
		}

		DrawFPS(10, 10);
		// end the frame and get ready for the next one  (display frame, poll input, etc...)
		EndDrawing();
	}
	
	CleanupLoad();

	// destroy the window and cleanup the OpenGL context
	CloseWindow();
	return 0;
}
