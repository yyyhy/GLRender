

#ifndef TAA_H
#define TAA_H
#include"frame_buffer.hpp"
#include"cPostProcess.hpp"

class TAA : public PostProcess {

private:
	unsigned currTBuffer;
	sp_shader taa;
	unsigned gLastWorldPosFrameBuffer;
	unsigned gLastWorldTexBuffer;
	unsigned gWorldPos;
public:
	TAA(unsigned gWP):PostProcess("shaders/bf.vs","shaders/taa.fs",2),currTBuffer(0),gWorldPos(gWP){
		taa = GetShader();
		glGenFramebuffers(1, &gLastWorldPosFrameBuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, gLastWorldPosFrameBuffer);
		glGenTextures(1, &gLastWorldTexBuffer);
		glBindTexture(GL_TEXTURE_2D, gLastWorldTexBuffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 1600, 900, 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gLastWorldTexBuffer, 0);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void excute() override{
		FrameBuffer out;
		out.frameBuffer = GetFrameBuffer(currTBuffer);
		out.texBuffer = GetOutBuffer(currTBuffer);
		taa->use();
		taa->setTexture("lastFrame", GetOutBuffer(1-currTBuffer));
		taa->setTexture("gLastWorldPos", gLastWorldTexBuffer);
		//taa->setTexture("lastFrame1", GetOutBuffer(1));
		BlitMap(GetInBuffer(), out, taa.get());
		glBindFramebuffer(GL_READ_FRAMEBUFFER, gLastWorldPosFrameBuffer);
		glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
			GL_TEXTURE_2D, gWorldPos, 0);
		glBindTexture(GL_TEXTURE_2D, gLastWorldTexBuffer);
		glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, 1600, 900);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	void SendBufferToNext(PostProcess* p) override{
		p->SetInTexBuffer(GetOutBuffer(currTBuffer));
		currTBuffer = 1 - currTBuffer;
	}

};
#endif // !TAA_H
