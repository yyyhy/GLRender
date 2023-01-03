#version 420 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;
layout (location = 3) in vec3 tangent;
layout (location = 4) in vec3 biTangent;

out vec3 FragPos;
out vec2 TexCoords;
out vec3 Normal;
out vec3 Tangent;
out vec3 Bitangent;

uniform mat4 LightMat;
uniform mat4 trans;
void main()
{
	vec4 worldPos=trans*vec4(aPos,1.0);
	FragPos=worldPos.xyz/worldPos.w;
    gl_Position = LightMat*worldPos;

	TexCoords=texCoords;

	mat3 normalMatrix = transpose(inverse(mat3(trans)));
    Normal = normalMatrix * normal;
    Tangent=normalMatrix*tangent;
    Bitangent=normalMatrix*biTangent;
}