#version 330 core


in vec2 texCoords;
uniform sampler2D prefilterInMap;
uniform float roughness;
void main()
{
    vec4 col;
    float cnt=0;
    vec2 texelSize=1/textureSize(prefilterInMap,0).xy;
    for(int i=-1;i<1;i++){
        for(int j=-1;j<1;j++){
            col+=texture2D(prefilterInMap,texCoords);
            cnt++;
        }
    }
    gl_FragColor= vec4(col/cnt);
    
}