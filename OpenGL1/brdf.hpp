#pragma once
#include"glm.hpp"


class BRDF {

public:
	virtual float eval(const Vector3f& V, const Vector3f& L, const float alpha,float& pdf) const = 0;

	virtual Vector3f sample(const Vector3f& V, const float alpha, const float U1, const float U2) const = 0;
};


