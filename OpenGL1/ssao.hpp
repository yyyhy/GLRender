

#ifndef SSAO_H
#define SSAO_H
#include"cPostProcess.hpp"


class SSAO : public PostProcess{
private:
	Texture2D ssaoTexBuffer;
	Texture2D ssaoBlurBuffer;
	
public:
	SSAO(sp_shader shader, unsigned w, unsigned h,unsigned quanlity = 64) : PostProcess("shaders/bf.vs", "shaders/ssao_apply.fs", w, h)
					, ssaoTexBuffer(w, h, GL_RED,GL_RED) 
					, ssaoBlurBuffer(w, h, GL_RED, GL_RED) {
		mainShader = std::make_shared<Shader>("shaders/bf.vs", "shaders/ssao.fs");
		filterShader = std::make_shared<Shader>("shaders/bf.vs", "shaders/commonblur.fs");

		mainShader->Use();
		std::vector<glm::vec3> samples;
		std::default_random_engine engine;
		std::uniform_real_distribution<float> real(0, 1);
		engine.seed(time(0));

		for (int i = 0; i < 64; i++) {
			glm::vec3 sample = { 2.0 * real(engine) - 1.0,2.0 * real(engine) - 1.0 ,real(engine) };
			sample = glm::normalize(sample);
			sample *= real(engine);
			GLfloat scale = GLfloat(i) / 64.0;
			scale = 0.1f + 0.9f * scale * scale;
			sample *= scale;
			samples.push_back(sample);
			mainShader->SetVec3("ssaoSample[" + std::to_string(i) + "]", samples[i]);
			//std::cout << samples[i].x << " " << samples[i].y << " "<<samples[i].z << "\n";
		}

		std::vector<glm::vec3> ssaoNoise;
		for (GLuint i = 0; i < 16; i++)
		{
			glm::vec3 noise(
				real(engine) * 2.0 - 1.0,
				real(engine) * 2.0 - 1.0,
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
		mainShader->SetTexture("ssaoNoise", noiseTexture);

	}

	~SSAO() override {
		ssaoBlurBuffer.Release();
		ssaoTexBuffer.Release();
	}

	void excute() override {
		BlitMap(0, ssaoTexBuffer, mainShader.get());
		BlitMap(ssaoTexBuffer, ssaoBlurBuffer, filterShader.get());
		GetShader()->SetTexture("ssaoMap", ssaoBlurBuffer);
		BlitMap(GetInTexBuffer(), GetOutTexBuffer(), GetShader().get());
	}

	sp_shader mainShader;
	sp_shader filterShader;
};
#endif // !SSAO_H
