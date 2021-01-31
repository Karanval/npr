#version 330 core
layout (location = 0) in vec3 vertex; // Original vertex position V
layout (location = 1) in vec3 normal; // Vertex normal N
layout (location = 2) in vec2 textCoord;
layout (location = 3) in vec3 vTangent;
layout (location = 4) in vec3 aBitangent;

out float nDotL;
out vec2 texCoord;

// transformations
uniform mat4 projection; // camera projection matrix
uniform mat4 view;  // represents the world in the eye coord space
uniform mat4 model; // represents model in the world coord space
uniform mat4 invTranspMV; // inverse of the transpose of (view * model) (used to multiply vectors if there is non-uniform scaling)

// light uniform variables
uniform vec3 lightPosition;

void main() {
    // vertex in eye space (for light computation in eye space)
    vec4 Pos_eye = view * model * vec4(vertex, 1.0);
    // normal in eye space (for light computation in eye space)
    vec3 N_eye = normalize((invTranspMV * vec4(normal, 0.0)).xyz);
    // light in eye space
    vec4 Light_eye = view * vec4(lightPosition, 1.0);

    // final vertex transform (for opengl rendering) without jitters
    vec4 worldPosition = projection  * Pos_eye;
    gl_Position = worldPosition;

    vec3 lightDirection = normalize(vertex - lightPosition);
    nDotL = dot(normal, lightDirection);
    texCoord = textCoord;
}