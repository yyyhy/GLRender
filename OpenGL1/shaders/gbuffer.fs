#version 430 core
layout (location = 0) out vec4 gPositionRoughness;
layout (location = 1) out vec4 gNormalDepth;
layout (location = 2) out vec4 gAlbedoMetallic;
layout (location = 3) out vec4 gTangentFlag;
layout (location = 4) out vec4 gLightMapDir;
layout (location = 5) out vec4 gLightMap;
layout (location = 6) out vec2 gVelo;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;
in vec3 Tangent;
in vec3 Bitangent;
in vec3 LightMapCol;
in vec3 LightMapDir;
in vec4 preScreenPos;
in vec4 currScreenPos;
uniform vec3 baseColor=vec3(0);

uniform sampler2D albedoMap;
uniform sampler2D metallicMap;
uniform sampler2D normalMap;
uniform sampler2D roughnessMap;
uniform sampler2D AOMap;
uniform sampler2D wetMap;
uniform sampler2D lightMap;
uniform sampler2D lightMapDir;
uniform bool roughInvert;

uniform float far=100.0;
uniform float near=0.1;

float LinearizeDepth(float depth) 
{
    float z = depth * 2.0 - 1.0; // back to NDC 
    return (2.0 * near * far) / (far + near - z * (far - near));    
}

void main()
{    
    float r=texture2D(roughnessMap,TexCoords).x;
    gPositionRoughness = vec4(FragPos,roughInvert? 1-r:r);
    
    gAlbedoMetallic.xyz = (texture2D(albedoMap, TexCoords).xyz+baseColor);
    
    gAlbedoMetallic.w = texture(metallicMap, TexCoords).x;
    gLightMap=vec4(LightMapCol,1);
    gTangentFlag=vec4(normalize(Tangent),1);

    gLightMapDir=vec4(LightMapDir,texture2D(wetMap,TexCoords,0).x);

    vec3 normal=texture2D(normalMap,TexCoords,0).xyz;
    if(normal.z>0.01){
        normal=normal*2.0-1.0;
        mat3 TBN=mat3(Tangent,Bitangent,Normal);
        //normalize(TBN*normal)
        gNormalDepth=vec4(normalize(TBN*normal),LinearizeDepth(gl_FragCoord.z));
    }
    else{
        gNormalDepth=vec4(Normal,LinearizeDepth(gl_FragCoord.z));
    }
    
    vec2 currPos=(currScreenPos.xy/currScreenPos.w)*0.5+0.5;
    vec2 prePos=(preScreenPos.xy/preScreenPos.w)*0.5+0.5;
    gVelo=currPos-prePos;
}