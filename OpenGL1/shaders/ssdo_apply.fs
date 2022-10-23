#version 330 core
  
in vec2 texCoords;
uniform sampler2D ssdoMap;
uniform sampler2D tex;

void main()
{             
    vec3 Albedo = texture2D(tex, texCoords).rgb;
    vec3 ssdo=texture2D(ssdoMap,texCoords).xyz;
    gl_FragColor = vec4(Albedo+ssdo,1.0);
}  