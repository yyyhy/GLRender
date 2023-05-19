

#ifndef BIDIRECTION_FILTER_H
#define BIDIRECTION_FILTER_H
#include"cPostProcess.hpp"


class BidirectionHFilter :public PostProcess {
public:
	BidirectionHFilter(unsigned w,unsigned h) :PostProcess("shaders/bf.vs", "shaders/bidirectionfilter_h.fs",w,h) {}

	void Excute() override {
		BlitMap(GetInTexBuffer(), GetOutTexBuffer(), GetShader().get());
	}

};

class BidirectionVFilter :public PostProcess {
public:
	BidirectionVFilter(unsigned w, unsigned h) :PostProcess("shaders/bf.vs", "shaders/bidirectionfilter_v.fs" , w, h) {}

	void Excute() override {
		BlitMap(GetInTexBuffer(), GetOutTexBuffer(), GetShader().get());
	}

};
#endif // !BIDIRECTION_FILTER_H
