

#ifndef BRDF_DDX_H
#define BRDF_DDX_H
#include"brdf.hpp"

class BRDFGGX : public BRDF {
public:

	virtual float eval(const Vector3f& V, const Vector3f& L, const float alpha, float& pdf) const override
	{
        if (V.z <= 0)
        {
            pdf = 0;
            return 0;
        }

        // masking
        const float LambdaV = lambda(alpha, V.z);

        // shadowing
        float G2;
        if (L.z <= 0.0f)
            G2 = 0;
        else
        {
            const float LambdaL = lambda(alpha, L.z);
            G2 = 1.0f / (1.0f + LambdaV + LambdaL);
        }

        // D
        const Vector3f H = normalize(V + L);
        const float slopex = H.x / H.z;
        const float slopey = H.y / H.z;
        float D = 1.0f / (1.0f + (slopex * slopex + slopey * slopey) / alpha / alpha);
        D = D * D;
        D = D / (3.14159f * alpha * alpha * H.z * H.z * H.z * H.z);

        pdf = fabsf(D * H.z / 4.0f / dot(V, H));
        float res = D * G2 / 4.0f / V.z;

        return res;
	}

	virtual Vector3f sample(const Vector3f& V, const float alpha, const float U1, const float U2) const override
	{
		const float phi = 2.0f * 3.14159f * U1;
		const float r = alpha * sqrtf(U2 / (1.0f - U2));
		const Vector3f N = glm::normalize(Vector3f(r * cosf(phi), r * sinf(phi), 1.0f));
		const Vector3f L = -V + 2.0f * N * dot(N, V);
		return L;
	}

private:
	float lambda(const float alpha, const float cosTheta) const
	{
		const float a = 1.0f / alpha / tanf(acosf(cosTheta));
		return (cosTheta < 1.0f) ? 0.5f * (-1.0f + sqrtf(1.0f + 1.0f / a / a)) : 0.0f;
	}

};

#endif // !BRDF_DDX_H

