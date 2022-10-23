


#ifndef CUBEMAP_H
#define CUBEMAP_H
#include"opengl.hpp"
#include"component.hpp"
#include"transform.hpp"
#include"stb.hpp"


#define CUBE_MAP_W 512
#define CUBE_MAP_H 512


enum ReflectProbeType {
	Static=0,Dynamic=1
};

class ReflectProbe :public Component {
private:
	unsigned frameBuffer;
	unsigned cubeMapBuffer;
	unsigned rbo;
	unsigned w, h;
	ReflectProbeType type;
	void GenFrameBuffer(){
		glGenFramebuffers(1, &frameBuffer);
		glGenTextures(1, &cubeMapBuffer);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapBuffer);
		for (int i = 0; i < 6; i++) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0, GL_RGB, w, h, 0, GL_RGB, GL_FLOAT, NULL
			);
		}

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
		glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X, cubeMapBuffer,0);

		glGenRenderbuffers(1, &rbo);
		glBindRenderbuffer(GL_RENDERBUFFER, rbo);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, w, h);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
		
public:
	explicit ReflectProbe(int w=CUBE_MAP_W, int h=CUBE_MAP_H):w(w),h(h),type(Static) {
		name = "refProbe";
		GenFrameBuffer();
	}

	~ReflectProbe() {
		glDeleteFramebuffers(1, &frameBuffer);
		glDeleteTextures(1, &cubeMapBuffer);
	}

	void SetWidth(int w) { this->w = w; }

	void SetHeight(int h) { this->h = h; }

	void SetCubeMap(unsigned t) { glDeleteTextures(1,&cubeMapBuffer); cubeMapBuffer = t; }

	unsigned GetCubeMap() const {  return cubeMapBuffer; }

	unsigned GetWidth() const { return w; }

	unsigned GetHeight() const { return h; }

	unsigned GetFrameBuffer() const { return frameBuffer; }

	unsigned GetRbo() const { return rbo; }

	ReflectProbeType GetReflectProbeType() const { return type; }
};


#endif // !CUBEMAP_H
