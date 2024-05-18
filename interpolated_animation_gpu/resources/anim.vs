#version 330

// Input vertex attributes
in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec2 vertexTexCoord2;         // bone IDs
in vec3 vertexNormal;
in vec4 vertexColor;
in vec4 vertexTangent;          // bone weights

#define     MAX_BONES              64

uniform mat4 bones[MAX_BONES];

// Input uniform values
uniform mat4 mvp;
uniform mat4 matModel;
uniform mat4 matNormal;

// Output vertex attributes (to fragment shader)
out vec3 fragPosition;
out vec2 fragTexCoord;
out vec4 fragColor;
out vec3 fragNormal;

void main()
{
    // non transformed data
    fragTexCoord = vertexTexCoord;
    fragColor = vertexColor;

    // bone ID stored in the UV2 chanel
    int bone0 = int(vertexTexCoord2.x / 1000);
    int bone1 = int(vertexTexCoord2.x - bone0*1000);
    int bone2 = int(vertexTexCoord2.y / 1000);
    int bone3 = int(vertexTexCoord2.y - bone2*1000);

    // vertex
    vec4 vertex = vec4(vertexPosition, 1.0f);

    vec4 blendedVert = bones[bone0] * vertex * vertexTangent.x;
    blendedVert += bones[bone1] * vertex * vertexTangent.y;
    blendedVert += bones[bone2] * vertex * vertexTangent.z;
    blendedVert += bones[bone3] * vertex * vertexTangent.w;
    blendedVert.w = 1.0f;

    gl_Position = mvp *  blendedVert;

    // normals
    vec4 normal = vec4(vertexNormal, 0.0f);
    vec4 blendedNormal = bones[bone0] * normal * vertexTangent.x;
    blendedNormal += bones[bone1] * normal * vertexTangent.y;
    blendedNormal += bones[bone2] * normal * vertexTangent.z;
    blendedNormal += bones[bone3] * normal * vertexTangent.w;
    blendedNormal.w = 0.0f;

    fragNormal = normalize(vec3(matNormal*blendedNormal));
}