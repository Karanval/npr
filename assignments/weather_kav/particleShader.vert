#version 330 core
layout (location = 0) in vec3 pos;
const float distanceThreshold = 30;
const float maxSize = 5.6;
const float minSize = 2.1;

uniform mat4 cameraMatrixView;
uniform vec3 cameraPos;
uniform vec3 cameraFor;
uniform vec3 offsets;
uniform float boxSize;
uniform bool rain;

vec3 lastPos = pos;

void main()
{

    lastPos = mod(lastPos + offsets, boxSize);
    lastPos += cameraPos + cameraFor - boxSize/2;
    float sqDistance = pow(pos.x- cameraPos.x,2) + pow(pos.y- cameraPos.y,2) + pow(pos.z- cameraPos.z,2);
//    if (sqDistance > distanceThreshold) {
//        gl_PointSize = minSize;
//    } else {
//        //output_start + ((output_end - output_start) / (input_end - input_start)) * (input - input_start)
//        gl_PointSize = minSize + ((maxSize - minSize) / distanceThreshold) * sqDistance;
//    }
    //SNOW
    if(rain)
        gl_PointSize = 1;
    else
        gl_PointSize = 6;
    gl_Position = cameraMatrixView * vec4(lastPos, 1) ;
}