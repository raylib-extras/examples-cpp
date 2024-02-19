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
        if (!IsRenderTextureReady(MiniMapRenderTexture))
            MiniMapRenderTexture = LoadRenderTexture(MiniMapSize, MiniMapSize);
    }

    void CleanupGraphics()
    {
        if (IsRenderTextureReady(MiniMapRenderTexture))
           UnloadRenderTexture(MiniMapRenderTexture);

        MiniMapRenderTexture.id = 0;
    }

    void UpdateMiniMapTexture(Map& map, PlayerInfo& player)
    {
        Camera3D camera = { 0 };
        camera.up.z = 1;
        camera.position = player.ViewCamera.position;
        camera.position.y = 100;
        camera.target = player.ViewCamera.position;
        camera.target.y = 0;
        camera.fovy = 30;
        camera.projection = CAMERA_ORTHOGRAPHIC;

        BeginTextureMode(MiniMapRenderTexture);
        ClearBackground(ColorAlpha(BLACK, 0.25f));
        BeginMode3D(camera);

        map.DrawWalls(camera);

        DrawCube(Vector3Zero(), 1, 1, 1, RED);

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

        DrawFPS(10, 10);

        DrawText("WADS to move, left click to shoot", 100, 10, 10, BLACK);
#ifdef _DEBUG
        DrawText("Hold Right Mouse to Rotate View", 100, 20, 10, BLACK);
#endif // _DEBUG
    }
}
