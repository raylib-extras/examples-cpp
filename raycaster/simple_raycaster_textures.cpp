/*
*   Raylib software Raycaster
*   Based on algorithms from
*   https://lodev.org/cgtutor/raycasting.html
*
*   LICENSE: zlib/libpng
*
*   raylib-extras are licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software:
*
*   Copyright (c) 2022 Jeffery Myers (jeffm)
*
*   This software is provided "as-is", without any express or implied warranty. In no event
*   will the authors be held liable for any damages arising from the use of this software.
*
*   Permission is granted to anyone to use this software for any purpose, including commercial
*   applications, and to alter it and redistribute it freely, subject to the following restrictions:
*
*     1. The origin of this software must not be misrepresented; you must not claim that you
*     wrote the original software. If you use this software in a product, an acknowledgment
*     in the product documentation would be appreciated but is not required.
*
*     2. Altered source versions must be plainly marked as such, and must not be misrepresented
*     as being the original software.
*
*     3. This notice may not be removed or altered from any source distribution.
*/

#include "raylib.h"
#include "raymath.h"

#include <stdint.h>

// the grid of map data
uint8_t MapData[] = {   4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 7, 7, 7, 7, 7, 7, 7, 7,
						4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 0 ,0, 0, 0, 0, 0, 7,
						4, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7,
						4, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7,
						4, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 7,
						4, 0, 4, 0, 0, 0, 0, 5, 5, 5, 5, 5, 5, 5, 5, 5, 7, 7, 0, 7, 7, 7, 7, 7,
						4, 0, 5, 0, 0, 0, 0, 5, 0, 5, 0, 5, 0, 5, 0, 5, 7, 0, 0, 0, 7, 7, 7, 1,
						4, 0, 6, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 5, 7, 0, 0, 0, 0, 0, 0, 8,
						4, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 7, 7, 1,
						4, 0, 8, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 5, 7, 0, 0, 0, 0, 0, 0, 8,
						4, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 5, 7, 0, 0, 0, 7, 7, 7, 1,
						4, 0, 0, 0, 0, 0, 0, 5, 5, 5, 5, 0, 5, 5, 5, 5, 7, 7, 7, 7, 7, 7, 7, 1,
						6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 0, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
						8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4,
						6, 6, 6, 6, 6, 6, 0, 6, 6, 6, 6, 0, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
						4, 4, 4, 4, 4, 4, 0, 4, 4, 4, 6, 0, 6, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3,
						4, 0, 0, 0, 0, 0, 0, 0, 0, 4, 6, 0, 6, 2, 0, 0, 0, 0, 0, 2, 0, 0, 0, 2,
						4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 2, 0, 0, 5, 0, 0, 2, 0, 0, 0, 2,
						4, 0, 0, 0, 0, 0, 0, 0, 0, 4, 6, 0, 6, 2, 0, 0, 0, 0, 0, 2, 2, 0, 2, 2,
						4, 0, 6, 0, 6, 0, 0, 0, 0, 4, 6, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 2,
						4, 0, 0, 5, 0, 0, 0, 0, 0, 4, 6, 0, 6, 2, 0, 0, 0, 0, 0, 2, 2, 0, 2, 2,
						4, 0, 6, 0, 6, 0, 0, 0, 0, 4, 6, 0, 6, 2, 0, 0, 5, 0, 0, 2, 0, 0, 0, 2,
						4, 0, 0, 0, 0, 0, 0, 0, 0, 4, 6, 0, 6, 2, 0, 0, 0, 0, 0, 2, 0, 0, 0, 2,
						4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 1, 1, 1, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3 };

// how big the map is world space
constexpr uint8_t MapWidth = 24;
constexpr uint8_t MapHeight = 24;

// how big each map grid is in pixels for the top view
constexpr uint8_t MapPixelSize = 16;

RenderTexture MapRenderTexture;	// render texture for the top view
RenderTexture ViewRenderTexture; // render texture for the 3d view

Texture2D WallTexture = { 0 };

// 3d view size
constexpr uint16_t ViewWidth = 256;
constexpr uint16_t ViewHeight = 196;

constexpr float ViewFOV = 90 * DEG2RAD;

Vector2 PlayerPos = { 4.5f,  2.5f };
Vector2 PlayerFacing = { 1, 0 };
Vector2 CameraPlane = { 0, -0.66f };	// the 2d equivalent of a camera plane, rotates with the player

// flag to control if textures are used
bool DrawFlatShaded = false;

// used to know what side of a grid was hit
enum class HitNormals : uint8_t
{
	North = 0,
	South,
	East,
	West
};

// a ray that has been cast, with cached info
struct RayResult
{
	// the ray's direction
	Vector2 Directon = { 0 };

	// the distance to the cell hit
	float Distance = 0;

	// the side of the grid that was hit
	HitNormals Normal;

	// what kind of grid cell was hit
	uint8_t HitGridType = 0;
};

// the rays that make up the view. This is a fixed size array based on the render view's width (one for each pixel in X)
RayResult RaySet[ViewWidth] = { 0 };


// get the grid for map coordinate
uint8_t GetMapGrid(uint8_t x, uint8_t y)
{
	if (x >= MapWidth || y >= MapHeight)
		return 0;

	return MapData[y * MapWidth + x];
}

uint8_t GetMapGrid(const Vector2& pos)
{
	return GetMapGrid(uint8_t(pos.x), uint8_t(pos.y));
}

// cast a ray and find out what it hits
void CastRay(RayResult& ray)
{
	ray.Distance = -1;

	// The current grid point we are in
	int mapX = int(floor(PlayerPos.x));
	int mapY = int(floor(PlayerPos.y));

	//length of ray from current position to next x or y-side
	float sideDistX = 0;
	float sideDistY = 0;

	// length of ray from one x or y-side to next x or y-side
	// these are derived as:
	// deltaDistX = sqrt(1 + (rayDirY * rayDirY) / (rayDirX * rayDirX))
	// deltaDistY = sqrt(1 + (rayDirX * rayDirX) / (rayDirY * rayDirY))
	// which can be simplified to abs(|rayDir| / rayDirX) and abs(|rayDir| / rayDirY)
	// where |rayDir| is the length of the vector (rayDirX, rayDirY). Its length,
	// unlike (dirX, dirY) is not 1, however this does not matter, only the
	// ratio between deltaDistX and deltaDistY matters, due to the way the DDA
	// stepping further below works. So the values can be computed as below.
	// Division through zero is prevented, even though technically that's not
	// needed in C++ with IEEE 754 floating point values.
	float deltaDistX = (ray.Directon.x == 0) ? float(1e30) : float(fabs(1.0f / ray.Directon.x));
	float deltaDistY = (ray.Directon.y == 0) ? float(1e30) : float(fabs(1.0f / ray.Directon.y));

	float perpWallDist = 0;

	// what direction to step in x or y-direction (either +1 or -1)
	int stepX = 0;
	int stepY = 0;

	bool hit = false; //was there a wall hit?
	bool side = false; //was a NS or a EW wall hit?

	// calculate step and initial sideDist
	if (ray.Directon.x < 0)
	{
		stepX = -1;
		sideDistX = (PlayerPos.x - mapX) * deltaDistX;
	}
	else
	{
		stepX = 1;
		sideDistX = (mapX + 1.0f - PlayerPos.x) * deltaDistX;
	}

	if (ray.Directon.y < 0)
	{
		stepY = -1;
		sideDistY = (PlayerPos.y - mapY) * deltaDistY;
	}
	else
	{
		stepY = 1;
		sideDistY = (mapY + 1.0f - PlayerPos.y) * deltaDistY;
	}

	// perform DDA Digital Differential Analyzer to walk the line
	while (!hit)
	{
		//jump to next map square, either in x-direction, or in y-direction
		if (sideDistX < sideDistY)
		{
			sideDistX += deltaDistX;
			mapX += stepX;
			side = false;
		}
		else
		{
			sideDistY += deltaDistY;
			mapY += stepY;
			side = true;
		}

		if (mapX >= MapWidth || mapX < 0 || mapY >= MapHeight || mapY < 0)
			break;

		ray.HitGridType = GetMapGrid(mapX, mapY);

		//Check if ray has hit a wall
		if (ray.HitGridType != 0)
			hit = true;
	}

	if (!hit)
	{
		ray.Distance = -1;
		return;
	}


	// Calculate distance projected on camera direction. This is the shortest distance from the point where the wall is
	// hit to the camera plane. Euclidean to center camera point would give fisheye effect!
	// This can be computed as (mapX - posX + (1 - stepX) / 2) / rayDirX for side == 0, or same formula with Y
	// for size == 1, but can be simplified to the code below thanks to how sideDist and deltaDist are computed:
	// because they were left scaled to |rayDir|. sideDist is the entire length of the ray above after the multiple
	// steps, but we subtract deltaDist once because one step more into the wall was taken above.
	if (!side)
	{
		perpWallDist = (sideDistX - deltaDistX);
		ray.Normal = stepX < 0 ? HitNormals::East : HitNormals::West;
	}
	else
	{
		perpWallDist = (sideDistY - deltaDistY);
		ray.Normal = stepY < 0 ? HitNormals::North : HitNormals::South;
	}

	ray.Distance = perpWallDist;
}

// compute the rays for the current view
void UpdateRayset()
{
	for (uint16_t i = 0; i < ViewWidth; i++)
	{
		RayResult& ray = RaySet[i];

		float cameraX = 2 * i / (float)ViewWidth - 1; //x-coordinate in camera space
		ray.Directon.x = PlayerFacing.x + CameraPlane.x * cameraX;
		ray.Directon.y = PlayerFacing.y + CameraPlane.y * cameraX;

		CastRay(ray);
	}
}

// draw the rays in the top view
void DrawRayset(const Vector2& playerPos, float scale)
{
	for (uint16_t i = 0; i < ViewWidth; i++)
	{
		RayResult& ray = RaySet[i];

		if (ray.Distance >= 0)
			DrawLineV(playerPos, Vector2Add(playerPos, Vector2Scale(ray.Directon, ray.Distance * scale)), ColorAlpha(GREEN, 0.5f));
	}
}

void DrawMapTopView()
{
	BeginTextureMode(MapRenderTexture);
	ClearBackground(DARKGRAY);

	// fill the map with cells
	for (uint8_t y = 0; y < MapHeight; y++)
	{
		for (uint8_t x = 0; x < MapWidth; x++)
		{
			if (GetMapGrid(x, y) != 0)
			{
				DrawRectangle(x * MapPixelSize, y * MapPixelSize, MapPixelSize, MapPixelSize, WHITE);
			}
			DrawRectangleLines(x * MapPixelSize, y * MapPixelSize, MapPixelSize, MapPixelSize, BLACK);
		}
	}

	Vector2 playerPixelSpace = Vector2Scale(PlayerPos, MapPixelSize);

	// draw rays
	DrawRayset(playerPixelSpace, MapPixelSize);

	// draw player
	DrawCircleV(playerPixelSpace, MapPixelSize * 0.25f, BLUE);

	// draw forward vector
	Vector2 forwardPixelSpace = Vector2Scale(PlayerFacing, MapPixelSize);

	DrawLineV(playerPixelSpace, Vector2Add(playerPixelSpace, forwardPixelSpace), SKYBLUE);

	// draw axis markers
	DrawLine(MapPixelSize / 4, MapPixelSize / 4, MapPixelSize, MapPixelSize / 4, RED);
	DrawLine(MapPixelSize / 4, MapPixelSize / 4, MapPixelSize / 4, MapPixelSize, GREEN);

	EndTextureMode();
}

// figure out how far from the edge of a cell the current ray is
float GetRayU(const RayResult& ray)
{
	Vector2 target = Vector2Add(PlayerPos, Vector2Scale(ray.Directon, ray.Distance));

	switch (ray.Normal)
	{
	case HitNormals::South:
		return  target.x - floorf(target.x);

	case HitNormals::North:
		return  1.0f - (target.x - floorf(target.x));

	case HitNormals::East:
		return  target.y - floorf(target.y);

	case HitNormals::West:
		return  1.0f - (target.y - floorf(target.y));
	}

	return 0;
}

// draw the 3d view
void DrawView()
{
	BeginTextureMode(ViewRenderTexture);

	// fill the texture with the ceiling color
	ClearBackground(DARKGRAY);

	// the middle of the screen
	int middle = ViewRenderTexture.texture.height / 2;

	// fill half the screen with the ground color
	DrawRectangle(0, middle, ViewRenderTexture.texture.width, middle, DARKBROWN);

	// an array of colors to use to tint each wall a different color based on direction
	static Color wallColors[4] = { WHITE, Color{128,128,128,255}, Color{196,196,196,255} , Color{200,200,200,255} };

	// for each ray in our rayset
	for (uint16_t i = 0; i < ViewWidth; i++)
	{
		const RayResult& ray = RaySet[i];

		if (ray.Distance < 0)
			continue;

		// use the distance to compute how high the wall will be
		int lineHeight = (int)(ViewRenderTexture.texture.height / ray.Distance);

		// get our tint based on what side of a grid the ray hit
		Color tint = wallColors[uint8_t(ray.Normal)];

		if (DrawFlatShaded || WallTexture.id == 0)
		{
			// draw a line from the center for this X column 
			DrawLine(i, middle - lineHeight / 2, i, middle + lineHeight / 2, tint);
		}
		else
		{
			// get the U coordinate of this ray (where on the texture it hits in x)
			float u = GetRayU(ray);

			// find the start of the texture for this grid type
			float uStart = WallTexture.height * (ray.HitGridType - 1.0f);

			// compute a source rect for a single strip of texture we want to draw for this pixel
			Rectangle sourceRect{ uStart + (u * WallTexture.height), 0, 0, float(WallTexture.height) };

			// compute where on the screen this column is going to be drawn
			Rectangle destRect{ float(i), middle - lineHeight / 2.0f, 1.0f,  float(lineHeight) };

			DrawTexturePro(WallTexture, sourceRect, destRect, Vector2Zero(), 0, tint);
		}
	}

	EndTextureMode();
}

// move the player around the map
void UpdateMovement()
{
	// speeds, based on time
	float rotationSpeed = 180.0f * DEG2RAD * GetFrameTime();
	float movementSpeed = 5.0f * GetFrameTime();

	// compute a rotation for this frame
	float rotation = 0;

	if (IsKeyDown(KEY_A))
		rotation += rotationSpeed;

	if (IsKeyDown(KEY_D))
		rotation -= rotationSpeed;

	// rotate the player and the camera plane
	PlayerFacing = Vector2Rotate(PlayerFacing, rotation);
	CameraPlane = Vector2Rotate(CameraPlane, rotation);

	// compute a new position based on movement
	Vector2 newPos = PlayerPos;

	// the vector that is to the left
	Vector2 sideStepVector = { -PlayerFacing.y, PlayerFacing.x };

	// move the new pos based on keys
	if (IsKeyDown(KEY_W))
		newPos = Vector2Add(newPos, Vector2Scale(PlayerFacing, movementSpeed));

	if (IsKeyDown(KEY_S))
		newPos = Vector2Add(newPos, Vector2Scale(PlayerFacing, -movementSpeed));

	if (IsKeyDown(KEY_Q))
		newPos = Vector2Add(newPos, Vector2Scale(sideStepVector, movementSpeed));

	if (IsKeyDown(KEY_E))
		newPos = Vector2Add(newPos, Vector2Scale(sideStepVector, -movementSpeed));

	// if the new pos is not inside the world, allow the player to move there
	if (GetMapGrid(newPos) == 0)
		PlayerPos = newPos;
}

int main()
{
	// set up the window
	SetConfigFlags(FLAG_VSYNC_HINT);
	InitWindow(1280, 800, "Raycaster Example");
	SetTargetFPS(144);

	// load render textures for the top view and 3d view
	MapRenderTexture = LoadRenderTexture(MapWidth * MapPixelSize, MapHeight * MapPixelSize);
	ViewRenderTexture = LoadRenderTexture(ViewWidth, ViewHeight);

	// textures for our walls
	WallTexture = LoadTexture("resources/textures.png");
	GenTextureMipmaps(&WallTexture);
	SetTextureFilter(WallTexture, TEXTURE_FILTER_TRILINEAR);

	// game loop
	while (!WindowShouldClose())
	{
		// let the user toggle flat shaded or not
		if (IsKeyPressed(KEY_SPACE))
			DrawFlatShaded = !DrawFlatShaded;

		// move the player
		UpdateMovement();

		// compute the rays for the current view
		// this is where the raycasting happens
		UpdateRayset();

		// update the top view render texture
		DrawMapTopView();

		// draw the 3d view to it's low res render texture
		DrawView();

		// Draw the results to the screen
		BeginDrawing();
		ClearBackground(BLACK);

		// draw the top view texture to the screen.
		// Note that this render texture is NOT flipped in Y, so that the view has Y be up not down
		DrawTexture(MapRenderTexture.texture, 0, GetScreenHeight() / 2 - MapRenderTexture.texture.height / 2, WHITE);

		// figure out how much is left for the 3d view
		Rectangle viewArea = { float(MapRenderTexture.texture.width), 0, float(GetScreenWidth() - MapRenderTexture.texture.width), float(GetScreenHeight()) };

		// fill the view area
		DrawRectangleRec(viewArea, BLACK);

		// figure how many times we can scale the 3d view to fit, but keep it an whole factor
		float scaleFactor = 1;

		scaleFactor = floorf(viewArea.width / ViewWidth);

		float renderHeight = ViewHeight * scaleFactor;
		float renderWidth = ViewWidth * scaleFactor;

		// center the texture in the view area
		Rectangle destRect = { viewArea.x + viewArea.width / 2 - renderWidth / 2, viewArea.height / 2 - renderHeight / 2, renderWidth,renderHeight };

		DrawTexturePro(ViewRenderTexture.texture, Rectangle{ 0,0,ViewWidth, -ViewHeight }, destRect, Vector2Zero(), 0, WHITE);

		// text overlay
		DrawFPS(2, 0);
		DrawText(TextFormat("Player X%2.1f, X%2.1f", PlayerPos.x, PlayerPos.y), 2, 20, 20, WHITE);
		DrawText("Space to toggle shaded vs textures", 2, 40, 20, WHITE);

		EndDrawing();
	}

	// cleanup
	UnloadTexture(WallTexture);
	UnloadRenderTexture(MapRenderTexture);
	UnloadRenderTexture(ViewRenderTexture);

	CloseWindow();
	return 0;
}