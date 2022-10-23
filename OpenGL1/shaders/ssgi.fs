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
uniform sampler2D gFlag;
layout(std140, binding=1) uniform RenderSettings{
    vec2 screenSize;
};
#define PI 3.1415926
#define PI2 6.283185307179586

#define MAX_STEP 1280     
#define STEP_SIZE 0.5
#define PIXEL_STEP_SIZE 0.01

float LinearizeDepth(float depth,float near,float far) 
{
    float z = depth * 2.0 - 1.0; // back to NDC 
    return (2.0 * near * far) / (far + near - z * (far - near));    
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

vec3 calcMicroFacedBRDF(vec3 N,vec3 L,vec3 V,vec2 uv){
    
    vec3 albedo=texture(tex,uv).xyz;
    float rough=texture(gPositionRoughness,uv).w;
    vec3 F0 = vec3(0.04); 
    float metallic=texture(gAlbedoMetallic,uv).w;
    F0 = mix(F0, albedo, metallic);

    vec3 H = normalize(V + L);
    float NdotL = max(dot(N, L), 0.0001); 
    float NdotV = max(dot(N, V), 0.0001);
    float VdotH = max(dot(V, H), 0.0001);
    float NdotH = max(dot(N, H), 0.0001);

    float NDF = DistributionGGX(NdotH, rough);   
    float G   = GeometrySmith(NdotL, NdotV, rough); 
    vec3 F = fresnelSchlick(F0, VdotH);

    vec3 numerator    = NDF * G * F; 
    float denominator = (4.0 * NdotL * NdotV);
    vec3 BRDF = numerator / denominator;
    return BRDF;
}

struct RayMarchingResult{
    bool IsHit;
    vec2 UV;
    vec3 Position;
    int IterationCount;
};

bool Query(vec2 z, vec2 uv)
{
    float depths = -texture(gNormalDepth, uv / vec2(screenSize.x,screenSize.y)).w;
    //return z.y < depths;
    return (z.y < depths && z.x > depths);
}

RayMarchingResult RayMarching(vec3 O,vec3 R)
{
	RayMarchingResult result;
#define u_RayLength 10000
	vec3 Begin = O;
	vec3 End = O + R * u_RayLength;

	vec3 V0 = (CameraV*CameraM*vec4(Begin,1.0)).xyz;
	vec3 V1 = (CameraV*CameraM*vec4(End,1.0)).xyz;

	vec4 H0 = CameraP*vec4(V0,1.0);
	vec4 H1 = CameraP*vec4(V1,1.0);

	float k0 = 1.0 / H0.w;
    float k1 = 1.0 / H1.w;

	vec3 Q0 = V0 * k0; 
    vec3 Q1 = V1 * k1;

	// NDC-space not Screen Space
    vec2 P0 = H0.xy * k0;
    vec2 P1 = H1.xy * k1;
	vec2 Size = screenSize;
	//Screen Space
	P0 = (P0 + 1) / 2 * Size;
	P1 = (P1 + 1) / 2 * Size;

	P1 += vec2((dot(P0-P1,P0-P1) < 0.0001) ? 0.01 : 0.0);

	vec2 Delta = P1 - P0;

	bool Permute = false;
    if (abs(Delta.x) < abs(Delta.y)) 
	{ 
        Permute = true;
        Delta = Delta.yx; P0 = P0.yx; P1 = P1.yx; 
    }
	float StepDir = sign(Delta.x);
    float Invdx = StepDir / Delta.x;
	vec3  dQ = (Q1 - Q0) * Invdx;
    float dk = (k1 - k0) * Invdx;
    vec2  dP = vec2(StepDir, Delta.y * Invdx);
	float Stride = 1.0f;

    dP *= Stride; dQ *= Stride; dk *= Stride;

    P0 += dP; Q0 += dQ; k0 += dk;
	
	int Step = 0;
    int StepSize=16;
	float k = k0;
	float EndX = P1.x * StepDir;
	vec3 Q = Q0;
	float prevZMaxEstimate = V0.z;

	for(vec2 P = P0;  Step < MAX_STEP;Step+=StepSize,P += dP*StepSize, Q.z += dQ.z*StepSize, k += dk*StepSize)
	{
        result.IterationCount+=StepSize;
		result.UV = Permute ? P.yx : P;
		vec2 Depths;
		Depths.x = prevZMaxEstimate;
		Depths.y = (dQ.z * 0.5 + Q.z) / (dk * 0.5 + k);
        Depths.y =Q.z/k;
		if(Depths.x < Depths.y)
			Depths.xy = Depths.yx;
		if(result.UV.x > screenSize.x || result.UV.x < 0 || result.UV.y > screenSize.y || result.UV.y < 0)
			break;
		result.IsHit = Query(Depths, result.UV);
		if (result.IsHit){
            if(StepSize<=2)
                break;
            else{
                Step-=StepSize;
                P -= dP*StepSize; Q.z -= dQ.z*StepSize; k -= dk*StepSize;
                StepSize/=2;
                continue;
            }
        }
        prevZMaxEstimate = Depths.y;
			
	}

	return result;
}


void main()
{             
    vec3 worldPos=texture2D(gPositionRoughness,texCoords).xyz;
    float depth=texture2D(gNormalDepth,texCoords).w;
    float flag=texture2D(gFlag,texCoords).w;
    vec3 col=texture2D(tex,texCoords).xyz;
    if(flag!=1){
        gl_FragColor=vec4(col,1);return;
    }
    vec3 N=texture2D(gNormalDepth,texCoords).xyz;
    vec3 V=normalize(uCameraPos-worldPos);
    vec3 R=normalize(reflect(-V,N));
    float NdotL=max(0.0,dot(N,R));
    vec3 gi;
    RayMarchingResult r=RayMarching(worldPos,R);
    if(r.IsHit){
        vec3 refCol=texture2D(tex,r.UV/screenSize).xyz*(MAX_STEP-r.IterationCount)/float(MAX_STEP);
        gi=refCol*calcMicroFacedBRDF(N,R,V,texCoords)*NdotL;
        //gl_FragColor=vec4(refCol,1.0);return;
    }
    gl_FragColor=vec4(col+gi,1.0);
    // if(r.intersect==1)
    //     gl_FragColor=vec4(refCol,1.0);
    // else
    //     gl_FragColor=vec4(col,1.0);
    
}  