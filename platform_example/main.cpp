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
#include "sprites.h"

#include <unordered_map>
#include <vector>

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

    float RunSpeed = 200;
	float jumpAcceleration = -250;
    float jumpVelocityDampen = 1.125f;
    Vector2 Velocity = { 0,0 };
};

int main(void)
{
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "sprite animation");

    // load our sprite sheet
    SpriteSheet warriorSheet = LoadSpriteSheet("resources/Warrior_Sheet-Effect.png", 6, 17);

    // create left facing versions of all the sprites
    int leftFrameStart = AddFippedFrames(warriorSheet, 0, (int)warriorSheet.Frames.size() - 1, true, false);

    float groundHeight = GetScreenHeight() - 50;
    float gravity = 500.0f;

    // create our warrior
    Actor warrior;
    warrior.Sprite = SpriteInstance{ Vector2{100, groundHeight}, Vector2{warriorSheet.Frames[0].width / 2,warriorSheet.Frames[0].height }, &warriorSheet };

    // define an animation for each state
    warrior.AnimationStates[ActorStates::Idle].emplace_back(SpriteAnimation{ "right_idle", 0, 5, 5 });
    warrior.AnimationStates[ActorStates::Idle].emplace_back(SpriteAnimation{ "left_idle", leftFrameStart + 0, leftFrameStart + 5, 5 });

	warrior.AnimationStates[ActorStates::Run].emplace_back(SpriteAnimation{ "right_run", 6, 13 });
	warrior.AnimationStates[ActorStates::Run].emplace_back(SpriteAnimation{ "left_run", leftFrameStart + 6, leftFrameStart + 13 });

	warrior.AnimationStates[ActorStates::JumpStart].emplace_back(SpriteAnimation{ "right_jump", 42, 44, 10  ,false});
	warrior.AnimationStates[ActorStates::JumpStart].emplace_back(SpriteAnimation{ "left_jump", leftFrameStart +42, leftFrameStart + 44, 10  ,false });

	warrior.AnimationStates[ActorStates::JumpUp].emplace_back(SpriteAnimation{ "right_fly", 43, 43, 10 });
	warrior.AnimationStates[ActorStates::JumpUp].emplace_back(SpriteAnimation{ "left_fly", leftFrameStart + 43, leftFrameStart + 43, 10 });

	warrior.AnimationStates[ActorStates::FallDown].emplace_back(SpriteAnimation{ "right_fall", 45, 47, 10 });
	warrior.AnimationStates[ActorStates::FallDown].emplace_back(SpriteAnimation{ "left_fall", leftFrameStart + 45, leftFrameStart + 47, 10 });

    SetSpriteAnimation(warrior.Sprite, warrior.AnimationStates[ActorStates::Idle][0]);

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second

    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // get the input
        int runDir = 0;
        if(IsKeyDown(KEY_D))
            runDir += 1;
		if (IsKeyDown(KEY_A))
			runDir += -1;

        bool wantJump = IsKeyDown(KEY_SPACE);

        // apply input and change any states as needed
        switch (warrior.State)
        {
        case ActorStates::Idle:
            if (wantJump)
            {
                warrior.State = ActorStates::JumpStart;
                warrior.Velocity.y = warrior.jumpAcceleration;
                warrior.Velocity.x = 0;
            }
            else if (runDir != 0)
            {
                warrior.State = ActorStates::Run;
                warrior.FacingRight = runDir > 0;
            }
            else
			{
				warrior.Velocity.x = 0;
				warrior.Velocity.y = 0;
			}
            break;

        case ActorStates::Run:
			if (wantJump)
			{
				warrior.State = ActorStates::JumpStart;
				warrior.Velocity.y = warrior.jumpAcceleration;
                warrior.Velocity.x *= warrior.jumpVelocityDampen;
			}
            else if (runDir == 0)
            {
                warrior.State = ActorStates::Idle;
            }
            else
            {
                warrior.FacingRight = runDir > 0;
                warrior.Velocity.x = runDir * warrior.RunSpeed;
                warrior.Velocity.y = 0;
            }

            break;

        case ActorStates::JumpStart:
            if (warrior.Velocity.y >= 0)
                warrior.State = ActorStates::FallDown;
            else if (warrior.Sprite.AnimationDone)
                warrior.State = ActorStates::JumpUp;
            break;

        case ActorStates::JumpUp:
            if (warrior.Velocity.y >= 0)
                warrior.State = ActorStates::FallDown;
            break;

        case ActorStates::FallDown:
            if (warrior.Sprite.Position.y >= groundHeight)
            {
                warrior.Velocity.x = 0;
                warrior.Velocity.y = 0;
                warrior.Sprite.Position.y = groundHeight;
				warrior.State = ActorStates::Idle;
            }
            break;
        }

        // if we can fall, make us fall
        if (warrior.State != ActorStates::Idle && warrior.State != ActorStates::Run)
            warrior.Velocity.y += gravity * GetFrameTime();

        // move us how we want to move
		warrior.Sprite.Position.x += warrior.Velocity.x * GetFrameTime();
		warrior.Sprite.Position.y += warrior.Velocity.y * GetFrameTime();

        // if we are not on the animation we want to be on, set us to that animation
        if (warrior.Sprite.Animation != &(warrior.AnimationStates[warrior.State][warrior.FacingRight ? 0 : 1]))
        {
            SetSpriteAnimation(warrior.Sprite, warrior.AnimationStates[warrior.State][warrior.FacingRight ? 0 : 1]);
        }

        // update this frame of animation
		UpdateSpriteAnimation(warrior.Sprite);

        // draw the world
        BeginDrawing();
            ClearBackground(SKYBLUE);
            DrawRectangle(0, groundHeight, GetScreenWidth(), GetScreenHeight() - groundHeight, BROWN);
            DrawSprite(warrior.Sprite);

            const char* stateName = "Unknown";
            switch (warrior.State)
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