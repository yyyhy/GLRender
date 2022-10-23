#version 330 core
  
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
uniform int range;
uniform vec3 L;
#define PI 3.1415926
#define PI2 6.283185307179586


vec3 fresnelSchlick(vec2 uv,vec3 V,vec3 L){
    float m=texture2D(gAlbedoMetallic,uv).w;
    vec3 a=texture2D(gAlbedoMetallic,uv).xyz;
    vec3 F0=vec3(0.04,0.04,0.04);
    F0=mix(F0,a,m);
    vec3 N=texture2D(gNormalDepth,uv).xyz;
    vec3 H=normalize(N+L);
    float VdotH=max(0.00001,dot(V,H));
    return F0+(1.0-F0)*pow(1.0-VdotH,5.0);
}

float Rd(vec3 p0,vec3 p1){
    float dis=abs(p0.x-p1.x)+abs(p0.y-p1.y)+abs(p0.z-p1.z)+1.0;
    return 1/dis;
}

vec3 calcBSSRDF(vec2 uvo,vec2 uvi,vec3 Vo,vec3 Vi,vec3 L){
    vec3 bssrdf=vec3(1.0);
    vec3 fo=fresnelSchlick(uvo,Vo,L);
    vec3 fi=fresnelSchlick(uvi,Vi,L);

    vec3 po=texture2D(gPositionRoughness,uvo).xyz;
    vec3 pi=texture2D(gPositionRoughness,uvi).xyz;
    bssrdf=fo*Rd(po,pi)*fi/PI;
    return bssrdf;
}


void main()
{             
    vec4 col=texture2D(tex,texCoords);
    vec3 pos=texture2D(gPositionRoughness,texCoords).xyz;
    vec3 sss;
    vec3 Vo=normalize(uCameraPos-pos);
    vec2 size=1/textureSize(tex,0);
    for(int i=-range;i<range;i++){
        for(int j=-range;j<range;j++){
            vec2 uv=texCoords+vec2(i,j)*size;
            vec3 col=texture2D(tex,uv).xyz;
            vec3 pos=texture2D(gPositionRoughness,texCoords).xyz;
            vec3 Vi=normalize(uCameraPos-pos);
            sss+=col*calcBSSRDF(texCoords,uv,Vo,Vi,-L);
        }
    }
    gl_FragColor=vec4(sss+col.xyz,col.w);
}  