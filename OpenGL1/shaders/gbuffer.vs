#version 420 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;
layout (location = 3) in vec3 tangent;
layout (location = 4) in vec3 biTangent;
layout (location = 5) in vec3 lightMapCol;
layout (location = 6) in vec3 lightMapDir;

out vec3 FragPos;
out vec2 TexCoords;
out vec3 Normal;
out vec3 Tangent;
out vec3 Bitangent;
out vec3 LightMapCol;
out vec3 LightMapDir;
out vec4 preScreenPos;
out vec4 currScreenPos;

layout(std140 , binding=0) uniform CameraMats{
    mat4 model;       //0
    mat4 view;        //64
    mat4 projection;  //128
    mat4 mvp;         //192
    mat4 lastMvp;     //256
    vec3 CameraPos;   //320
    vec2 Offset;      //336  272 
};

layout(std140, binding=1) uniform RenderSettings{
    vec2 screenSize;
};
uniform mat4 trans;
uniform mat4 preTrans;
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
    vec4 worldPos = trans * vec4(position, 1.0f);
    vec4 preWorldPos=preTrans*vec4(position,1.0f);
    FragPos = worldPos.xyz/worldPos.w;
    vec2 offset=Hammersley(uint(Offset.x),uint(Offset.y))*2.0-1.0; 
    offset.x/=screenSize.x;
    offset.y/=screenSize.y;
    mat4 jitter=projection;
    jitter[2][0]+=offset.x;
    jitter[2][1]+=offset.y;
    vec4 pos=jitter*view*model*worldPos;
    gl_Position = pos//+vec4(offset*4,0,0)*pos.w;
    ;
    TexCoords = texCoords;
    
    mat3 normalMatrix = transpose(inverse(mat3(trans)));
    Normal = normalMatrix * normal;
    Tangent=normalMatrix*tangent;
    Bitangent=normalMatrix*biTangent;
    LightMapCol=lightMapCol;
    LightMapDir= normalize(lightMapDir);
    preScreenPos=lastMvp*preWorldPos;
    currScreenPos=projection*view*model*worldPos;
}