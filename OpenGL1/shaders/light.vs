#version 330 core
layout(location = 0) in vec3 aPos;

out vec3 vWorldPos;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 trans;

void main()
{
    vWorldPos=aPos;
    mat4 m;
    m[0][0]=0.1;
    m[1][1]=0.1;
    m[2][2]=0.1;
    m[3][3]=1;
    gl_Position = projection*view*model*trans*m*vec4(aPos, 1.0);
}