#version 330 core

in vec3 TexCoords;

uniform samplerCube skybox;

void main()
{    
    gl_FragColor = textureLod(skybox, TexCoords,0);
    //gl_FragColor = vec4(0.5);
}