#pragma once


#ifndef DFGI_RAY_STRUCT
#define DFGI_RAY_STRUCT

#include"glm.hpp"

struct DFGIRay
{
	glm::vec3 direction; char tmp0[4];
	glm::vec3 hitPosition; char tmp1[4];
	glm::vec3 flux; char tmp2[4];
	unsigned refCount; char tmp3[12];
};
#endif // !DFGI_RAY_STRUCT
