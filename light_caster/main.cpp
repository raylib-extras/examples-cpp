#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"

// constants from OpenGL
#define GL_SRC_ALPHA 0x0302
#define GL_MIN 0x8007


#include <vector>

class LightInfo
{
public:
	Vector2 Position = { 0,0 };

	RenderTexture LightTexture;

	LightInfo()
	{
		LightTexture = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());
	}

	void Move(const Vector2& position)
	{
		Position = position;
		Dirty = true;
	}

	void SetRadius(float outerRadius)
	{
		OuterRadius = outerRadius;
		Dirty = true;
	}

	bool BoxInLight(const Rectangle& box)
	{
		return CheckCollisionRecs(Bounds, box);
	}

	void ShadowEdge(const Vector2& sp, const Vector2& ep)
	{
		float extension = OuterRadius;

		Vector2 spVector = Vector2Normalize(Vector2Subtract(sp, Position));
		Vector2 spProjection = Vector2Add(sp, Vector2Scale(spVector, extension));

		Vector2 epVector = Vector2Normalize(Vector2Subtract(ep, Position));
		Vector2 epProjection = Vector2Add(ep, Vector2Scale(epVector, extension));

		std::vector<Vector2> polygon;
		polygon.push_back(sp);
		polygon.push_back(spProjection);
		polygon.push_back(epProjection);
		polygon.push_back(ep);

		Shadows.push_back(polygon);
	}

	void UpdateLightTexture()
	{
		BeginTextureMode(LightTexture);

		ClearBackground(BLACK);

		float flicker = 0;// GetRandomValue(0, 3) / 100.0f;
		DrawCircleGradient(Position.x, Position.y, OuterRadius, ColorAlpha(WHITE, 0.85f + flicker), BLANK);

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

	void Update(const std::vector<Rectangle>& boxes)
	{
		if (!Dirty)
			return;

		Bounds.x = Position.x - OuterRadius;
		Bounds.y = Position.y - OuterRadius;
		Bounds.width = OuterRadius * 2;
		Bounds.height = OuterRadius * 2;

		Shadows.clear();

		for (const auto& box : boxes)
		{
			// are we in a box
			if (CheckCollisionPointRec(Position, box))
				return;

			if (!CheckCollisionRecs(box, Bounds))
				continue;

			// top
			Vector2 sp = { box.x, box.y };
			Vector2 ep = { box.x + box.width, box.y };

			if (Position.y < ep.y)
				ShadowEdge(sp, ep);

			// right
			sp = ep;
			ep.y += box.height;
			if (Position.x > ep.x)
				ShadowEdge(sp, ep);

			// bottom
			sp = ep;
			ep.x -= box.width;
			if (Position.y > ep.y)
				ShadowEdge(sp, ep);

			// left
			sp = ep;
			ep.y -= box.height;
			if (Position.x < ep.x)
				ShadowEdge(sp, ep);
		}
			
		UpdateLightTexture();
	}

	float OuterRadius = 300;
	Rectangle Bounds = { -150,-150,300,300 };

	std::vector<std::vector<Vector2>> Shadows;

	bool Dirty = true;
};

std::vector<Rectangle> Boxes;

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

int main()
{
	SetConfigFlags(/*FLAG_VSYNC_HINT ||*/ FLAG_MSAA_4X_HINT);
	InitWindow(1280, 800, "LightCaster");
//	SetTargetFPS(144);
	SetupBoxes();

	LightInfo light1;
	light1.Move(Vector2{ 600, 400 });

	Image img = GenImageChecked(64, 64, 32, 32, DARKBROWN, DARKGRAY);
	Texture2D tile = LoadTextureFromImage(img);
	UnloadImage(img);

	bool showLines = false;
	while (!WindowShouldClose())
	{
		if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
			light1.Move(GetMousePosition());

		if (IsKeyPressed(KEY_F1))
			showLines = !showLines;

		light1.Update(Boxes);

		BeginDrawing();
		ClearBackground(BLACK);

		DrawTextureRec(tile, Rectangle{ 0,0,(float)GetScreenWidth(),(float)GetScreenHeight() }, Vector2Zero(), WHITE);

		DrawTextureRec(light1.LightTexture.texture, Rectangle{ 0, 0, (float)GetScreenWidth(), -(float)GetScreenHeight() }, Vector2Zero(), ColorAlpha(WHITE, showLines ? 0.75f : 1.0f));

		DrawCircle(int(light1.Position.x), int(light1.Position.y), 10, YELLOW);

		if (showLines)
		{
			for (std::vector<Vector2> shadow : light1.Shadows)
			{
				DrawTriangleFan(&shadow[0], 4, DARKPURPLE);
			}

			for (const auto& box : Boxes)
			{
				if (light1.BoxInLight(box))
					DrawRectangleRec(box, PURPLE);
			}

			for (const auto& box : Boxes)
			{
				DrawRectangleLines(box.x,box.y, box.width, box.height, DARKBLUE);
			}


			DrawText("(F1) Hide Shadow Volumes", 0, 0, 20, GREEN);
		}
		else
		{
			DrawText("(F1) Show Shadow Volumes", 0, 0, 20, GREEN);
		}

		DrawFPS(1200, 0);
		DrawText(TextFormat("Shadows %d", (int)light1.Shadows.size()), 1150, 20, 20, GREEN);

		EndDrawing();
	}

	CloseWindow();
	return 0;
}