#version 330 core

float far=100.0;
float near=0.01;

float LinearizeDepth(float depth) 
{
    float z = depth * 2.0 - 1.0; // back to NDC 
    return (2.0 * near * far) / (far + near - z * (far - near));    
}

void main()
{
    gl_FragColor=vec4(LinearizeDepth(gl_FragCoord.z));
}