

#ifndef SSSSS_H
#define SSSSS_H

#include"cPostProcess.hpp"
#include"map_filter.hpp"
#include"frame_buffer.hpp"


class SSSSS :public PostProcess {

public:
	SSSSS() :PostProcess("shaders/bf.vs", "shaders/sssss.fs") {

	}

	void excute() override {
		BlitMap(GetInTexBuffer(), GetOutTexBuffer(), GetShader().get());
	}


};

#endif // !SSSSS_H
