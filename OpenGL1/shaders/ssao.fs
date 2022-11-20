#version 420 core
 layout(location = 0) out vec4 ssaoColor;

in vec2 texCoords;
in vec3 uCameraPos;
in mat4 CameraMVP;
in mat4 CameraV;
in mat4 CameraM;
in mat4 CameraP;

uniform vec3 ssaoSample[64];
uniform sampler2D ssaoNoise;

layout(std140, binding=1) uniform RenderSettings{
    vec2 screenSize;
};


float radius = 0.1;

uniform sampler2D gPosition;
uniform sampler2D gNormalDepth;

void main()
{             
    float v=64;
    vec3 N=texture2D(gNormalDepth,texCoords).xyz;
    vec3 vWorldPos=texture2D(gPosition,texCoords).xyz;
    float depth=texture2D(gNormalDepth,texCoords).w;
    vec2 ssaoTexelScale=vec2(screenSize.x/4.0,screenSize.y/4.0);
    float occlusion=0.0;
    for(int i=0;i<64;i++){
        vec3 off=ssaoSample[i];
        vec3 randomVec=texture2D(ssaoNoise,texCoords*ssaoTexelScale).xyz;
        vec3 tangent = normalize(randomVec - N * dot(randomVec, N));
        vec3 bitangent = cross(N, tangent);
        mat3 TBN = mat3(tangent, bitangent, N);
        off=normalize(TBN*off);
        vec3 samplePos=vWorldPos+off*radius;
        vec4 viewSpace=CameraV*CameraM*vec4(samplePos,1.0);
        vec4 clip=CameraP*vec4(viewSpace);
        clip.xyz/=clip.w;
        clip.xyz=clip.xyz*0.5+0.5;
        if(clip.x<0||clip.x>1||clip.y<0||clip.y>1)
            continue;
        float sampleDepth= -(texture2D(gNormalDepth,clip.xy).w);
        
        //float rangeCheck = smoothstep(0.0, 1.0, radius / abs(viewSpace.z - sampleDepth ));
        occlusion += (sampleDepth >= viewSpace.z &&sampleDepth<viewSpace.z+0.1 ? 1.0 : 0.0);     
    }
    ssaoColor=1.0-vec4(occlusion/64.0);
}  