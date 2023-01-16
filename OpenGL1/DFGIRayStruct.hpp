#pragma once


#ifndef DFGI_RAY_STRUCT
#define DFGI_RAY_STRUCT

#include"glm.hpp"

struct DFGIRay
{
	glm::vec3 direction; char tmp0[4];
	glm::vec3 hitPosition; char tmp1[4];
	glm::vec3 flux; 
	unsigned refCount; 
};

struct DFGIGridLightInfo
{
	glm::vec3 flux; char tmp0[4];
	glm::vec3 importanceDirection; char tmp1[4];
	glm::vec3 importancePosition; char tmp2[4];
};
#endif // !DFGI_RAY_STRUCT
