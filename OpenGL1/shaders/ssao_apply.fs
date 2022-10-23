#version 330 core
  
in vec2 texCoords;
uniform sampler2D ssaoMap;
uniform sampler2D tex;

void main()
{             
    vec3 Albedo = texture2D(tex, texCoords).rgb;
    float ao=texture2D(ssaoMap,texCoords).x;
    gl_FragColor = vec4(Albedo*ao,1.0);
}  