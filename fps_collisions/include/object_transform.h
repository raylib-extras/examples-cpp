/**********************************************************************************************
*
*   object_transform.h * a sample 3d object transfrom class
*
*   LICENSE: ZLIB
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

#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"

#include <vector>
#include <algorithm>

class ObjectTransform
{
private:
    Vector3 Position = { 0 };
    Quaternion Orientation = QuaternionIdentity();

    bool Dirty = true;

    Matrix WorldMatrix = { 0 };
    Matrix GlWorldMatrix = { 0 };

    ObjectTransform* Parent = nullptr;
    std::vector<ObjectTransform*> Children;

public:

    ObjectTransform(bool faceY = true)
    {
        if (faceY)
            Orientation = QuaternionFromAxisAngle(Vector3{ 0,1,0 }, 0);
    }

    virtual ~ObjectTransform() = default;

    inline ObjectTransform* GetParent() const { return Parent; }

    inline const std::vector<ObjectTransform*>& GetChildren() const { return Children; }

    Quaternion GetOrientation() const { return Orientation; }

    inline ObjectTransform* AddChild(ObjectTransform* child)
    {
        if (!child)
            return nullptr;

        child->Reparent(this);

        return child;
    }

    inline ObjectTransform& AddChild(ObjectTransform& child)
    {
        child.Reparent(this);

        return child;
    }

    inline void Reparent(ObjectTransform* newParent)
    {
        if (Parent == newParent)
            return;

        if (newParent)
        {
            auto child = std::find(newParent->Children.begin(), newParent->Children.end(), this);
            if (child != newParent->Children.end())
                newParent->Children.erase(child);
        }

        Parent = newParent;
        if (Parent)
            Parent->Children.push_back(this);
    }

    inline void Detach()
    {
        if (!GetParent())
            return;

        Matrix worldTransform = GetWorldMatrix();
        Position = Vector3Transform(Vector3Zero(), WorldMatrix);

        Matrix translateMatrix = MatrixTranslate(Position.x, Position.y, Position.z);
        Matrix orientationMatrix = MatrixMultiply(worldTransform, translateMatrix);

        Orientation = QuaternionFromMatrix(WorldMatrix);

        Reparent(nullptr);
    }

    inline void SetDirty()
    {
        Dirty = true;
        for (ObjectTransform* childTransform : Children)
        {
            if (childTransform != nullptr)
                childTransform->SetDirty();
        }
    }

    inline const Vector3& GetPosition() const { return Position; }

    inline Quaternion GetOrientation()
    {
        return Orientation;
    }

    inline Vector3 GetEulerAngles()
    {
        return QuaternionToEuler(Orientation);
    }

    inline Vector3 GetDVector() const
    {
        return Vector3Transform(Vector3{ 0, 0, 1 }, MatrixInvert(QuaternionToMatrix(Orientation)));
    }

    inline Vector3 GeVVector() const
    {
        return Vector3Transform(Vector3{ 0, 1, 0 }, MatrixInvert(QuaternionToMatrix(Orientation)));
    }

    inline Vector3 GetHNegVector()
    {
        return Vector3CrossProduct(GeVVector(), GetDVector());
    }

    inline Vector3 GetHPosVector()
    {
        return Vector3CrossProduct(GetDVector(), GeVVector());
    }

    inline Vector3 GetWorldPosition()
    {
        return Vector3Transform(Vector3Zero(), GetWorldMatrix());
    }

    inline Vector3 GetWorldTargetPoint()
    {
        return Vector3Transform(Vector3{ 0,1,0 }, GetWorldMatrix());
    }

    inline void SetPosition(float x, float y, float z)
    {
        Position.x = x;
        Position.y = y;
        Position.z = z;
        SetDirty();
    }

    inline void MovePosition(float x, float y, float z)
    {
        Position.x += x;
        Position.y += y;
        Position.z += z;
        SetDirty();
    }

    inline void SetPosition(const Vector3& pos)
    {
        Position = pos;
        SetDirty();
    }

    inline void SetOrientation(const Vector3& eulerAngles)
    {
        Vector3 angles = Vector3Scale(eulerAngles, DEG2RAD);
        Orientation = QuaternionFromEuler(angles.x, angles.y, angles.z);
        SetDirty();
    }

    inline bool IsDirty()
    {
        return Dirty;
    }

    inline void LookAt(const Vector3& target, const Vector3& up)
    {
        SetDirty();
        Matrix mat = MatrixLookAt(Position, target, up);
        Orientation = QuaternionFromMatrix(mat);
    }

    inline Matrix GetLocalMatrix()
    {
        Matrix orient = QuaternionToMatrix(Orientation);
        Matrix translation = MatrixTranslate(Position.x, Position.y, Position.z);

        return MatrixMultiply(MatrixInvert(orient), translation);
    }

    inline void UpdateWorldMatrix()
    {
        Matrix parentMatrix = MatrixIdentity();

        if (Parent)
            parentMatrix = Parent->GetWorldMatrix();

        WorldMatrix = MatrixMultiply(GetLocalMatrix(), parentMatrix);
        GlWorldMatrix = MatrixTranspose(WorldMatrix);

        Dirty = false;
    }

    inline const Matrix& GetWorldMatrix()
    {
        if (!IsDirty())
            return WorldMatrix;

        UpdateWorldMatrix();
        return WorldMatrix;
    }

    inline const Matrix& GetGLWorldMatrix()
    {
        if (!IsDirty())
            return GlWorldMatrix;

        UpdateWorldMatrix();
        return GlWorldMatrix;
    }

    inline Vector3 ToLocalPos(const Vector3& inPos)
    {
        return Vector3Transform(inPos, MatrixInvert(GetWorldMatrix()));
    }

    inline void MoveV(float distance)
    {
        SetDirty();
        Position = Vector3Add(Position, Vector3Scale(GeVVector(), distance));
    }

    inline void MoveD(float distance)
    {
        SetDirty();
        Position = Vector3Add(Position, Vector3Scale(GetDVector(), distance));
    }

    inline void MoveH(float distance)
    {
        SetDirty();
        Position = Vector3Add(Position, Vector3Scale(GetHNegVector(), distance));
    }


    inline void SetV(float value)
    {
        SetDirty();
        Position.y = value;
    }

    inline void SetD(float value)
    {
        SetDirty();
        Position.z = value;
    }

    inline void SeteH(float value)
    {
        SetDirty();
        Position.x = value;
    }

    inline void RotateY(float angle)
    {
        SetDirty();
        auto rot = QuaternionFromEuler(0, -angle * DEG2RAD, 0);
        Orientation = QuaternionMultiply(Orientation, rot);
    }

    inline void RotateX(float angle)
    {
        SetDirty();
        auto rot = QuaternionFromEuler(angle * DEG2RAD, 0, 0);
        Orientation = QuaternionMultiply(Orientation, rot);
    }

    inline void RotateZ(float angle)
    {
        SetDirty();
        auto rot = QuaternionFromEuler(0, 0, -angle * DEG2RAD);
        Orientation = QuaternionMultiply(Orientation, rot);
    }

    inline void RotateH(float angle)
    {
        SetDirty();
        auto rot = QuaternionFromEuler(angle * DEG2RAD, 0, 0);
        Orientation = QuaternionMultiply(rot, Orientation);
    }

    inline void RotateV(float angle)
    {
        SetDirty();
        auto rot = QuaternionFromEuler(0, -angle * DEG2RAD, 0);
        Orientation = QuaternionMultiply(rot, Orientation);
    }

    inline void RotateD(float angle)
    {
        SetDirty();
        auto rot = QuaternionFromEuler(0, 0, -angle * DEG2RAD);
        Orientation = QuaternionMultiply(rot, Orientation);
    }

    inline void SetCamera(Camera3D& camera)
    {
        camera.position = Vector3Transform(Vector3Zero(), GetWorldMatrix());
        camera.target = Vector3Transform(Vector3{ 0,0,1 }, WorldMatrix);
        camera.up = Vector3Subtract(Vector3Transform(Vector3{ 0,1,0 }, WorldMatrix), camera.target);
    }

    inline void PushMatrix()
    {
        const Matrix& glMatrix = GetGLWorldMatrix();
        rlPushMatrix();
        rlMultMatrixf((float*)(&glMatrix.m0));
    }

    inline void PopMatrix()
    {
        rlPopMatrix();
    }
};