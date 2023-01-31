#pragma once


#ifndef SH_H
#define SH_H

#include<math.h>
#include"glm.hpp"


#define PI 3.1415926
#define INV_PI 1.0/PI

inline float RadicalInverse_VdC(unsigned bits)
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

inline glm::vec2 Hammersley(unsigned i, unsigned N)
{
    return glm::vec2(float(i) / float(N), RadicalInverse_VdC(i));
}

inline glm::vec3 ImportanceSampleGGX(const glm::vec2& Xi, const glm::vec3& N, float roughness)
{
	
	float a = roughness * roughness;

	float phi = 2.0 * glm::pi<float>() * Xi.x;
	float cosTheta = std::sqrt((1.0 - Xi.y) / (1.0 + (a * a - 1.0) * Xi.y));
	float sinTheta = std::sqrt(1.0 - cosTheta * cosTheta);

	// from spherical coordinates to cartesian coordinates - halfway vector
	glm::vec3 H;
	H.x = cos(phi) * sinTheta;
	H.y = sin(phi) * sinTheta;
	H.z = cosTheta;

	// from tangent-space H vector to world-space sample vector
	glm::vec3 up = abs(N.z) < 0.999 ? glm::vec3(0.0, 0.0, 1.0) : glm::vec3(1.0, 0.0, 0.0);
	glm::vec3 tangent = normalize(cross(up, N));
	glm::vec3 bitangent = cross(N, tangent);

	glm::vec3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
	return glm::normalize(sampleVec);

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
