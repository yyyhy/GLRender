#pragma once


#ifndef DIRECT_LIGHT_H
#define DIRECT_LIGHT_H
#include "light.hpp"

class DirectLight :public Light {
private:
	glm::vec3 direction;
	std::default_random_engine engine;
	std::uniform_real_distribution<float> dis;
public:

	DirectLight():Light(),direction(glm::vec3(0.f,-1.f,0.f)){}

	DirectLight(const Spectrum& col, float inten, const glm::vec3& dir, bool genRsm) :Light(col, inten, genRsm), direction(glm::normalize(dir)) { if (genRsm) initBuffer(RSM_W, RSM_H); dis = std::uniform_real_distribution<float>(-20, 20); }

	const glm::vec3& GetDirection() const{ return direction; }
	
	void LoadToShader(Shader& s) override {
		s.use();
		s.setVec3("mainLight.uLightColor", GetSpectrum());
		s.setVec3("mainLight.uLightDir", direction);
		s.setBool("mainLight.exist", true);
		if (genShadowMap) {
			s.setTexture("mainLight.shadowMap0", rsmBuffer);
			s.setMat4("mainLight.lightMVP", GetLightMat());
		}
	}

	glm::mat4 GetLightMat() override {
		glm::mat4 projection = glm::ortho(-30.f, 30.f, -30.f, 30.f, 0.1f, 100.f);
		glm::mat4 view = glm::lookAt(-glm::normalize(direction)*60.f, glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f,1.f, 0.f));
		glm::mat4 tmp = projection * view;
		return tmp;
	}

	void initBuffer(int w, int h) override{
		rsmShader = std::make_shared<Shader>("shaders/RSM.vs", "shaders/RSM.fs");
		rsmH = h;
		rsmW = w;
		glGenFramebuffers(1, &frameBuffer);

		glGenTextures(1, &rsmBuffer);
		glBindTexture(GL_TEXTURE_2D, rsmBuffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, w, h, 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, rsmBuffer, 0);

		glGenRenderbuffers(1, &rbo);
		glBindRenderbuffer(GL_RENDERBUFFER, rbo);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, w, h);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

		glBindTexture(GL_TEXTURE_2D, 0);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!11" << std::endl;
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	Photon samplePhoton() override {
		glm::vec3 pos(dis(engine), 18, dis(engine));
		Photon p;
		p.positon = pos;
		p.color = color;
		p.dir = direction;
		return p;
	}
};
#endif // !DIRECT_LIGHT_H
