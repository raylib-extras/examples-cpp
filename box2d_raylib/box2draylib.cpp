/**********************************************************************************************
*
*   raylib-extras, examples-cpp * examples for Raylib in C++
*
*   Box2d example
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

int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    // setup raylib
    InitWindow(screenWidth, screenHeight, "raylib example - Box2d");
	SetTargetFPS(60);               // Set our game to run at 60 frames-per-second

    // setup a camera
    Camera2D cam = { 0 };
    cam.offset.y = GetScreenHeight() - 20.0f;
    cam.offset.x = GetScreenWidth() / 2.0f;
    cam.zoom = 4;

    // setup box2d
    b2Vec2 gravity(0.0f, 10.0f);    // Y+ is down, so gravity is not negative
    b2World world(gravity);
    
    // ground box
    Vector2 groundSize = { 100, 2 };    // save a vector of our ground size, so we know what to draw
    b2BodyDef groundBodyDef;
    groundBodyDef.position.Set(0.0f, 0.0f);
    groundBodyDef.userData.pointer = uintptr_t(&groundSize);
    b2Body* groundBody = world.CreateBody(&groundBodyDef);

    b2PolygonShape groundBox;
    groundBox.SetAsBox(groundSize.x / 2, groundSize.y / 2);
    groundBody->CreateFixture(&groundBox, 0.0f);

    // dynamic shape
    Vector2 boxSize = { 10, 10 };       // save a vector of our box size, so we know what to draw

    b2BodyDef bodyDef;
    bodyDef.type = b2_dynamicBody;
    bodyDef.position.Set(0.0f, -50.0f);
    bodyDef.angle = 30 * DEG2RAD;
    bodyDef.userData.pointer = uintptr_t(&boxSize);
    b2Body* body = world.CreateBody(&bodyDef);

    b2PolygonShape dynamicBox;
    dynamicBox.SetAsBox(boxSize.x/2, boxSize.y/2);

    b2FixtureDef fixtureDef;
    fixtureDef.shape = &dynamicBox;
    fixtureDef.density = 1.0f; 
    fixtureDef.friction = 0.3f;

    body->CreateFixture(&fixtureDef);

    float timeStep = 1.0f / 60.0f;

    int32 velocityIterations = 6;
    int32 positionIterations = 2;

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // apply some forces on space pressed
        if (IsKeyPressed(KEY_SPACE))
        {
            body->ApplyLinearImpulseToCenter(b2Vec2(0, -2000), true);
            body->ApplyAngularImpulse(5000, true);
        }

        // update the world for the new frame
        world.Step(timeStep, velocityIterations, positionIterations);

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

            // ground box
            // get the position and size from box2d
            Vector2* size = (Vector2*)groundBody->GetUserData().pointer;
            b2Vec2 pos = groundBody->GetPosition();
            Rectangle rect = { pos.x - size->x/2, pos.y - size->y/2,size->x,size->y };
            DrawRectangleRec(rect, BROWN);

            // dynamic box
            // get the position, rotation, and size data we saved from box2d
            size = (Vector2*)body->GetUserData().pointer;
            pos = body->GetPosition();
            rect = { pos.x, pos.y,size->x,size->y };
            DrawRectanglePro(rect, Vector2{ size->x/2, size->y/2 },body->GetAngle()* RAD2DEG, RED);
           
            EndMode2D();

        EndDrawing();
    }

    CloseWindow();

    return 0;
}