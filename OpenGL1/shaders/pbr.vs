#version 420 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;
layout(location=3) in vec3 Tangent;
layout(location=4) in vec3 Bitangent;

out vec2 TexCoords;
out vec3 Normal;
out vec3 vWorldPos;
out mat3 TBN;
out vec3 uCameraPos;
uniform mat4 LightMat;
uniform mat4 trans;
uniform vec3 pos[1000];
bool instanced;

layout(std140 , binding=0) uniform CameraProperty{
    mat4 model;       //0
    mat4 view;        //64
    mat4 projection;  //128
    mat4 mvp;         //192
    mat4 lastMvp;     //256
    vec3 CameraPos;   //320
    vec2 Offset;      //336  272 
};

void main()
{
    TexCoords = aTexCoords;
    mat3 normalMatrix = mat3(transpose(inverse(trans)));
    Normal=normalMatrix*aNormal;
    TBN=mat3(Tangent,Bitangent,Normal);
    uCameraPos=CameraPos;
    if(!instanced)
        gl_Position = projection*view*model*trans*vec4(aPos, 1.0);
    else{
        mat4 tran;
        vec3 v=pos[gl_InstanceID];
        tran[0][0]=v.x;
        tran[1][1]=v.y;
        tran[2][2]=v.z;
        tran[3][3]=1;
        gl_Position = projection*view*model*tran*vec4(aPos, 1.0);
    }
    }
        