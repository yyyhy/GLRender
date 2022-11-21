#pragma once


#ifndef DIRECT_LIGHT_H
#define DIRECT_LIGHT_H
#include "light.hpp"
#include"sh.hpp"

const static unsigned CSM_MAX_LEVEL = 4;
const static float CSM_AUJUST_FACTOR = 0.9f;

class DirectLight :public Light {
private:
	glm::vec3 direction;
	std::default_random_engine engine;
	std::uniform_real_distribution<float> dis;
	std::uniform_real_distribution<float> dis1;
	std::vector<glm::mat4> currMats;
	glm::vec4 csmLevelDivide;
	float radius;
	void setUpRandomEngine() {
		engine = std::default_random_engine(time(NULL));
		dis = std::uniform_real_distribution<float>(-18, 18);
		dis1 = std::uniform_real_distribution<float>(-8, 8);
	}

public:

	DirectLight():Light(),direction(glm::vec3(0.f,-1.f,0.f)),radius(50){
		setUpRandomEngine();
	}

	DirectLight(const Spectrum& col, float flux, const glm::vec3& dir, bool genRsm) :
		Light(col, flux, genRsm,false,CSM_MAX_LEVEL), direction(glm::normalize(dir)),radius(50) { 
		if (genRsm) initBuffer(RSM_W/4, RSM_H/4); 
		setUpRandomEngine();
	}

	const glm::vec3& GetDirection() const{ return direction; }
	
	void LoadToShader(Shader& s) override {
		s.use();
		s.setVec3("mainLight.uLightColor", GetSpectrum()/(radius*radius*glm::pi<float>()));
		s.setVec3("mainLight.uLightDir", direction);
		s.setBool("mainLight.exist", true);
		if (genShadowMap) {
			//auto div = GetCsmDivide();
			for (unsigned i = 0; i < CSM_MAX_LEVEL; ++i) {
				s.setTexture("mainLight.shadowMap["+std::to_string(i)+"]", rsmBuffer[i]);
				s.setMat4("mainLight.lightMVP[" + std::to_string(i)+"]", currMats.at(i));
			}
			s.setInt("mainLight.csmLevel", CSM_MAX_LEVEL);
			s.setVec4("mainLight.csmLevelDivide", csmLevelDivide);
		}
	}

	void GetCsmDivide() {
		auto n = mainCamera->near;
		auto f = mainCamera->far;
		for (unsigned i = 0; i < CSM_MAX_LEVEL; ++i) {
			auto d = CSM_AUJUST_FACTOR * n * std::pow(f / n, float(i + 1) / float(CSM_MAX_LEVEL))
				+ (1 - CSM_AUJUST_FACTOR) * (n + (float(i + 1) / float(CSM_MAX_LEVEL)) * (f - n));
			csmLevelDivide[i] = d;
		}
	}

	std::vector<glm::mat4> GetLightMat() override {
		std::vector<glm::mat4> tmpMats;
		auto d1 = mainCamera->near;
		float asp = mainCamera->Fov;
		float tanAsp = glm::tan(asp / 2.f * PI / 180.f);
		float k = mainCamera->w / mainCamera->h;
		auto pos = mainCamera->object->GetComponent<Transform>()->GetPosition();
		auto dirC = mainCamera->Front;
		auto n = mainCamera->near;
		auto f = mainCamera->far/2;
		auto dirL = glm::normalize(direction);
		float h = 80.f;
		GetCsmDivide();
		for (unsigned i = 0; i < CSM_MAX_LEVEL; ++i) {
			auto d2 = csmLevelDivide[i];
			float h2 = tanAsp * d2;
			float h1 = tanAsp * d1;
			float w2 = k * h2;
			float w1 = k * h1;
			float x = (d2 * d2 + h2 * h2 + w2 * w2 - d1 * d1 - h1 * h1 -w1 * w1) / 2.f / (d2-d1);
			float r = glm::sqrt(std::pow(x - d1, 2) + w1 * w1 + d1 * d1);
			auto center = pos + x * glm::normalize(dirC);
			float t =std::abs( (h - center.y) / dirL.y);
			auto eye = center - dirL*t;
			glm::mat4 projection = glm::ortho(-r, r, -r, r, 0.1f, 150.f);
			glm::mat4 view = glm::lookAt(eye, center, glm::vec3(0.f, 1.f, 0.f));
			tmpMats.push_back(projection * view);
			d1 = std::move(d2);
		}
		currMats.swap(tmpMats);
		return currMats;
	}

	void initBuffer(int w, int h) override{
		rsmShader = std::make_shared<Shader>("shaders/RSM.vs", "shaders/RSM.fs");
		rsmH = h;
		rsmW = w;
		glGenFramebuffers(bufferSize, frameBuffer);
		glGenTextures(bufferSize, rsmBuffer);
		glGenRenderbuffers(bufferSize, rbo);
		for (unsigned i = 0; i < bufferSize; ++i) {
			glBindTexture(GL_TEXTURE_2D, rsmBuffer[i]);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, w, h, 0, GL_RGB, GL_FLOAT, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
			glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer[i]);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, rsmBuffer[i], 0);

			glBindRenderbuffer(GL_RENDERBUFFER, rbo[i]);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, w, h);
			glBindRenderbuffer(GL_RENDERBUFFER, 0);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo[i]);

			glBindTexture(GL_TEXTURE_2D, 0);

			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
				std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!11" << std::endl;
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}
		
	}

	Photon samplePhoton() override {
		glm::vec3 pos(dis(engine), 17, dis1(engine));
		Photon p;
		p.positon = pos;
		float pdfs = 1.0 / glm::pi<float>() / ((18*18+8*8) * glm::pi<float>());
		p.color = GetSpectrum()*pdfs;
		p.dir = direction;
		return p;
	}
};
#endif // !DIRECT_LIGHT_H
