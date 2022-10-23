#version 430 core

layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aTexCoords;

layout(std140 , binding=0) uniform CameraProperty{
    mat4 model;       //0
    mat4 view;        //64
    mat4 projection;  //128
    mat4 mvp;         //192
    mat4 lastMvp;     //256
    vec3 CameraPos;   //320
    vec2 Offset;      //336  272 
};

out vec2 texCoords;
out vec3 uCameraPos;
out mat4 CameraMVP;
out mat4 CameraV;
out mat4 CameraM;
out mat4 CameraP;

#define TRANSFER_I_AND_N

#ifdef TRANSFER_I_AND_N
    out float I;
    out float N;
#endif

float RadicalInverse_VdC(uint bits) 
{
     bits = (bits << 16u) | (bits >> 16u);
     bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
     bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
     bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
     bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
     return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}
// ----------------------------------------------------------------------------
vec2 Hammersley(uint i, uint N)
{
	return vec2(float(i)/float(N), RadicalInverse_VdC(i));
}

void main()
{
    vec4 uv=vec4(aPos.x,aPos.y,0.0, 1.0);
    gl_Position = uv;
    texCoords=aTexCoords;
    uCameraPos=CameraPos;
    CameraMVP=mvp;
    CameraV=view;
    CameraM=model;
    CameraP=projection;
    #ifdef TRANSFER_I_AND_N
        I=Offset.x;
        N=Offset.y;
    #endif
}