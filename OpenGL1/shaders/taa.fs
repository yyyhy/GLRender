#version 330 core
  
in vec2 texCoords;


uniform sampler2D tex;
uniform sampler2D lastFrame;
uniform sampler2D gVelo;
uniform sampler2D gNormalDepth;
uniform bool init=true;
uniform mat4 lastCameraMVP;
#define INF 0x777ffff
#define MAX_VEC3 vec3(INF)
#define MIN_VEC3 vec3(-INF)

in float I;
in float N;

#define CLIP_TO_CENTER
vec3 ClipAABB(vec3 aabbMin, vec3 aabbMax, vec3 prevSample, vec3 avg){
#ifdef CLIP_TO_CENTER
	// note: only clips towards aabb center (but fast!)
	vec3 p_clip = 0.5 * (aabbMax + aabbMin);
	vec3 e_clip = 0.5 * (aabbMax - aabbMin);

	vec3 v_clip = prevSample - p_clip;
	vec3 v_unit = v_clip.xyz / e_clip;
	vec3 a_unit = abs(v_unit);
	float ma_unit = max(a_unit.x, max(a_unit.y, a_unit.z));

	if (ma_unit > 1.0)
		return p_clip + v_clip / ma_unit;
	else
		return prevSample;// point inside aabb
#else
	vec3 r = prevSample - avg;
	vec3 rmax = aabbMax - avg.xyz;
	vec3 rmin = aabbMin - avg.xyz;

	const float eps = 0.000001f;

	if (r.x > rmax.x + eps)
		r *= (rmax.x / r.x);
	if (r.y > rmax.y + eps)
		r *= (rmax.y / r.y);
	if (r.z > rmax.z + eps)
		r *= (rmax.z / r.z);

	if (r.x < rmin.x - eps)
		r *= (rmin.x / r.x);
	if (r.y < rmin.y - eps)
		r *= (rmin.y / r.y);
	if (r.z < rmin.z - eps)
		r *= (rmin.z / r.z);

	return avg + r;
#endif
}

vec2 getClosestUV(){
    vec2 size=1.0/textureSize(gNormalDepth,0);
    vec2 closestUV=texCoords;
    float closestDepth=99999;
    for(int i=-1;i<=1;i++){
        for(int j=-1;j<=1;j++){
            vec2 uv=texCoords+size*vec2(i,j);
            float depth=texture2D(gNormalDepth,uv).w;
            if(depth<closestDepth){
                closestDepth=depth;
                closestUV=uv;
            }
        }
    }
    return closestUV;
}

void main()
{         
    vec3 Albedo = texture2D(tex, texCoords).rgb;
    vec3 color=Albedo;
    vec2 lastUV;
    vec3 lastcol;
    vec2 closestUV=getClosestUV();
    vec2 velo=texture2D(gVelo,closestUV).xy;
    lastUV=texCoords-velo;
    if(lastUV.x<0||lastUV.x>1||lastUV.y<0||lastUV.y>1){
        gl_FragColor=vec4(Albedo,1);
        return;
    }
    lastcol=texture2D(lastFrame,lastUV).xyz;
    
    vec3 minAABB=MAX_VEC3;
    vec3 maxAABB=MIN_VEC3;
    vec3 avg=vec3(0);
    vec2 texelSize=1.0/textureSize(tex,0);
    for(int i=-1;i<=1;++i){
        for(int j=-1;j<=1;j++){
            vec2 offset=texelSize*vec2(i,j);
            vec3 col=texture2D(tex,offset+lastUV).xyz;
            avg+=col;
            minAABB=min(minAABB,col);
            maxAABB=max(maxAABB,col);
        }
    }
    avg/=9.0;
    lastcol=ClipAABB(minAABB,maxAABB,lastcol,avg);
    if(lastcol.x+lastcol.y+lastcol.z>=0)
        color=mix(color,lastcol,0.95);
    
    gl_FragColor = vec4(color, 1.0);
}  