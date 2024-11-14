#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include "rcamera.h"

// lighting
#define RLIGHTS_IMPLEMENTATION
#include "rlights.h"

// voxel size constants
constexpr int ChunkDepth = 16;
constexpr int ChunkSize = 16;

// the voxel data for a single chunk. Tells us what is in each voxel
char VoxelChunk[ChunkSize * ChunkSize * ChunkDepth] = { 0 };

// texture rectangles for various block colors
Rectangle BlockColors[4] = { Rectangle{0,0,0.25f,1}, Rectangle{0.25f,0,0.5f,1}, Rectangle{0.5f,0,0.75f,1}, Rectangle{0.75f,0,1,1} };

// get the index into the voxel array for a h,v,d coordinate
int GetIndex(int h, int v, int d)
{
	return (d * (ChunkSize * ChunkSize)) + (v * ChunkSize) + h;
}

// a simple class to help build up faces of a cube
// can be made to be pure C and take the global data in a structure or global data
class CubeGeometryBuilder
{
public:

	// setup the builder with the mesh it is going to fill out
	CubeGeometryBuilder(Mesh & mesh) : MeshRef(mesh)
	{
	}

	// indexes for the 6 faces of a cube
	static constexpr int SouthFace = 0;
	static constexpr int NorthFace = 1;
	static constexpr int WestFace = 2;
	static constexpr int EastFace = 3;
	static constexpr int UpFace = 4;
	static constexpr int DownFace = 5;

	// we need to know how many triangles are going to be in the mesh before we start
	// this way we can allocate the correct buffer sizes for the mesh
	void Allocate(int triangles)
	{
		// there are 
		MeshRef.vertexCount = triangles * 6;
		MeshRef.triangleCount = triangles * 2;

		MeshRef.vertices = static_cast<float*>(MemAlloc(sizeof(float) * 3 * MeshRef.vertexCount));
		MeshRef.normals = static_cast<float*>(MemAlloc(sizeof(float) * 3 * MeshRef.vertexCount));
		MeshRef.texcoords = static_cast<float*>(MemAlloc(sizeof(float) * 2 * MeshRef.vertexCount));
		MeshRef.colors = nullptr;	// static_cast<unsigned char*>(MemAlloc(sizeof(unsigned char) * 4 * MeshRef.vertexCount));

		MeshRef.animNormals = nullptr;
		MeshRef.animVertices = nullptr;
		MeshRef.boneIds = nullptr;
		MeshRef.boneWeights = nullptr;
		MeshRef.tangents = nullptr;
		MeshRef.indices = nullptr;
		MeshRef.texcoords2 = nullptr;
	}

	inline void SetNormal(Vector3& value) { Normal = value; }
	inline void SetNormal(float x, float y, float z) { Normal = Vector3{ x,y,z }; }
	inline void SetSetUV(Vector2& value) { UV = value; }
	inline void SetSetUV(float x, float y ) { UV = Vector2{ x,y }; }

	inline void PushVertex(Vector3& vertex, float xOffset = 0, float yOffset = 0, float zOffset = 0)
	{ 
		size_t index = 0;

		if (MeshRef.colors != nullptr)
		{
			index = TriangleIndex * 12 + VertIndex * 4;
			MeshRef.colors[index] = VertColor.r;
			MeshRef.colors[index + 1] = VertColor.g;
			MeshRef.colors[index + 2] = VertColor.b;
			MeshRef.colors[index + 3] = VertColor.a;
		}

		if (MeshRef.texcoords != nullptr)
		{
			index = TriangleIndex * 6 + VertIndex * 2;
			MeshRef.texcoords[index] = UV.x;
			MeshRef.texcoords[index + 1] = UV.y;
		}

		if (MeshRef.normals != nullptr)
		{
			index = TriangleIndex * 9 + VertIndex * 3;
			MeshRef.normals[index] = Normal.x;
			MeshRef.normals[index + 1] = Normal.y;
			MeshRef.normals[index + 2] = Normal.z;
		}

		index = TriangleIndex * 9 + VertIndex * 3;
		MeshRef.vertices[index] = vertex.x + xOffset;
		MeshRef.vertices[index + 1] = vertex.y + yOffset;
		MeshRef.vertices[index + 2] = vertex.z + zOffset;

		VertIndex++;
		if (VertIndex > 2)
		{
			TriangleIndex++;
			VertIndex = 0;
		}
	}

	void AddCube(Vector3&& position, bool faces[6], int block)
	{
		Rectangle& uvRect = BlockColors[block];
		SetSetUV(0,0);
		//z-
		if (faces[NorthFace])
		{
			SetNormal( 0, 0, -1 );
			SetSetUV(uvRect.width, uvRect.height);
			PushVertex(position);

			SetSetUV(uvRect.x, uvRect.y);
			PushVertex(position, 1, 1, 0);

			SetSetUV(uvRect.x, uvRect.height);
			PushVertex(position, 1, 0, 0);

			SetSetUV(uvRect.width, uvRect.height);
			PushVertex(position);

			SetSetUV(uvRect.width, uvRect.y);
			PushVertex(position, 0, 1, 0);

			SetSetUV(uvRect.x, uvRect.y);
			PushVertex(position, 1, 1, 0);
		}
		
		// z+
		if (faces[SouthFace])
		{
			SetNormal(0, 0 ,1);

			SetSetUV(uvRect.width, uvRect.height);
			PushVertex(position, 0, 0, 1);

			SetSetUV(uvRect.x, uvRect.height);
			PushVertex(position, 1, 0, 1);

			SetSetUV(uvRect.x, uvRect.y);
			PushVertex(position, 1, 1, 1);

			SetSetUV(uvRect.width, uvRect.height);
			PushVertex(position, 0, 0, 1);

			SetSetUV(uvRect.x, uvRect.y);
			PushVertex(position, 1, 1, 1);

			SetSetUV(uvRect.width, uvRect.y);
			PushVertex(position, 0, 1, 1);
		}

		// x+
		if (faces[WestFace])
		{
			SetNormal(1, 0, 0 );
			SetSetUV(uvRect.x, uvRect.height);
 			PushVertex(position, 1, 0, 1);

			SetSetUV(uvRect.width, uvRect.height);
			PushVertex(position, 1, 0, 0);

			SetSetUV(uvRect.width, uvRect.y);
 			PushVertex(position, 1, 1, 0);

			SetSetUV(uvRect.x, uvRect.height);
 			PushVertex(position, 1, 0, 1);

			SetSetUV(uvRect.width, uvRect.y);
 			PushVertex(position, 1, 1, 0);

			SetSetUV(uvRect.x, uvRect.y);
 			PushVertex(position, 1, 1, 1);
		}

		// x-
		if (faces[EastFace])
		{
			SetNormal(-1, 0, 0);

			SetSetUV(uvRect.width, uvRect.height);
			PushVertex(position, 0, 0, 1);

			SetSetUV(uvRect.x, uvRect.y);
			PushVertex(position, 0, 1, 0);

			SetSetUV(uvRect.x, uvRect.height);
			PushVertex(position, 0, 0, 0);

			SetSetUV(uvRect.width, uvRect.height);
			PushVertex(position, 0, 0, 1);

			SetSetUV(uvRect.width, uvRect.y);
			PushVertex(position, 0, 1, 1);

			SetSetUV(uvRect.x, uvRect.y);
			PushVertex(position, 0, 1, 0);
		}

		if (faces[UpFace])
		{
			SetNormal(0, 1, 0 );

			SetSetUV(uvRect.x, uvRect.y);
			PushVertex(position, 0, 1, 0);

			SetSetUV(uvRect.width, uvRect.height);
			PushVertex(position, 1, 1, 1);

			SetSetUV(uvRect.width, uvRect.y);
			PushVertex(position, 1, 1, 0);

			SetSetUV(uvRect.x, uvRect.y);
			PushVertex(position, 0, 1, 0);

			SetSetUV(uvRect.x, uvRect.height);
			PushVertex(position, 0, 1, 1);

			SetSetUV(uvRect.width, uvRect.height);
			PushVertex(position, 1, 1, 1);
		}

		SetSetUV(0, 0);
		if (faces[DownFace])
		{
			SetNormal(0, -1, 0);

			SetSetUV(uvRect.x, uvRect.y);
			PushVertex(position, 0, 0, 0);

			SetSetUV(uvRect.width, uvRect.y);
			PushVertex(position, 1, 0, 0);

			SetSetUV(uvRect.width, uvRect.height);
			PushVertex(position, 1, 0, 1);

			SetSetUV(uvRect.x, uvRect.y);
			PushVertex(position, 0, 0, 0);

			SetSetUV(uvRect.width, uvRect.height);
			PushVertex(position, 1, 0, 1);

			SetSetUV(uvRect.x, uvRect.height);
			PushVertex(position, 0, 0, 1);
		}
	}

protected:
	Mesh& MeshRef;

	size_t TriangleIndex = 0;
	size_t VertIndex = 0;

	Vector3 Normal = { 0,0,0 };
	Color VertColor = WHITE;
	Vector2 UV = { 0,0 };
};


// build a simple random voxel chunk
void BuildChunk()
{
	//fill the chunk with layers of blocks
	for (int d = 0; d < ChunkDepth; d++)
	{
		char block = 0;
		if (d > 6)
		{
			block = 1;
			if (d > 8)
			{
				block = 2;
				if (d > 10)
					block = -1;
			}
		}
	
		for (int v = 0; v < ChunkSize; v++)
		{
			for (int h = 0; h < ChunkSize; h++)
			{
				int index = GetIndex(h, v, d);
	
				VoxelChunk[index] = block;
			}
		}
	}

	// Remove some chunks 
	for (int i = 0; i < 600; i++)
	{
		int h = GetRandomValue(0, ChunkSize-1);
		int v = GetRandomValue(0, ChunkSize-1);
		int d = GetRandomValue(0, 10);

		int index = GetIndex(h, v, d);

		VoxelChunk[index] = -1;
	}

	// Add some gold
	for (int i = 0; i < 100; i++)
	{
		int h = GetRandomValue(0, ChunkSize - 1);
		int v = GetRandomValue(0, ChunkSize - 1);
		int d = GetRandomValue(0, 10);

		int index = GetIndex(h, v, d);

		VoxelChunk[index] = 3;
	}
}

bool BlockIsSolid(int h, int v, int d)
{
	if (h < 0 || h >= ChunkSize)
		return false;

	if (v < 0 || v >= ChunkSize)
		return false;

	if (d < 0 || d >= ChunkDepth)
		return false;

	return VoxelChunk[GetIndex(h, v, d)] >= 0;
}

//check all the adjacent blocks to see if they are open, if they are, we need a face for that side of the block.
int GetChunkFaceCount()
{
	int count = 0;
	for (int d = 0; d < ChunkDepth; d++)
	{
		for (int v = 0; v < ChunkSize; v++)
		{
			for (int h = 0; h < ChunkSize; h++)
			{
				if (!BlockIsSolid(h,v,d))
					continue;

				if (!BlockIsSolid(h + 1, v, d))
					count++;

				if (!BlockIsSolid(h - 1, v, d))
					count++;

				if (!BlockIsSolid(h, v + 1, d))
					count++;

				if (!BlockIsSolid(h, v - 1, d))
					count++;

				if (!BlockIsSolid(h, v, d + 1))
					count++;

				if (!BlockIsSolid(h, v, d - 1))
					count++;
			}
		}
	} 

	return count;
}

Mesh MeshChunk()
{
	Mesh mesh = { 0 };
	CubeGeometryBuilder builder(mesh);

	// figure out how many faces will be in this chunk and allocate a mesh that can store that many
	builder.Allocate(GetChunkFaceCount());

	size_t count = 0;
	for (int d = 0; d < ChunkDepth; d++)
	{
		for (int v = 0; v < ChunkSize; v++)
		{
			for (int h = 0; h < ChunkSize; h++)
			{
				if (!BlockIsSolid(h, v, d))
					continue;

				// build up the list of faces that this block needs
				bool faces[6] = { false, false, false, false, false, false };

				if (!BlockIsSolid(h - 1, v, d))
					faces[CubeGeometryBuilder::EastFace] = true;

				if (!BlockIsSolid(h + 1, v, d))
					faces[CubeGeometryBuilder::WestFace] = true;

				if (!BlockIsSolid(h, v - 1, d))
					faces[CubeGeometryBuilder::NorthFace] = true;

				if (!BlockIsSolid(h, v + 1, d))
					faces[CubeGeometryBuilder::SouthFace] = true;

				if (!BlockIsSolid(h, v, d + 1))
					faces[CubeGeometryBuilder::UpFace] = true;

				if (!BlockIsSolid(h, v, d - 1))
					faces[CubeGeometryBuilder::DownFace] = true;

				// build the faces that hit open air for this voxel block
				builder.AddCube(Vector3{ (float)h, (float)d, (float)v }, faces, (int)VoxelChunk[GetIndex(h, v, d)]);
			}
		}
	}

	UploadMesh(&mesh, false);

	return mesh;
}

int main()
{
	InitWindow(1200, 800, "voxels!");
	SetTargetFPS(144);

	// setup a render texture with 4 block textures

	RenderTexture tileTexture = LoadRenderTexture(64, 16);
	BeginTextureMode(tileTexture);
	ClearBackground(BLANK);
	DrawRectangle(0, 0, 16, 16, DARKBROWN);
	DrawRectangle(16, 0, 16, 16, BROWN);
	DrawRectangle(32, 0, 16, 16, GREEN);
	DrawRectangle(48, 0, 16, 16, GOLD);
	EndTextureMode();

	// setup a simple camera
	Camera3D camera = { 0 };

	camera.fovy = 45;
	camera.up.y = 1;
	camera.target.x = 8;
	camera.target.z = 8;
	camera.target.y = 4;

    camera.position.x = 32;
	camera.position.z = 32;
	camera.position.y = 16;

	// load basic lighting
	Shader shader = LoadShader("resources/shaders/base_lighting.vs", "resources/shaders/lighting.fs");
	shader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(shader, "viewPos");

	// Ambient light level (some basic lighting)
	int ambientLoc = GetShaderLocation(shader, "ambient");
	float val[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
	SetShaderValue(shader, ambientLoc, val , SHADER_UNIFORM_VEC4);

	Light lights[MAX_LIGHTS] = { 0 };
	lights[0] = CreateLight(LIGHT_DIRECTIONAL, Vector3Zero(), Vector3{ -2, -4, -3 }, WHITE, shader);
	lights[1] = CreateLight(LIGHT_DIRECTIONAL, Vector3Zero(), Vector3{ 2, 2, 5 }, GRAY, shader);

	// build a single chunk of voxel data
	BuildChunk();

	// build a mesh for the chunk
	Mesh mesh = MeshChunk();
	
	// set the mesh to the correct material/shader
	Material mat = LoadMaterialDefault();
	mat.maps[0].color = WHITE;
	mat.maps[0].texture = tileTexture.texture;
	mat.shader = shader;

	while (!WindowShouldClose())
	{
		CameraYaw(&camera, GetFrameTime() * DEG2RAD * 15, true);

		// update lights
		UpdateLightValues(shader, lights[0]);
		UpdateLightValues(shader, lights[1]);

		// Update the shader with the camera view vector (points towards { 0.0f, 0.0f, 0.0f })
		SetShaderValue(shader, shader.locs[SHADER_LOC_VECTOR_VIEW], &camera.position, SHADER_UNIFORM_VEC3);

		BeginDrawing();
		ClearBackground(SKYBLUE);

		BeginMode3D(camera);


		// draw some simple origin and axis markers
		DrawGrid(10, 10);
		DrawSphere(Vector3Zero(), 0.125f, GRAY);
		DrawSphere(Vector3{ 1,0,0 }, 0.0125f, RED);
		DrawSphere(Vector3{ 0,0,1 }, 0.0125f, GREEN);

		// draw the chunk
		DrawMesh(mesh, mat, MatrixIdentity());

		EndMode3D();

		DrawFPS(0, 0);
		EndDrawing();
	}
	
	UnloadMesh(mesh);
	UnloadRenderTexture(tileTexture);
	UnloadShader(shader);
	CloseWindow();
	return 0;
}
