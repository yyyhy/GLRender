#version 330 core

uniform vec3 col;

void main()
{    
    gl_FragColor =vec4(col,1);
    float depth=gl_FragCoord.z;
    //float delpthc=gl_FragDepth;
    
}