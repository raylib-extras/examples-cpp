/**********************************************************************************************
*
*   raylib-extras, examples-cpp * examples for Raylib in C++
*
*   Hardpoint and compound transforms
*
*   LICENSE: MIT
*
*   Copyright (c) 2023 Jeffery Myers
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
#include "rlgl.h"

#include <vector>
#include <map>
#include <string>

class GunPart
{
public:
	Texture2D Texture = { 0 };
	Vector2 Orign = { 0 };

	std::vector<Vector2> Hardpoints;

	std::string Name;

	virtual ~GunPart()
	{
		// really should be part of a sprite sheet, but whatever
		if (IsTextureReady(Texture))
			UnloadTexture(Texture);
	}
};

std::vector<GunPart*> Guns;
std::vector<GunPart*> Scopes;
std::vector<GunPart*> BarrelMods;

int GunIndex = 0;
int ScopeIndex = 0;
int BarrelIndex = 0;

class GunAttachmentNode
{
public:
	bool Active = true;

	GunPart* Part = nullptr;

	Color Tint = WHITE;

	std::map<size_t, GunAttachmentNode> Children;

	GunAttachmentNode() = default;
	GunAttachmentNode(GunPart* part) : Part(part) {}

	void Draw(bool flipY) const
	{
		if (!Active || !Part)
			return;

		rlPushMatrix();
		// compute our local origin offset
		float yOrigin = Part->Orign.y;
		float yScale = 1;

		// if we are flipped all y data needs to be mirrored
		if (flipY)
		{
			yOrigin = Part->Texture.height - yOrigin;
			yScale *= -1;
		}

		// translate back so we are the right origin
		rlTranslatef(-Part->Orign.x, -yOrigin, 0);

		// draw the texture for this part
		DrawTextureRec(Part->Texture, Rectangle{ 0,0,float(Part->Texture.width), float(Part->Texture.height) * yScale }, Vector2Zero(), Tint);

		// walk all the children and offset them by the hardpoint location
		// these transforms are nested, so they will pick up any translation or rotation from the parent
		for (const auto& child : Children)
		{
			rlPushMatrix();

			// if we are flipped all Y data is mirrored
			float yPos = Part->Hardpoints[child.first].y;
			if (flipY)
			{
				yPos = Part->Texture.height - yPos;
			}
			rlTranslatef(Part->Hardpoints[child.first].x, yPos, 0);
			child.second.Draw(flipY);
			rlPopMatrix();
		}
		rlPopMatrix();
	}
};

GunAttachmentNode* Gun = nullptr;

Texture2D Player = { 0 };
float GunAngle = 0;

Vector2 PlayerPos = { 0,0 };

void GameInit()
{
	// build up our gun parts
	
	GunPart* gun1 = new GunPart();
    gun1->Texture = LoadTexture("resources/gun.png");
    gun1->Orign = Vector2{ 28, 16 };
    gun1->Name = "Rifle";
    gun1->Hardpoints.push_back(Vector2{ 122, 14 });		// barrel mount hardpoint
    gun1->Hardpoints.push_back(Vector2{ 54, 2 });		// scope mount hardpoint
	Guns.push_back(gun1);
	
	// a smaller gun
	GunPart* gun2 = new GunPart();
    gun2->Texture = LoadTexture("resources/gun2.png");
    gun2->Orign = Vector2{ -10, 14 };
    gun2->Name = "SMG";
    gun2->Hardpoints.push_back(Vector2{ 61, 14 });		// barrel mount hardpoint
    gun2->Hardpoints.push_back(Vector2{ 18, 11 });		// scope mount hardpoint
	Guns.push_back(gun2);


	// barrel modes
	GunPart* silencer1 = new GunPart();
    silencer1->Texture = LoadTexture("resources/Silencer.png");
    silencer1->Orign = Vector2{ 0, 4 };					// where the silencer attaches to the hardpoint on the parent
	silencer1->Name = "Silencer";
	BarrelMods.push_back(silencer1);

	GunPart* underBarelLight = new GunPart();
    underBarelLight->Texture = LoadTexture("resources/Light.png");
    underBarelLight->Orign = Vector2{ 6, 10 };			// where the light attaches to the hardpoint on the parent
    underBarelLight->Name = "Light";
    BarrelMods.push_back(underBarelLight);
	
	// scopes
	GunPart* scope1 = new GunPart();
    scope1->Texture = LoadTexture("resources/Scope.png");
    scope1->Orign = Vector2{ 5, 9 };						// where the scope attaches to the hardpoint on the parent
    scope1->Name = "Scope";
	Scopes.push_back(scope1);

	GunPart* scope2 = new GunPart();
    scope2->Texture = LoadTexture("resources/RedDot.png");
    scope2->Orign = Vector2{ 0, 9 };
    scope2->Name = "Red Dot Sight";
	Scopes.push_back(scope2);

	GunPart* scope3 = new GunPart();
    scope3->Texture = LoadTexture("resources/Laser.png");
    scope3->Orign = Vector2{ 3, 8 };
	scope3->Name = "Laser";
	Scopes.push_back(scope3);

	// player data
	Player = LoadTexture("resources/player.png");
    PlayerPos.x = GetScreenWidth() * 0.5f;
    PlayerPos.y = GetScreenHeight() * 0.5f;

	// build an attachment of gun parts for rendering
	Gun = new GunAttachmentNode(gun1);	
	Gun->Children.emplace(0, silencer1);	// add a silencer to the barrel mount hardpoint
	Gun->Children.emplace(1, scope3);		// add a scope to the scope mount hardpoint
}

void GameCleanup()
{
	UnloadTexture(Player);
	Guns.clear();
	BarrelMods.clear();
	Scopes.clear();
	delete(Gun);
}

bool GameUpdate()
{
	if (Vector2DistanceSqr(PlayerPos, GetMousePosition()) > 10)
		GunAngle = Vector2LineAngle(PlayerPos, GetMousePosition()) * -RAD2DEG;

	Vector2 input = { 0 };
	if (IsKeyDown(KEY_W))
		input.y -= 1;
    if (IsKeyDown(KEY_S))
        input.y += 1;

    if (IsKeyDown(KEY_A))
        input.x -= 1;
    if (IsKeyDown(KEY_D))
        input.x += 1;

	PlayerPos = Vector2Add(PlayerPos, Vector2Scale(input, 300 * GetFrameTime()));


	bool rebuildGun = false;
	// cycle guns
	if (IsKeyPressed(KEY_O))
	{
		rebuildGun = true;
		GunIndex++;
		if (GunIndex >= Guns.size())
			GunIndex = 0;
	}

    if (IsKeyPressed(KEY_P))
    {
		rebuildGun = true;
        GunIndex--;
        if (GunIndex < 0)
            GunIndex = Guns.size()-1;
    }

    // cycle barrel modes
    if (IsKeyPressed(KEY_K))
    {
		rebuildGun = true;
        BarrelIndex++;
        if (BarrelIndex >= BarrelMods.size())
			BarrelIndex = -1;
    }

    if (IsKeyPressed(KEY_L))
    {
		rebuildGun = true;
		BarrelIndex--;
        if (BarrelIndex < -1)
			BarrelIndex = BarrelMods.size() - 1;
    }

    // cycle scopes
    if (IsKeyPressed(KEY_Y))
    {
        rebuildGun = true;
        ScopeIndex++;
        if (ScopeIndex >= Scopes.size())
			ScopeIndex = -1;
    }

    if (IsKeyPressed(KEY_U))
    {
        rebuildGun = true;
		ScopeIndex--;
        if (ScopeIndex < -1)
			ScopeIndex = Scopes.size() - 1;
    }

	if (rebuildGun)
	{
		// optimally this should cache the assembled gun to a render texture, but we'll just draw it every frame.

		// set the current parts
		Gun->Part = Guns[GunIndex];

		auto barelSlot = Gun->Children.find(0);
		if (barelSlot != Gun->Children.end())
		{
            if (BarrelIndex == -1)
				barelSlot->second.Part = nullptr;
            else
				barelSlot->second.Part = BarrelMods[BarrelIndex];
		}

        auto scopeSlot = Gun->Children.find(1);
        if (scopeSlot != Gun->Children.end())
        {
            if (ScopeIndex == -1)
				scopeSlot->second.Part = nullptr;
            else
				scopeSlot->second.Part = Scopes[ScopeIndex];
        }
	}

	return true;
}

void GameDraw()
{
	BeginDrawing();
	ClearBackground(SKYBLUE);

	// draw the player
	DrawTexturePro(Player,
		Rectangle{ 0,0,float(Player.width), float(Player.height) },
		Rectangle{ PlayerPos.x,PlayerPos.y,float(Player.width), float(Player.height) },
		Vector2{ Player.width * 0.5f,Player.height * 0.5f },
		0,
		WHITE);

	// move to the origin of the gun on the player
	rlPushMatrix();
	rlTranslatef(PlayerPos.x, PlayerPos.y + 20, 0);
	// rotate to the gun angle
	rlRotatef(GunAngle, 0, 0, 1);

	// draw the gun from it's origin
	Gun->Draw(fabsf(GunAngle) > 90);

	// put the transform back
	rlPopMatrix();

	DrawFPS(0, 0);
	DrawText(TextFormat("Gun Angle %0.1f", GunAngle), 100, 0, 20, WHITE);

	// instructions for how to change parts
	DrawText(TextFormat("Gun : <O %s P>", Gun->Part->Name.c_str()), 10, 20, 20, WHITE);

	auto& barrel = Gun->Children[0];
	DrawText(TextFormat("Barrel : <K %s L>", BarrelIndex == -1 ? "None" : barrel.Part->Name.c_str()), 10, 40, 20, WHITE);

    auto& scope = Gun->Children[1];
    DrawText(TextFormat("Scope : <Y %s U>", ScopeIndex == -1 ? "None" : scope.Part->Name.c_str()), 10, 60, 20, WHITE);
	EndDrawing();
}

int main()
{
	SetConfigFlags(FLAG_VSYNC_HINT);
	InitWindow(1280, 800, "Example");
	SetTargetFPS(144);

	GameInit();

	while (!WindowShouldClose())
	{
		if (!GameUpdate())
			break;
		
		GameDraw();
	}
	
	GameCleanup();
	CloseWindow();
	return 0;
}