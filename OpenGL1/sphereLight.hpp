#pragma once

#ifndef SPHERE_LIGHT_H
#define SPHERE_LIGHT_H
#include"light.hpp"
#include"glm.hpp"
#include"shader.hpp"

class SphereLight : public Light {
public:
	Vector3f position;
	float radius;
	int index = 0;
	std::default_random_engine engine;
	std::uniform_real_distribution<float> dir;
	void setUpRandomEngine() {
		engine = std::default_random_engine(time(NULL));
		dir = std::uniform_real_distribution<float>(-1, 1);
	}
	SphereLight(const Spectrum& col, float flux) :Light(0,0,0,col, flux, false, false), position({0,0,0}),radius(0) {
		setUpRandomEngine();
	}

	void LoadToShader(Shader& s) override {
		s.setVec3("sphereLight["+std::to_string(index)+"].position", position);
		s.setVec3("sphereLight[" + std::to_string(index) + "].luminance", GetSpectrum());
		s.setFloat("sphereLight[" + std::to_string(index) + "].radius", radius);
	}

	Spectrum GetSpectrum() const override {
		return flux * color / (glm::pi<float>() *4.f);
	}

	Photon samplePhoton() override {
		Vector3f d = { dir(engine),dir(engine) ,dir(engine) };
		Photon p;
		p.positon = position;
		p.dir = glm::normalize(d);
		p.color = GetSpectrum();
		return p;
	}
};

#endif // !SPHERE_LIGHT_H
