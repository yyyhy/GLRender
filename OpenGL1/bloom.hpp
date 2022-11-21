#pragma once

#ifndef BLOOM_H
#define BLOOM_H
#include"cPostProcess.hpp"


class Bloom : public PostProcess {

private:
	Texture2D bloomMainTexBuffer;
	Texture2D bloomBlurHTexBuffer;
	Texture2D bloomBlurVTexBuffer;

public:
	Bloom() :PostProcess("shaders/bf.vs", "shaders/bloom_apply.fs") ,bloomMainTexBuffer(SCR_WIDTH,SCR_HEIGHT,GL_RGB,GL_RGB) 
																	, bloomBlurHTexBuffer(SCR_WIDTH, SCR_HEIGHT, GL_RGB, GL_RGB)
																	, bloomBlurVTexBuffer(SCR_WIDTH, SCR_HEIGHT, GL_RGB, GL_RGB) {
		mainShader = std::make_shared<Shader>("shaders/bf.vs", "shaders/bloomMain.fs");
		filterHShader = std::make_shared<Shader>("shaders/bf.vs", "shaders/gaussianblur_h.fs");
		filterVShader = std::make_shared<Shader>("shaders/bf.vs", "shaders/gaussianblur_v.fs");
		
	}

	void excute() override {
		BlitMap(GetInTexBuffer(), bloomMainTexBuffer, mainShader.get());
		BlitMap(bloomMainTexBuffer, bloomBlurHTexBuffer, filterHShader.get());
		BlitMap(bloomBlurHTexBuffer, bloomBlurVTexBuffer, filterVShader.get());
		GetShader()->use();
		GetShader()->setTexture("bloomMap", bloomBlurVTexBuffer);
		BlitMap(GetInTexBuffer(), GetOutTexBuffer(), GetShader().get());
	}

	sp_shader mainShader;
	sp_shader filterHShader;
	sp_shader filterVShader;
};

#endif // !BLOOM_H
