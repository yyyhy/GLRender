#pragma once


#ifndef SH_H
#define SH_H

#include<math.h>
#include"glm.hpp"

#define PI 3.1415926
#define INV_PI 1.0/PI

float RadicalInverse_VdC(unsigned bits)
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

glm::vec2 Hammersley(unsigned i, unsigned N)
{
    return glm::vec2(float(i) / float(N), RadicalInverse_VdC(i));
}

float sh_0_0(float thi, float phi) {
	return 0.5 * sqrt(INV_PI);
}

float sh_1_n1(float thi, float phi) {
	float x = sin(thi) * cos(phi);
	float y = sin(thi) * sin(phi);
	return 0.5 * sqrt(1.5 * INV_PI) * (x - y);
}



#endif // !SH_H
