#version 330 core
  
in vec2 texCoords;
in vec3 uCameraPos;
in mat4 CameraMVP;
in mat4 CameraV;
in mat4 CameraM;
in mat4 CameraP;

uniform sampler2D tex;
uniform float threshhold=0.9;

float brightness(vec3 col){
    return col.x*0.3+col.y*0.6+col.z*0.1;
}

void main()
{             
    vec3 col=texture2D(tex,texCoords).xyz;
    float bright=brightness(col);
    if(bright>threshhold)
        gl_FragColor=vec4(col,1);
    else
        gl_FragColor=vec4(0);
    
}  