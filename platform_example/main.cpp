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

#include "raylib.h"
#include "raymath.h"
#include "sprites.h"

#include <unordered_map>
#include <vector>

bool IsDebuggerPresent();

std::vector<Rectangle> Obstacles;
NPatchInfo ObstacleNpatch = { 0 };
Texture2D ObstacleTexture = { 0 };

float GetDeltaTime()
{
    //return 1 / 60.0f;
    return GetFrameTime();
}

enum class ActorStates
{
    Idle,
    Run,
    JumpStart,
    JumpUp,
    FallDown,
};

struct Actor 
{
    ActorStates State = ActorStates::Idle;
    bool FacingRight = true;

    SpriteInstance Sprite;
    std::unordered_map<ActorStates, std::vector<SpriteAnimation>> AnimationStates;

    float Width = 0;
    float Height = 0;

    float RunSpeed = 200;
    float jumpAcceleration = -350;
    float jumpVelocityDampen = 1.125f;
    Vector2 Velocity = { 0,0 };
};
Actor Warrior;

void LoadObstacles()
{
    ObstacleTexture = LoadTexture("resources/panel_blue.png");
    ObstacleNpatch.source = Rectangle{ 0,0, (float)ObstacleTexture.width, (float)ObstacleTexture.height };
    ObstacleNpatch.top = 20;
    ObstacleNpatch.bottom = 20;
    ObstacleNpatch.right = 20;
    ObstacleNpatch.left = 20;

    Obstacles.emplace_back(Rectangle{ 200,-100, 200, 50 });
    Obstacles.emplace_back(Rectangle{ 200,-200, 200, 25 });

    Obstacles.emplace_back(Rectangle{ 600,-50, 200, 50 });
    Obstacles.emplace_back(Rectangle{ 900,-150, 200, 50 });
}


int main(void)
{
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "sprite animation");

    // setup the world
    Camera2D cam = { 0 };
    cam.zoom = 1;
    cam.offset.x = 20;
    cam.offset.y = GetScreenHeight() - 50;
    cam.target.x = 0;
    cam.target.y = 0;

    float gravity = 500.0f;

    LoadObstacles();

    // load our sprite sheet
    SpriteSheet warriorSheet = LoadSpriteSheet("resources/Warrior_Sheet-Effect.png", 6, 17);

    // create left facing versions of all the sprites
    int leftFrameStart = AddFippedFrames(warriorSheet, 0, (int)warriorSheet.Frames.size() - 1, true, false);

    // create our warrior
    Warrior.Sprite = SpriteInstance{ Vector2{0, 0}, Vector2{warriorSheet.Frames[0].width / 2,warriorSheet.Frames[0].height }, &warriorSheet };

    Warrior.Width = warriorSheet.Frames[0].width;
    Warrior.Height = warriorSheet.Frames[0].height;

    // define an animation for each state
    Warrior.AnimationStates[ActorStates::Idle].emplace_back(SpriteAnimation{ "right_idle", 0, 5, 5 });
    Warrior.AnimationStates[ActorStates::Idle].emplace_back(SpriteAnimation{ "left_idle", leftFrameStart + 0, leftFrameStart + 5, 5 });

    Warrior.AnimationStates[ActorStates::Run].emplace_back(SpriteAnimation{ "right_run", 6, 13 });
    Warrior.AnimationStates[ActorStates::Run].emplace_back(SpriteAnimation{ "left_run", leftFrameStart + 6, leftFrameStart + 13 });

    Warrior.AnimationStates[ActorStates::JumpStart].emplace_back(SpriteAnimation{ "right_jump", 41, 43, 15  ,false});
    Warrior.AnimationStates[ActorStates::JumpStart].emplace_back(SpriteAnimation{ "left_jump", leftFrameStart +41, leftFrameStart + 43, 15  ,false });

    Warrior.AnimationStates[ActorStates::JumpUp].emplace_back(SpriteAnimation{ "right_fly", 43, 43, 10 });
    Warrior.AnimationStates[ActorStates::JumpUp].emplace_back(SpriteAnimation{ "left_fly", leftFrameStart + 43, leftFrameStart + 43, 10 });

    Warrior.AnimationStates[ActorStates::FallDown].emplace_back(SpriteAnimation{ "right_fall", 46, 48, 10 });
    Warrior.AnimationStates[ActorStates::FallDown].emplace_back(SpriteAnimation{ "left_fall", leftFrameStart + 46, leftFrameStart + 48, 10 });

    SetSpriteAnimation(Warrior.Sprite, Warrior.AnimationStates[ActorStates::Idle][0]);

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second

    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // get the input
        int runDir = 0;
        if (IsKeyDown(KEY_D))
            runDir += 1;
        if (IsKeyDown(KEY_A))
            runDir += -1;

        bool wantJump = IsKeyDown(KEY_SPACE);

        if (IsKeyPressed(KEY_Z))
        {
            if (cam.zoom == 1)
                cam.zoom = 1.5f;
            else
                cam.zoom = 1;
        }

        // apply input and change any states as needed
        switch (Warrior.State)
        {
        case ActorStates::Idle:
            if (wantJump)
            {
                if (IsKeyDown(KEY_S))
                {
                    Warrior.State = ActorStates::FallDown;
                    Warrior.Sprite.Position.y += gravity * GetDeltaTime();
                }
                else
                {
                    Warrior.State = ActorStates::JumpStart;
                    Warrior.Velocity.y = Warrior.jumpAcceleration;
                    Warrior.Velocity.x = 0;
                }
            }
            else if (runDir != 0)
            {
                Warrior.State = ActorStates::Run;
                Warrior.FacingRight = runDir > 0;
            }
            else
            {
                Warrior.Velocity.x = 0;
                Warrior.Velocity.y = 0;
            }
            break;

        case ActorStates::Run:
            if (wantJump)
            {
                Warrior.State = ActorStates::JumpStart;
                Warrior.Velocity.y = Warrior.jumpAcceleration;
                Warrior.Velocity.x *= Warrior.jumpVelocityDampen;
            }
            else if (runDir == 0)
            {
                Warrior.State = ActorStates::Idle;
            }
            else
            {
                Warrior.FacingRight = runDir > 0;
                Warrior.Velocity.x = runDir * Warrior.RunSpeed;
                Warrior.Velocity.y = 0;
            }

            break;

        case ActorStates::JumpStart:
            if (Warrior.Velocity.y >= 0)
                Warrior.State = ActorStates::FallDown;
            else if (Warrior.Sprite.AnimationDone)
                Warrior.State = ActorStates::JumpUp;
            break;

        case ActorStates::JumpUp:
            if (Warrior.Velocity.y >= 0)
                Warrior.State = ActorStates::FallDown;
            break;

        case ActorStates::FallDown:
            break;
        }

        // if we can fall, make us fall
        Warrior.Velocity.y += gravity * GetDeltaTime();

        // move us how we want to move

        Vector2 oldPos = Warrior.Sprite.Position;
        Vector2 newPos = Vector2Add(Warrior.Sprite.Position, Vector2Scale(Warrior.Velocity, GetDeltaTime()));
        float halfWidth = Warrior.Width / 2;
     
        if (newPos.y > 0)
        {
            newPos.y = 0;
            Warrior.Velocity.y = 0;
            if (Warrior.State == ActorStates::FallDown)
                Warrior.State = ActorStates::Idle;
        }

        for (const Rectangle& obstacle : Obstacles)
        {
            Rectangle warriorRect = { newPos.x - Warrior.Width / 2, newPos.y - Warrior.Height, Warrior.Width, Warrior.Height };


            if (CheckCollisionRecs(warriorRect, obstacle))
            {
                float playerLeft = warriorRect.x;
                float playerRight = warriorRect.x + warriorRect.width;
                float playerTop = warriorRect.y;

                float obstacleLeft = obstacle.x;
                float obstacleRight = obstacle.x + obstacle.width;
                float obstacleBottom = obstacle.y + obstacle.height;

                switch (Warrior.State)
                {
                case ActorStates::FallDown:
                    // only two things can happen here
                    // 1) We landed on something
                    if (playerRight >= obstacleLeft && playerLeft <= obstacleRight && oldPos.y <= obstacle.y && newPos.y >= obstacle.y)
                    {
                        Warrior.Velocity.y = 0;
                        Warrior.Velocity.x = 0;
                        newPos.y = obstacle.y;
                        Warrior.State = ActorStates::Idle;
                    } // 2) we slapped the side of something
                    else if (Warrior.Velocity.x < 0 && playerRight >= obstacleLeft)
                    {
                        Warrior.Velocity.x = 0;
                        newPos.x = obstacleLeft - halfWidth;
                    }
                    else if (Warrior.Velocity.x > 0 && playerLeft <= obstacleRight)
                    {
                        Warrior.Velocity.x = 0;
                        newPos.x = obstacleRight + halfWidth;
                    }
                    break;

                case ActorStates::Idle:
                    if (playerRight >= obstacleLeft && playerLeft <= obstacleRight && oldPos.y <= obstacle.y && newPos.y >= obstacle.y)
                    {
                        Warrior.Velocity.y = 0;
                        Warrior.Velocity.x = 0;
                        newPos.y = obstacle.y;
                        Warrior.State = ActorStates::Idle;
                    }
                    break;

                case ActorStates::Run:
                    if (playerRight >= obstacleLeft && playerLeft <= obstacleRight && oldPos.y <= obstacle.y && newPos.y >= obstacle.y)
                    {
                        Warrior.Velocity.y = 0;
                        Warrior.Velocity.x = 0;
                        newPos.y = obstacle.y;
                    }
                    else if (Warrior.Velocity.x > 0 && playerRight >= obstacleLeft)
                    {
                        Warrior.Velocity.x = 0;
                        newPos.x = obstacleLeft - halfWidth;
                    }
                    else if (Warrior.Velocity.x < 0 && playerLeft <= obstacleRight)
                    {
                        Warrior.Velocity.x = 0;
                        newPos.x = obstacleRight + halfWidth;
                    }
                    break;

                case ActorStates::JumpStart:
                case ActorStates::JumpUp:
   
					if (playerRight >= obstacleLeft && playerLeft <= obstacleRight && oldPos.y + Warrior.Height >= obstacleBottom && playerTop <= obstacleBottom)
					{
                        Warrior.Velocity.x = 0;
						Warrior.Velocity.y = 0;
						newPos.y = oldPos.y;
						Warrior.State = ActorStates::FallDown;
					}
                    else if (Warrior.Velocity.x > 0 && playerRight >= obstacleLeft)
					{
						Warrior.Velocity.x = 0;
                        Warrior.Velocity.y = 0;
						newPos.x = obstacleLeft - halfWidth;
						Warrior.State = ActorStates::FallDown;
					}
					else if (Warrior.Velocity.x < 0 && playerLeft <= obstacleRight)
					{
						Warrior.Velocity.x = 0;
                        Warrior.Velocity.y = 0;
						newPos.x = obstacleRight + halfWidth;
						Warrior.State = ActorStates::FallDown;
					}
                    break;
                }
            }
        }

        // are we not falling, but now we are falling?
        if (Warrior.State != ActorStates::FallDown && Warrior.Velocity.y > 0)
        {
            Warrior.State = ActorStates::FallDown;
        }

        Warrior.Sprite.Position = newPos;
        // if we are not on the animation we want to be on, set us to that animation
        if (Warrior.Sprite.Animation != &(Warrior.AnimationStates[Warrior.State][Warrior.FacingRight ? 0 : 1]))
        {
            SetSpriteAnimation(Warrior.Sprite, Warrior.AnimationStates[Warrior.State][Warrior.FacingRight ? 0 : 1]);
        }

        // update this frame of animation
        UpdateSpriteAnimation(Warrior.Sprite);

        Vector2 playerScreenPos = GetWorldToScreen2D(Warrior.Sprite.Position, cam);

        if (playerScreenPos.x < 10)
            cam.target.x -= (GetScreenWidth() - 50)/cam.zoom;

        if (playerScreenPos.x > GetScreenWidth() - 25)
            cam.target.x += (GetScreenWidth() - 50)/cam.zoom;

        if (playerScreenPos.y < 10)
            cam.target.y -= (GetScreenHeight() - 50) / cam.zoom;

        if (playerScreenPos.y > GetScreenHeight() - 25)
            cam.target.y += (GetScreenHeight() - 50) / cam.zoom;

        Rectangle warriorRect = { newPos.x - Warrior.Width / 2, newPos.y - Warrior.Height, Warrior.Width, Warrior.Height };
        // draw the world
        BeginDrawing();
            ClearBackground(SKYBLUE);
            BeginMode2D(cam);
                DrawRectangle(cam.target.x - cam.offset.x, 0, GetScreenWidth(), 50, BROWN);

                for (Rectangle& rect : Obstacles)
                {
                   // DrawRectangleRec(rect, GRAY);
                    DrawTextureNPatch(ObstacleTexture, ObstacleNpatch, rect, Vector2Zero(), 0, WHITE);
                }
                DrawSprite(Warrior.Sprite);

                DrawCircleV(Vector2{ Warrior.Sprite.Position.x,Warrior.Sprite.Position.y }, 3, YELLOW);
                DrawRectangleLinesEx(warriorRect, 2, RED);
            EndMode2D();

            const char* stateName = "Unknown";
            switch (Warrior.State)
            {
            case ActorStates::Idle:
                stateName = "Idle";
                break;

            case ActorStates::Run:
                stateName = "Running";
                break;
            case ActorStates::JumpStart:
                stateName = "Jump";
                break;
            case ActorStates::JumpUp:
                stateName = "Flying Up";
                break;
            case ActorStates::FallDown:
                stateName = "Falling";
                break;
            }

            DrawText(TextFormat("state:%s",stateName), 10, 10, 20, BLACK);

        EndDrawing();
      
    }
    UnloadTexture(warriorSheet.SheetTexture);
    CloseWindow();

    return 0;
}