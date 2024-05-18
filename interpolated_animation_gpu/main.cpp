/**********************************************************************************************
*
*   raylib-extras, Interpolated Animation using the GPU
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

#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include "external/glad.h"

#define RLIGHTS_IMPLEMENTATION
#include "rlights.h"

#include <vector>


// utility functions

// upload an array of matrices
void SetUniformMatrixTransposeV(Shader& shader, int locIndex, Matrix* mats, int count)
{
    rlEnableShader(shader.id);
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    glUniformMatrix4fv(locIndex, count, true, &mats[0].m0);
#endif
}

// Unload mesh from VRAM only, keep RAM data.
void UnloadMeshNoFree(Mesh& mesh)
{
    // just unload the GPU buffers
    rlUnloadVertexArray(mesh.vaoId);

    if (mesh.vboId != NULL)
    {
        for (int i = 0; i < 7; i++)
        {
            rlUnloadVertexBuffer(mesh.vboId[i]);
        }
    }

    RL_FREE(mesh.vboId);

    mesh.vboId = nullptr;
    mesh.vaoId = 0;
}


// animation classes
class AnimationShader
{
public:
    static constexpr int MaxBones = 64;

    Shader BaseShader = { 0 };

    std::vector<int> ShaderBoneLocations;

    void Setup(Shader& shader)
    {
        BaseShader = shader;

        ShaderBoneLocations.resize(MaxBones);

        for (int i = 0; i < MaxBones; i++)
        {
            ShaderBoneLocations[i] = GetShaderLocation(BaseShader, TextFormat("bones[%d]", i));

            SetShaderValueMatrix(BaseShader, ShaderBoneLocations[i], MatrixIdentity());
        }
    }

};

// global animation shader

AnimationShader AnimShader;

class GPUAnimatedModelInstance;

class GPUAnimatedModel
{
public:
    Model BaseModel;

    std::vector<ModelAnimation> Animations;

    void Setup(Model model)
    {
        BaseModel = model;

        for (int i = 0; i < BaseModel.meshCount; i++)
        {
            Mesh& mesh = BaseModel.meshes[i];

            // unload the old mesh, because we are going to upload a new one with more buffers.
            UnloadMeshNoFree(mesh);

            mesh.tangents = (float*)MemAlloc(sizeof(float) * 4 * mesh.vertexCount);
            memcpy(mesh.tangents, mesh.boneWeights, sizeof(float) * 4 * mesh.vertexCount);

            mesh.texcoords2 = (float*)MemAlloc(sizeof(float) * 2 * mesh.vertexCount);
            for (int vert = 0; vert < mesh.vertexCount; vert++)
            {
                float u = mesh.boneIds[vert * 4 + 0] * 1000.0f + mesh.boneIds[vert * 4 + 1];
                float v = mesh.boneIds[vert * 4 + 2] * 1000.0f + mesh.boneIds[vert * 4 + 3];
                mesh.texcoords2[vert * 2 + 0] = u;
                mesh.texcoords2[vert * 2 + 1] = v;
            }
            UploadMesh(&mesh, false);
        }

        for (int shader = 0; shader < BaseModel.materialCount; shader++)
            BaseModel.materials[shader].shader = AnimShader.BaseShader;
    }

    void Cleanup()
    {
        for (auto& anim : Animations)
        {
            UnloadModelAnimation(anim);
        }
        Animations.clear();

        UnloadModel(BaseModel);
    }

    void AddAnimation(ModelAnimation* animations, size_t count)
    {
        for (size_t i = 0; i < count; i++)
        {
            Animations.push_back(animations[i]);
        }
    }
};

class GPUAnimatedModelInstance
{
public:
    GPUAnimatedModel& SourceModel;

    Vector3 Position = { 0,0,0 };
    Quaternion Orientation = QuaternionIdentity();
    Color Tint = WHITE;

    int CurrentAnimation = 0;
    int CurrentFrame = 0;
    int NextFrame = 0;

    float CurrentFrameTime = 0;

    float AnimationFPS = 1.0f / 15.0f;

    std::vector<Matrix> BoneTransforms;

    GPUAnimatedModelInstance(GPUAnimatedModel& model)
        :SourceModel(model)
    {
        BoneTransforms.resize(SourceModel.BaseModel.boneCount);
        for (size_t i = 0; i < SourceModel.BaseModel.boneCount; i++)
            BoneTransforms[i] = MatrixIdentity();
    }

    void Update()
    {
        if (CurrentAnimation < 0 || CurrentAnimation >= SourceModel.Animations.size())
            return;

        CurrentFrameTime += GetFrameTime();

        while (CurrentFrameTime > AnimationFPS)
        {
            CurrentFrame++;
            NextFrame++;
            if (NextFrame >= SourceModel.Animations[CurrentAnimation].frameCount)
            {
                NextFrame = 0;
            }
            if (CurrentFrame >= SourceModel.Animations[CurrentAnimation].frameCount)
            {
                CurrentFrame = 0;
            }

            CurrentFrameTime -= AnimationFPS;
        }

        float param = CurrentFrameTime / AnimationFPS;

        int realFrame = CurrentFrame;
        int realNextFrame = NextFrame;

        if (realFrame >= SourceModel.Animations[CurrentAnimation].frameCount)
            realFrame = 0;
        if (realNextFrame >= SourceModel.Animations[CurrentAnimation].frameCount)
            realNextFrame = 0;

        for (int boneId = 0; boneId < SourceModel.BaseModel.boneCount; boneId++)
        {
            Transform& thisBoneTransform = SourceModel.Animations[CurrentAnimation].framePoses[realFrame][boneId];
            Transform& nextBoneTransform = SourceModel.Animations[CurrentAnimation].framePoses[realNextFrame][boneId];

            Vector3 inTranslation = SourceModel.BaseModel.bindPose[boneId].translation;
            Quaternion inRotation = SourceModel.BaseModel.bindPose[boneId].rotation;

            Vector3 thisTranslation = thisBoneTransform.translation;
            Quaternion thisRotation = thisBoneTransform.rotation;
            Vector3 thisScale = thisBoneTransform.scale;

            Vector3 nextTranslation = nextBoneTransform.translation;
            Quaternion nextRotation = nextBoneTransform.rotation;
            Vector3 nextScale = nextBoneTransform.scale;

            Vector3 outTranslation = Vector3Lerp(thisBoneTransform.translation, nextBoneTransform.translation, param);
            Quaternion outRotation = QuaternionSlerp(thisBoneTransform.rotation, nextBoneTransform.rotation, param);
            Vector3 outScale = Vector3Lerp(thisBoneTransform.scale, nextBoneTransform.scale, param);

            BoneTransforms[boneId] = MatrixTranslate(-inTranslation.x, -inTranslation.y, -inTranslation.z);
            BoneTransforms[boneId] = MatrixMultiply(BoneTransforms[boneId], MatrixScale(outScale.x, outScale.y, outScale.z));
            BoneTransforms[boneId] = MatrixMultiply(BoneTransforms[boneId], QuaternionToMatrix(QuaternionMultiply(outRotation, QuaternionInvert(inRotation))));
            BoneTransforms[boneId] = MatrixMultiply(BoneTransforms[boneId], MatrixTranslate(outTranslation.x, outTranslation.y, outTranslation.z));
        }
    }

    void Draw()
    {
        SetUniformMatrixTransposeV(AnimShader.BaseShader, AnimShader.ShaderBoneLocations[0], &BoneTransforms[0], int(BoneTransforms.size()));

        float angle = 0;
        Vector3 axis = { 0,1,0 };

        QuaternionToAxisAngle(Orientation, &axis, &angle);

        DrawModelEx(SourceModel.BaseModel, Position, axis, angle * RAD2DEG, Vector3{1,1,1}, Tint);
    }
};


// globals
Camera3D ViewCamera = { 0 };
GPUAnimatedModel TheModel;
std::vector<GPUAnimatedModelInstance> Instances;

Light Lights[MAX_LIGHTS] = { 0 };

void AddInstance()
{
    float spacing = 5;
    int gridWidth = 10;

    int x = 0;
    int y = 0;

    y = int(Instances.size()) / gridWidth;
    x = int(Instances.size()) - y * gridWidth;

    Instances.emplace_back(TheModel);

    GPUAnimatedModelInstance& newInstnace = Instances[Instances.size() - 1];
    newInstnace.Position.x = x * spacing;
    newInstnace.Position.z = y * spacing;
    newInstnace.Orientation = QuaternionFromEuler(0, GetRandomValue(-180, 180) * DEG2RAD, 0);

    newInstnace.CurrentAnimation = GetRandomValue(0, int(TheModel.Animations.size()) - 1);
}

void GameInit()
{
    ViewCamera.fovy = 45;
    ViewCamera.up.y = 1;
    ViewCamera.target.y = 2;
    ViewCamera.target.x = 15;
    ViewCamera.position.y = 2;
    ViewCamera.position.z = 20;

    AnimShader.Setup(LoadShader("resources/anim.vs", "resources/anim.fs"));

    TheModel.Setup(LoadModel("resources/robot.glb"));

    int animsSize = 0;
    ModelAnimation* anims = LoadModelAnimations("resources/robot.glb", &animsSize);
    TheModel.AddAnimation(anims, animsSize);
    MemFree(anims);

    Instances.emplace_back(TheModel);
    for (int i = 0; i < 5; i++)
        AddInstance();

    // setup lights
    // Ambient light level (some basic lighting)
    int ambientLoc = GetShaderLocation(AnimShader.BaseShader, "ambient");
    float ambient[4] = { 2, 2, 2, 1.0f };
    SetShaderValue(AnimShader.BaseShader, ambientLoc, ambient, SHADER_UNIFORM_VEC4);

    // Create lights
    Lights[0] = CreateLight(LIGHT_POINT, Vector3{ -20, 10, 20 }, Vector3Zero(), WHITE, AnimShader.BaseShader);
    Lights[1] = CreateLight(LIGHT_POINT, Vector3{ 20, 10, -20 }, Vector3Zero(), GRAY, AnimShader.BaseShader);
    Lights[2] = CreateLight(LIGHT_POINT, Vector3{ -20, 10, 20 }, Vector3Zero(), DARKGREEN, AnimShader.BaseShader);
    Lights[3] = CreateLight(LIGHT_POINT, Vector3{ 20, -10, 20 }, Vector3Zero(), DARKBLUE, AnimShader.BaseShader);

    //lights[1].enabled = false;
    Lights[2].enabled = false;
    Lights[3].enabled = false;

}

void GameCleanup()
{
    Instances.clear();
    TheModel.Cleanup();
}

bool GameUpdate()
{
    if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
        UpdateCamera(&ViewCamera, CAMERA_THIRD_PERSON);

    // Update the shader with the camera view vector so lights work (points towards { 0.0f, 0.0f, 0.0f })
    float cameraPos[3] = { ViewCamera.position.x, ViewCamera.position.y, ViewCamera.position.z };
    SetShaderValue(AnimShader.BaseShader, AnimShader.BaseShader.locs[SHADER_LOC_VECTOR_VIEW], cameraPos, SHADER_UNIFORM_VEC3);

    if (IsKeyPressed(KEY_SPACE))
        AddInstance();

    for (auto& instance : Instances)
        instance.Update();

    return true;
}

void Draw3D()
{
    BeginMode3D(ViewCamera);
    DrawGrid(50, 1.0f);

    Vector3 pos = { 0,0,0 };
    Quaternion orientation = QuaternionIdentity();

    for (auto& instance : Instances)
        instance.Draw();

    EndMode3D();
}

void Draw2D()
{
    DrawText(TextFormat("Animation %s", TheModel.Animations[Instances[0].CurrentAnimation].name), 10, 10, 20, DARKBROWN);
    DrawText("Press space to add an instance", 10, 30, 20, DARKBROWN);
}

void GameDraw()
{
    BeginDrawing();
    ClearBackground(SKYBLUE);
    Draw3D();
    Draw2D();
    DrawFPS(0, GetScreenHeight() - 20);
    EndDrawing();
}

int main()
{
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE);
    InitWindow(1280, 800, "GPU Animation");
    SetTargetFPS(300);

    GameInit();

    while (!WindowShouldClose())
    {
        if (!GameUpdate())
            break;

        GameDraw();
    }

    GameCleanup();
    CloseWindow();
    return 0;
}