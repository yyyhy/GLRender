#version 330 core

struct ReflectCube{
    samplerCube reflectCube;
    float distance;
    bool exist;
};
struct DirectLight{
    vec3 uLightColor;
    vec3 uLightDir;
    bool exist;
    sampler2D shadowMap0;
    mat4 lightMVP;
};
struct PointLight{
    vec3 col;
    vec3 pos;
    float Kl;
    float Kq;
    bool exist;
};

uniform float uRoughness;
uniform float uMetallic;
uniform sampler2D albedoMap;
uniform sampler2D metallicMap;
uniform sampler2D normalMap;
uniform ReflectCube skyBox;
uniform ReflectCube reflectCube0;

#define RSM 0
#define PCF 1
#define PCSS 2
uniform int shadowType=RSM;

uniform DirectLight mainLight;
uniform PointLight pointLight0;
uniform PointLight pointLight1;
uniform PointLight pointLight2;
uniform PointLight pointLight3;

in vec2 TexCoords;
in vec3 Normal;
in vec3 vWorldPos;
in mat3 TBN;
in vec3 uCameraPos;

#define PI 3.1415926
#define PI2 6.283185307179586
#define LIGHT_WIDTH 0.3
#define NUM_SAMPLES 20
#define BLOCKER_SEARCH_NUM_SAMPLES 10
#define EPS 0.0001

float unpack(vec4 rgbaDepth) {
    const vec4 bitShift = vec4(1.0, 1.0/256.0, 1.0/(256.0*256.0), 1.0/(256.0*256.0*256.0));
    return dot(rgbaDepth, bitShift);
}

vec2 poissonDisk[NUM_SAMPLES];

highp float rand_1to1(highp float x ) { 
  // -1 -1
  return fract(sin(x)*10000.0);
}

highp float rand_2to1(vec2 uv ) { 
  // 0 - 1
	const highp float a = 12.9898, b = 78.233, c = 43758.5453;
	highp float dt = dot( uv.xy, vec2( a,b ) ), sn = mod( dt, PI );
	return fract(sin(sn) * c);
}

void uniformDiskSamples( const in vec2 randomSeed ) {

  float randNum = rand_2to1(randomSeed);
  float sampleX = rand_1to1( randNum ) ;
  float sampleY = rand_1to1( sampleX ) ;

  float angle = sampleX * PI2;
  float radius = sqrt(sampleY);

  for( int i = 0; i < NUM_SAMPLES; i ++ ) {
    poissonDisk[i] = vec2( radius * cos(angle) , radius * sin(angle)  );

    sampleX = rand_1to1( sampleY ) ;
    sampleY = rand_1to1( sampleX ) ;

    angle = sampleX * PI2;
    radius = sqrt(sampleY);
  }
}

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

vec3 calcMicroFacedBRDF(vec3 N,vec3 L,vec3 V){
    
    vec3 albedo=texture(albedoMap,TexCoords).xyz;
    
    float rough=1-texture(metallicMap,TexCoords).x;
    rough=0.5;
    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, uMetallic);

    vec3 H = normalize(V + L);
    float NdotL = max(dot(N, L), 0.0001); 
    float NdotV = max(dot(N, V), 0.0001);
    float VdotH=max(dot(V,H),0.0001);
    float NdotH=max(dot(N,H),0.0001);

    float NDF = DistributionGGX(NdotH, rough);   
    float G   = GeometrySmith(NdotL,NdotV, rough); 
    vec3 F = fresnelSchlick(F0, VdotH);

    vec3 numerator    = NDF * G * F; 
    float denominator = (4.0 * NdotL * NdotV);
    vec3 BRDF = numerator / denominator;
    return BRDF;
}

vec3 calcDiffuse(vec3 N,vec3 L){
    return mainLight.uLightColor*max(0.0,dot(N,L));
}

vec3 calcSpec(vec3 N,vec3 L,vec3 V){
    vec3 H=normalize(L+V);
    return mainLight.uLightColor*pow(max(0.0,dot(H,N)),60.0);
}

vec3 calcAmbient(){
    return vec3(0.1,0.1,0.1);
}

float RSMShadow(sampler2D shadowMap, vec4 shadowCoords,float bias){
    shadowCoords.xyz/=shadowCoords.w;
    vec3 uv=shadowCoords.xyz*0.5+0.5;
    float depthInRSM=unpack(texture2D(shadowMap,uv.xy));

    return depthInRSM-uv.z<-bias? 0.0:1.0;
}

float PCFShadow(sampler2D shadowMap, vec4 shadowCoords,float bias){
    uniformDiskSamples(shadowCoords.xy);
    shadowCoords.xyz/=shadowCoords.w;
    vec3 uv=shadowCoords.xyz*0.5+0.5;
    float v=1.0;
    for(int i=0;i<NUM_SAMPLES;i++){
        float depthInRSM=unpack(texture2D(shadowMap,uv.xy+poissonDisk[i]/NUM_SAMPLES/10));
        if(depthInRSM-uv.z<-bias)
            v-=1.0/float(NUM_SAMPLES);
    }

    return v;
}

float findBlocker( sampler2D shadowMap,  vec2 uv, float zReceiver ) {
    float alldepth;
    float cnt;
    for(int i=-BLOCKER_SEARCH_NUM_SAMPLES/2;i<BLOCKER_SEARCH_NUM_SAMPLES/2;i++){
        for(int j=-BLOCKER_SEARCH_NUM_SAMPLES/2;j<BLOCKER_SEARCH_NUM_SAMPLES/2;j++){
            float depth=unpack(texture2D(shadowMap,uv+vec2(i,j)/200).rgba);
            if(depth-zReceiver<-EPS){
            alldepth+=depth;
            cnt++;
        }
    }
        
    }
    float avgdepth=alldepth/cnt;
    return avgdepth;
}

float PCSSShadow(sampler2D shadowMap, vec4 shadowCoords,float bias){
    float v=1.0;
    uniformDiskSamples(shadowCoords.xy);
    shadowCoords.xyz/=shadowCoords.w;
    vec3 uv=shadowCoords.xyz*0.5+0.5;

    float avgdepth=findBlocker(shadowMap,uv.xy,uv.z);
    float b=(uv.z-avgdepth)/avgdepth;

    float w=float(LIGHT_WIDTH)*b;
    for(int i=0;i<NUM_SAMPLES;i++){
        float depth=unpack(texture2D(shadowMap,uv.xy+poissonDisk[i]/float(NUM_SAMPLES*2)*w).rgba);
        if(depth-uv.z<-EPS)
        v-=1.0/float(NUM_SAMPLES);
  }
  return v;
    return v;
}

float calcShadow(sampler2D shadowMap, vec4 shadowCoords,float bias){
    
    float shadow;
    if(shadowType==PCF)
        shadow=PCFShadow(mainLight.shadowMap0,shadowCoords,bias);
    else if(shadowType==PCSS)
        shadow=PCSSShadow(mainLight.shadowMap0,shadowCoords,bias);
    else
        shadow=RSMShadow(mainLight.shadowMap0,shadowCoords,bias);

    return shadow;
}

void main()
{
    
    vec3 N=normalize(Normal);
    //gl_FragColor = vec4(N,1);return;
    vec3 normal=texture2D(normalMap,TexCoords).xyz;
    normal=normal*2.0-1.0;
    N=TBN*normal;
    vec3 V=normalize(uCameraPos - vWorldPos);;
    vec3 L;
    vec3 Lo=vec3(0.0,0.0,0.0);
    vec3 radiance;
    float NdotL; 
    vec3 BRDF;
    vec3 color = Lo;
    
    float alpha=texture(albedoMap,TexCoords).w;
    if(mainLight.exist){
        vec4 shadowCoords=mainLight.lightMVP*vec4(vWorldPos,1.0);
        L = normalize(-mainLight.uLightDir);
        radiance=mainLight.uLightColor;
        NdotL=max(dot(N, L), 0.0); 
        float bias=max(EPS,(1-NdotL)*0.01);
        float shadow=calcShadow(mainLight.shadowMap0,shadowCoords,bias);
        BRDF = calcMicroFacedBRDF(N,L,V);
        Lo += max(vec3(0.0,0.0,0.0),BRDF * radiance * NdotL)*shadow;
        
    }

    if(pointLight0.exist){
        L=normalize(pointLight0.pos-vWorldPos);
        NdotL=max(dot(N, L), 0.0); 
        float dis=sqrt(dot(pointLight0.pos-vWorldPos,pointLight0.pos-vWorldPos));
        radiance=pointLight0.col/(pointLight0.Kl*dis+pointLight0.Kq*dis*dis+1.0);
        BRDF = calcMicroFacedBRDF(N,L,V);
        Lo += max(vec3(0.0,0.0,0.0),BRDF * radiance * NdotL);
    }

    if(skyBox.exist){
        vec3 R=reflect(-V,N);
        NdotL=max(dot(N, R), 0.0); 
        BRDF = calcMicroFacedBRDF(N,R,V);
        vec3 refCol=NdotL*BRDF*texture(skyBox.reflectCube,R).xyz;
        //Lo+=refCol;
    }
    
    color = Lo;
    // color = color / (color + vec3(1.0));
    // color = pow(color, vec3(1.0/2.2)); 
    
    gl_FragColor = vec4(color,alpha);
    return;
}