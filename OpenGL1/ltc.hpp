

#ifndef LTC_H
#define LTC_H


#include "glm.hpp"
#include <iostream>


struct LTC {

	// lobe magnitude
	//nd
	float magnitude;

	// Average Schlick Fresnel term
	//fd
	float fresnel;

	// parametric representation
	float m11, m22, m13;
	Vector3f X, Y, Z;

	// matrix representation
	glm::mat3 M;
	glm::mat3 invM;
	float detM;

	LTC()
	{
		magnitude = 1;
		fresnel = 1;
		m11 = 1;
		m22 = 1;
		m13 = 0;
		X = Vector3f(1, 0, 0);
		Y = Vector3f(0, 1, 0);
		Z = Vector3f(0, 0, 1);
		update();
	}

	void update() // compute matrix from parameters
	{
		M = glm::mat3(X, Y, Z) *
			glm::mat3(m11, 0, 0,
				0, m22, 0,
				m13, 0, 1);
		invM = glm::inverse(M);
		detM = abs(glm::determinant(M));
	}

	float eval(const Vector3f& L) const
	{	
		Vector3f Loriginal = normalize(invM * L);
		Vector3f L_ = M * Loriginal;

		float l = glm::length(L_);
		float Jacobian = detM / (l * l * l);

		float D = 1.0f / 3.14159f * glm::max<float>(0.0f, Loriginal.z);

		float res = magnitude * D / Jacobian;
		return res;
	}

	Vector3f sample(const float U1, const float U2) const
	{
		const float theta = acosf(sqrtf(U1));
		const float phi = 2.0f * 3.14159f * U2;
		const Vector3f L = glm::normalize(M * Vector3f(sinf(theta) * cosf(phi), sinf(theta) * sinf(phi), cosf(theta)));
		return L;
	}
};

#endif
