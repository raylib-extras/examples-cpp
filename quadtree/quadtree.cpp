#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"

#include <vector>

Camera2D TheCamera = { 0 };
float Radius = 300;

class QuadTreeNode 
{
public:
	Rectangle Bounds;

	QuadTreeNode* Chidren[4] = { 0 };

	bool ContentValid = false;
	Vector2 Contents;

	bool WasSplit = false;

	bool Split()
	{
		if (Chidren[0] != nullptr)
			return true;

		if (Bounds.width <= 1)
			return false;

		WasSplit = true;
		Chidren[0] = new QuadTreeNode(Rectangle{Bounds.x,Bounds.y,Bounds.width / 2,Bounds.height / 2} );
		Chidren[1] = new QuadTreeNode(Rectangle{ Bounds.x + Chidren[0]->Bounds.width,Bounds.y,Bounds.width / 2,Bounds.height / 2 });
		Chidren[2] = new QuadTreeNode(Rectangle{ Bounds.x,Bounds.y + Chidren[0]->Bounds.height,Bounds.width / 2,Bounds.height / 2 });
		Chidren[3] = new QuadTreeNode(Rectangle{ Bounds.x + Chidren[0]->Bounds.width,Bounds.y + Chidren[0]->Bounds.height,Bounds.width / 2,Bounds.height / 2 });

		return true;
	}

	void AddChildPoint(int child, const Vector2& point)
	{
		if (CheckCollisionPointRec(point, Chidren[child]->Bounds))
			Chidren[child]->AddPoint(point);
	}

	void AddPoint(const Vector2& point)
	{
		if (WasSplit || ContentValid)
		{
			if (!Split())
				return; // discard the point the node would be too small

			for (int i = 0; i < 4; i++)
			{
				AddChildPoint(i, point);
				if (ContentValid)
					AddChildPoint(i, Contents);
			}

			ContentValid = false;
		}
		else
		{
			ContentValid = true;
			Contents = point;
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

bool RectFullyInRadius(const Rectangle& rect)
{
	if (!CheckCollisionPointCircle(Vector2{ rect.x, rect.y }, TheCamera.target, Radius))
		return false;

	if (!CheckCollisionPointCircle(Vector2{ rect.x + rect.width, rect.y }, TheCamera.target, Radius))
		return false;

	if (!CheckCollisionPointCircle(Vector2{ rect.x, rect.y + rect.height }, TheCamera.target, Radius))
		return false;

	if (!CheckCollisionPointCircle(Vector2{ rect.x + rect.width, rect.y + rect.height }, TheCamera.target, Radius))
		return false;

	return true;
}

void DrawQuadTreeContents(QuadTreeNode* node, bool drawPoints, bool drawRects)
{
	if (node == nullptr)
		return;

	if (drawPoints)
	DrawCircleV(node->Contents, 2, DARKGRAY);

	if (drawRects)
	{
		if (!node->WasSplit && node->ContentValid)
		{
			DrawRectangleLinesEx(node->Bounds, 2, ColorAlpha(GREEN, 0.25f));
		}
		else
		{
			DrawRectangleLinesEx(node->Bounds, 1, ColorAlpha(GRAY, 0.25f));
		}
	}
		
	for (int i = 0; i < 4; i++)
		DrawQuadTreeContents(node->Chidren[i], drawPoints, drawRects);
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

std::vector<Vector2*> VissiblePoints;
std::vector<Rectangle*> VisibleRects;


void AddAllContents(QuadTreeNode* node)
{
	if (node == nullptr)
		return;
	if (node->ContentValid)
		VissiblePoints.push_back(&node->Contents);
	if (node->WasSplit)
	{
		for (int i = 0; i < 4; i++)
			AddAllContents(node->Chidren[i]);
	}
}

void FindRectsINRadius(QuadTreeNode* node)
{
	if (node == nullptr)
		return;

	if (CheckCollisionCircleRec(TheCamera.target, Radius, node->Bounds))
	{
		if ( RectFullyInRadius(node->Bounds) )
		{
			VisibleRects.push_back(&node->Bounds);
			AddAllContents(node);
		}
		else
		{
 			if (node->ContentValid)
 			{
 				VisibleRects.push_back(&node->Bounds);
				VissiblePoints.push_back(&node->Contents);
 			}
 			else
			if (node->WasSplit)
 			{
				for (int i = 0; i < 4; i++)
					FindRectsINRadius(node->Chidren[i]);
			}
		}
	}
}

int main()
{
	InitWindow(1280, 800, "Quadtree");
	SetTargetFPS(144);

	TheCamera.zoom = 1;
	TheCamera.offset.x = GetScreenWidth() / 2.0f;
	TheCamera.offset.y = GetScreenHeight() / 2.0f;

	for (int i = 0; i < 500; i++)
		QuadTreeRoot.AddPoint(Vector2{(float)GetRandomValue(QuadTreeRoot.Bounds.x,QuadTreeRoot.Bounds.x + QuadTreeRoot.Bounds.width), (float)GetRandomValue(QuadTreeRoot.Bounds.y,QuadTreeRoot.Bounds.y + QuadTreeRoot.Bounds.height) });

	bool DrawQuadTree = false;
	bool DrawAllPoints = false;
	bool DrawVisRects = false;

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

		if (IsKeyPressed(KEY_F1))
			DrawQuadTree = !DrawQuadTree;

		if (IsKeyPressed(KEY_F2))
			DrawAllPoints = !DrawAllPoints;

		if (IsKeyPressed(KEY_F3))
			DrawVisRects = !DrawVisRects;

		if (TheCamera.zoom < 0.01f)
			TheCamera.zoom = 0.01f;

		VisibleRects.clear();
		VissiblePoints.clear();
		FindRectsINRadius(&QuadTreeRoot);

		BeginDrawing();
		ClearBackground(BLACK);

		BeginMode2D(TheCamera);

		DrawLine(-20, 0, 20, 0, RED);
		DrawLine(0, -20, 0, 20, GREEN);
		DrawCircle(0, 0, 10, ColorAlpha(WHITE, 0.25f));

		DrawCircleV(TheCamera.target, 5, BLUE);
		DrawCircleLinesV(TheCamera.target, Radius, ColorAlpha(SKYBLUE, 0.5f));

		DrawRectangleLinesEx(QuadTreeRoot.Bounds, 3, WHITE);

		if (DrawQuadTree || DrawAllPoints)
			DrawQuadTreeContents(&QuadTreeRoot, DrawAllPoints, DrawQuadTree);

		if (DrawVisRects)
		{
			for (Rectangle* rect : VisibleRects)
			{
				DrawRectangleLinesEx(*rect, 2, RED);
			}
		}

		for (Vector2* pos : VissiblePoints)
		{
			DrawCircleV(*pos, 4, YELLOW);
		}

		EndMode2D();

		if (DrawQuadTree)
			DrawText("QUAD TREE Shown(F1)", 0, 0, 20, LIGHTGRAY);
		else
			DrawText("QUAD TREE Hidden(F1)", 0, 0, 20, LIGHTGRAY);

		if (DrawAllPoints)
			DrawText("ALL POINTS Shown(F2)", 400, 0, 20, LIGHTGRAY);
		else
			DrawText("ALL POINTS Hidden(F2)", 400, 0, 20, LIGHTGRAY);

		if (DrawVisRects)
			DrawText("VIS RECTS Shown(F3)", 700, 0, 20, LIGHTGRAY);
		else
			DrawText("VIS RECTS Hidden(F3)", 700, 0, 20, LIGHTGRAY);

		DrawFPS(1000, 0);
		EndDrawing();
	}

	CloseWindow();
}