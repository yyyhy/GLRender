

#ifndef BLIT_H
#define BLIT_H
#include"cPostProcess.hpp"
#include"computeShader.hpp"



class Blit :public PostProcess {

private:
	int f = 1;
	ComputeShader cs;
public:
	Blit(unsigned w, unsigned h) 
		:PostProcess("shaders/bf.vs", "shaders/blit.fs", w, h),
		cs("shaders/cs/test.csr")
		{}

	void excute() override {
		auto s = GetShader();
		cs.SetTexture("input_image", GetInTexBuffer());
		cs.SetTexture("out_image", GetOutTexBuffer());
		//cs.Dispath(width, height, 1);
		BlitMap(GetInTexBuffer(), GetOutTexBuffer(), s.get(),GetOutFrameBuffer());
	}
};

#endif // !BLIT_H
