#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"

// constants from OpenGL
#define GL_SRC_ALPHA 0x0302
#define GL_MIN 0x8007


#include <vector>

Vector2 LightPos = { 600, 400 };
float LightRadius = 300;

std::vector<Rectangle> Boxes;

std::vector<std::pair<Vector2, Vector2>> Rays;

std::vector<std::vector<Vector2>> Shadows;

RenderTexture LightTexture;

bool CullReverseShadows = true;

void SetupBoxes()
{
	Boxes.emplace_back(Rectangle{ 50,50, 40, 40 });
	Boxes.emplace_back(Rectangle{ 1200, 700, 40, 40 });
	Boxes.emplace_back(Rectangle{ 200, 600, 40, 40 });
	Boxes.emplace_back(Rectangle{ 1000, 50, 40, 40 });
	Boxes.emplace_back(Rectangle{ 500, 350, 40, 40 });

	for (int i = 0; i < 50; i++)
	{
		Boxes.emplace_back(Rectangle{ (float)GetRandomValue(0,GetScreenWidth()), (float)GetRandomValue(0,GetScreenHeight()), (float)GetRandomValue(10,100), (float)GetRandomValue(10,100) });
	}
}

bool BoxInLight(const Rectangle& box)
{
	Vector2 center = Vector2{ box.x + box.width / 2, box.y + box.height / 2 };
	float radSquare = (box.width * box.width) / 2 + (box.height * box.height) / 2;

	float distSquare = Vector2LengthSqr(Vector2Subtract(center, LightPos));

	return distSquare <= radSquare + LightRadius * LightRadius;
}

bool Vector2Equal(const Vector2& p1, const Vector2& p2)
{
	return fabs(p1.x-p2.x) < 0.5f && fabs(p1.y-p2.y) < 0.5f;
}

bool PointIsEP(const Vector2& point, const Vector2& sp, const Vector2& ep)
{
	return Vector2Equal(sp, point) || Vector2Equal(ep, point);
}

bool LineIntersectsBox(const Rectangle& box, const Vector2& p1, const Vector2& p2, Vector2& collision)
{
	Vector2 b1 = { box.x, box.y };
	Vector2 b2 = { box.x + box.width, box.y };

	collision = { 0,0 };

	// top
	if (CheckCollisionLines(p1, p2, b1, b2, &collision) && !PointIsEP(p2,b1,b2))
		return true;

	// right
	b1 = b2;
	b2.y += box.height;
	if (CheckCollisionLines(p1, p2, b1, b2, &collision) && !PointIsEP(p2, b1, b2))
		return true;

	// bottom
	b1 = b2;
	b2.x -= box.width;
	if (CheckCollisionLines(p1, p2, b1, b2, &collision) && !PointIsEP(p2, b1, b2))
		return true;

	b1 = b2;
	b2.y -= box.height;
	if (CheckCollisionLines(p1, p2, b1, b2, &collision) && !PointIsEP(p2, b1, b2))
		return true;

	return false;
}

bool BoxCornerIsGood(const Vector2& boxCorner)
{
	Vector2 collision = { 0,0 };
	for (const auto& otherBox : Boxes)
	{
		if (CheckCollisionPointRec(LightPos, otherBox))
			return false;

		if (LineIntersectsBox(otherBox, LightPos, boxCorner, collision))
		{
			if (collision.x != boxCorner.x && collision.y != boxCorner.y)
				return false;
		}
	}

	return true;
}

void AddBoxRays(const Rectangle& box)
{
	if (!BoxInLight(box))
		return;

	Vector2 boxCorner = { box.x, box.y };

	// TODO, compute all box corners first, THEN test all the boxes in the light

	if (BoxCornerIsGood(boxCorner))
		Rays.push_back({ LightPos, boxCorner });

	boxCorner.x += box.width;
	if (BoxCornerIsGood(boxCorner))
		Rays.push_back({ LightPos, boxCorner });

	boxCorner.y += box.height;
	if (BoxCornerIsGood(boxCorner))
		Rays.push_back({ LightPos, boxCorner });
	
	boxCorner.x -= box.width;
	if (BoxCornerIsGood(boxCorner))
		Rays.push_back({ LightPos, boxCorner });
}

void ComputePlayerRays()
{
	Rays.clear();
	for (const auto& box : Boxes)
		AddBoxRays(box);
}

void ComputeShadowForEdge(const Vector2& sp, const Vector2& ep, bool reverse)
{
	if (CullReverseShadows && reverse)
		return;
	float extension = LightRadius;

	Vector2 spVector = Vector2Normalize(Vector2Subtract(sp, LightPos));
	Vector2 spProjection = Vector2Add(sp, Vector2Scale(spVector, extension));

	Vector2 epVector = Vector2Normalize(Vector2Subtract(ep, LightPos));
	Vector2 epProjection = Vector2Add(ep, Vector2Scale(epVector, extension));

	std::vector<Vector2> polygon;
	polygon.push_back(sp);
	
	if (reverse)
	{
		polygon.push_back(spProjection);
		polygon.push_back(epProjection);
		polygon.push_back(ep);
	}
	else
	{
		polygon.push_back(ep);
		polygon.push_back(epProjection);
		polygon.push_back(spProjection);
	}

	Shadows.push_back(polygon);
}

void ComputeBoxShadows(const Rectangle& box)
{
	if (CheckCollisionPointRec(LightPos, box) || !BoxInLight(box))
		return;

	// top
	Vector2 sp = { box.x, box.y };
	Vector2 ep = { box.x + box.width, box.y };

	ComputeShadowForEdge(sp, ep, LightPos.y < ep.y);

	// right
	sp = ep;
	ep.y += box.height;
	ComputeShadowForEdge(sp, ep, LightPos.x > ep.x);

	// bottom
	sp = ep;
	ep.x -= box.width;
	ComputeShadowForEdge(sp, ep, LightPos.y > ep.y);

	// left
	sp = ep;
	ep.y -= box.height;
	ComputeShadowForEdge(sp, ep, LightPos.x < ep.x);
}

void ComputeShadows()
{
	Shadows.clear();
	for (const auto& box : Boxes)
		ComputeBoxShadows(box);
}

void ComputeLightMask()
{
	BeginTextureMode(LightTexture);

	ClearBackground(BLACK);

	float flicker = 0;// GetRandomValue(0, 3) / 100.0f;
	DrawCircleGradient(LightPos.x, LightPos.y, LightRadius, ColorAlpha(WHITE, 0.85f + flicker), BLANK );

	// force the blend mode to only set the alpha of the destination
	rlSetBlendFactors(GL_SRC_ALPHA, GL_SRC_ALPHA, GL_MIN);
	rlSetBlendMode(BLEND_CUSTOM);

	// go back to normal
	rlSetBlendMode(BLEND_ALPHA);

	for (std::vector<Vector2> shadow : Shadows)
	{
		DrawTriangleFan(&shadow[0], 4, ColorAlpha(BLACK, 1));
	}

	EndTextureMode();
}

int main()
{
	SetConfigFlags(/*FLAG_VSYNC_HINT ||*/ FLAG_MSAA_4X_HINT);
	InitWindow(1280, 800, "LightCaster");
//	SetTargetFPS(144);
	SetupBoxes();

	LightTexture = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());

	Image img = GenImageChecked(64, 64, 32, 32, GRAY, DARKGRAY);
	Texture2D tile = LoadTextureFromImage(img);
	UnloadImage(img);

	bool showLines = false;
	while (!WindowShouldClose())
	{
		if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
			LightPos = GetMousePosition();

		if (IsKeyPressed(KEY_F1))
			showLines = !showLines;

		if (IsKeyPressed(KEY_F2))
			CullReverseShadows = !CullReverseShadows;

		ComputePlayerRays();
		ComputeShadows();

		ComputeLightMask();

		BeginDrawing();
		ClearBackground(BLACK);

		DrawTextureRec(tile, Rectangle{ 0,0,(float)GetScreenWidth(),(float)GetScreenHeight() }, Vector2Zero(), WHITE);

		DrawTextureRec(LightTexture.texture, Rectangle{ 0, 0, (float)GetScreenWidth(), -(float)GetScreenHeight() }, Vector2Zero(), ColorAlpha(WHITE,1.0f));

		for (const auto& box : Boxes)
		{
			if (BoxInLight(box))
				DrawRectangleRec(box, BLACK);
		}
			

		DrawCircle(int(LightPos.x), int(LightPos.y), 10, YELLOW);

		if (showLines)
		{
			for (std::vector<Vector2> shadow : Shadows)
			{
				DrawTriangleFan(&shadow[0], 4, DARKPURPLE);
			}

			for (const auto& box : Boxes)
			{
				if (BoxInLight(box))
					DrawRectangleRec(box, PURPLE);
			}

			for (const auto& box : Boxes)
			{
				DrawRectangleLines(box.x,box.y, box.width, box.height, DARKBLUE);
			}

			for (const auto& ray : Rays)
			{
				DrawLine(ray.first.x, ray.first.y, ray.second.x, ray.second.y, GREEN);
			}

			DrawText("(F1) Hide Shadow Vectors", 0, 0, 20, GREEN);
		}
		else
		{
			DrawText("(F1) Show Shadow Vectors", 0, 0, 20, GREEN);
		}

		if (CullReverseShadows)
			DrawText("(F2) Show Reversed Shadows", 0, 20, 20, GREEN);
		else
			DrawText("(F2) Cull Reversed Shadows", 0, 20, 20, GREEN);
		
		DrawFPS(1200, 0);
		DrawText(TextFormat("Rays %d, Shadows %d", (int)Rays.size(), (int)Shadows.size()), 1050, 20, 20, GREEN);

		EndDrawing();
	}

	CloseWindow();
	return 0;
}