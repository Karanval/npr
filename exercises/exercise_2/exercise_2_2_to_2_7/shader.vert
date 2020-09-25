#version 330 core
layout (location = 0) in vec2 pos;   // the position variable has attribute position 0
// TODO 2.2 add velocity and timeOfBirth as vertex attributes
layout (location = 1) in vec2 velocity;
layout (location = 2) in float timeOfBirth;

out float ageFrag;

// TODO 2.3 create and use a float uniform for currentTime
uniform float currentTime;

const float maxAge = 5.0f;
void main()
{
    // TODO 2.3 use the currentTime to control the particle in different stages of its lifetime
    vec2 finalPos = pos;
    float age = currentTime - timeOfBirth;
    if (timeOfBirth == 0 || age > maxAge){
       finalPos = vec2(-2.0f, -2.0f);
    } else {
       finalPos += velocity * age;
    }


    gl_Position = vec4(finalPos, 0.0, 1.0);
    gl_PointSize = (age * 2.0) + 1.0;
    ageFrag = age;
}