#pragma once

#ifndef BLOOM_H
#define BLOOM_H
#include"cPostProcess.hpp"
#include"map_filter.hpp"

class Bloom : public PostProcess {

private:
	unsigned bloomMainFrameBuffer;
	unsigned bloomMainTexBuffer;
	unsigned bloomBlurHFrameBuffer;
	unsigned bloomBlurHTexBuffer;
	unsigned bloomBlurVFrameBuffer;
	unsigned bloomBlurVTexBuffer;

public:
	Bloom() :PostProcess("shaders/bf.vs", "shaders/bloom_apply.fs") {
		mainShader = std::make_shared<Shader>("shaders/bf.vs", "shaders/bloomMain.fs");
		filterHShader = std::make_shared<Shader>("shaders/bf.vs", "shaders/gaussianblur_h.fs");
		filterVShader = std::make_shared<Shader>("shaders/bf.vs", "shaders/gaussianblur_v.fs");
		glGenFramebuffers(1, &bloomMainFrameBuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, bloomMainFrameBuffer);

		glGenTextures(1, &bloomMainTexBuffer);
		glBindTexture(GL_TEXTURE_2D, bloomMainTexBuffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bloomMainTexBuffer, 0);
		glBindTexture(GL_TEXTURE_2D, 0);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
		glBindFramebuffer(GL_FRAMEBUFFER, 0);


		glGenFramebuffers(1, &bloomBlurHFrameBuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, bloomBlurHFrameBuffer);

		glGenTextures(1, &bloomBlurHTexBuffer);
		glBindTexture(GL_TEXTURE_2D, bloomBlurHTexBuffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bloomBlurHTexBuffer, 0);
		glBindTexture(GL_TEXTURE_2D, 0);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glGenFramebuffers(1, &bloomBlurVFrameBuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, bloomBlurVFrameBuffer);

		glGenTextures(1, &bloomBlurVTexBuffer);
		glBindTexture(GL_TEXTURE_2D, bloomBlurVTexBuffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bloomBlurVTexBuffer, 0);
		glBindTexture(GL_TEXTURE_2D, 0);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void excute() override {
		FrameBuffer out;
		out.frameBuffer = bloomMainFrameBuffer;
		out.texBuffer = bloomMainTexBuffer;
		BlitMap(GetInTexBuffer(), out, mainShader.get());
		out.frameBuffer = bloomBlurHFrameBuffer;
		out.texBuffer = bloomBlurHTexBuffer;
		BlitMap(bloomMainTexBuffer, out, filterHShader.get());
		out.frameBuffer = bloomBlurVFrameBuffer;
		out.texBuffer = bloomBlurVTexBuffer;
		BlitMap(bloomBlurHTexBuffer, out, filterVShader.get());
		out.frameBuffer = GetOutFrameBuffer();
		out.texBuffer = GetOutTexBuffer();
		GetShader()->use();
		GetShader()->setTexture("bloomMap", bloomBlurVTexBuffer);
		BlitMap(GetInTexBuffer(), out, GetShader().get());
	}

	sp_shader mainShader;
	sp_shader filterHShader;
	sp_shader filterVShader;
};

#endif // !BLOOM_H
