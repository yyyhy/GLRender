#version 330 core
  
in vec2 texCoords;
uniform sampler2D bloomMap;
uniform sampler2D tex;
uniform float intensity=0.2;

void main()
{             
    vec3 Albedo = texture2D(tex, texCoords).rgb;
    vec3 bloom=texture2D(bloomMap,texCoords).xyz;
    gl_FragColor = vec4(Albedo+intensity*bloom,1.0);
}  