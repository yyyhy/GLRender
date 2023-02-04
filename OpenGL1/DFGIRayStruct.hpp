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
	unsigned liveCount; char tmp2[12];
};

struct DFGIGridLightInfo
{
	glm::vec3 flux; char tmp0[4];
	glm::vec3 importanceDirection; char tmp1[4];
	glm::vec3 importancePosition; char tmp2[4];
};

struct DFGISH
{
	glm::vec4 shCi[9];
};

struct RangePair
{
	unsigned left, right;
	RangePair(unsigned v):left(v),right(v){}
	RangePair(unsigned l,unsigned r):left(l),right(r){}
	bool operator>(const RangePair& p) const {
		return right > left && left >= p.right && p.right > p.left;
	}
	bool operator<(const RangePair& p) const {
		return p.right > p.left && p.left >= right && right > left;
	}
	bool operator==(const int& p) const {
		return left <= p && right > p;
	}
};

#endif // !DFGI_RAY_STRUCT
