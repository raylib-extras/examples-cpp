/*
Raylib example file.
This is an example main file for a simple raylib project.
Use this as a starting point or replace it with your code.

by Jeffery Myers is marked with CC0 1.0. To view a copy of this license, visit https://creativecommons.org/publicdomain/zero/1.0/

*/

#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"

#include <vector>

#define RLIGHTS_IMPLEMENTATION
#include "rlights.h"

#include "resource_dir.h"	// utility header for SearchAndSetResourceDir

Camera3D ViewCamera = { 0 };

Model PlayerModel = { 0 };
Model HatModel = { 0 };
Model SwordModel = { 0 };
Model ShieldModel = { 0 };

ModelAnimation* Animations = nullptr;

Shader LightShader = { 0 };

RenderTexture MiniMap = { 0 };

struct EntityInfo
{
	Vector2 Position = Vector2Zeros;
	float Facing = 0;

    float MoveSpeed = 8;

	Color Tint = WHITE;
	bool Hovered = false;
};

struct PlayerInfo : public EntityInfo
{
	enum class State
	{
		Idle,
		Moving,
		Attacking,
	};
	State MoveState = State::Idle;

	Vector2 MoveTarget = Vector2Zeros;

	Vector3 ViewOffset = Vector3Normalize(Vector3{ 0, 1, 1 });
	float ViewDistance = 30;

    float AttackRange = 2.0f;

	int CurrentAnimationSequence = 0;
	int CurrentAnimFrame = 0;
    float AnimFrameTime = 0;

	bool HasHat = false;
	int HatBoneIndex = -1;

    bool HasSword = false;
    int SwordBoneIndex = -1;

    bool HasShield = false;
    int ShieldBoneIndex = -1;

    EntityInfo* CurrentTarget = nullptr;
};

PlayerInfo Player;

struct EnemyInfo : public EntityInfo
{
    int Health = 10;
    int AttackDamanage = 2;
    float AttackTime = 1;
    float AttackTimer = 0;
};

std::vector<EnemyInfo> Enemies;

Vector3 MouseProjectionPos = Vector3Zeros;

struct ObstacleInfo
{
	Vector2 Position = Vector2Zeros;
	float Rotation = 0;

    Vector3 Scale = { 1,1,1 };
	Color Tint = GRAY;

	Vector2 RelativePoint = Vector2Zeros;
};
std::vector<ObstacleInfo> Obstacles;

struct LootInfo
{
	enum class LootType
	{
		Hat,
		Sword,
		Shield
    };

    Vector2 Position = Vector2Zeros;
	LootType Type = LootType::Hat;
    bool PickedUp = false;
};

std::vector<LootInfo> Pickups;

std::vector<Sound> Footsteps;

Sound PickupSound = { 0 };

bool AllowViewRotation = false;

void SetModelShader(Model& model, Shader& shader)
{
	for (int i = 0; i < model.materialCount; i++)
	{
		model.materials[i].shader = shader;
	}
}

void SetupScene()
{
    Obstacles.push_back(ObstacleInfo{ Vector2{25,0}, 0, Vector3{5, 5, 25}, BROWN });
    Obstacles.push_back(ObstacleInfo{ Vector2{-25,0}, 0, Vector3{5, 5, 25}, DARKBLUE });
    Obstacles.push_back(ObstacleInfo{ Vector2{0,-25}, 0, Vector3{25, 5, 5}, DARKPURPLE });
    Obstacles.push_back(ObstacleInfo{ Vector2{0,25}, 0, Vector3{25, 5, 5}, DARKGREEN });
    Obstacles.push_back(ObstacleInfo{ Vector2{15,15}, 45, Vector3{7, 5, 3}, ORANGE });

    Pickups.push_back(LootInfo{ Vector2{ 10,-10 }, LootInfo::LootType::Hat });
    Pickups.push_back(LootInfo{ Vector2{ -10,10 }, LootInfo::LootType::Sword });
    Pickups.push_back(LootInfo{ Vector2{ -10,-10 }, LootInfo::LootType::Shield });

    Enemies.push_back(EnemyInfo{ Vector2{-20,10}, 33 });
}

void InitScene()
{
	ViewCamera.fovy = 45;
	ViewCamera.up.y = 1;
    ViewCamera.position = { 0, 10, 10 };
    ViewCamera.target = { 0, 0, 0 };

	PlayerModel = LoadModel("greenman.glb");
	HatModel = LoadModel("greenman_hat.glb");
    SwordModel = LoadModel("greenman_sword.glb");
    ShieldModel = LoadModel("greenman_shield.glb");

	PlayerModel.transform = MatrixRotateY(90 * DEG2RAD);

    for (int i = 0; i < PlayerModel.boneCount; i++)
    {
        if (TextIsEqual(PlayerModel.bones[i].name, "socket_hat"))
        {
            Player.HatBoneIndex = i;
            continue;
        }

        if (TextIsEqual(PlayerModel.bones[i].name, "socket_hand_R"))
        {
			Player.SwordBoneIndex = i;
            continue;
        }

        if (TextIsEqual(PlayerModel.bones[i].name, "socket_hand_L"))
        {
            Player.ShieldBoneIndex = i;
            continue;
        }
    }

    LightShader = LoadShader("shaders/lighting.vs", "shaders/lighting.fs");

	LightShader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(LightShader, "viewPos");

    int ambientLoc = GetShaderLocation(LightShader, "ambient");
	float ambientColor[4] = { 0.3f, 0.3f, 0.3f, 1.0f };

    SetShaderValue(LightShader, ambientLoc, ambientColor, SHADER_UNIFORM_VEC4);

    CreateLight(LIGHT_DIRECTIONAL, Vector3{ 10, 10, 10 }, Vector3{ -14, -10, -4 }, WHITE, LightShader);
    CreateLight(LIGHT_DIRECTIONAL, Vector3{ -10, -10, 10 }, Vector3{ -14, 10, 4 }, GRAY, LightShader);

	SetModelShader(PlayerModel, LightShader);
	SetModelShader(HatModel, LightShader);
	SetModelShader(SwordModel, LightShader);
	SetModelShader(ShieldModel, LightShader);

    int animCount = 0;
	Animations = LoadModelAnimations("greenman.glb", &animCount);

	Footsteps.push_back(LoadSound("footstep_concrete_000.ogg"));
	for (int i = 0; i < 8; i++)
	{
		Footsteps.push_back(LoadSoundAlias(Footsteps[0]));
    }

	PickupSound = LoadSound("swordMetal3.ogg");

    MiniMap = LoadRenderTexture(256, 256);

    SetupScene();
}

void PlayNextFootstep()
{
	for (auto& sound : Footsteps)
	{
		if (!IsSoundPlaying(sound))
		{
			PlaySound(sound);
			break;
		}
    }
}

void SetupCamera()
{
	ViewCamera.target = Vector3{ Player.Position.x, 0, Player.Position.y };
	ViewCamera.position = ViewCamera.target + (Player.ViewOffset * Player.ViewDistance);

	auto ray = GetScreenToWorldRay(GetMousePosition(), ViewCamera);

    float size = 1000;
	auto results = GetRayCollisionQuad(ray, ViewCamera.target + Vector3{ size,0,size }, ViewCamera.target + Vector3{ -size,0,size }, ViewCamera.target + Vector3{ -size,0,-size }, ViewCamera.target + Vector3{ size,0,-size });

	if (results.hit)
	{
		MouseProjectionPos = results.point;
		constexpr bool snap = false;

		if (snap)
		{
			MouseProjectionPos.x = floorf(MouseProjectionPos.x) + 0.5f;
			MouseProjectionPos.z = floorf(MouseProjectionPos.z) + 0.5f;
		}

		if (Player.MoveState != PlayerInfo::State::Moving)
			Player.Facing = -atan2f(MouseProjectionPos.z - Player.Position.y, MouseProjectionPos.x - Player.Position.x) * RAD2DEG;
	}
}

void ProcessPlayerActions()
{
    if (!IsMouseButtonDown(MOUSE_BUTTON_LEFT))
        return;

    for (auto& enemy : Enemies)
    {
        if (!enemy.Hovered || Player.MoveState == PlayerInfo::State::Attacking)
            continue;

        float distSqr = Vector2DistanceSqr(enemy.Position, Player.Position);

        if (distSqr > Player.AttackRange * Player.AttackRange)
            continue;

        Player.MoveState = PlayerInfo::State::Attacking;
        Player.CurrentAnimFrame = 0;
    }
}

void MovePlayer()
{
    if (Player.MoveState == PlayerInfo::State::Moving)
    {
        Vector2 newPos = Player.Position;

        Vector2 moveDir = Vector2Subtract(Player.MoveTarget, Player.Position);
        float dist = Vector2Length(moveDir);
        if (dist < 0.1f)
        {
            Player.MoveState = PlayerInfo::State::Idle;
            newPos = Player.MoveTarget;
        }
        else
        {
            moveDir = Vector2Scale(moveDir, 1.0f / dist);
            float moveAmount = Player.MoveSpeed * GetFrameTime();
            if (moveAmount > dist)
                moveAmount = dist;
            newPos = Vector2Add(newPos, Vector2Scale(moveDir, moveAmount));
        }

        for (auto& obstacle : Obstacles)
        {
            Vector2 relativePos = newPos - obstacle.Position;
            obstacle.RelativePoint = Vector2Rotate(relativePos, obstacle.Rotation * DEG2RAD);

            Rectangle rect = { -obstacle.Scale.x / 2.0f,-obstacle.Scale.z / 2.0f, obstacle.Scale.x, obstacle.Scale.z };
            if (CheckCollisionCircleRec(relativePos, 0.75f, rect))
            {
                Player.MoveState = PlayerInfo::State::Idle;
                newPos = Player.Position;
            }
        }
        Player.Position = newPos;

		// check for pickups
        for (auto& loot : Pickups)
        {
            if (loot.PickedUp)
                continue;

            float dist = Vector2Distance(loot.Position, Player.Position);

            if (dist < 3.0f)
            {
                loot.PickedUp = true;
                PlaySound(PickupSound);
                switch (loot.Type)
                {
                case LootInfo::LootType::Hat:
                    Player.HasHat = true;
                    break;
                case LootInfo::LootType::Sword:
                    Player.HasSword = true;
                    break;
                case LootInfo::LootType::Shield:
                    Player.HasShield = true;
                    break;
                }
            }
        }
    }
}

void UpdateInput()
{
	Player.ViewDistance -= GetMouseWheelMove();
	if (Player.ViewDistance < 2)
        Player.ViewDistance = 2;

	if (IsKeyPressed(KEY_F1))
        AllowViewRotation = !AllowViewRotation;

	if (!AllowViewRotation)
		Player.ViewOffset = Vector3Normalize(Vector3{ 0, 1, 1 });
	else if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
 		Player.ViewOffset = Vector3RotateByAxisAngle(Player.ViewOffset, Vector3UnitY, -GetMouseDelta().x * (0.3f * DEG2RAD));
	
	SetupCamera();

	// handle movement
	if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
	{
		if (Player.MoveState == PlayerInfo::State::Idle && !(IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)))
		{
			Player.AnimFrameTime = 0;
			Player.MoveState = PlayerInfo::State::Moving;
		}
		Player.Facing = -atan2f(MouseProjectionPos.z - Player.Position.y, MouseProjectionPos.x - Player.Position.x) * RAD2DEG;
        Player.MoveTarget = Vector2{ MouseProjectionPos.x, MouseProjectionPos.z };
    }

	for (auto& enemy : Enemies)
	{
		float dist = Vector2Distance(Vector2{ MouseProjectionPos.x, MouseProjectionPos.z }, enemy.Position);
		enemy.Hovered = dist < 0.5f;

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && enemy.Hovered)
            Player.CurrentTarget = &enemy;
    }
}

void PreRenderScene()
{
    float cameraPos[3] = { ViewCamera.position.x, ViewCamera.position.y, ViewCamera.position.z };
    SetShaderValue(LightShader, LightShader.locs[SHADER_LOC_VECTOR_VIEW], cameraPos, SHADER_UNIFORM_VEC3);
}

void UpdatePlayerAnimation()
{
	Player.AnimFrameTime += GetFrameTime();

	float animFPS = 1.0f / 60.0f;

	Player.CurrentAnimationSequence = 1;
	if (Player.MoveState == PlayerInfo::State::Moving)
	{
		animFPS = 1.0f / 160.0f;
		Player.CurrentAnimationSequence = 2;
    }
    if (Player.MoveState == PlayerInfo::State::Attacking)
    {
        animFPS = 1.0f / 160.0f;
        Player.CurrentAnimationSequence = 3;
    }

	while (Player.AnimFrameTime >= animFPS)
	{
        Player.AnimFrameTime -= animFPS;

        Player.AnimFrameTime = 0;
        Player.CurrentAnimFrame++;
        if (Player.CurrentAnimFrame >= Animations[Player.CurrentAnimationSequence].frameCount)
        {
            Player.CurrentAnimFrame = 0;
            if (Player.MoveState == PlayerInfo::State::Attacking)
            {
                Player.MoveState = PlayerInfo::State::Idle;
            }
        }

		if (Player.MoveState == PlayerInfo::State::Moving)
		{
			if (Player.CurrentAnimFrame == 12 || Player.CurrentAnimFrame == 52)
				PlayNextFootstep();
		}

		UpdateModelAnimation(PlayerModel, Animations[Player.CurrentAnimationSequence], Player.CurrentAnimFrame);
	}
}

Matrix GetMatrixForBone (int boneIndex)
{
	if (boneIndex < 0 || boneIndex >= PlayerModel.boneCount)
		return MatrixIdentity();
	
    Transform* transform = &Animations[Player.CurrentAnimationSequence].framePoses[Player.CurrentAnimFrame][boneIndex];
    Quaternion inRotation = PlayerModel.bindPose[boneIndex].rotation;
    Quaternion outRotation = transform->rotation;

    // Calculate socket rotation (angle between bone in initial pose and same bone in current animation frame)
    Quaternion rotate = QuaternionMultiply(outRotation, QuaternionInvert(inRotation));
    Matrix matrixTransform = QuaternionToMatrix(rotate);
    // Translate socket to its position in the current animation
    matrixTransform = MatrixMultiply(matrixTransform, MatrixTranslate(transform->translation.x, transform->translation.y, transform->translation.z));
    // Transform the socket using the transform of the character (angle and translate)
    matrixTransform = MatrixMultiply(matrixTransform, PlayerModel.transform);

	return matrixTransform;
}

void DrawPlayer()
{
	UpdatePlayerAnimation();

    rlPushMatrix();
    rlTranslatef(Player.Position.x, 0, Player.Position.y);
    rlRotatef(Player.Facing, 0, 1, 0);

    DrawModel(PlayerModel, Vector3Zeros, 1, Player.Tint);

	if (Player.HasHat)
        DrawMesh(HatModel.meshes[0], HatModel.materials[HatModel.meshMaterial[0]], GetMatrixForBone(Player.HatBoneIndex));

    if (Player.HasSword)
        DrawMesh(SwordModel.meshes[0], SwordModel.materials[HatModel.meshMaterial[0]], GetMatrixForBone(Player.SwordBoneIndex));

    if (Player.HasShield)
        DrawMesh(ShieldModel.meshes[0], ShieldModel.materials[HatModel.meshMaterial[0]], GetMatrixForBone(Player.ShieldBoneIndex));

    rlPopMatrix();
}

void DrawEnemy(const EntityInfo& enemy)
{
    float bobble = sinf(float(GetTime()) * 2) * 0.1f;
    float bobble2 = sinf(float(GetTime()) * 4) * 0.2f;

	rlPushMatrix();
	rlTranslatef(enemy.Position.x, 0.5f, enemy.Position.y);
	rlRotatef(enemy.Facing, 0, 1, 0);

	DrawSphere(Vector3UnitY * bobble, 0.35f + bobble, DARKGREEN);
	DrawSphere(Vector3UnitY * bobble, 0.5f + bobble, ColorAlpha(LIME, 0.25f));

	if (enemy.Hovered)
		DrawCylinderWires(Vector3{ 0,-0.5f, 0 }, 0.75f + bobble, 0.75f + bobble, 1.2f + bobble, 32, ColorAlpha(PINK, 0.25f));
	else
		DrawCylinder(Vector3{ 0,-0.5f, 0 }, 0.65f + bobble, 0.65f + bobble, 0.1f, 12, ColorAlpha(BLACK, 0.25f));

	rlPopMatrix();
}

void DrawEnemies()
{
	for (auto& enemy : Enemies)
		DrawEnemy(enemy);
}

void DrawLoot()
{
    for (auto& loot : Pickups)
    {
        if (loot.PickedUp)
            continue;

        Model* model = nullptr;
        Color color = WHITE;

        switch (loot.Type)
        {
        case LootInfo::LootType::Hat:
            model = &HatModel;
            break;
        case LootInfo::LootType::Sword:
            model = &SwordModel;
            break;
        case LootInfo::LootType::Shield:
            model = &ShieldModel;
            break;
        }

        if (model)
        {
            float bobble = sinf(float(GetTime()) * 3) * 0.2f;

            Vector3 pos = { loot.Position.x, 0, loot.Position.y };

            DrawCylinder(pos, 0.5f + bobble, 0.5f, 0.1f, 16, ColorAlpha(YELLOW, 0.35f));

            pos.y = 1 + bobble;
            DrawModelEx(*model, pos, Vector3UnitY, float(GetTime()) * 90.0f, Vector3Ones, color);
        }
    }
}

void DrawViewNavElemens()
{
    DrawCylinder(MouseProjectionPos, 0.5f, 0.5f, 0.02f, 16, ColorAlpha(YELLOW, 0.5f));

    Vector3 viewVec = Vector3UnitX * 3;
    viewVec = Vector3RotateByAxisAngle(viewVec, Vector3UnitY, Player.Facing * DEG2RAD);

    DrawLine3D(ViewCamera.target, ViewCamera.target + viewVec, PURPLE);

    if (Player.MoveState == PlayerInfo::State::Moving)
    {
        float throb = (sinf(float(GetTime()) * 2) * 0.125f);

        DrawCapsule(Vector3{ Player.MoveTarget.x, 0, Player.MoveTarget.y }, Vector3{ Player.MoveTarget.x, 1 + throb, Player.MoveTarget.y }, 0.25f + throb, 32, 32, ColorAlpha(ORANGE, 0.5f));
    }
}

void DrawObstacles()
{
    BeginShaderMode(LightShader);

    DrawCube(Vector3{ 5,0,0 }, 1, 0.01f, 1, RED);
    DrawCube(Vector3{ 0,0,5 }, 1, 0.01f, 1, BLUE);

    for (auto& obstacle : Obstacles)
    {
        rlPushMatrix();
        rlTranslatef(obstacle.Position.x, obstacle.Scale.y / 2.0f, obstacle.Position.y);
        rlRotatef(obstacle.Rotation, 0, 1, 0);
        DrawCube(Vector3Zeros, obstacle.Scale.x, obstacle.Scale.y, obstacle.Scale.z, obstacle.Tint);
        rlPopMatrix();
    }

    EndShaderMode();
}

void DrawScene()
{
	PreRenderScene();

	rlSetClipPlanes(0.1f, 200.0f);
	BeginMode3D(ViewCamera);

	DrawGrid(200, 1);

	rlDrawRenderBatchActive();

	DrawPlayer();

	DrawEnemies();

    DrawObstacles();

	DrawLoot();

	DrawViewNavElemens();

	EndMode3D();
}

void DrawMiniMap()
{
    BeginTextureMode(MiniMap);
    ClearBackground(DARKGRAY);

    Camera2D cam = { 0 };
    cam.offset = Vector2{ MiniMap.texture.width / 2.0f, MiniMap.texture.height / 2.0f };
    cam.target = Player.Position;
    cam.zoom = 4;

    BeginMode2D(cam);

    BeginBlendMode(BLEND_ALPHA_PREMULTIPLY);
    DrawCircleV(Player.Position, 1.0f, LIME);

    Vector2 facing = Vector2Rotate(Vector2UnitX * 5, -Player.Facing * DEG2RAD);
    DrawLineV(Player.Position, Player.Position + facing, GREEN);

    DrawCircleV(Vector2{ MouseProjectionPos.x, MouseProjectionPos.z }, 1, YELLOW);
    DrawCircleV(Player.MoveTarget, 0.5f, ORANGE);

    DrawRectangleRec(Rectangle{ 5,0,1,1 }, RED);
    DrawRectangleRec(Rectangle{ 0,5, 1, 1 }, BLUE);
   
    for (auto& enemy : Enemies)
        DrawCircleV(enemy.Position, 0.5f, RED);

    for (auto& loot : Pickups)
    {
        if (loot.PickedUp)
            continue;

        DrawCircleV(loot.Position, 0.25f, YELLOW);
    }

    for (auto& obstacle : Obstacles)
    {
        Vector2 halfScale = Vector2{ obstacle.Scale.x / 2.0f, obstacle.Scale.z / 2.0f };
        Rectangle rect = { obstacle.Position.x, obstacle.Position.y, obstacle.Scale.x, obstacle.Scale.z };
        DrawRectanglePro(rect, halfScale, -obstacle.Rotation, obstacle.Tint);
    }
    EndBlendMode();
    EndMode2D();
    EndTextureMode();

    Vector2 pos = Vector2{ GetScreenWidth() - MiniMap.texture.width - 2.0f, 2.0f };

    DrawTextureRec(MiniMap.texture, Rectangle{ 0,0,256,-256 }, pos, ColorAlpha(WHITE, 1.0f));
    DrawRectangleLinesEx(Rectangle{ pos.x-2, pos.y-2, float(MiniMap.texture.width+4), float(MiniMap.texture.height+4) }, 4, DARKBROWN);
}

void Cleanup()
{
    Footsteps.push_back(LoadSound("footstep_concrete_000.ogg"));
    for (int i = 0; i < Footsteps.size(); i++)
    {
		if (i == 0)
			UnloadSound(Footsteps[i]);
		else
			UnloadSoundAlias(Footsteps[i]);
    }

	UnloadSound(PickupSound);

	UnloadModel(PlayerModel);
    UnloadModel(HatModel);
    UnloadModel(SwordModel);
    UnloadModel(ShieldModel);
	UnloadShader(LightShader);
}

int main ()
{
	auto flags = FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT;

	// Tell the window to use vsync and work on high DPI displays
#if defined(__APPLE__)
    flags |= FLAG_WINDOW_HIGHDPI;  // macOS Retina display support
#endif // defined(__APPLE__)

	SetConfigFlags(flags);

	// Create the window and OpenGL context
	InitWindow(1280, 800, "Diabolical Example");

	InitAudioDevice();

	// Utility function from resource_dir.h to find the resources folder and set it as the current working directory so we can load from it
	SearchAndSetResourceDir("resources");

	InitScene();
	
	// game loop
	while (!WindowShouldClose())		// run the loop until the user presses ESCAPE or presses the Close button on the window
	{
		// drawing
		BeginDrawing();

		// Setup the back buffer for drawing (clear color and depth buffers)
		ClearBackground(DARKBROWN);
		UpdateInput();

        ProcessPlayerActions();
		MovePlayer();

		DrawScene();

        DrawMiniMap();

		DrawFPS(5, 0);
		DrawText(TextFormat("Player Facing Angle %0.1f", Player.Facing), 5, 20, 20, WHITE);

		if (AllowViewRotation)
			DrawText("Free View (RMB to rotate) [F1 to toggle]", 5, 40, 20, LIGHTGRAY);
		else
			DrawText("Locked View [F1 to toggle]", 5, 40, 20, LIGHTGRAY);

		EndDrawing();
	}

	Cleanup();

	// destroy the window and cleanup the OpenGL context
	CloseWindow();
	return 0;
}
