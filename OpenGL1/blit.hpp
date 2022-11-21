

#ifndef BLIT_H
#define BLIT_H
#include"cPostProcess.hpp"



class Blit :public PostProcess {

private:
	int f = 1;
public:
	Blit() :PostProcess("shaders/bf.vs", "shaders/blit.fs"){}

	void excute() override {
		auto s = GetShader();
		//s->debug();
		BlitMap(GetInTexBuffer(), GetOutTexBuffer(), s.get(),GetOutFrameBuffer());
	}
};

#endif // !BLIT_H
