#version 330 core
layout (location = 0) in vec3 vertex; // Original vertex position V
layout (location = 1) in vec3 normal; // Vertex normal N
layout (location = 2) in vec2 textCoord;
layout (location = 3) in vec3 vTangent;
layout (location = 4) in vec3 aBitangent;

out VS_OUT {
    vec3 Pos_eye;
    vec3 N_eye;
    vec3 Light_eye;
    vec2 textCoord;
    float nDotV;
} vs_out;
out vec2 fsInUV;


// transformations
uniform mat4 projection; // camera projection matrix
uniform mat4 view;  // represents the world in the eye coord space
uniform mat4 viewInv;
uniform mat4 model; // represents model in the world coord space
uniform mat4 invTranspMV; // inverse of the transpose of (view * model) (used to multiply vectors if there is non-uniform scaling)

// light uniform variables
uniform vec3 lightPosition;

// Watercolor
uniform float time; // T
uniform vec2 texelSize; // Relative pixel size of the projection space Pp
uniform float tremorSpeed; // s
uniform float tremorFrequency; // f
uniform float tremorAmount; // t
uniform float tremorFront; // alpha?
uniform bool applyDeformations;

void main() {
    // vertex in eye space (for light computation in eye space)
    vec4 Pos_eye = view * model * vec4(vertex, 1.0);
    // normal in eye space (for light computation in eye space)
    vec3 N_eye = normalize((invTranspMV * vec4(normal, 0.0)).xyz);
    // light in eye space
    vec4 Light_eye = view * vec4(lightPosition, 1.0);

    // final vertex transform (for opengl rendering) without jitters
    vec4 worldPosition = projection  * Pos_eye;

    // Deformations
    // Vo = sin(T x s + V x f) x t x Pp
    //vec3 offset = sin(time * tremorSpeed + vertex * tremorFrequency) * tremorAmount * vec3(texelSize, 1.0); // *100 and 10 come from code MNPR
    vec3 offset = sin( tremorSpeed + worldPosition.xyz * tremorFrequency) * tremorAmount * vec3(texelSize, 1.0); // *100 and 10 come from code MNPR
    vec3 viewDirection = normalize(viewInv[3].xyz - worldPosition.xyz);
    // Vt = V + Vo(1-a(V dot N))
    float nDotV = dot(normal, viewDirection);
    float angle = min(clamp(nDotV * 1.2, 0.0, 1.0), (1 - tremorFront));
//    vec3 deformations = vertex + offset * (0.6*nDotV);
    if (!applyDeformations)
        gl_Position = worldPosition;
    else
        gl_Position = vec4(worldPosition.xyz + offset * ( angle * nDotV), worldPosition.w);
    // out info
    vs_out.Pos_eye = Pos_eye.xyz;
    vs_out.N_eye = N_eye;
    vs_out.Light_eye = Light_eye.xyz;
    vs_out.textCoord = textCoord;
    vs_out.nDotV = nDotV;

}