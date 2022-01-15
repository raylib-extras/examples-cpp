/*******************************************************************************************
*
*   Sprite animation and movement example
*
*   Welcome to raylib!
*
*   To test examples, just press F6 and execute raylib_compile_execute script
*   Note that compiled executable is placed in the same folder as .c file
*
*   You can find all basic examples on C:\raylib\raylib\examples folder or
*   raylib official webpage: www.raylib.com
*
*   Enjoy using raylib. :)
*
*   This example has been created using raylib 4.0 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2022 Jeffery Myers
*
********************************************************************************************/
#pragma once

#include "raylib.h"
#include <vector>
#include <string>

struct SpriteSheet
{
    Texture2D SheetTexture = { 0 };
    std::vector<Rectangle> Frames;
};

struct SpriteAnimation
{
    std::string Name;
    int StartFrame = -1;
    int EndFrame = -1;
    float FPS = 15;
    bool Loops = true;
};

struct SpriteInstance
{
    Vector2 Position = { 0 };

    Vector2 Offset = { 0,0 };
    const SpriteSheet* Sheet = nullptr;
    const SpriteAnimation* Animation = nullptr;
    bool AnimationDone = false;
    int CurrentFrame = -1;
    float FrameLifetime = 0;
};

SpriteSheet LoadSpriteSheet(const char* file, int cols, int rows);
int AddFippedFrames(SpriteSheet& sheet, int start, int end, bool flipX, int flipY);

void DrawSprite(SpriteInstance& sprite);
void UpdateSpriteAnimation(SpriteInstance& sprite);
void SetSpriteAnimation(SpriteInstance& sprite, const SpriteAnimation& animation);
