#pragma once


#ifndef POINT_LIGHT_H
#define POINT_LIGHT_H
#include "light.hpp"

class PointLight :public Light {
private:
	float Kl;
	float Kq;
	glm::vec3 position;

public:
	PointLight() :Light(), position(glm::vec3(0.f, 0.f, 0.f)),Kl(0.22f),Kq(0.2f){}
	PointLight(const Spectrum& col, float inten, const glm::vec3& pos) :Light(col, inten), position(pos), Kl(0.22f), Kq(0.2f) {}

	void SetKl(float k) { Kl = k; }

	void SetKq(float k) { Kq = k; }

	void LoadToShader(Shader& s) override {
		std::string pre = "pointLight[";
		pre.push_back(index+48);
		s.use();
		s.setVec3(pre+"].col", color * flux);
		s.setVec3(pre + "].pos", position);
		s.setFloat(pre + "].Kl", Kl);
		s.setFloat(pre + "].Kq", Kq);
		s.setBool(pre + "].exist", true);
	}

	void initBuffer(int w, int h) override{

	}
	unsigned index = 0;
};
#endif // !POINT_LIGHT_H
