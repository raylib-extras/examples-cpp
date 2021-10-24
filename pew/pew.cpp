/**********************************************************************************************
*
*   raylib-extras, examples-cpp * examples for Raylib in C++
*
*   pew * an example of movement and shots
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
#include "raymath.h"

#include <cmath>
#include <list>
#include <string>
#include <vector>

constexpr int TargetSize = 20;
constexpr float RefireTime = 0.25f;

void SetRandomTarget(Vector2& pos)
{
	pos.x = (float)GetRandomValue(TargetSize, GetScreenWidth() - TargetSize);
	pos.y = (float)GetRandomValue(TargetSize, GetScreenHeight() - TargetSize);
}

void KeepPosInBounds(Vector2& pos, float size, Vector2* velocity = nullptr)
{
	if (pos.x < size)
	{
		if (velocity)
			velocity->x *= -1;
		pos.x = size;
	}

	if (pos.y < size)
	{
		if (velocity)
			velocity->y *= -1;
		pos.y = size;
	}

	if (pos.x > GetScreenWidth() - size)
	{
		if (velocity)
			velocity->x *= -1;
		pos.x = GetScreenWidth() - size;
	}

	if (pos.y > GetScreenHeight() - size)
	{
		if (velocity)
			velocity->y *= -1;
		pos.y = GetScreenHeight() - size;
	}
}

class Target
{
public:
	Vector2 Pos = { 0 };
	Vector2 Velocity = { 0 };
	float Speed = 50;
	float Lifetime = 0;
	float Size = 1;

	Target()
	{
		Reset();
	}

	inline void Reset()
	{
		SetRandomTarget(Pos);
		Velocity.x = GetRandomValue(-1000, 1000) / 1000.0f;
		Velocity.y = GetRandomValue(-1000, 1000) / 1000.0f;
		Velocity = Vector2Normalize(Velocity);

		Lifetime = 0;
		Size = 1;
		Speed = (float)GetRandomValue(50, 150);
	}

	inline void Update()
	{
		Lifetime += GetFrameTime();

		if (Lifetime < 1)
			Size = 1 + (Lifetime / 1.5f) * (TargetSize - 1);
		else
			Size = TargetSize;

		Pos = Vector2Add(Pos, Vector2Scale(Velocity, Speed * GetFrameTime()));

		KeepPosInBounds(Pos, Size, &Velocity);
	}

	bool Hit(Vector2& shot, float size)
	{
		if (CheckCollisionCircles(Pos, Size, shot, size))
		{
			Reset();
			return true;
		}

		return false;
	}

	inline void Draw()
	{
		DrawCircle((int)Pos.x, (int)Pos.y, Size, RED);
		DrawCircle((int)Pos.x, (int)Pos.y, Size * 0.75f, WHITE);
		DrawCircle((int)Pos.x, (int)Pos.y, Size * 0.5f, RED);
		DrawCircle((int)Pos.x, (int)Pos.y, Size * 0.25f, WHITE);
	}
};

class Bullet
{
public:
	Vector2 Pos = { 0 };
	Vector2 Velocity = { 0 };
	float Speed = 750;
	float Lifetime = 0;
	const int Size = 2;

public:
	Bullet(Vector2& playerPos, Vector2& velocityVector)
	{
		Velocity = velocityVector;

		// if the shot isn't moving, kill it
		if (Vector2LengthSqr(Velocity) == 0)
		{
			Lifetime = -1;
			return;
		}

		// put it a bit outside of our player pos
		Pos = Vector2Add(playerPos, Vector2Scale(Velocity, 10));
		Lifetime = 2;

		// compute an angle for our shot drawing
		ShotAngle = atan2f(Velocity.y, Velocity.x) * RAD2DEG;
	}

	inline bool IsDead()
	{
		return Lifetime <= 0;
	}

	inline void Update()
	{
		// see if we die
		Lifetime -= GetFrameTime();
		if (Lifetime <= 0)
			return;

		Pos = Vector2Add(Pos, Vector2Scale(Velocity, Speed * GetFrameTime()));

		// see if we went out of bounds, if so, we die
		if (Pos.x < -Size || Pos.y < -Size || Pos.x > GetScreenWidth() + Size || Pos.y > GetScreenHeight() + Size)
		{
			Lifetime = -1;
			return;
		}
	}

	inline void Draw()
	{
		Rectangle rect = { Pos.x, Pos.y, Size * 6.0f, Size * 2.0f };
		Vector2 center = { Size * 3.0f, (float)Size };
		DrawRectanglePro(rect, center, ShotAngle, RED);
	}

protected:
	float ShotAngle = 0;
};

void main()
{
	SetConfigFlags(FLAG_VSYNC_HINT);
	InitWindow(1280, 800, "Shooter Sample");
	SetTargetFPS(144);

	Vector2 PlayerPos = { 600, 400 };
	float PlayerAngle = -90;
	float AimAngle = 0;

	Rectangle screenRect = Rectangle{ 0, 0 ,(float)GetScreenWidth(), (float)GetScreenHeight() };

	bool wantFire = false;
	float reloadTime = -RefireTime;

	std::list<Bullet> bullets;

	std::vector<Target> targets;

	for (int i =0; i < 10; i++)
	{ 
		targets.emplace_back(Target());
	}

	int score = 0;
	int lives = 3;

	bool gameOn = true;

	while (!WindowShouldClose())
	{
		// save the vector to the mouse
		Vector2 aimVector = Vector2Normalize(Vector2Subtract(GetMousePosition(), PlayerPos));
		Vector2 movementVector = Vector2{ cosf(PlayerAngle * DEG2RAD), sinf(PlayerAngle * DEG2RAD) };

		if (gameOn)
		{
			// move player
			float moveSpeed = 200 * GetFrameTime();
			float turnSpeed = 180 * GetFrameTime();

			if (IsKeyDown(KEY_A))
				PlayerAngle -= turnSpeed;
			if (IsKeyDown(KEY_D))
				PlayerAngle += turnSpeed;

			movementVector = Vector2{ cosf(PlayerAngle * DEG2RAD), sinf(PlayerAngle * DEG2RAD) };

			if (IsKeyDown(KEY_W))
				PlayerPos = Vector2Add(PlayerPos, Vector2Scale(movementVector, moveSpeed));
			if (IsKeyDown(KEY_S))
				PlayerPos = Vector2Add(PlayerPos, Vector2Scale(movementVector, -moveSpeed));

			// keep player in bounds
			KeepPosInBounds(PlayerPos, 30);

			// see where we are aiming
			aimVector = Vector2Normalize(Vector2Subtract(GetMousePosition(), PlayerPos));
			if (Vector2LengthSqr(aimVector) > 0) // don't update the angle if the mouse is on the player
				AimAngle = atan2f(aimVector.y, aimVector.x) * RAD2DEG;

			// set the vector to match the angle (in case it was invalid)
			aimVector.x = cosf(AimAngle * DEG2RAD);
			aimVector.y = sinf(AimAngle * DEG2RAD);
		}
		else
		{
			if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
			{
				// restart the game
				gameOn = true;
				score = 0;
				lives = 3;
				SetRandomTarget(PlayerPos);

				// get us back down to 10 targets
				while (targets.size() > 10)
					targets.erase(targets.begin());
			}
		}

		// move targets
		for (auto& target : targets)
		{
			target.Update();

			// see if we hit the target, if so, we die
			if (target.Hit(PlayerPos, 30))
			{
				lives--;
				SetRandomTarget(PlayerPos);
				if (lives <= 0)
				{
					gameOn = false;
				}
			}
		}

		if (gameOn)
		{
			// make a shot
			reloadTime -= GetFrameTime();
			if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) && reloadTime <= 0)
			{
				wantFire = false;
				reloadTime = RefireTime;
				bullets.emplace_back(Bullet(PlayerPos, aimVector));
			}
		}

		// update shots and cull any dead ones
		for (std::list<Bullet>::iterator itr = bullets.begin(); itr != bullets.end();)
		{
			itr->Update();
			if (itr->IsDead() || !gameOn)
			{
				itr = bullets.erase(itr);
			}
			else
			{
				for (auto& target : targets)
				{
					if (target.Hit(itr->Pos, itr->Size*2.0f))
					{
						itr->Lifetime = 0;
						score++;

						if (score % 10 == 0)
							targets.emplace_back(Target());
					}
				}
				itr++;
			}
		}

		BeginDrawing();
		ClearBackground(DARKGREEN);

		for (auto& target : targets)
			target.Draw();

		for (auto& bullet : bullets)
			bullet.Draw();

		// draw player
		if (gameOn)
		{
			DrawRectanglePro(Rectangle{ PlayerPos.x, PlayerPos.y,40,30 }, Vector2{ 20,15 }, PlayerAngle, DARKGRAY);

			DrawCircle((int)(PlayerPos.x + movementVector.x * 15), (int)(PlayerPos.y + movementVector.y * 15), 7, GRAY);

			DrawCircle((int)PlayerPos.x, (int)PlayerPos.y, 10, BLUE);

			Rectangle aimRectangle = { PlayerPos.x, PlayerPos.y, 26, 4 };
			DrawRectanglePro(aimRectangle, Vector2{ -5, 2 }, AimAngle, DARKBLUE);

			DrawText(TextFormat("Score %d Lives %d", score, lives), 0, 0, 20, RAYWHITE);
		}
		else
		{
			std::string message = "Game Over, Click to Restart";
			int textSize = MeasureText(message.c_str(), 20);
			DrawText(message.c_str(), GetScreenWidth()/2 - textSize/2, 400, 20, RAYWHITE);
		}

		EndDrawing();
	}

	CloseWindow();
}