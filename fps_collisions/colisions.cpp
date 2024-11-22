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

#include <limits>

#include "collisions.h"
#include "raylib.h"
#include "raymath.h"

// check if a cylinder hits a bounding box
bool IntersectBBoxCylinder(BoundingBox bounds, Vector3& center, Vector3 initalPosition, float radius, float height, Vector3& intersectionPoint, Vector3& hitNormal)
{
    Rectangle rect = { bounds.min.x, bounds.min.z, bounds.max.x - bounds.min.x, bounds.max.z - bounds.min.z };
    Vector2 center2d = { center.x, center.z };

    if (!CheckCollisionCircleRec(center2d, radius, rect))
        return false;

    // we are above or below
    if (center.y > bounds.max.y)
        return false;

    if (center.y + height < bounds.min.y)
        return false;


    // see if we landed on top
    if (center.y <= bounds.max.y && initalPosition.y > bounds.max.y && initalPosition.y > center.y)
    {
        // we have hit the top of the obstacle, so clamp our position to where we hit that Y
        Vector3 movementVec = Vector3Subtract(center, initalPosition);

        float yParam = (initalPosition.y - bounds.max.y) / movementVec.y;
        movementVec = Vector3Scale(movementVec, yParam);
        center = Vector3Add(initalPosition, movementVec);
        intersectionPoint = center;
        hitNormal = Vector3{ 0,1,0 };
        return true;
    }

    // see if we hit the bottom
    float centerTop = center.y + height;
    float oldTop = initalPosition.y + height;

    if (centerTop >= bounds.min.y && oldTop < bounds.min.y && initalPosition.y < center.y)
    {
        // simple situation of 
        center = initalPosition;
        intersectionPoint = center;
        hitNormal = Vector3{ 0,-1,0 };
        return true;
    }

    Vector2 newPosOrigin = { center.x, center.z };
    Vector2 hitPoint = { std::numeric_limits<float>::min(), std::numeric_limits<float>::min() };
    Vector2 hitNormal2d = { 0 };

    PointNearestRectanglePoint(rect, newPosOrigin, hitPoint, hitNormal2d);

    Vector2 vectorToHit = Vector2Subtract(hitPoint, newPosOrigin);

    if (Vector2LengthSqr(vectorToHit) >= radius * radius)
        return false;

    intersectionPoint = Vector3{ hitPoint.x, center.y, hitPoint.y };
    hitNormal = Vector3{ hitNormal2d.x, 0, hitNormal2d.y };

    // normalize the vector along the point to where we are nearest
    vectorToHit = Vector2Normalize(vectorToHit);

    // project that out to the radius to find the point that should be 'deepest' into the rectangle.
    Vector2 projectedPoint = Vector2Add(newPosOrigin, Vector2Scale(vectorToHit, radius));

    // compute the shift to take the deepest point out to the edge of our nearest hit, based on the vector direction
    Vector2 delta = { 0,0 };

    if (hitNormal.x != 0)
        delta.x = hitPoint.x - projectedPoint.x;
    else
        delta.y = hitPoint.y - projectedPoint.y;

    // shift the new point by the delta to push us outside of the rectangle
    newPosOrigin = Vector2Add(newPosOrigin, delta);

    center = Vector3{ newPosOrigin.x, center.y, newPosOrigin.y };
    return true;
}

/// <summary>
/// Returns the point on a rectangle that is nearest to a provided point
/// </summary>
/// <param name="rect">The rectangle to test against</param>
/// <param name="point">The point you want to start from</param>
/// <param name="nearest">A pointer that will be filed out with the point on the rectangle that is nearest to your passed in point</param>
/// <param name="normal">A pointer that will be filed out with the the normal of the edge the nearest point is on</param>
void PointNearestRectanglePoint(Rectangle rect, Vector2 point, Vector2& nearest, Vector2& normal)
{
    // get the closest point on the vertical sides
    float hValue = rect.x;
    float hNormal = -1;
    if (point.x > rect.x + rect.width)
    {
        hValue = rect.x + rect.width;
        hNormal = 1;
    }

    Vector2 vecToPoint = Vector2Subtract(Vector2{ hValue, rect.y }, point);
    // get the dot product between the ray and the vector to the point
    float dotForPoint = Vector2DotProduct(Vector2{ 0, -1 }, vecToPoint);
    Vector2 nearestPoint = { hValue, 0 };

    if (dotForPoint < 0)
        nearestPoint.y = rect.y;
    else if (dotForPoint >= rect.height)
        nearestPoint.y = rect.y + rect.height;
    else
        nearestPoint.y = rect.y + dotForPoint;

    // get the closest point on the horizontal sides
    float vValue = rect.y;
    float vNormal = -1;
    if (point.y > rect.y + rect.height)
    {
        vValue = rect.y + rect.height;
        vNormal = 1;
    }

    vecToPoint = Vector2Subtract(Vector2{ rect.x, vValue }, point);
    // get the dot product between the ray and the vector to the point
    dotForPoint = Vector2DotProduct(Vector2{ -1, 0 }, vecToPoint);
    nearest = Vector2{ 0,vValue };

    if (dotForPoint < 0)
        nearest.x = rect.x;
    else if (dotForPoint >= rect.width)
        nearest.x = rect.x + rect.width;
    else
        nearest.x = rect.x + dotForPoint;

    if (Vector2LengthSqr(Vector2Subtract(point, nearestPoint)) <= Vector2LengthSqr(Vector2Subtract(point, nearest)))
    {
        nearest = nearestPoint;
        normal.x = hNormal;
        normal.y = 0;
    }
    else
    {
        normal.y = vNormal;
        normal.x = 0;
    }
}