

#ifndef BLIT_H
#define BLIT_H
#include"cPostProcess.hpp"
#include"frame_buffer.hpp"
#include"map_filter.hpp"


class Blit :public PostProcess {

private:
	Render* render;
	int f = 1;
public:
	Blit(Render* r) :PostProcess("shaders/bf.vs", "shaders/blit.fs"),render(r){}

	void excute() override {
		FrameBuffer out;
		out.frameBuffer = GetFrameBuffer();
		out.texBuffer = GetOutBuffer();
		auto s = GetShader();
		//s->debug();
		BlitMap(GetInBuffer(), out, s.get());
	}
};

#endif // !BLIT_H
