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
	PointLight(unsigned rsmW, unsigned rsmH) :Light(rsmW,rsmH,6), position(glm::vec3(0.f, 0.f, 0.f)),Kl(0.22f),Kq(0.2f){}
	PointLight(unsigned rsmW, unsigned rsmH,const Spectrum& col, float inten, const glm::vec3& pos) :Light(rsmW, rsmH,6,col, inten), position(pos), Kl(0.22f), Kq(0.2f) {}

	void SetKl(float k) { Kl = k; }

	void SetKq(float k) { Kq = k; }

	void LoadToShader(Shader& s) override {
		std::string pre = "pointLight[";
		pre.push_back(index+48);
		s.Use();
		s.SetVec3(pre+"].col", color * flux);
		s.SetVec3(pre + "].pos", position);
		s.SetFloat(pre + "].Kl", Kl);
		s.SetFloat(pre + "].Kq", Kq);
		s.SetBool(pre + "].exist", true);
	}

	void initBuffer(int w, int h) override{

	}
	unsigned index = 0;
};
#endif // !POINT_LIGHT_H
