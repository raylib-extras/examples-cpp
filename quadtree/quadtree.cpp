#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"

#include <vector>

Camera2D TheCamera = { 0 };

class QuadTreeNode 
{
public:
	Rectangle Bounds;

	QuadTreeNode* Chidren[4] = { 0 };

	std::vector<Vector2> Contents;

	void Split()
	{
		if (Chidren[0] != nullptr)
			return;

		Chidren[0] = new QuadTreeNode(Rectangle{Bounds.x,Bounds.y,Bounds.width / 2,Bounds.height / 2} );
		Chidren[1] = new QuadTreeNode(Rectangle{ Bounds.x + Chidren[0]->Bounds.width,Bounds.y,Bounds.width / 2,Bounds.height / 2 });
		Chidren[2] = new QuadTreeNode(Rectangle{ Bounds.x,Bounds.y + Chidren[0]->Bounds.height,Bounds.width / 2,Bounds.height / 2 });
		Chidren[3] = new QuadTreeNode(Rectangle{ Bounds.x + Chidren[0]->Bounds.width,Bounds.y + Chidren[0]->Bounds.height,Bounds.width / 2,Bounds.height / 2 });
	}

	void AddChildPoint(int child, const Vector2& point)
	{
		if (CheckCollisionPointRec(point, Chidren[child]->Bounds))
			Chidren[child]->AddPoint(point);
	}

	void AddPoint(const Vector2& point)
	{
		if (Contents.size() != 0)
		{
			Split();

			for (int i = 0; i < 4; i++)
			{
				AddChildPoint(i, point);
				AddChildPoint(i, Contents[0]);
			}

			Contents.clear();
		}
		else
		{
			Contents.push_back(point);
		}
	}

	QuadTreeNode(const Rectangle& bounds)
	{
		Bounds = bounds;
	}

	~QuadTreeNode()
	{
		for (int i = 0; i < 4; i++)
		{
			if (Chidren[i] != nullptr)
				delete(Chidren[i]);
		}
	}
};

QuadTreeNode QuadTreeRoot(Rectangle{ -1000,-1000,2000,2000 });


void DrawQuadTreeContents(QuadTreeNode* node)
{
	if (node == nullptr)
		return;

	for (auto& pt : node->Contents)
		DrawCircleV(pt, 2, YELLOW);

	for (int i = 0; i < 4; i++)
		DrawQuadTreeContents(node->Chidren[i]);

	DrawRectangleLinesEx(node->Bounds, 1, ColorAlpha(GREEN, 0.25f));
}

// Draw circle outline
void DrawCircleLinesV(Vector2 center, float radius, Color color)
{
	rlCheckRenderBatchLimit(2 * 36);

	rlBegin(RL_LINES);
	rlColor4ub(color.r, color.g, color.b, color.a);

	// NOTE: Circle outline is drawn pixel by pixel every degree (0 to 360)
	for (int i = 0; i < 360; i += 10)
	{
		rlVertex2f(center.x + sinf(DEG2RAD * i) * radius, center.y + cosf(DEG2RAD * i) * radius);
		rlVertex2f(center.x + sinf(DEG2RAD * (i + 10)) * radius, center.y + cosf(DEG2RAD * (i + 10)) * radius);
	}
	rlEnd();
}

int main()
{
	InitWindow(1280, 800, "Quadtree");
	SetTargetFPS(144);

	TheCamera.zoom = 1;
	TheCamera.offset.x = GetScreenWidth() / 2.0f;
	TheCamera.offset.y = GetScreenHeight() / 2.0f;

	for (int i = 0; i < 100; i++)
		QuadTreeRoot.AddPoint(Vector2{(float)GetRandomValue(-900,900), (float)GetRandomValue(-900,900) });

	while (!WindowShouldClose())
	{
		float speed = 400 * GetFrameTime();
		if (IsKeyDown(KEY_W))
			TheCamera.target.y -= speed;
		if (IsKeyDown(KEY_S))
			TheCamera.target.y += speed;
		if (IsKeyDown(KEY_A))
			TheCamera.target.x -= speed;
		if (IsKeyDown(KEY_D))
			TheCamera.target.x += speed;
		if (IsKeyDown(KEY_UP))
			TheCamera.zoom += GetFrameTime();
		if (IsKeyDown(KEY_DOWN))
			TheCamera.zoom -= GetFrameTime();

		if (TheCamera.zoom < 0.01f)
			TheCamera.zoom = 0.01f;

		BeginDrawing();
		ClearBackground(BLACK);

		BeginMode2D(TheCamera);

		DrawLine(-20, 0, 20, 0, RED);
		DrawLine(0, -20, 0, 20, GREEN);
		DrawCircle(0, 0, 10, ColorAlpha(WHITE, 0.25f));

		DrawCircleV(TheCamera.target, 5, BLUE);
		DrawCircleLinesV(TheCamera.target, 300, ColorAlpha(SKYBLUE, 0.5f));

		DrawRectangleLinesEx(QuadTreeRoot.Bounds, 3, WHITE);

		DrawQuadTreeContents(&QuadTreeRoot);

		EndMode2D();

		EndDrawing();
	}

	CloseWindow();
}