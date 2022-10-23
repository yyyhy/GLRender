#version 330 core
in vec2 texCoords;

uniform sampler2D tex;

uniform float weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);
uniform float h0=2;
uniform float h1=0.5;
uniform float h2=0.25;
uniform float theta0=4;
uniform float theta1=8;
uniform float theta2=16;

uniform int R=64;
const int baseR=32;

float Gaussian(float theta,float x,float height){
    return height/sqrt(2*3.14)/theta*pow(2.7,-x*x/2/theta/theta);
}

float g(float x){
    return 10.0/sqrt(2*3.14)*pow(2.7,-x*x/2);
}

void main()
{             
    float factor=float(R)/float(baseR);
    vec2 tex_offset = 1.0 / textureSize(tex, 0); // gets size of single texel
    vec3 result = texture(tex, texCoords).rgb*(Gaussian(theta0*factor,0,h0)
                                                +Gaussian(theta0*factor,0,h0)
                                                +Gaussian(theta0*factor,0,h0)); // current fragment's contribution
    for(int i = 1; i < R; ++i)
    {
        float g0=Gaussian(theta0*factor,i,h0);
        float g1=Gaussian(theta1*factor,i,h1);
        float g2=Gaussian(theta2*factor,i,h2);
        result += texture(tex, texCoords + vec2(tex_offset.x * i, 0.0)).rgb * (g0+g1+g2);
        result += texture(tex, texCoords - vec2(tex_offset.x * i, 0.0)).rgb * (g0+g1+g2);
    }
    gl_FragColor = vec4(result, 1.0);
}