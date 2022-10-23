#version 330 core
  
in vec2 texCoords;


uniform sampler2D tex;
uniform float exposure=0.8;

void main()
{             
    vec3 Albedo = texture2D(tex, texCoords).rgb;
    vec3 color=Albedo;
    
    color=vec3(1.0)-exp(-color*exposure);
    color=pow(color,vec3(1.0/2.2));
    
    gl_FragColor = vec4(color, 1.0);
}  