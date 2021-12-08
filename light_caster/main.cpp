#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"

// constants from OpenGL
#define GL_SRC_ALPHA 0x0302
#define GL_MIN 0x8007
#define GL_MAX 0x8008

#include <vector>


// Draw a gradient-filled circle
// NOTE: Gradient goes from inner radius (color1) to border (color2)
void DrawLightGradient(int centerX, int centerY, float innerRadius, float outterRadius, Color color1, Color color2)
{
	rlCheckRenderBatchLimit(3 * 3 * 36);

	if (innerRadius == 0)
	{
		DrawCircleGradient(centerX, centerY, outterRadius, color1, color2);
		return;
	}

	rlBegin(RL_TRIANGLES);
	for (int i = 0; i < 360; i += 10)
	{
		// inner triangle at color1
		rlColor4ub(color1.r, color1.g, color1.b, color1.a);
		rlVertex2f((float)centerX, (float)centerY);
		rlVertex2f((float)centerX + sinf(DEG2RAD * i) * innerRadius, (float)centerY + cosf(DEG2RAD * i) * innerRadius);
		rlVertex2f((float)centerX + sinf(DEG2RAD * (i + 10)) * innerRadius, (float)centerY + cosf(DEG2RAD * (i + 10)) * innerRadius);

		if (outterRadius > innerRadius)
		{
			rlVertex2f((float)centerX + sinf(DEG2RAD * (i + 10)) * innerRadius, (float)centerY + cosf(DEG2RAD * (i + 10)) * innerRadius);
			rlVertex2f((float)centerX + sinf(DEG2RAD * i) * innerRadius, (float)centerY + cosf(DEG2RAD * i) * innerRadius);
			rlColor4ub(color2.r, color2.g, color2.b, color2.a);
			rlVertex2f((float)centerX + sinf(DEG2RAD * i) * outterRadius, (float)centerY + cosf(DEG2RAD * i) * outterRadius);

			rlColor4ub(color1.r, color1.g, color1.b, color1.a);
			rlVertex2f((float)centerX + sinf(DEG2RAD * (i + 10)) * innerRadius, (float)centerY + cosf(DEG2RAD * (i + 10)) * innerRadius);
			rlColor4ub(color2.r, color2.g, color2.b, color2.a);
			rlVertex2f((float)centerX + sinf(DEG2RAD * i) * outterRadius, (float)centerY + cosf(DEG2RAD * i) * outterRadius);
			rlVertex2f((float)centerX + sinf(DEG2RAD * (i + 10)) * outterRadius, (float)centerY + cosf(DEG2RAD * (i + 10)) * outterRadius);
		}

	}
	rlEnd();
}

class LightInfo
{
public:
	Vector2 Position = { 0,0 };

	RenderTexture LightMask;

	bool Valid = false;

	LightInfo()
	{
		LightMask = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());
		UpdateLightMask();
	}

	LightInfo(const Vector2& pos)
	{
		LightMask = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());
		UpdateLightMask();
		Position = pos;
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
		float extension = OuterRadius*2;

		Vector2 spVector = Vector2Normalize(Vector2Subtract(sp, Position));
		Vector2 spProjection = Vector2Add(sp, Vector2Scale(spVector, extension));

		Vector2 epVector = Vector2Normalize(Vector2Subtract(ep, Position));
		Vector2 epProjection = Vector2Add(ep, Vector2Scale(epVector, extension));

		std::vector<Vector2> polygon;
		polygon.push_back(sp);
		polygon.push_back(ep);
		polygon.push_back(epProjection);
		polygon.push_back(spProjection);

		Shadows.push_back(polygon);
	}

	void UpdateLightMask()
	{
		BeginTextureMode(LightMask);

		ClearBackground(WHITE);

 		// force the blend mode to only set the alpha of the destination
 		rlSetBlendFactors(GL_SRC_ALPHA, GL_SRC_ALPHA, GL_MIN);
 		rlSetBlendMode(BLEND_CUSTOM);

		if (Valid)
			DrawLightGradient(Position.x, Position.y, InnerRadius, OuterRadius, ColorAlpha(WHITE,0), WHITE);
		rlDrawRenderBatchActive();
		rlSetBlendMode(BLEND_ALPHA);

		rlSetBlendFactors(GL_SRC_ALPHA, GL_SRC_ALPHA, GL_MAX);
		rlSetBlendMode(BLEND_CUSTOM);

		for (std::vector<Vector2> shadow : Shadows)
		{
			DrawTriangleFan(&shadow[0], 4, WHITE);
		}

		rlDrawRenderBatchActive();
		// go back to normal
		rlSetBlendMode(BLEND_ALPHA);

		EndTextureMode();
	}

	void Update(const std::vector<Rectangle>& boxes)
	{
		if (!Dirty)
			return;

		Dirty = false;
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

			// compute shadow volumes for the faces we are opposite to
			// top
			Vector2 sp = { box.x, box.y };
			Vector2 ep = { box.x + box.width, box.y };

			if (Position.y > ep.y)
				ShadowEdge(sp, ep);

			// right
			sp = ep;
			ep.y += box.height;
			if (Position.x < ep.x)
				ShadowEdge(sp, ep);

			// bottom
			sp = ep;
			ep.x -= box.width;
			if (Position.y < ep.y)
				ShadowEdge(sp, ep);

			// left
			sp = ep;
			ep.y -= box.height;
			if (Position.x > ep.x)
				ShadowEdge(sp, ep);


			// add the actual box as a shadow to get the corner of it.
			// If the map is going to draw the box, then don't do this
			std::vector<Vector2> polygon;
			polygon.emplace_back(Vector2{ box.x, box.y });
			polygon.emplace_back(Vector2{ box.x, box.y + box.height });
			polygon.emplace_back(Vector2{ box.x + box.width, box.y + box.height });
			polygon.emplace_back(Vector2{ box.x + box.width, box.y });

			Shadows.push_back(polygon);
		}

		Valid = true;
		UpdateLightMask();
	}


	float OuterRadius = 200;
	float InnerRadius = 50;
	Rectangle Bounds = { -150,-150,300,300 };

	std::vector<std::vector<Vector2>> Shadows;

	bool Dirty = true;
};

std::vector<Rectangle> Boxes;

Rectangle RandomBox()
{
	float x = GetRandomValue(0, GetScreenWidth());
	float y = GetRandomValue(0, GetScreenHeight());

	float w = GetRandomValue(10,100);
	float h = GetRandomValue(10,100);

	return Rectangle{ x,y,w,h };
}

void SetupBoxes(const Vector2& startPos)
{
	Boxes.emplace_back(Rectangle{ 50,50, 40, 40 });
	Boxes.emplace_back(Rectangle{ 1200, 700, 40, 40 });
	Boxes.emplace_back(Rectangle{ 200, 600, 40, 40 });
	Boxes.emplace_back(Rectangle{ 1000, 50, 40, 40 });
	Boxes.emplace_back(Rectangle{ 500, 350, 40, 40 });

	for (int i = 0; i < 50; i++)
	{
		Rectangle rect = RandomBox();
		while (CheckCollisionPointRec(startPos,rect))
			rect = RandomBox();

		Boxes.emplace_back(rect);
	}
}

int main()
{
	SetConfigFlags(/*FLAG_VSYNC_HINT ||*/ FLAG_MSAA_4X_HINT);
	InitWindow(1280, 800, "LightCaster");
//	SetTargetFPS(144);


	RenderTexture LightMask = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());

	std::vector<LightInfo> Lights;

	Lights.emplace_back();
	Lights[0].Move(Vector2{ 600, 400 });

	SetupBoxes(Lights[0].Position);

	Image img = GenImageChecked(64, 64, 32, 32, DARKBROWN, DARKGRAY);
	Texture2D tile = LoadTextureFromImage(img);
	UnloadImage(img);

	bool showLines = false;
	while (!WindowShouldClose())
	{
		if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
			Lights[0].Move(GetMousePosition());

		if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
			Lights.emplace_back(GetMousePosition());

		float delta = GetMouseWheelMove();
		if (delta != 0)
		{
			float newRad = Lights[0].OuterRadius;
			newRad += delta * 10;
			if (newRad > Lights[0].InnerRadius)
				Lights[0].SetRadius(newRad);
		}

		if (IsKeyPressed(KEY_F1))
			showLines = !showLines;

		bool dirtyLights = false;
		for (auto& light : Lights)
		{
			if (light.Dirty)
				dirtyLights = true;

			light.Update(Boxes);
		}

		// update the light mask
		if (dirtyLights)
		{
			// build up the light mask
			BeginTextureMode(LightMask);
			ClearBackground(BLACK);

			// force the blend mode to only set the alpha of the destination
			rlSetBlendFactors(GL_SRC_ALPHA, GL_SRC_ALPHA, GL_MIN);
			rlSetBlendMode(BLEND_CUSTOM);

			for (auto& light : Lights)
			{
				//	if (light.Valid)
				DrawTextureRec(light.LightMask.texture, Rectangle{ 0, 0, (float)GetScreenWidth(), -(float)GetScreenHeight() }, Vector2Zero(), WHITE);
			}

			rlDrawRenderBatchActive();
			// go back to normal
			rlSetBlendMode(BLEND_ALPHA);
			EndTextureMode();
		}

		BeginDrawing();
		ClearBackground(BLACK);

		DrawTextureRec(tile, Rectangle{ 0,0,(float)GetScreenWidth(),(float)GetScreenHeight() }, Vector2Zero(), WHITE);

		DrawTextureRec(LightMask.texture, Rectangle{ 0, 0, (float)GetScreenWidth(), -(float)GetScreenHeight() }, Vector2Zero(), ColorAlpha(WHITE, showLines ? 0.75f : 1.0f));

		for (auto& light : Lights)
			DrawCircle(int(light.Position.x), int(light.Position.y), 10, YELLOW);

		if (showLines)
		{
			for (std::vector<Vector2> shadow : Lights[0].Shadows)
			{
				DrawTriangleFan(&shadow[0], 4, DARKPURPLE);
			}

			for (const auto& box : Boxes)
			{
				if (Lights[0].BoxInLight(box))
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
		DrawText(TextFormat("Lights %d", (int)Lights.size()), 1050, 20, 20, GREEN);

		EndDrawing();
	}

	CloseWindow();
	return 0;
}