/**********************************************************************************************
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

#include "raylib.h"

#include "collisions.h"
#include "hud.h"
#include "map.h"
#include "object_transform.h"
#include "player.h"

// global player
PlayerInfo Player;

void GameInit(Map &map)
{
    InitAudioDevice();

    Map::SetupGraphics();
    PlayerInfo::SetupGraphics();
    HUD::SetupGraphics();

    BuildDemoMap(map);

    map.Setup();
    Player.Setup();
}

void GameCleanup(Map& map)
{
    map.Cleanup();
    Map::CleanupGraphics();
    PlayerInfo::CleanupGraphics();
    HUD::CleanupGraphics();
}

bool GameUpdate(Map& map)
{
    Player.Update(map);

    return true;
}

void Draw3D(Map& map)
{
    BeginMode3D(Player.ViewCamera);

    map.Draw(Player.ViewCamera);
    Player.Draw();

    EndMode3D();
}

void GameDraw(Map& map)
{
    BeginDrawing();
    ClearBackground(SKYBLUE);

    Draw3D(map);

    HUD::Draw(map, Player);

    EndDrawing();
}

int main()
{
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE);
    InitWindow(1280, 800, "FPS Collisions");
    SetTargetFPS(144);

    Map map;

    GameInit(map);

    while (!WindowShouldClose())
    {
        if (!GameUpdate(map))
            break;

        GameDraw(map);
    }

    GameCleanup(map);
    CloseWindow();
    return 0;
}
