

#ifndef RENDER_H
#define RENDER_H
#include"scene.hpp"
#include"reflect_probe.hpp"
#include"frame_buffer.hpp"
#include"map_filter.hpp"
#include<random>
#include<time.h>
#include"ssao.hpp"
#include"blit.hpp"
#include"ssdo.hpp"
#include"bidirection_filter.hpp"
#include"ssgi.hpp"
#include"sssss.hpp"
#include"sh.hpp"
#include"taa.hpp"

#define CAMERA_UB_SIZE 208+64
#define CAMERA_UB_POINT 0
#define GBUFFER_SIZE 6
#define T_BUFFER 1
const unsigned int SCR_WIDTH = 1600;
const unsigned int SCR_HEIGHT = 900;


class Render {
private:
	unsigned frameBuffer[T_BUFFER];
	unsigned texBuffer[T_BUFFER];
	unsigned rbo[T_BUFFER];
	unsigned currTBuffer = T_BUFFER-1;
	unsigned gBufferFrameBuffer;
	unsigned gBufferTexBuffer[GBUFFER_SIZE];
	unsigned gBufferRbo;
	FrameBuffer defaultFrameBuffer;
	bool firstCall = true;
	bool enableDeffered;
	bool ssdoOn = false;
	bool ssaoOn = false;
	bool ssgiOn = false;
	bool antiNoiseOn = false;
	sp_shader defferedShader;
	sp_shader cubeMipMapShader;
	sp_shader texMipMapShader;
	sp_shader taaShader;
	glm::mat4 lastCameraMVP=glm::mat4(0);
	unsigned i = 0, n = 8;

	std::vector<PostProcess*> postProcess;
	unsigned cameraPropertyUbo;
	
	void GenShadowMaps(Scene* scene) {
		/*glCullFace(GL_FRONT);*/
		for (auto& l : scene->lights)
			if (l && l->hasRsm() && !l->isStatic) {
				auto rsmShader = l->rsmShader;
				if (rsmShader == NULL)
					continue;
				rsmShader->use();
				rsmShader->setMat4("LightMat", l->GetLightMat());
				glViewport(0, 0, l->GetRSMWidth(), l->GetRSMHeigth());
				glBindFramebuffer(GL_FRAMEBUFFER, l->GetFrameBuffer());
				glClearColor(0, 0, 0, 1);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				for (auto& o : scene->objects) {
					rsmShader->setMat4("trans", (*o->GetComponent<Transform>())());
					o->draw(l->rsmShader.get());
				}
				
			}
	}

	void GenStaticShadowMaps(Scene* scene) {
		for (auto& l : scene->lights)
			if (l && l->hasRsm() && l->isStatic) {
				auto rsmShader = l->rsmShader;
				if (rsmShader == NULL)
					continue;
				rsmShader->use();

				//std::cout << l->GetRSMWidth() << " " << l->GetRSMHeigth() << " " << l->GetFrameBuffer() << " " << l->GetRSMBuffer() << "\n\n";
				auto mat = l->GetLightMat();
				rsmShader->setMat4("LightMat", mat);
				
				glBindFramebuffer(GL_FRAMEBUFFER, l->GetFrameBuffer());
				glViewport(0, 0, l->GetRSMWidth(), l->GetRSMHeigth());
				glClearColor(0.3, 1, 1, 1);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				for (auto& o : scene->objects) {
					rsmShader->setMat4("trans", (*o->GetComponent<Transform>())());
					o->draw(l->rsmShader.get());
				}
			}
	}

	void GenFrameBuffer() {
		glGenFramebuffers(T_BUFFER, frameBuffer);
		glGenTextures(T_BUFFER, texBuffer);
		glGenRenderbuffers(T_BUFFER, rbo);
		for (unsigned i = 0; i < T_BUFFER; i++) {
			glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer[i]);
			glBindTexture(GL_TEXTURE_2D, texBuffer[i]);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 1600, 900, 0, GL_RGB, GL_FLOAT, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glBindTexture(GL_TEXTURE_2D, 0);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texBuffer[i], 0);

			glBindRenderbuffer(GL_RENDERBUFFER, rbo[i]);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 1600, 900);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo[i]);

			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
				std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}
		
	}

	void GenUbos() {
		glGenBuffers(1, &cameraPropertyUbo);
		glBindBuffer(GL_UNIFORM_BUFFER, cameraPropertyUbo);
		glBufferData(GL_UNIFORM_BUFFER, CAMERA_UB_SIZE, NULL, GL_STATIC_DRAW);
		glBindBufferBase(GL_UNIFORM_BUFFER, CAMERA_UB_POINT, cameraPropertyUbo);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

	}

	void GenGbuffer() {
		glGenFramebuffers(1, &gBufferFrameBuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, gBufferFrameBuffer);
		GLuint attachments[GBUFFER_SIZE];
		for (int i = 0; i < GBUFFER_SIZE; i++) {
			glGenTextures(1, &gBufferTexBuffer[i]);
			glBindTexture(GL_TEXTURE_2D, gBufferTexBuffer[i]);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glGenerateMipmap(GL_TEXTURE_2D);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0+i, GL_TEXTURE_2D, gBufferTexBuffer[i], 0);
			attachments[i] = GL_COLOR_ATTACHMENT0 + i;
		}

		glDrawBuffers(GBUFFER_SIZE, attachments);
		glGenRenderbuffers(1, &gBufferRbo);
		glBindRenderbuffer(GL_RENDERBUFFER, gBufferRbo);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 1600, 900);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, gBufferRbo);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	int GenCubeMipMap(FrameBuffer Buffer, unsigned level = 4) {
		int map = ::GenCubeMipMap(cubeMipMapShader.get(), Buffer, level, true);
		return map;
	}

	void GenReflectProbe(Scene* s) {
		std::vector<ReflectProbe*> reflectProbes;
		for (auto& o : s->objects) {
			auto probe = o->GetComponent<ReflectProbe>();
			if (probe)
				reflectProbes.push_back(probe);
		}
		for (auto& i : reflectProbes) {
			if (i->GetReflectProbeType() == ReflectProbeType::Dynamic) {
				auto pos = i->object->GetComponent<Transform>()->GetPosition();
				auto tmpCamera = s->GetMainCamera();
				auto tmpFront = tmpCamera->Front;
				auto tmpFov = tmpCamera->Fov;
				auto tmpW = tmpCamera->w;
				auto tmpH = tmpCamera->h;
				auto tmpPos = tmpCamera->Position;
				tmpCamera->Fov = 90;
				tmpCamera->w = i->GetWidth();
				tmpCamera->h = i->GetHeight();
				tmpCamera->Position = pos;
				FrameBuffer out;
				out.frameBuffer = i->GetFrameBuffer();
				out.w = i->GetWidth();
				out.h = i->GetHeight();
				for (int j = 0; j < 6; j++) {
					tmpCamera->Front = (captureViews[j * 2]);
					tmpCamera->Up = captureViews[j * 2 + 1];
					glBindFramebuffer(GL_FRAMEBUFFER, i->GetFrameBuffer());
					glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + j, i->GetCubeMap(), 0);
					(*this)(s, out, false, true);
					int w = 512, h = 512;
				}
				tmpCamera->Fov = tmpFov;
				tmpCamera->SetFront(tmpFront);
				tmpCamera->w = tmpW;
				tmpCamera->h = tmpH;
				tmpCamera->Position = tmpPos;
				FrameBuffer b;
				b.frameBuffer = i->GetFrameBuffer();
				b.texBuffer = i->GetCubeMap();
				b.w = i->GetWidth();
				b.h = i->GetHeight();
				b.rbo = i->GetRbo();
				int p = GenCubeMipMap(b, 4);
				i->SetCubeMap(p);
			}
		}
	}

	glm::mat4 ApplyCameraProperties(Camera* c) {
		auto trans = c->object->GetComponent<Transform>();
		glBindBuffer(GL_UNIFORM_BUFFER, cameraPropertyUbo);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, 64, glm::value_ptr(c->GetModel()));
		glBufferSubData(GL_UNIFORM_BUFFER, 64, 64, glm::value_ptr(c->GetView()));
		glBufferSubData(GL_UNIFORM_BUFFER, 128, 64, glm::value_ptr(c->GetProjection()));
		auto mvp = c->GetMVP();
		auto offset = (Hammersley(i, n) * 2.f - 1.f);
		mvp[2][0] += offset.x / SCR_WIDTH/10;
		mvp[2][1] += offset.y / SCR_HEIGHT/10;
		i++;
		i %= n;
		glBufferSubData(GL_UNIFORM_BUFFER, 192, 64, glm::value_ptr(mvp));
		glBufferSubData(GL_UNIFORM_BUFFER, 256, 16, glm::value_ptr(trans->GetPosition()));
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
		return mvp;
	}

	void ApplyLightProperties(std::shared_ptr<Shader> s,Scene* scene) {

		for (int i = 0; i < scene->lights.size(); i++) {
			auto l = scene->lights[i];
			if (l != NULL) {
				//l->
				l->LoadToShader(*s);

			}
		}
	}

	void ApplyPostProcess() {
		postProcess.at(0)->SetInTexBuffer(texBuffer[currTBuffer]);
		for (int i = 0; i < postProcess.size(); i++) {
			postProcess.at(i)->excute();
			if (i < postProcess.size() - 1)
				postProcess.at(i)->SendBufferToNext(postProcess.at(i + 1));
		}
	}

	void RenderGbuffer(Scene* s) {
		auto camaraTransform = s->mainCamera->object->GetComponent<Transform>();
		glBindFramebuffer(GL_FRAMEBUFFER, gBufferFrameBuffer);
		glViewport(0, 0, 1600, 900);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		for (auto& i : s->objects) {
			if (!i->IsActive())
				continue;
			auto transform = i->GetComponent<Transform>();
			if (transform == NULL)
				continue;
			for (int j = 0; j < i->GetMeshLength(); j++) {
				auto shader = i->GetShader(j,Deffered);
				if (shader != NULL) {
					shader->use();
					shader->setMat4("trans", (*transform)());
				}
				
			}
			i->drawGBuffer();
		}
	}

	void initSSDO() {
		auto ssdo = new SSDO();
		auto ssdoShader = ssdo->mainShader;
		ssdoShader->setTexture("gPositionRoughness", GetGBuffer(0));
		ssdoShader->setTexture("gNormalDepth", GetGBuffer(1));
		ssdoShader->setTexture("gAlbedoMetallic", GetGBuffer(2));

		AddPostProcess(ssdo);
		ssdoOn = true;
	}

	void initSSAO() {
		auto ssao = new SSAO(defferedShader);
		ssao->mainShader->setTexture("gPosition", gBufferTexBuffer[0]);
		ssao->mainShader->setTexture("gNormalDepth", gBufferTexBuffer[1]);
		AddPostProcess(ssao);
		ssaoOn = true;
	}

	void initSSGI() {
		auto ssgi = new SSGI();
		auto ssgiShader = ssgi->GetShader();
		ssgiShader->use();
		ssgiShader->setTexture("gPositionRoughness", GetGBuffer(0));
		ssgiShader->setTexture("gNormalDepth", GetGBuffer(1));
		ssgiShader->setTexture("gAlbedoMetallic", GetGBuffer(2));
		AddPostProcess(ssgi);
		ssgiOn = true;
	}

	void initAntiNoise() {
		auto v = new BidirectionVFilter();
		auto h = new BidirectionHFilter();
		auto vShader = v->GetShader();
		vShader->setTexture("gNormal", GetGBuffer(1));
		vShader->setTexture("gPosition", GetGBuffer(0));
		auto hShader = h->GetShader();
		hShader->setTexture("gNormal", GetGBuffer(1));
		hShader->setTexture("gPosition", GetGBuffer(0));
		AddPostProcess(v);
		AddPostProcess(h);
		antiNoiseOn = true;
	}

	void updateCurrTBuffer() {
		taaShader->use();
		taaShader->setMat4("lastCameraMVP", lastCameraMVP);
		
		currTBuffer += 1;
		currTBuffer %= T_BUFFER;
		defaultFrameBuffer.frameBuffer = frameBuffer[currTBuffer];
		defaultFrameBuffer.texBuffer = texBuffer[currTBuffer];
		defaultFrameBuffer.rbo = rbo[currTBuffer];
	}

#ifdef _DEBUG
	void initSSSSS() {
		auto s = new SSSSS();
		auto shader = s->GetShader();
		shader->use();
		shader->setTexture("gPositionRoughness", GetGBuffer(0));
		shader->setTexture("gNormalDepth", GetGBuffer(1));
		shader->setTexture("gAlbedoMetallic", GetGBuffer(2));
		shader->setVec3("L", glm::normalize(glm::vec3(-0.5f, -1.0f, 0.3f)));
		shader->setInt("range", 2);
		AddPostProcess(s);
	}

#endif // _DEBUG

public:
	Render() { 
		GenFrameBuffer();  GenUbos(); GenGbuffer();
		auto p = new Blit(this);
		auto taa = new TAA(gBufferTexBuffer[0]);
		AddPostProcess(taa);
		taaShader = taa->GetShader();
		AddPostProcess(p);
		taaShader->use();
		taaShader->setTexture("gWorldPos", gBufferTexBuffer[0]);
		cubeMipMapShader = std::make_shared<Shader>("shaders/mipmap.vs", "shaders/mipmap.fs");
		texMipMapShader = std::make_shared<Shader>("shaders/bf.vs", "shaders/mip.fs");
		defaultFrameBuffer.frameBuffer = frameBuffer[0];
		defaultFrameBuffer.texBuffer = texBuffer[0];
		defaultFrameBuffer.w = SCR_WIDTH;
		defaultFrameBuffer.h = SCR_HEIGHT;
		defaultFrameBuffer.rbo = rbo[0];
	}

	~Render() {
		glDeleteBuffers(T_BUFFER, frameBuffer);
		glDeleteBuffers(T_BUFFER, texBuffer);
		glDeleteBuffers(T_BUFFER, rbo);
		glDeleteBuffers(GBUFFER_SIZE, gBufferTexBuffer);
		glDeleteFramebuffers(1, &gBufferFrameBuffer);
		glDeleteRenderbuffers(1, &gBufferRbo);
		for (auto& i : postProcess) {
			delete i;
			i = nullptr;
		}
	}

	void InitReflectProbe(Scene* s) {
		std::vector<ReflectProbe*> reflectProbes;
		for (auto& o : s->objects) {
			auto probe = o->GetComponent<ReflectProbe>();
			if (probe)
				reflectProbes.push_back(probe);
		}
		for (auto& i : reflectProbes) {
			if (i->GetReflectProbeType() == ReflectProbeType::Static||1) {
				auto pos = i->object->GetComponent<Transform>()->GetPosition();
				auto tmpCamera = s->GetMainCamera();
				auto tmpFront = tmpCamera->Front;
				auto tmpFov = tmpCamera->Fov;
				auto tmpW = tmpCamera->w;
				auto tmpH = tmpCamera->h;
				auto tmpPos = tmpCamera->Position;
				tmpCamera->Fov = 90;
				tmpCamera->w = i->GetWidth();
				tmpCamera->h = i->GetHeight();
				tmpCamera->Position = pos;
				FrameBuffer out;
				out.frameBuffer = i->GetFrameBuffer();
				out.w = i->GetWidth();
				out.h = i->GetHeight();
				for (int j = 0; j < 6; j++) {
					tmpCamera->Front=(captureViews[j * 2]);
					tmpCamera->Up = captureViews[j * 2 + 1];
					glBindFramebuffer(GL_FRAMEBUFFER, i->GetFrameBuffer());
					glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + j, i->GetCubeMap(), 0);
					(*this)(s, out, false,true);
					int w = 512, h = 512;
					/*unsigned char* imageData = new unsigned char[w * h * 3]{ 255 };
					glBindFramebuffer(GL_FRAMEBUFFER, i->GetFrameBuffer());
					glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, (unsigned char*)imageData);
					std::string file = "baking/test" + std::to_string(j);
					file.append(".png");
					stbi_write_png(file.c_str(), w, h, 3, imageData, 0);*/
				}
				tmpCamera->Fov = tmpFov;
				tmpCamera->SetFront(tmpFront);
				tmpCamera->w=tmpW ;
				tmpCamera->h=tmpH;
				tmpCamera->Position = tmpPos;
				FrameBuffer b;
				b.frameBuffer = i->GetFrameBuffer();
				b.texBuffer = i->GetCubeMap();
				b.w = i->GetWidth();
				b.h = i->GetHeight();
				b.rbo = i->GetRbo();
				auto p=GenCubeMipMap(b, 4);
				i->SetCubeMap(p);
			}
		}
	}

	void operator()(Scene* scene, FrameBuffer buffer = emptyBuffer, bool applyPost = true,bool applySky=true) {
		
		if (scene->mainCamera == NULL)
			return;
		auto camaraTransform = scene->mainCamera->object->GetComponent<Transform>();
		if (camaraTransform == NULL)
			return;
		if (firstCall) {
			GenStaticShadowMaps(scene);
			firstCall = false;
		}
		GenShadowMaps(scene);
		auto mvp=ApplyCameraProperties(scene->mainCamera);

		RenderGbuffer(scene);
		FrameBuffer currBuffer;
		if (buffer.w != emptyBuffer.w) {
			currBuffer = buffer;
		}
		else {
			updateCurrTBuffer();
			currBuffer = defaultFrameBuffer;
		}
			
		glBindFramebuffer(GL_FRAMEBUFFER, currBuffer.frameBuffer);
		glViewport(0, 0, currBuffer.w, currBuffer.h);
		glClearColor(0.7, 1, 1, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		if (defferedShader != NULL) {
			defferedShader->use();
			ApplyLightProperties(defferedShader, scene);
			FrameBuffer out;
			out.frameBuffer = currBuffer.frameBuffer;
			out.texBuffer = currBuffer.texBuffer;
			BlitMap(0, out, defferedShader.get());
			glBindFramebuffer(GL_READ_FRAMEBUFFER, gBufferFrameBuffer);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, currBuffer.frameBuffer);
			glBlitFramebuffer(
				0, 0, SCR_WIDTH, SCR_HEIGHT, 0, 0, SCR_WIDTH, SCR_HEIGHT, GL_DEPTH_BUFFER_BIT, GL_NEAREST
			);
		}

		/*glCullFace(GL_BACK);*/
		for (auto& i : scene->objects) {
			if (!i->IsActive())
				continue;
			auto transform = i->GetComponent<Transform>();
			if (transform == NULL)
				continue;
			for (int j = 0; j < i->GetMeshLength(); j++) {
				auto shader = i->GetShader(j);
				if (i->GetShader(j, Deffered) != NULL)
					continue;
				if (shader != NULL) {
					if (shader->GetRenderType() == Transparent) {
						glEnable(GL_BLEND);
						glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
					}
					else if (shader->GetRenderType() == Opaque) {
						glDisable(GL_BLEND);
					}
					shader->use();
					shader->setMat4("trans", (*transform)());
					ApplyLightProperties(shader, scene);
					i->draw(j);
				}
			}
		}	

		//draw sky
		if (scene->sky != NULL&&applySky) {
			scene->sky->use();
			scene->sky->setMat4("projection", scene->mainCamera->GetProjection());
			scene->sky->setMat4("view", scene->mainCamera->GetView());
			glDepthFunc(GL_LEQUAL); 
			glBindVertexArray(scene->skyVAO);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_CUBE_MAP, scene->skyBuffer);
			glDrawArrays(GL_TRIANGLES, 0, 36);
			glBindVertexArray(0);
			glDepthFunc(GL_LESS); // set depth function back to default
		}

		if(applyPost)
			ApplyPostProcess();

		lastCameraMVP = mvp;
		
	}

	unsigned GetTexBuffer() const { return texBuffer[0]; }

	unsigned GetFrameBuffer() const { return frameBuffer[0]; }

	unsigned GetGBuffer(int index) { if (index >= 0 && index < GBUFFER_SIZE) return gBufferTexBuffer[index]; return -1; }

	void AddPostProcess(PostProcess* p) {
		
		if (postProcess.size() > 0) {
			postProcess.back()->GenFrameBuffer();
			p->SetInTexBuffer(postProcess.back()->GetOutBuffer());
		}
		else {
			p->SetInTexBuffer(texBuffer[0]);
		}
		postProcess.push_back(p);
	}

	void SetDefferedShader(std::shared_ptr<Shader> s) { 
		defferedShader = s; 
		defferedShader->use();
		for (auto i = 0; i < GBUFFER_SIZE; i++) {
			defferedShader->setTexture("gBuffer"+std::to_string(i), gBufferTexBuffer[i]);
		}
	}

	unsigned GetGBuffer(unsigned i) const { if (i >= 0 && i < GBUFFER_SIZE) return gBufferTexBuffer[i]; return 0; }

	void openSSDO() &  {
		if(!ssdoOn)
			initSSDO();
	}

	void openSSAO() &  {
		if(!ssaoOn)
			initSSAO();
	}

	void openAntiNoise() &  {
		if (!antiNoiseOn)
			initAntiNoise();
	}

	void openSSGI()& {
		if (!ssgiOn)
			initSSGI();
	}
};



#endif // !RENDER_H
