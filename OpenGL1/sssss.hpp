

#ifndef SSSSS_H
#define SSSSS_H

#include"cPostProcess.hpp"


class SSSSS :public PostProcess {

public:
	SSSSS(unsigned w, unsigned h) :PostProcess("shaders/bf.vs", "shaders/sssss.fs", w, h) {

	}

	void Excute() override {
		BlitMap(GetInTexBuffer(), GetOutTexBuffer(), GetShader().get());
	}


};

#endif // !SSSSS_H
