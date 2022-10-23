#version 420 core
  
in vec2 texCoords;
in vec3 uCameraPos;
in mat4 CameraMVP;
in mat4 CameraV;
in mat4 CameraM;
in mat4 CameraP;
uniform sampler2D tex;
uniform sampler2D gPositionRoughness;
uniform sampler2D gNormalDepth;
uniform sampler2D gAlbedoMetallic;
layout(std140, binding=1) uniform RenderSettings{
    vec2 screenSize;
};
#define PI 3.1415926
#define PI2 6.283185307179586

#define MAX_SAMPLE 16     
uniform vec3 ssdoSample[MAX_SAMPLE];
uniform sampler2D ssdoNoise;

#define MAX_SSDO_EPS 0.2
#define MIN_SSDO_EPS 0.005

float DistributionGGX(float NdotH, float roughness){
    float a=roughness;
    float ggx=a*a/(PI*pow(NdotH*NdotH*(a*a-1.0)+1.0,2.0));
    return ggx;
}

float GeometrySchlickGGX(float NdotV, float roughness){
    float k=pow(1.0+roughness,2.0)/8.0;
    float g=NdotV/(NdotV*(1.0-k)+k);
    return g;
}

float GeometrySmith(float NdotV,float NdotL, float roughness){
    return GeometrySchlickGGX(NdotV,roughness)*GeometrySchlickGGX(NdotL,roughness);
}

vec3 fresnelSchlick(vec3 F0, float VdotH){
    return F0+(1.0-F0)*pow(1.0-VdotH,5.0);
}

vec3 calcMicroFacedBRDF(vec3 N,vec3 L,vec3 V,vec2 uv){
    
    vec3 albedo=texture(tex,uv).xyz;
    float rough=texture(gPositionRoughness,uv).w;
    //rough=0.5;
    vec3 F0 = vec3(0.04); 
    float metallic=texture(gAlbedoMetallic,uv).w;
    //metallic=0.3;
    F0 = mix(F0, albedo, metallic);

    vec3 H = normalize(V + L);
    float NdotL = max(dot(N, L), 0.0001); 
    float NdotV = max(dot(N, V), 0.0001);
    float VdotH = max(dot(V,H),0.0001);
    float NdotH = max(dot(N,H),0.0001);

    float NDF = DistributionGGX(NdotH, rough);   
    float G   = GeometrySmith(NdotL,NdotV, rough); 
    vec3 F = fresnelSchlick(F0, VdotH);

    vec3 numerator    = NDF * G * F; 
    float denominator = (4.0 * NdotL * NdotV);
    vec3 BRDF = numerator / denominator;
    return BRDF;
}

void main()
{             
    vec3 worldPos=texture2D(gPositionRoughness,texCoords).xyz;
    vec3 N=texture2D(gNormalDepth,texCoords).xyz;
    vec3 V=normalize(uCameraPos-worldPos);
    vec3 allssdo;
    vec2 ssdoTexelScale=vec2(screenSize.x/4.0,screenSize.y/4.0);
    for(int i=0;i<MAX_SAMPLE;i++){
        float scale=5;
        vec3 offset=ssdoSample[i];
        vec3 randomVec=texture2D(ssdoNoise,texCoords*ssdoTexelScale).xyz;
        vec3 tangent = normalize(randomVec - N * dot(randomVec, N));
        vec3 bitangent = cross(N, tangent);
        mat3 TBN = mat3(tangent, bitangent, N);
        offset=normalize(TBN*offset);
        vec3 sampleWorldPos=worldPos+offset/scale;
        vec4 sampleViewSpace=CameraV*CameraM*vec4(sampleWorldPos,1.0);
        sampleViewSpace/=sampleViewSpace.w;
        vec4 sampleClipSpace=CameraP*vec4(sampleViewSpace);
        sampleClipSpace.xyz/=sampleClipSpace.w;
        sampleClipSpace.xyz=sampleClipSpace.xyz*0.5+0.5;
        if(sampleClipSpace.x<0||sampleClipSpace.x>1||sampleClipSpace.y<0||sampleClipSpace.y>1)
            continue;
        float realDepth= texture2D(gNormalDepth,sampleClipSpace.xy).w;
        if( -sampleViewSpace.z>realDepth+MIN_SSDO_EPS&&-sampleViewSpace.z<realDepth+MAX_SSDO_EPS){
            vec3 radiance=texture2D(tex,sampleClipSpace.xy).xyz;
            vec3 L=normalize(sampleWorldPos-worldPos);
            vec3 brdf=calcMicroFacedBRDF(N,L,V,texCoords);
            float NdotL=max(0.0,dot(N,L));
            allssdo+=NdotL*radiance*brdf;
        }
    }

    gl_FragColor=vec4(allssdo,0.0);
}  