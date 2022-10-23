

#ifndef C_POST_PROCESS_H
#define C_POST_PROCESS_H

#include"shader.hpp"
#include"settings.hpp"

class PostProcess {
private:
	std::shared_ptr<Shader> shader;
	unsigned *frameBuffer;
	unsigned inTexBuffer;
	unsigned *outTexBuffer;
	unsigned size;
	
public:
	PostProcess(const char* vertexPath, const char* fragmentPath,unsigned bufferSize=1, std::vector<std::string>* preComplieCmd = NULL):enable(true),
		frameBuffer(new unsigned[bufferSize]), outTexBuffer(new unsigned[bufferSize]),size(bufferSize) {
		shader = std::make_shared<Shader>(vertexPath, fragmentPath,nullptr, preComplieCmd);
		for (int i = 0; i < size; i++) {
			frameBuffer[i] = 0;
			outTexBuffer[i] = 0;
		}
	}

	PostProcess(const std::shared_ptr<Shader> s) noexcept{
		shader = s;
	}

	PostProcess() = default;

	virtual	~PostProcess() {
		glDeleteFramebuffers(size, frameBuffer);
		glDeleteTextures(size, outTexBuffer);
		delete[] frameBuffer;
		delete[] outTexBuffer;
	}

	virtual void GenFrameBuffer() {
		glGenFramebuffers(size, frameBuffer);
		glGenTextures(size, outTexBuffer);
		for (unsigned i = 0; i < size; ++i) {
			glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer[i]);
			glBindTexture(GL_TEXTURE_2D, outTexBuffer[i]);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glBindTexture(GL_TEXTURE_2D, 0);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, outTexBuffer[i], 0);


			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
				std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}
	}

	virtual void SendBufferToNext(PostProcess* p) {
		if(p)
			p->SetInTexBuffer(outTexBuffer[0]);
	}

	void SetInTexBuffer(unsigned buffer) { 
		inTexBuffer = buffer; 
		shader->use();
		shader->setTexture("tex", inTexBuffer); 
	}

	unsigned GetOutTexBuffer(unsigned index=0) const { return outTexBuffer[index]; }

	unsigned GetInTexBuffer() const { return inTexBuffer; }

	unsigned GetOutFrameBuffer(unsigned index = 0) const { return frameBuffer[index]; }

	virtual void excute() = 0;

	std::shared_ptr<Shader> GetShader() const { return shader; }
	bool enable;
};

class SSAO;
class SSDO;
class SSGI;
class BidirectionHFilter;
class BidirectionVFilter;
class Blit;
#endif // !POST_PROCESS_H
