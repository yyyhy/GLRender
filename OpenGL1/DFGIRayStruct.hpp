#pragma once


#ifndef DFGI_RAY_STRUCT
#define DFGI_RAY_STRUCT

#include"glm.hpp"

struct DFGIRay
{
	glm::vec3 direction;
	glm::vec3 hitPosition;
	glm::vec3 flux;

	unsigned refCount;
};
#endif // !DFGI_RAY_STRUCT
