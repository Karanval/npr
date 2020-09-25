#version 330 core
in vec2 gl_PointCoord;
in float ageFrag;
out vec4 fragColor;

const vec3 startCol = vec3(1.0, 1.0, 0.05);
const vec3 midCol = vec3(1.0, 0.5, 0.01);
const vec3 endCol = vec3(0.0, 0.0, 0.0);

const float midAge = 5.0;
const float maxAge = 10.0;

void main()
{
    // TODO 2.4 set the alpha value to 0.2 (alpha is the 4th value)
    //float sqDistance = ((gl_PointCoord - 0.5) * 2) + ((gl_PointCoord.y - 0.5) * 2);
    vec2 vecFromCenter = (gl_PointCoord - vec2(.5, .5)) * 2;
    float distance = sqrt(dot(vecFromCenter, vecFromCenter));
    // after 2.4, TODO 2.5 and 2.6: improve the particles appearance
    vec3 finalCol = mix(startCol, midCol, ageFrag / midAge);
    if (ageFrag > midAge){
        finalCol = mix(midCol, endCol, (ageFrag - midAge) / midAge);
    }

    float alpha = mix(1.0 - distance, 0, ageFrag / maxAge);

    fragColor = vec4(finalCol, alpha);

// circles https://mmmovania.blogspot.com/search?q=circular+point+sprite
    if(dot(gl_PointCoord-0.5,gl_PointCoord-0.5)>0.25)
       discard;
    else
       fragColor = vec4(finalCol, alpha);

    //others
    vec2 p = gl_PointCoord* 2.0 - vec2(1.0);
    float r = sqrt(dot(p,p));
    float theta = atan(p.y,p.x);

    //flowers
    if(dot(p,p) > cos(theta*5))
      discard;
    else
      fragColor = vec4(finalCol, alpha);

    // round ring
    //if(dot(p,p) > r || dot(p,p) < r*0.75)
    //  discard;
    //else
    //  fragColor = vec4(finalCol, alpha);

    // spiral
   // if(dot(p,p)> 5.0/cos(theta-20*r))
   //     discard;
   // else
      //  fragColor = vec4(finalCol, alpha);
    //rounded star
    //if(dot(p,p) > 0.5*(exp(cos(theta*5)*0.75)) )
    //  discard;
   // else
    //  fragColor = vec4(finalCol, alpha);
}