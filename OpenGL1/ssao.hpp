

#ifndef SSAO_H
#define SSAO_H
#include"cPostProcess.hpp"
#include"map_filter.hpp"

class SSAO : public PostProcess{
private:
	unsigned ssaoFrameBuffer;
	unsigned ssaoTexBuffer;
	unsigned ssaoBlurFrameBuffer;
	unsigned ssaoBlurBuffer;
	
public:
	SSAO(sp_shader shader,unsigned quanlity = 64) : PostProcess("shaders/bf.vs", "shaders/ssao_apply.fs") {
		mainShader = std::make_shared<Shader>("shaders/bf.vs", "shaders/ssao.fs");
		filterShader = std::make_shared<Shader>("shaders/bf.vs", "shaders/commonblur.fs");
		glGenFramebuffers(1, &ssaoFrameBuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, ssaoFrameBuffer);

		glGenTextures(1, &ssaoTexBuffer);
		glBindTexture(GL_TEXTURE_2D, ssaoTexBuffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, SCR_WIDTH, SCR_HEIGHT, 0, GL_RED, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoTexBuffer, 0);
		glBindTexture(GL_TEXTURE_2D, 0);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
		glBindFramebuffer(GL_FRAMEBUFFER, 0);


		glGenFramebuffers(1, &ssaoBlurFrameBuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFrameBuffer);

		glGenTextures(1, &ssaoBlurBuffer);
		glBindTexture(GL_TEXTURE_2D, ssaoBlurBuffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, SCR_WIDTH, SCR_HEIGHT, 0, GL_RED, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoBlurBuffer, 0);
		glBindTexture(GL_TEXTURE_2D, 0);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		mainShader->use();
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
			mainShader->setVec3("ssaoSample[" + std::to_string(i) + "]", samples[i]);
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
		mainShader->setTexture("ssaoNoise", noiseTexture);

	}

	~SSAO() override {
		glDeleteBuffers(1, &ssaoTexBuffer);
		glDeleteBuffers(1, &ssaoBlurBuffer);
		glDeleteFramebuffers(1, &ssaoFrameBuffer);
		glDeleteFramebuffers(1, &ssaoBlurFrameBuffer);
	}

	void excute() override {
		FrameBuffer out;
		out.frameBuffer = ssaoFrameBuffer;
		out.texBuffer = ssaoTexBuffer;
		BlitMap(0, out, mainShader.get());
		out.frameBuffer = ssaoBlurFrameBuffer;
		out.texBuffer = ssaoBlurBuffer;
		BlitMap(ssaoTexBuffer, out, filterShader.get());
		out.frameBuffer = GetOutFrameBuffer();
		out.texBuffer = GetOutTexBuffer();
		GetShader()->setTexture("ssaoMap", ssaoBlurBuffer);
		BlitMap(GetInTexBuffer(), out, GetShader().get());
	}

	sp_shader mainShader;
	sp_shader filterShader;
};
#endif // !SSAO_H
