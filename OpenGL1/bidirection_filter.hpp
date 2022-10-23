

#ifndef BIDIRECTION_FILTER_H
#define BIDIRECTION_FILTER_H
#include"cPostProcess.hpp"
#include"frame_buffer.hpp"
#include"map_filter.hpp"
class BidirectionHFilter :public PostProcess {
public:
	BidirectionHFilter() :PostProcess("shaders/bf.vs", "shaders/bidirectionfilter_h.fs") {}

	void excute() override {
		FrameBuffer out;
		out.frameBuffer = GetOutFrameBuffer();
		out.texBuffer = GetOutTexBuffer();
		BlitMap(GetInTexBuffer(), out, GetShader().get());
	}

};

class BidirectionVFilter :public PostProcess {
public:
	BidirectionVFilter() :PostProcess("shaders/bf.vs", "shaders/bidirectionfilter_v.fs") {}

	void excute() override {
		FrameBuffer out;
		out.frameBuffer = GetOutFrameBuffer();
		out.texBuffer = GetOutTexBuffer();
		BlitMap(GetInTexBuffer(), out, GetShader().get());
	}

};
#endif // !BIDIRECTION_FILTER_H
