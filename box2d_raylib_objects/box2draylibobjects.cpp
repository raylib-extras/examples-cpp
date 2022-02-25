/**********************************************************************************************
*
*   raylib-extras, examples-cpp * examples for Raylib in C++
*
*   Box2d advanced object example
*
*   LICENSE: MIT
*
*   Copyright (c) 2022 Jeffery Myers
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
#include "rlgl.h"
#include "box2d/box2d.h"

#include <vector>

// setup box2d
b2Vec2 Gravity(0.0f, 20.0f);    // Y+ is down, so gravity is not negative
b2World World(Gravity);

class PhysicsObject
{
public:

    b2Body* RigidBody = nullptr; 
    Color ShapeColor = RED;

    virtual void Draw() = 0;
};

class BoxObject : public PhysicsObject
{
public:

    Vector2 Size = { 0,0 };
    Vector2 HalfSize = { 0,0 };

    b2PolygonShape Box;

    BoxObject(Vector2 pos, Vector2 size, Color c, float angle = 0, bool isDynamic = true)
    {
        ShapeColor = c;
        Size = size;
        HalfSize.x = size.x / 2;
        HalfSize.y = size.y / 2;

        b2BodyDef bodyDef;
        bodyDef.type = isDynamic ? b2_dynamicBody : b2_staticBody;
        bodyDef.position.Set(pos.x,pos.y);
        bodyDef.angle = angle * DEG2RAD;
        bodyDef.userData.pointer = uintptr_t(this);
        RigidBody = World.CreateBody(&bodyDef);

        Box.SetAsBox(Size.x / 2, Size.y / 2);

        b2FixtureDef fixtureDef;
        fixtureDef.shape = &Box;
        fixtureDef.density = 1.0f;
        fixtureDef.friction = 0.3f;

        RigidBody->CreateFixture(&fixtureDef);
    }

    void Draw() override
    {
        DrawRectanglePro(Rectangle{ RigidBody->GetPosition().x,RigidBody->GetPosition().y, Size.x, Size.y }, HalfSize, RigidBody->GetAngle() * RAD2DEG, ShapeColor);
    }
};

class BallObject : public PhysicsObject
{
public:
    float Radius = 0;

    b2CircleShape CircleShape;

    BallObject(Vector2 pos, float radius, Color c)
    {
        Radius = radius;
        ShapeColor = c;

        b2BodyDef bodyDef;
        bodyDef.type = b2_dynamicBody;
        bodyDef.position.Set(pos.x, pos.y);
        bodyDef.userData.pointer = uintptr_t(this);
        RigidBody = World.CreateBody(&bodyDef);
        
        CircleShape.m_radius = radius;

        b2FixtureDef fixtureDef;
        fixtureDef.shape = &CircleShape;
        fixtureDef.density = 1.0f;
        fixtureDef.friction = 0.3f;

        RigidBody->CreateFixture(&fixtureDef);
    }

    void Draw() override
    {
        DrawCircleV(Vector2{ RigidBody->GetPosition().x,RigidBody->GetPosition().y }, Radius, ShapeColor);
    }
};


std::vector<PhysicsObject*> Objects;

int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    // setup raylib
    InitWindow(screenWidth, screenHeight, "raylib example - Box2d Objects");
	SetTargetFPS(60);               // Set our game to run at 60 frames-per-second

    // setup a camera
    Camera2D cam = { 0 };
    cam.offset.y = GetScreenHeight() - 20.0f;
    cam.offset.x = GetScreenWidth() / 2.0f;
    cam.zoom = 4;


    BoxObject ground(Vector2{ 0,0 }, Vector2{ 200, 2 }, BROWN, 0, false);
    
    // ground box
    Vector2 groundSize = { 100, 2 };    // save a vector of our ground size, so we know what to draw

    Objects.push_back(new BoxObject(Vector2{ 0,-50 }, Vector2{ 10,10 }, RED, 45));
    Objects.push_back(new BoxObject(Vector2{ 20,-60 }, Vector2{ 12,8 }, GREEN, 30));
    Objects.push_back(new BoxObject(Vector2{ -20,-60 }, Vector2{ 8,12 }, BLUE , -30));

    Objects.push_back(new BallObject(Vector2{ 0, -80 },3, PURPLE));

    float timeStep = 1.0f / 60.0f;

    int32 velocityIterations = 6;
    int32 positionIterations = 2;

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        Vector2 pos = GetScreenToWorld2D(GetMousePosition(), cam);
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            Objects.push_back(new BallObject(pos, 3, ORANGE));
        }
        if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
        {
            Objects.push_back(new BoxObject(pos, Vector2{ float(GetRandomValue(10,20)),float(GetRandomValue(10,20)) }, MAROON, GetRandomValue(0, 90)));
        }

        // update the world for the new frame
        World.Step(timeStep, velocityIterations, positionIterations);

        BeginDrawing();
            ClearBackground(RAYWHITE);

            // set our camera 
            BeginMode2D(cam);

            // draw a grid
            rlPushMatrix();
            rlRotatef(90, 1, 0, 0);
            DrawGrid(50, 10);
            rlPopMatrix();

            // and some axes
            DrawLine(0, 0, 100, 0, RED);
            DrawLine(0, 0, 0, -100, BLUE);

            ground.Draw();
           
            for (auto object : Objects)
                object->Draw();
           
            EndMode2D();

            DrawFPS(0, 0);
            DrawText("Click to add a ball", 100, 0, 20, BLACK);
        EndDrawing();
    }

    for (auto object : Objects)
        delete(object);

    CloseWindow();

    return 0;
}