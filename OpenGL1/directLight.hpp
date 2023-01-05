#pragma once


#ifndef DIRECT_LIGHT_H
#define DIRECT_LIGHT_H
#include "light.hpp"
#include"sh.hpp"
#include"texture.hpp"
#include"debugtool.hpp"

const static unsigned CSM_MAX_LEVEL = 4;
const static float CSM_AUJUST_FACTOR = 0.9f;


//RSM Struct
//      0       8      16      24       32
//Tex0:              Depth
//Tex1:       Albedo            |  Flag
//Tex2:       Normal            |  Roughness
//Tex3:       Position          |  Metallic

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

	Texture2D* RSMTextureBuffers;

	const unsigned RSM_SIZE = 4;
public:

	DirectLight():Light(RSM_W / 4, RSM_H / 4, CSM_MAX_LEVEL),direction(glm::vec3(0.f,-1.f,0.f)),radius(50){
		setUpRandomEngine();
	}

	DirectLight(const Spectrum& col, float flux, const glm::vec3& dir, bool genRsm) :
		Light(RSM_W / 4, RSM_H / 4, CSM_MAX_LEVEL,col, flux, genRsm,false), direction(glm::normalize(dir)),radius(50) {
		if (genRsm)
			initBuffer(RSM_W/4, RSM_H/4); 
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
				s.SetTexture("mainLight.shadowMap["+std::to_string(i)+"]", RSMTextureBuffers[i*RSM_SIZE]);
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
		
		
		RSMTextureBuffers = new Texture2D[RSM_SIZE*CSM_MAX_LEVEL];

		for (int j = 0; j < CSM_MAX_LEVEL; j++) {
			frameBuffers[j].Construct(w, h, true);
			for (int i = 0; i < RSM_SIZE; ++i) {
				if(i==0)
					RSMTextureBuffers[j * RSM_SIZE + i] = Texture2D(w, h, GL_RGBA32F, GL_RGBA, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER, false);
				else if(i==1)
					RSMTextureBuffers[j * RSM_SIZE + i] = Texture2D(w, h, GL_RGB32F, GL_RGB, GL_LINEAR, GL_LINEAR, GL_REPEAT, GL_REPEAT, false);
				else
					RSMTextureBuffers[j * RSM_SIZE + i] = Texture2D(w, h, GL_RGBA32F, GL_RGBA, GL_LINEAR, GL_LINEAR, GL_REPEAT, GL_REPEAT, false);
			}
			frameBuffers[j].AttachTexture(&RSMTextureBuffers[j * RSM_SIZE],RSM_SIZE);
		}
		
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		
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

	void SaveRSM(unsigned csmLevel, unsigned rsmLevel) const {
		frameBuffers[csmLevel].Bind();
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, RSMTextureBuffers[csmLevel * RSM_SIZE+rsmLevel].id, 0);
		SaveFrameBuffer(frameBuffers[csmLevel]);
	}

	const Texture& GetRSM(unsigned csmLevel, unsigned rsmLevel) const {
		return RSMTextureBuffers[csmLevel * RSM_SIZE + rsmLevel];
	}
};
#endif // !DIRECT_LIGHT_H
