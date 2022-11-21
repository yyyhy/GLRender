


#ifndef CUBEMAP_H
#define CUBEMAP_H
#include"opengl.hpp"
#include"component.hpp"
#include"transform.hpp"
#include"stb.hpp"
#include"frameBuffer.hpp"

#define CUBE_MAP_W 512
#define CUBE_MAP_H 512


enum ReflectProbeType {
	Static = 0, Dynamic = 1
};

class ReflectProbe :public Component {
private:
	FrameBufferO frameBuffer;
	TextureCube cubeMapBuffer;

	unsigned w, h;
	ReflectProbeType type;
	

public:
	explicit ReflectProbe(int w = CUBE_MAP_W, int h = CUBE_MAP_H) :w(w), h(h), type(Static),frameBuffer(w,h,true)
																	,cubeMapBuffer(w,h,GL_RGB,GL_RGB) {
		name = "refProbe";
		frameBuffer.AttachTexture(&cubeMapBuffer);
	}

	~ReflectProbe() {
		
	}

	void SetWidth(int w) { this->w = w; }

	void SetHeight(int h) { this->h = h; }

	void SetCubeMap(unsigned t) { glDeleteTextures(1, &cubeMapBuffer.id); cubeMapBuffer.id = t; }

	const TextureCube& GetCubeMap() const { return cubeMapBuffer; }

	unsigned GetWidth() const { return w; }

	unsigned GetHeight() const { return h; }

	const FrameBufferO& GetFrameBuffer() const { return frameBuffer; }

	ReflectProbeType GetReflectProbeType() const { return type; }
};


#endif // !CUBEMAP_H
