/*********************************************************************************************
*
*   raylib-extras, FPS collision example
*
*   LICENSE: MIT
*
*   Copyright (c) 2024 Jeffery Myers
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

#pragma once

#include "object_transform.h"
#include "map.h"

struct PlayerInfo
{
    static constexpr float GunDefaultH = -0.4f;
    static constexpr float GunDefaultV = -0.75f;
    static constexpr float GunDefaultD = 1.0f;

    static constexpr float CollisionRadius = 1.5f;

    static constexpr float ReloadTime = 0.1f;
    static constexpr float RecoilDistance = -0.125f;

    ObjectTransform PlayerNode;
    ObjectTransform CameraNode;
    ObjectTransform PivotNode;
    ObjectTransform ShoulderNode;
    ObjectTransform GunNode;

    Camera3D ViewCamera = { 0 };

    float TitltAngle = 0;

    float DesiredPullback = 10;

    Vector3 DesiredMovement = { 0 };

    float Speed = 30.0f;

    bool HitLastFrame = false;

    bool HitCameraLastFrame = false;

    float BobbleTime = 0;

    RayCollision LastGunCollision = { 0 };
    RayCollision LastCameraCollision = { 0 };

    float Reload = 0;

    static void SetupGraphics();
    static void CleanupGraphics();

    void Setup();

    void Update(Map& map);

    void Draw();
};