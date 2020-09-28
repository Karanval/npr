#version 330 core
out vec4 FragColor;

void main()
{
    // circles https://mmmovania.blogspot.com/search?q=circular+point+sprite
    if (dot(gl_PointCoord-0.5, gl_PointCoord-0.5)>0.25)
    discard;
    else
    FragColor = vec4(1,1,1,1);
}