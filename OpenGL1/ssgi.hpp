

#ifndef SSGI_H
#define SSGI_H
#include"cPostProcess.hpp"


class SSGI :public PostProcess {

public:
	SSGI(unsigned w, unsigned h):PostProcess("shaders/bf.vs","shaders/ssgi.fs", w, h){}

	void excute() override {
		BlitMap(GetInTexBuffer(), GetOutTexBuffer(), GetShader().get());
	}
};


#endif // !SSGI_H
