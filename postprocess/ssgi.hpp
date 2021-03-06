

#ifndef SSGI_H
#define SSGI_H
#include"cPostProcess.hpp"
#include"map_filter.hpp"
#include"frame_buffer.hpp"

class SSGI :public PostProcess {

public:
	SSGI():PostProcess("shaders/bf.vs","shaders/ssgi.fs"){}

	void excute() override {
		FrameBuffer out;
		out.frameBuffer = GetFrameBuffer();
		out.texBuffer = GetOutBuffer();
		BlitMap(GetInBuffer(), out, GetShader().get());
	}
};


#endif // !SSGI_H
