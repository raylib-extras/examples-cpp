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

#include "hud.h"
#include "raylib.h"

namespace HUD
{
    static RenderTexture MiniMapRenderTexture = { 0 };

    constexpr int MiniMapSize = 250;

    void SetupGraphics()
    {
        if (!IsRenderTextureValid(MiniMapRenderTexture))
            MiniMapRenderTexture = LoadRenderTexture(MiniMapSize, MiniMapSize);
    }

    void CleanupGraphics()
    {
        if (IsRenderTextureValid(MiniMapRenderTexture))
           UnloadRenderTexture(MiniMapRenderTexture);

        MiniMapRenderTexture.id = 0;
    }

    void UpdateMiniMapTexture(Map& map, PlayerInfo& player)
    {
        Vector3 playerPos = player.PivotNode.GetWorldPosition();
        Vector3 cameraPos = player.CameraNode.GetWorldPosition();

        Camera3D camera = { 0 };
        camera.up.z = 1;
        camera.position = playerPos;
        camera.position.y = 100;
        camera.target = playerPos;
        camera.target.y = 0;
        camera.fovy = 30;
        camera.projection = CAMERA_ORTHOGRAPHIC;

        BeginTextureMode(MiniMapRenderTexture);
        ClearBackground(ColorAlpha(BLACK, 0.25f));
        BeginMode3D(camera);

        map.DrawWalls(camera);

        DrawCube(Vector3Zero(), 1, 1, 1, RED);

        player.CameraNode.PushMatrix();
        float camSize = 0.5f;
        DrawSphere(Vector3Zeros, camSize, PURPLE);
        player.CameraNode.PopMatrix();

		if (player.LastCameraCollision.hit)
		{
			DrawSphere(player.LastCameraCollision.point, 1.0f, MAGENTA);
		}

        EndMode3D();

        Vector2 center = Vector2{ MiniMapRenderTexture.texture.width * 0.5f, MiniMapRenderTexture.texture.height * 0.5f };

        DrawCircleV(center, 3, DARKBLUE);
        Vector2 viewVec = Vector2Normalize(Vector2{ player.ViewCamera.target.x - player.ViewCamera.position.x,player.ViewCamera.target.z - player.ViewCamera.position.z });
        viewVec = Vector2Scale(viewVec, -30);
        DrawLineV(center, Vector2Add(center, viewVec), SKYBLUE);

        EndTextureMode();
    }

    void Draw(Map& map, PlayerInfo& player)
    {
        UpdateMiniMapTexture(map, player);

        DrawTextureRec(MiniMapRenderTexture.texture, Rectangle{ 0,0, float(MiniMapRenderTexture.texture.width), -float(MiniMapRenderTexture.texture.height) }, Vector2{ float(GetScreenWidth() - MiniMapRenderTexture.texture.width), 0 }, WHITE);

        DrawCircle(GetScreenWidth() / 2, GetScreenHeight() / 2, 2, ColorAlpha(WHITE, 0.75f));
        DrawFPS(10, 10);

        float size = ceilf(GetWindowScaleDPI().x);
		int fontSize = 10 * int(size);

        DrawText(TextFormat("WADS to move, left click to shoot, wheel to zoom"), 100, 10, fontSize, BLACK);
#ifdef _DEBUG
        DrawText("Hold Right Mouse to Rotate View", 100, 10 + fontSize, fontSize, BLACK);
#endif // _DEBUG
    }
}
