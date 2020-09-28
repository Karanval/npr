#version 330 core
out vec4 FragColor;
uniform bool rain;

void main()
{
    if (rain) {
        // circles https://mmmovania.blogspot.com/search?q=circular+point+sprite
        if (dot(gl_PointCoord-0.5, gl_PointCoord-0.5)>0.25)
            discard;
        else
            FragColor = vec4(1,1,1,1);
    } else {
        //others
        vec2 p = gl_PointCoord* 2.0 - vec2(1.0);
        float r = sqrt(dot(p, p));
        float theta = atan(p.y, p.x);

        //flowers
        if (dot(p, p) > cos(theta*5))
         discard;
        else
            FragColor = vec4(1, 1, 1, 1);
    }
}