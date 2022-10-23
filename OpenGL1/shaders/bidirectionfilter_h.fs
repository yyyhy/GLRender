#version 330 core


in vec2 texCoords;
uniform sampler2D tex;
uniform sampler2D gPosition;
uniform sampler2D gNormal;
#define SAMPLE_RANGE 16
#define E 2.7
#define INV_PI 0.3989422804
#define PI 3.1415926
#define PI2 6.283185307179586
float GaussianDistribution(float x){
    return pow(exp(-x*x/2.0),8.0);
}

float norm(vec2 v){
    return dot(v,v);
}

float norm(vec3 v){
    return dot(v,v);
}

void main()
{
    vec3 colors;
    float factors;
    vec3 col=texture2D(tex,texCoords).xyz;
    vec3 N=texture2D(gNormal,texCoords).xyz;
    vec3 P=texture2D(gPosition,texCoords).xyz;
    vec2 texelSize=1.0/textureSize(tex,0);
    for(int i=-SAMPLE_RANGE;i<SAMPLE_RANGE;i++){
        vec2 offset=vec2(0,i)*texelSize;
        vec2 sample=offset+texCoords;
        vec3 sampleColor=texture2D(tex,sample).xyz;
        vec3 sampleColorOff=texture2D(tex,sample).xyz-col;
        vec3 sampleNOff=texture2D(gNormal,sample).xyz-N;
        vec3 samplePOff=texture2D(gPosition,sample).xyz-P;
        float factor=GaussianDistribution(norm(offset))*GaussianDistribution(norm(sampleColorOff))
        *GaussianDistribution(norm(sampleNOff))*GaussianDistribution(norm(samplePOff));
        factors+=factor;
        colors+=sampleColor*factor;
    }
    gl_FragColor=vec4(colors/factors,1.0);
}