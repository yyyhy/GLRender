#version 330 core
  
in vec2 texCoords;
uniform sampler2D tex;

void main()
{             
    vec3 col;
    int cnt=0;
    vec2 size=1.0/textureSize(tex,0);
    for(int i=-4;i<=4;i++){
        for(int j=-4;j<=4;j++){
            col+=texture2D(tex,texCoords+vec2(float(i),float(j))*size).xyz;
            cnt++;
        }
    }
    gl_FragColor=vec4(col/cnt,1.0);
}  