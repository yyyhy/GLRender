#version 330 core
struct ReflectCube{
    samplerCube reflectCube;
    vec3 pos;
    bool exist;
};
struct DirectLight{
    vec3 uLightColor;
    vec3 uLightDir;
    bool exist;
    sampler2D shadowMap[4];
    mat4 lightMVP[4];
    int csmLevel;
    vec4 csmLevelDivide;
};
const float csmDidide=3.16228;
struct PointLight{
    vec3 col;
    vec3 pos;
    float Kl;
    float Kq;
    bool exist;
};
struct SpotLight{
    vec3 uLightColor;
    vec3 uLightDir;
    vec3 pos;
    float radius;
    bool exist;
    sampler2D shadowMap0;
    mat4 lightMVP;
};

struct SphereLight {
    vec3 position;
    vec3 luminance;
    float radius;
};

struct CapsuleLight{
    vec3 p0;
    vec3 p1;
    float radius;
    vec3 L;
};
uniform ReflectCube reflectCube[4];
uniform ReflectCube skyBox;

uniform DirectLight mainLight;
uniform PointLight pointLight[4];
uniform SpotLight spotLight;
uniform SphereLight sphereLight[4]; 
uniform bool lightMapOff;

uniform sampler2D gBuffer0;
uniform sampler2D gBuffer1;
uniform sampler2D gBuffer2;
uniform sampler2D gBuffer3;
uniform sampler2D gBuffer4;
uniform sampler2D gBuffer5;
uniform sampler2D gBuffer6;
uniform sampler2D uBRDFLut;
uniform sampler2D uEavgLut;
uniform sampler3D uSDF;

#define gPositionRoughness gBuffer0
#define gNormalDepth gBuffer1
#define gAlbedoMetallic gBuffer2
#define gTangentFlag gBuffer3
#define gLightMapDir gBuffer4
#define gLightMap gBuffer5
#define gVelo gBuffer6;

in vec2 texCoords;
in vec3 uCameraPos;
in mat4 CameraMVP;
in mat4 CameraV;
in mat4 CameraM;
in mat4 CameraP;


#define PI 3.1415926
#define PI2 6.283185307179586
#define LIGHT_WIDTH 0.3
#define NUM_SAMPLES 20
#define BLOCKER_SEARCH_NUM_SAMPLES 10
#define EPS 0.0001

float far=100.0;
float near=0.01;


float saturatef(float v){
    if(v<0)
        return 0;
    if(v>1)
        return 1;
    return v;
}

vec3 SphereLight_Illuminance(SphereLight light, vec3 vertex, vec3 norm) {
    vec3 diff = light.position - vertex;
    float H = length(diff);
    vec3 wi = diff / H;
    float cosBeta = dot(wi, norm);

    float h = H / light.radius;
    float h2 = h * h;

    float formFactor;
    if (h * cosBeta > 1)
        formFactor = cosBeta / h2;
    else {
        float sinBeta = sqrt(1 - cosBeta * cosBeta);
        float cotBeta = cosBeta / sinBeta;

        float x = sqrt(h2 - 1);
        float y = -x * cotBeta;

        formFactor = (1 / (PI * h2)) *
            (cosBeta * acos(y) - x * sinBeta * sqrt(1 - y * y)) +
            (1 / PI) * atan(sinBeta * sqrt(1 - y * y) / x);
    }
    formFactor = max(0.0, formFactor);

    return PI * formFactor * light.luminance;
}

vec3 SphereLight_MRP(SphereLight light, vec3 vertex, vec3 R) {
	vec3 fragToLight = light.position - vertex;
	vec3 LtoR = dot(fragToLight, R) * R - fragToLight;
	return light.position + saturatef(light.radius / length(LtoR)) * LtoR;
}

vec3 CapsuleLight_MRP(CapsuleLight light, vec3 p, vec3 R) {
    vec3 o1 = light.p0;
    vec3 d1 = light.p1 - light.p0;
    vec3 o2 = p;
    vec3 d2 = R;

    vec3 o12 = o2 - o1;

    float o12d1 = dot(o12, d1);
    float o12d2 = dot(o12, d2);
    float d1d1 = dot(d1, d1);
    float d1d2 = dot(d1, d2);
    float d2d2 = dot(d2, d2);

    float t = (o12d1 * d2d2 - o12d2 * d1d2) / (d1d1 * d2d2 - d1d2 * d1d2);
    vec3 closestP = o1 + saturatef(t) * d1;

    SphereLight sphereLight = SphereLight(closestP, light.L, light.radius);
    return SphereLight_MRP(sphereLight, p, R);
}

float LinearizeDepth(float depth,float near,float far) 
{
    float z = depth * 2.0 - 1.0; // back to NDC 
    return (2.0 * near * far) / (far + near - z * (far - near));    
}

float unpack(vec4 rgbaDepth) {
    const vec4 bitShift = vec4(1.0, 1.0/256.0, 1.0/(256.0*256.0), 1.0/(256.0*256.0*256.0));
    return dot(rgbaDepth, bitShift);
}

vec2 poissonDisk[NUM_SAMPLES];

float RadicalInverse_VdC(uint bits) 
{
     bits = (bits << 16u) | (bits >> 16u);
     bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
     bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
     bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
     bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
     return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

vec2 Hammersley(uint i, uint N)
{
	return vec2(float(i)/float(N), RadicalInverse_VdC(i));
}

vec3 hemisphereSample_uniform(float u, float v) {
     float phi = v * 2.0 * PI;
     float cosTheta = 1.0 - u;
     float sinTheta = sqrt(1.0 - cosTheta * cosTheta);
     return vec3(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta);
 }

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

highp float ramdom(float seed){
    float r=seed*432121.2;
    return fract(sin(r));
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

float saturate(float a){
    if(a<0)
        return 0.0;
    if(a>1)
        return 1.0;
    return a;
}

vec3 MicrofacedLUT(float roughness,float smoothness,vec3 BRDFspe,vec3 F0,float NdotV){
   
    float SurReduction=1.0/(roughness*roughness+1);
    float Reflectivity=max(max(BRDFspe.x,BRDFspe.y),BRDFspe.z);
    float GrazingTSection=saturate(Reflectivity+smoothness);
    float Fre=pow(1-NdotV,4);
    return mix(F0,vec3(GrazingTSection),Fre)*SurReduction;
} 

vec3 calcMicroFacedBRDF(vec3 N,vec3 L,vec3 V,vec2 tex){
    
    vec3 albedo=texture(gBuffer2,tex).xyz;
    float rough=texture(gBuffer0,tex).w;
    //rough=0.5;
    vec3 F0 = vec3(0.04); 
    float metallic=texture(gBuffer2,tex).w;
    //metallic=0.5;
    F0 = mix(F0, albedo, metallic);

    vec3 H = normalize(V + L);
    float NdotL = max(dot(N, L), 0.001); 
    float NdotV = max(dot(N, V), 0.001);
    float VdotH = max(dot(V,H),0.001);
    float NdotH = max(dot(N,H),0.001);

    float NDF = DistributionGGX(NdotH, rough);   
    float G   = GeometrySmith(NdotL,NdotV, rough); 
    vec3 F = fresnelSchlick(F0, VdotH);

    vec3 numerator    = NDF * G * F; 
    float denominator = (4.0 * NdotL * NdotV);
    vec3 BRDF = numerator / vec3(denominator,denominator,denominator);
    return BRDF;
}

vec3 AverageFresnel(vec3 r, vec3 g)
{
    return vec3(0.087237) + 0.0230685*g - 0.0864902*g*g + 0.0774594*g*g*g
           + 0.782654*r - 0.136432*r*r + 0.278708*r*r*r
           + 0.19744*g*r + 0.0360605*g*g*r - 0.2586*g*r*r;
}

vec3 MultiScatterBRDF(float NdotL, float NdotV)
{
    vec3 albedo = pow(texture2D(gBuffer2, texCoords).rgb, vec3(2.2));
    float rough=texture2D(gBuffer0,texCoords).w;
    vec3 E_o = texture2D(uBRDFLut, vec2(NdotL, rough)).xyz;
    vec3 E_i = texture2D(uBRDFLut, vec2(NdotV, rough)).xyz;

    vec3 E_avg = texture2D(uEavgLut, vec2(0, rough)).xyz;
    // copper
    vec3 edgetint = vec3(0.827, 0.792, 0.678);
    vec3 F_avg = AverageFresnel(albedo, edgetint);
  
    // TODO: To calculate fms and missing energy here
    vec3 fs=(1-E_o)*(1-E_i)/PI*(1-E_avg);
    vec3 fadd=F_avg*E_avg/(1-F_avg*(1-E_avg));

    return fs*fadd;
  
}

vec3 calcWaterBRDF(vec3 N,vec3 L,vec3 V,vec3 F0){
    float rough=0.001;

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

float RSMShadow(sampler2D shadowMap, vec4 shadowCoords,float bias){
    shadowCoords.xyz/=shadowCoords.w;
    vec3 uv=shadowCoords.xyz*0.5+0.5;
    float depthInRSM=unpack(texture2D(shadowMap,uv.xy));

    return depthInRSM-uv.z<-bias? 0.0:1.0;
}

float PCFShadow(sampler2D shadowMap, vec4 shadowCoords,float bias,bool unPack,bool linearZ){
    //uniformDiskSamples(shadowCoords.xy);
    shadowCoords.xyz/=shadowCoords.w;
    vec3 uv=shadowCoords.xyz*0.5+0.5;
    if(uv.x<0||uv.y>1||uv.x>1||uv.y<0)
        return 1;
    float d=uv.z;
    if(linearZ){
        d=LinearizeDepth(uv.z,0.01,100)/100.0;
    }
        
    float v=1.0;
    // for(int i=0;i<NUM_SAMPLES;i++){
    //     float depthInRSM=unpack(textureLod(shadowMap,uv.xy+poissonDisk[i]/NUM_SAMPLES/10,0));
    //     if(depthInRSM-uv.z<-bias)
    //         v-=1.0/float(NUM_SAMPLES);
    // }
    vec2 size=1.0/textureSize(shadowMap,0);
    
    for(int i=-4;i<4;i++){
        for(int j=-4;j<4;j++){
            float depth=1;
            if(unPack)
                depth=unpack(texture2D(shadowMap,uv.xy+vec2(i,j)*size));
            else
                depth=texture2D(shadowMap,uv.xy+vec2(i,j)*size).w/100.0;
            if(depth-d<-bias)
                v-=1.0/float(64);
        }
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
}

float calcShadow(sampler2D shadowMap, vec4 shadowCoords,float bias,bool unPack,bool lz){
    
    float shadow;
    //if(shadowType==PCF)
        shadow=PCFShadow(shadowMap,shadowCoords,bias,unPack,lz);
    // else if(shadowType==PCSS)
    //     shadow=PCSSShadow(mainLight.shadowMap0,shadowCoords,bias);
    // else
    //     shadow=RSMShadow(mainLight.shadowMap0,shadowCoords,bias);

    return shadow;
}

uint getCsmLevel(float depth){
    uint level=0U;
    if(depth<mainLight.csmLevelDivide.x)
        level=0U;
    else if(depth<mainLight.csmLevelDivide.y)
        level=1U;
    else if(depth<mainLight.csmLevelDivide.z)
        level=2U;
    else if(depth<mainLight.csmLevelDivide.w)
        level=3U;
    return level;
}

vec3 calcIBL(ReflectCube cube,vec3 N,vec3 V,vec2 texCoords,vec3 BRDFdir){
    vec3 ibl=vec3(0);
    vec3 R=reflect(-V,N);
    vec3 BRDF = calcMicroFacedBRDF(N,R,V,texCoords);
    vec3 F0 = vec3(0.04); 
    float metallic=texture(gBuffer2,texCoords).w;
    float roughness=texture2D(gBuffer0,texCoords).w;
    vec3 albedo=texture(gBuffer2,texCoords).xyz;
    F0 = mix(F0, albedo, metallic);
    float NdotV=max(0.0001,dot(N,V));
    float NdotL=max(0.0001,dot(N,R));
    BRDF=MicrofacedLUT(roughness,1-roughness,BRDFdir,F0,NdotV);
    vec3 radiance=textureLod(cube.reflectCube,R,roughness*3.0).xyz;
    ibl=BRDF*radiance;
    return ibl;
}

bool usIsValid(vec4 texCoords){
    vec2 uv=texCoords.xy/texCoords.w;
    return uv.x>=-1&&uv.x<=1&&uv.y>=-1&&uv.y<=1;
}

void main()
{             
    vec3 N=texture2D(gBuffer1,texCoords).xyz;
    vec3 albedo=texture(gBuffer2,texCoords).xyz;
    float AO=texture2D(gBuffer3,texCoords).w;
    float wet=texture2D(gBuffer4,texCoords).w;
    vec3 vWorldPos=texture2D(gBuffer0,texCoords).xyz;
    vec3 V=normalize(uCameraPos - vWorldPos);
    float NdotV=max(0,dot(N,V));
    float depth=texture2D(gBuffer1,texCoords).w;
    float sdf=texture(uSDF, normalize(abs(vWorldPos))).r;
    //gl_FragColor=vec4(sdf);return;
    // vec3 lightmapcol=texture2D(gLightMap,texCoords).xyz;
    // vec3 lightMapDir=normalize(texture2D(gLightMapDir,texCoords).xyz);
    vec3 L=vec3(0);
    vec3 Lo=vec3(0);
    vec3 radiance=vec3(0);
    float NdotL=0; 
    vec3 BRDF=vec3(0);
    vec3 BRDFdir=vec3(0);
    uint level=getCsmLevel(depth);
    vec4 shadowCoords=mainLight.lightMVP[level]*vec4(vWorldPos,1.0);
    
    
    if(mainLight.exist){
        L = normalize(-mainLight.uLightDir);
        radiance=mainLight.uLightColor;
        NdotL=max(dot(N, L), 0.0); 
        float bias=max(EPS,(1-NdotL)*0.0001);
        float shadow=0;
        if(level==0U)
       	    shadow=calcShadow(mainLight.shadowMap[0],shadowCoords,bias,true,false);
        else if(level==1U)
       	    shadow=calcShadow(mainLight.shadowMap[1],shadowCoords,bias,true,false);
        else if(level==2U)
       	    shadow=calcShadow(mainLight.shadowMap[2],shadowCoords,bias,true,false);
        else 
       	    shadow=calcShadow(mainLight.shadowMap[3],shadowCoords,bias,true,false);
        // if(level<3U){
        //     float csmD=mainLight.csmLevelDivide[level];
        //     float csmDNext=mainLight.csmLevelDivide[level+1U];
        //     float layer=csmDPre+(csmDNext-csmD)*0.2;
        //     if(depth<layer){
        //         float radio=(depth-csmDPre)/(csmD-csmDPre);
        //         int l=int(level)-1;
        //         shadowCoords=mainLight.lightMVP[l]*vec4(vWorldPos,1.0);
        //         if(shadowCoords.x>0&&shadowCoords.y>0&&shadowCoords.x<1&&shadowCoords.y<1){
        //             float s=calcShadow(mainLight.shadowMap[l],shadowCoords,bias,true,false);
        //             shadow=shadow*radio+s*(1-radio);
        //         }
        //     }
        // }
        
        BRDF = calcMicroFacedBRDF(N,L,V,texCoords);
        BRDF +=MultiScatterBRDF(NdotL,NdotV);
        BRDFdir = BRDF;
        Lo += BRDF*radiance * NdotL*shadow;

	    //gl_FragColor = vec4(Lo, 1.0);return;
        // if(shadow<0.5){
        //     //if(lightmapcol.x+lightmapcol.y+lightmapcol.z>0.1){
        //         BRDF=calcMicroFacedBRDF(N,-lightMapDir,V,texCoords);
        //         vec3 albedo=texture2D(gAlbedoMetallic,texCoords).xyz;
        //         NdotL=max(0,dot(N,-lightMapDir));
        //         Lo+=BRDF*lightmapcol*(0.5-shadow)/8*NdotL;
        //         //gl_FragColor=vec4(Lo,1);return;
        //     //}
        // }
        
        //Lo+=mainLight.uLightColor*wet*pow(max(0.0,dot(N,normalize(L+V))),500)*shadow;

    }
    for(int i=0;i<4;i++){
        if(pointLight[i].exist){
            L=normalize(pointLight[i].pos-vWorldPos);
            NdotL=max(dot(N, L), 0.0); 
            float dis=sqrt(dot(pointLight[i].pos-vWorldPos,pointLight[i].pos-vWorldPos));
            radiance=pointLight[i].col/(pointLight[i].Kl*dis+pointLight[i].Kq*dis*dis+1.0);
            BRDF = calcMicroFacedBRDF(N,L,V,texCoords);
            Lo += max(vec3(0.0,0.0,0.0),BRDF * radiance * NdotL);
        }
    }
    float dis=999;
    vec3 ibl=vec3(0);
    float totolFactor=0;
    for(int i=0;i<4;i++){
        if(reflectCube[i].exist){
            float d=sqrt(dot(vWorldPos-reflectCube[i].pos,vWorldPos-reflectCube[i].pos));
            dis=min(dis,d);
        }
    }
    for(int i=0;i<4;i++){
        if(reflectCube[i].exist){
            float d=sqrt(dot(vWorldPos-reflectCube[i].pos,vWorldPos-reflectCube[i].pos));
            float factor=dis/max(d,0.001);
            totolFactor+=factor;
            ibl+=calcIBL(reflectCube[i],N,V,texCoords,BRDFdir)*factor;
        }
    }
    if(ibl.x>0&&ibl.y>0&&ibl.z>0)
    Lo+=ibl/totolFactor;
    
    if(spotLight.exist){
        vec3 l2o=normalize(vWorldPos-spotLight.pos);
        float anglecos=dot(l2o,spotLight.uLightDir);
        float angle=acos(anglecos);
        if(anglecos>0&&angle*180/PI<spotLight.radius/2){
            L=-spotLight.uLightDir;
            float dis2=dot(vWorldPos-spotLight.pos,vWorldPos-spotLight.pos);
            radiance=spotLight.uLightColor/dis2*(1-angle*180/PI/(spotLight.radius/2));
            NdotL=max(dot(N, L), 0.0); 
            float bias=max(EPS,(1-NdotL)*0.01);
            shadowCoords=spotLight.lightMVP*vec4(vWorldPos,1.0);
            float shadow=calcShadow(spotLight.shadowMap0,shadowCoords,bias,false,true);
            BRDF = calcMicroFacedBRDF(N,L,V,texCoords);
            BRDF+=MultiScatterBRDF(NdotL,NdotV);
            Lo += BRDF*radiance * NdotL*shadow;
        }
    }
    {
        // if(!lightMapOff){
        //     vec3 lm=texture2D(gLightMap,texCoords).xyz;
        //     vec3 ld=normalize(texture2D(gLightMapDir,texCoords).xyz);
        //     BRDF=calcMicroFacedBRDF(N,-ld,V,texCoords);
        //     vec3 pm=BRDF*lm;
        //     Lo+=pm;
        //     //gl_FragColor=vec4(lm,1);return;

        // }
    }
    for(int i=0;i<4;i++){
        vec3 R=normalize(reflect(-V,N));
        vec3 p=SphereLight_MRP(sphereLight[i],vWorldPos,R);
        L=normalize(p-vWorldPos);
        BRDF=calcMicroFacedBRDF(N,L,V,texCoords);
        Lo+=sphereLight[i].luminance/dot(p-vWorldPos,p-vWorldPos)*BRDF*max(0,dot(L,N));
    }
    
    vec3 color=Lo;
    
    gl_FragColor = vec4(color, 1.0);
}  