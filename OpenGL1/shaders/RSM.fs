#version 430 core
#define PACK 0
#define LinearZ 1
uniform int DepthPackWay;

layout (location = 0) out vec4 RSMAlbedoDepth;
layout (location = 1) out vec4 RSMNormalRoughness;
layout (location = 2) out vec4 RSMPositionMetallic;

vec4 pack (float depth) {
    // 使用rgba 4字节共32位来存储z值,1个字节精度为1/256
    const vec4 bitShift = vec4(1.0, 256.0, 256.0 * 256.0, 256.0 * 256.0 * 256.0);
    const vec4 bitMask = vec4(1.0/256.0, 1.0/256.0, 1.0/256.0, 0.0);
    // gl_FragCoord:片元的坐标,fract():返回数值的小数部分
    vec4 rgbaDepth = fract(depth * bitShift); //计算每个点的z值
    rgbaDepth -= rgbaDepth.gbaa * bitMask; // Cut off the value which do not fit in 8 bits
    return rgbaDepth;
}

float far=200.0;
float near=0.1;

float LinearizeDepth(float depth) 
{
    float z = depth * 2.0 - 1.0; // back to NDC 
    return (2.0 * near * far) / (far + near - z * (far - near));    
}

void main()
{
    if(DepthPackWay==PACK)
        RSMAlbedoDepth=vec4(pack(gl_FragCoord.z));
    else if(DepthPackWay==LinearZ)
        RSMAlbedoDepth=vec4(LinearizeDepth(gl_FragCoord.z));
}