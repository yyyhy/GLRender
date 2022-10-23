

#ifndef TAA_H
#define TAA_H
#include"frame_buffer.hpp"
#include"cPostProcess.hpp"

std::vector<std::string> preComplie = { "ADJUST_UV" };

class TAA : public PostProcess {

private:
	unsigned currTBuffer;
	sp_shader taa;

public:
	TAA() :PostProcess("shaders/bf.vs", "shaders/taa.fs", 2,&preComplie), currTBuffer(0) {
		taa = GetShader();
		
	}

	void excute() override{
		FrameBuffer out;
		out.frameBuffer = GetOutFrameBuffer(currTBuffer);
		out.texBuffer = GetOutTexBuffer(currTBuffer);
		taa->use();
		taa->setTexture("lastFrame", GetOutTexBuffer(1-currTBuffer));
		BlitMap(GetInTexBuffer(), out, taa.get());
		
	}

	void SendBufferToNext(PostProcess* p) override{
		p->SetInTexBuffer(GetOutTexBuffer(currTBuffer));
		currTBuffer = 1 - currTBuffer;
		taa->use();
		taa->setBool("init", false);
	}


};
#endif // !TAA_H
