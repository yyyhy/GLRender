

#ifndef TAA_H
#define TAA_H
#include"cPostProcess.hpp"

std::vector<std::string> preComplie = { "ADJUST_UV" };

class TAA : public PostProcess {

private:
	unsigned currTBuffer;
	sp_shader taa;

public:
	TAA(unsigned w, unsigned h) :PostProcess("shaders/bf.vs", "shaders/taa.fs" , w, h, 2,&preComplie), currTBuffer(0) {
		taa = GetShader();
		
	}

	void excute() override{
		taa->use();
		taa->SetTexture("lastFrame", GetOutTexBuffer(1-currTBuffer).id);
		BlitMap(GetInTexBuffer(), GetOutTexBuffer(currTBuffer), taa.get(),GetOutFrameBuffer(currTBuffer));
		
	}

	void SendBufferToNext(PostProcess* p) override{
		p->SetInTexBuffer(GetOutTexBuffer(currTBuffer));
		currTBuffer = 1 - currTBuffer;
		taa->use();
		taa->setBool("init", false);
	}


};
#endif // !TAA_H
