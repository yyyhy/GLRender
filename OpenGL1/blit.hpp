

#ifndef BLIT_H
#define BLIT_H
#include"cPostProcess.hpp"
#include"frame_buffer.hpp"
#include"map_filter.hpp"


class Blit :public PostProcess {

private:
	int f = 1;
public:
	Blit() :PostProcess("shaders/bf.vs", "shaders/blit.fs"){}

	void excute() override {
		FrameBuffer out;
		out.frameBuffer = GetOutFrameBuffer();
		out.texBuffer = GetOutTexBuffer();
		auto s = GetShader();
		//s->debug();
		BlitMap(GetInTexBuffer(), out, s.get());
	}
};

#endif // !BLIT_H
