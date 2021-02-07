#version 330 core
layout (location = 0) in vec3 vertex; // Original vertex position V
layout (location = 1) in vec3 normal; // Vertex normal N
layout (location = 2) in vec2 textCoord;
layout (location = 3) in vec3 tangent;
layout (location = 4) in vec3 bitangent;

out float nDotL;
out vec2 texCoord;
out vec3 Pos_tangent;
out vec3 CamPos_tangent;
out vec3 LightDir_tangent;
out vec3 Norm_tangent;

// light uniform variables
uniform vec3 lightDirection;
uniform vec3 viewPosition;

// transformations
uniform mat4 projection; // camera projection matrix
uniform mat4 view;  // represents the world in the eye coord space
uniform mat4 model; // represents model in the world coord space
uniform mat4 modelInvT; // inverse of the transpose of  model

void main() {
    mat3 normalModelInvT = mat3(modelInvT);
    vec3 N = normalize(normalModelInvT * normal);
    mat3 TBN = transpose(mat3( normalize(normalModelInvT * tangent),
    normalize(normalModelInvT * bitangent),
    normalize(normalModelInvT * normal)));

    nDotL = dot(normal, lightDirection);
    texCoord = textCoord;
    LightDir_tangent = TBN * lightDirection;
    CamPos_tangent = TBN * viewPosition;
    Pos_tangent = vec3(0.0);
    Norm_tangent = TBN * N;

    gl_Position = projection * view * model * vec4(vertex, 1.0);
}