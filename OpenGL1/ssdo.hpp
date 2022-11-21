

#ifndef SSDO_H
#define SSDO_H
#include"cPostProcess.hpp"
#include<time.h>
#include<random>

class SSDO :public PostProcess{
private:
	Texture2D ssdoTexBuffer;
	Texture2D ssdoBlurBuffer;

public:
	SSDO() :PostProcess("shaders/bf.vs", "shaders/ssdo_apply.fs"),ssdoTexBuffer(SCR_WIDTH, SCR_HEIGHT, GL_RGB,GL_RGB)
		, ssdoBlurBuffer(SCR_WIDTH, SCR_HEIGHT, GL_RGB, GL_RGB) {
		
		mainShader = std::make_shared<Shader>("shaders/bf.vs", "shaders/ssdo.fs");
		filterShader = std::make_shared<Shader>("shaders/bf.vs", "shaders/commonblur.fs");

		auto ssdoShader = mainShader;
		ssdoShader->use();
		std::default_random_engine engine;
		std::uniform_real_distribution<float> dis(0, 1);
		engine.seed(time(0));
		for (int i = 0; i < 16; i++) {
			glm::vec3 v = { 2.0 * dis(engine) - 1,2.0 * dis(engine) - 1,dis(engine) };
			ssdoShader->setVec3("ssdoSample[" + std::to_string(i) + "]", v);
		}

		std::vector<glm::vec3> ssaoNoise;
		for (GLuint i = 0; i < 16; i++)
		{
			glm::vec3 noise(
				dis(engine) * 2.0 - 1.0,
				dis(engine) * 2.0 - 1.0,
				0.0f);
			ssaoNoise.push_back(noise);
		}

		GLuint noiseTexture;
		glGenTextures(1, &noiseTexture);
		glBindTexture(GL_TEXTURE_2D, noiseTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		ssdoShader->setTexture("ssdoNoise", noiseTexture);
	}

	void excute() override {
		BlitMap(GetInTexBuffer(), ssdoTexBuffer, mainShader.get());
		BlitMap(ssdoTexBuffer, ssdoBlurBuffer, filterShader.get());
		GetShader()->setTexture("ssdoMap", ssdoBlurBuffer);
		BlitMap(GetInTexBuffer(), GetOutTexBuffer(), GetShader().get());
	}

	sp_shader mainShader;
	sp_shader filterShader;
};
#endif // !SSDO_H
