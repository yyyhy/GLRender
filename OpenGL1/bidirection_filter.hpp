

#ifndef BIDIRECTION_FILTER_H
#define BIDIRECTION_FILTER_H
#include"cPostProcess.hpp"


class BidirectionHFilter :public PostProcess {
public:
	BidirectionHFilter() :PostProcess("shaders/bf.vs", "shaders/bidirectionfilter_h.fs") {}

	void excute() override {
		BlitMap(GetInTexBuffer(), GetOutTexBuffer(), GetShader().get());
	}

};

class BidirectionVFilter :public PostProcess {
public:
	BidirectionVFilter() :PostProcess("shaders/bf.vs", "shaders/bidirectionfilter_v.fs") {}

	void excute() override {
		BlitMap(GetInTexBuffer(), GetOutTexBuffer(), GetShader().get());
	}

};
#endif // !BIDIRECTION_FILTER_H
