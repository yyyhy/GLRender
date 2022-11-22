


#ifndef CUBEMAP_H
#define CUBEMAP_H
#include"opengl.hpp"
#include"component.hpp"
#include"transform.hpp"
#include"stb.hpp"
#include"frameBuffer.hpp"
#include"shader.hpp"
#include"render.hpp"

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
	
	//sp_shader cubeMipMapShader;
	//Render localRender;
public:
	explicit ReflectProbe(int w = CUBE_MAP_W, int h = CUBE_MAP_H) :
		w(w), h(h), type(Static),frameBuffer(w,h,true),
		cubeMapBuffer(w,h,GL_RGB,GL_RGB, GL_LINEAR,GL_LINEAR)
		/*,localRender(w,h)*/ {
		name = "refProbe";
		frameBuffer.AttachTexture(&cubeMapBuffer);
		//cubeMipMapShader = std::make_shared<Shader>("shaders/mipmap.vs", "shaders/mipmap.fs");
	}

	~ReflectProbe() {
		cubeMapBuffer.Release();

	}

	void SetWidth(int w) { this->w = w; }

	void SetHeight(int h) { this->h = h; }

	void SetCubeMap(const TextureCube& t) { 
		cubeMapBuffer.Release();
		cubeMapBuffer = t; 
	}

	const TextureCube& GetCubeMap() const { return cubeMapBuffer; }

	unsigned GetWidth() const { return w; }

	unsigned GetHeight() const { return h; }

	const FrameBufferO& GetFrameBuffer() const { return frameBuffer; }

	ReflectProbeType GetReflectProbeType() const { return type; }

	//void GenerateCubemap(Scene* s) {
	//	
	//	if (GetReflectProbeType() == ReflectProbeType::Static || 1) {
	//		auto pos = object->GetComponent<Transform>()->GetPosition();
	//		auto tmpCamera = mainCamera;
	//		auto tmpFront = tmpCamera->Front;
	//		auto tmpFov = tmpCamera->Fov;
	//		auto tmpW = tmpCamera->w;
	//		auto tmpH = tmpCamera->h;
	//		auto trans = tmpCamera->object->GetComponent<Transform>();
	//		auto tmpPos = trans->GetPosition();
	//		tmpCamera->Fov = 90;
	//		tmpCamera->w = GetWidth();
	//		tmpCamera->h = GetHeight();
	//		trans->SetPosition(pos);
	//		//tmpCamera->Position = pos;
	//		for (int j = 0; j < 6; j++) {
	//			tmpCamera->Front = (captureViews[j * 2]);
	//			tmpCamera->Up = captureViews[j * 2 + 1];
	//			glBindFramebuffer(GL_FRAMEBUFFER, GetFrameBuffer().frameBuffer);
	//			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + j, GetCubeMap().id, 0);
	//			localRender(s, GetFrameBuffer(), true);

	//		}
	//		tmpCamera->Fov = tmpFov;
	//		tmpCamera->SetFront(tmpFront);
	//		tmpCamera->w = tmpW;
	//		tmpCamera->h = tmpH;
	//		trans->SetPosition(tmpPos);
	//		//tmpCamera->Position = tmpPos;

	//		TextureCube cube(GetFrameBuffer().w, GetFrameBuffer().h, GL_RGB, GL_RGB, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
	//		GenCubeMipMap(cubeMipMapShader.get(), GetFrameBuffer(), cube, 4);
	//		SetCubeMap(cube);
	//	}
	//}
};


#endif // !CUBEMAP_H
