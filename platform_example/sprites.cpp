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

#include "sprites.h"

SpriteSheet LoadSpriteSheet(const char* file, int cols, int rows)
{
	SpriteSheet sheet = { 0 };
	sheet.SheetTexture = LoadTexture(file);
	if (sheet.SheetTexture.id >= 0)
	{
		float w = (float)sheet.SheetTexture.width / cols;
		float h = (float)sheet.SheetTexture.height / rows;

		for (int y = 0; y < rows; y++)
		{
			for (int x = 0; x < cols; x++)
			{
				sheet.Frames.emplace_back(Rectangle{ x * w,y * h,w,h });
			}
		}
	}

	return std::move(sheet);
}

int AddFippedFrames(SpriteSheet& sheet, int start, int end, bool flipX, int flipY)
{
	int delta = end - start;
	int offset = delta / abs(delta);


	int newStart = (int)sheet.Frames.size();
	for (int i = start; i != end + offset; i += offset)
	{
		if (i >= 0 || i < (int)sheet.Frames.size())
		{
			Rectangle rect = sheet.Frames[i];
			if (flipX)
				rect.width *= -1;
			if (flipY)
				rect.height *= -1;

			sheet.Frames.emplace_back(std::move(rect));
		}
	}

	return newStart;
}

void DrawSprite(SpriteInstance& sprite)
{
	if (sprite.Sheet == nullptr || sprite.Sheet->SheetTexture.id == 0 || sprite.Animation == nullptr)
		return;

	Rectangle frame = sprite.Sheet->Frames[sprite.CurrentFrame];
	DrawTexturePro(sprite.Sheet->SheetTexture, frame, Rectangle{ sprite.Position.x,sprite.Position.y,fabsf(frame.width),fabsf(frame.height)}, sprite.Offset, 0, WHITE);
}

void UpdateSpriteAnimation(SpriteInstance& sprite)
{
	if (sprite.Animation == nullptr)
		return;

	sprite.FrameLifetime += GetFrameTime();
	if (sprite.FrameLifetime > 1.0f / sprite.Animation->FPS)
	{
		sprite.FrameLifetime = 0;
		sprite.CurrentFrame++;
		sprite.AnimationDone = false;
		if (sprite.CurrentFrame > sprite.Animation->EndFrame)
		{
			if (sprite.Animation->Loops)
				sprite.CurrentFrame = sprite.Animation->StartFrame;
			else
			{
				sprite.AnimationDone = true;
				sprite.CurrentFrame--;
			}
		}
	}
}

void SetSpriteAnimation(SpriteInstance& sprite, const SpriteAnimation& animation)
{
	sprite.Animation = &animation;
	sprite.CurrentFrame = animation.StartFrame;
	sprite.FrameLifetime = 0;
	sprite.AnimationDone = false;
}
