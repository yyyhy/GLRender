#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out VS_OUT {
    vec3 normal;
} vs_out;

uniform mat4 view;
uniform mat4 model;
uniform mat4 trans;
out vec3 Normal;
void main()
{
    mat3 normalMatrix = mat3(transpose(inverse(view * trans)));
    vs_out.normal = vec3(vec4(normalMatrix * aNormal, 0.0));
    Normal=vs_out.normal;
    gl_Position = view * model * trans*vec4(aPos, 1.0); 
}