#version 330 core
layout(location = 0) in vec3 aPos;

out vec3 TexCoords;
uniform mat4 view;
uniform mat4 projection;


void main()
{
    TexCoords = aPos;
    vec4 v = projection*view*vec4(aPos, 1.0);
    gl_Position=vec4(v.xy,v.w,v.w);
}