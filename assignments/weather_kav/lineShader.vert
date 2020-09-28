#version 330 core
layout (location = 0) in vec3 pos;

uniform mat4 cameraMatrixView;
uniform vec3 cameraPos;
uniform vec3 cameraFor;
uniform vec3 offsets;
uniform float boxSize;
uniform vec3 g_fHeightScale;
uniform mat4 g_mViewProjPrev;
uniform vec3 g_vVelocity ;

void main()
{
    vec3 lastPos = pos;
    float idMod = mod(gl_VertexID, 2);// -> 0 top, 1 bottom
    lastPos = mod(lastPos + offsets, boxSize);
    lastPos += cameraPos + cameraFor - boxSize/2;
    if (idMod == 0){
        // bottom
        gl_Position = cameraMatrixView * vec4(lastPos, 1);
    } else {
        // top
        lastPos = lastPos + g_vVelocity  * g_fHeightScale;
        gl_Position = g_mViewProjPrev * vec4(lastPos, 1);
    }
    //gl_Position = vec4(finalpos, 1);
    //gl_Position = cameraMatrixView * vec4(pos, 1) ;
    //gl_Position = vec4(pos, 1) ;
}