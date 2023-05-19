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
	Bloom(unsigned w, unsigned h) :PostProcess("shaders/bf.vs", "shaders/bloom_apply.fs", w, h) ,bloomMainTexBuffer(w,h,GL_RGB,GL_RGB)
																	, bloomBlurHTexBuffer(w, h, GL_RGB, GL_RGB)
																	, bloomBlurVTexBuffer(w, h, GL_RGB, GL_RGB) {
		mainShader = std::make_shared<Shader>("shaders/bf.vs", "shaders/bloomMain.fs");
		filterHShader = std::make_shared<Shader>("shaders/bf.vs", "shaders/gaussianblur_h.fs");
		filterVShader = std::make_shared<Shader>("shaders/bf.vs", "shaders/gaussianblur_v.fs");
		
	}

	void Excute() override {
		BlitMap(GetInTexBuffer(), bloomMainTexBuffer, mainShader.get());
		BlitMap(bloomMainTexBuffer, bloomBlurHTexBuffer, filterHShader.get());
		BlitMap(bloomBlurHTexBuffer, bloomBlurVTexBuffer, filterVShader.get());
		GetShader()->Use();
		GetShader()->SetTexture("bloomMap", bloomBlurVTexBuffer);
		BlitMap(GetInTexBuffer(), GetOutTexBuffer(), GetShader().get());
	}

	~Bloom() {
		bloomMainTexBuffer.Release();
		bloomBlurHTexBuffer.Release();
		bloomBlurVTexBuffer.Release();
	}

	sp_shader mainShader;
	sp_shader filterHShader;
	sp_shader filterVShader;
};

#endif // !BLOOM_H
