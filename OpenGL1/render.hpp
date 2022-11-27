

#ifndef RENDER_H
#define RENDER_H
#include"scene.hpp"
#include"frameBuffer.hpp"
#include"filter.hpp"
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
#include"bloom.hpp"
#include<list>
#include"debugTool.hpp"

#define CAMERA_UBO_SIZE 208+64+16+64
#define	RENDER_SEETINGS_SIZE 16
#define CAMERA_UBO_POINT 0
#define RENDER_UBO_POINT 1
#define GBUFFER_SIZE 7

static unsigned CameraPropertyUbo;
static unsigned RenderSettingsUbo;

class Render {
private:

	FrameBufferO BackFrameBuffer;
	Texture2D ColorBuffer;

	FrameBufferO gFrameBuffer;
	Texture2D gColorBuffers[GBUFFER_SIZE];

	unsigned RenderWidth;
	unsigned RenderHeight;

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
	sp_shader blitShader;
	glm::mat4 lastCameraMVP=glm::mat4(0);
	unsigned i = 0, n = 4;

	std::list<PostProcess*> postProcess;
	

	void GenShadowMaps(Scene* scene) {
		/*glCullFace(GL_FRONT);*/
		for (auto& l : scene->lights)
			if (l && l->hasRsm() && !l->isStatic) {
				auto rsmShader = l->rsmShader;
				if (rsmShader == NULL)
					continue;
				rsmShader->use();
				auto mats = l->GetLightMat();
				for (unsigned i = 0; i < l->bufferSize; ++i) {
					rsmShader->setMat4("LightMat", mats.at(i));
					glViewport(0, 0, l->GetRSMWidth(), l->GetRSMHeigth());
					glBindFramebuffer(GL_FRAMEBUFFER, l->GetFrameBuffer(i));
					glClearColor(0, 0, 0, 1);
					glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
					for (auto& o : scene->objects) {
						rsmShader->setMat4("trans", (*o->GetComponent<Transform>())());
						o->draw(l->rsmShader.get());
					}
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
				for (unsigned i = 0; i < l->bufferSize; ++i) {
					rsmShader->setMat4("LightMat", mat.at(i));

					glBindFramebuffer(GL_FRAMEBUFFER, l->GetFrameBuffer(i));
					glViewport(0, 0, l->GetRSMWidth(), l->GetRSMHeigth());
					glClearColor(0.3, 1, 1, 1);
					glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
					for (auto& o : scene->objects) {
						rsmShader->setMat4("trans", (*o->GetComponent<Transform>())());
						o->draw(l->rsmShader.get());
					}
				}
				
			}
	}

	void GenUbos() {
		if (CameraPropertyUbo == 0) {
			glGenBuffers(1, &CameraPropertyUbo);
			glBindBuffer(GL_UNIFORM_BUFFER, CameraPropertyUbo);
			glBufferData(GL_UNIFORM_BUFFER, CAMERA_UBO_SIZE, NULL, GL_STATIC_DRAW);
			glBindBufferBase(GL_UNIFORM_BUFFER, CAMERA_UBO_POINT, CameraPropertyUbo);
			glBindBuffer(GL_UNIFORM_BUFFER, 0);
		}
		
		if (RenderSettingsUbo == 0) {
			glGenBuffers(1, &RenderSettingsUbo);
			glBindBuffer(GL_UNIFORM_BUFFER, RenderSettingsUbo);
			glBufferData(GL_UNIFORM_BUFFER, RENDER_SEETINGS_SIZE, NULL, GL_STATIC_DRAW);
			glBindBufferBase(GL_UNIFORM_BUFFER, RENDER_UBO_POINT, RenderSettingsUbo);
			glBufferSubData(GL_UNIFORM_BUFFER, 0, 16, glm::value_ptr(glm::vec2(RenderWidth, RenderHeight)));
			glBindBuffer(GL_UNIFORM_BUFFER, 0);
		}	
	}

	glm::mat4 ApplyCameraProperties(Camera* c) {
		auto trans = c->object->GetComponent<Transform>();
		glBindBuffer(GL_UNIFORM_BUFFER, CameraPropertyUbo);
		auto m = c->GetModel();
		auto v = c->GetView();
		auto p = c->GetProjection();
		glBufferSubData(GL_UNIFORM_BUFFER, 0, 64, glm::value_ptr(m));
		glBufferSubData(GL_UNIFORM_BUFFER, 64, 64, glm::value_ptr(v));
		glBufferSubData(GL_UNIFORM_BUFFER, 128, 64, glm::value_ptr(p));
		auto mvp = p * v * m;;
		glBufferSubData(GL_UNIFORM_BUFFER, 192, 64, glm::value_ptr(mvp));
		glBufferSubData(GL_UNIFORM_BUFFER, 256, 64, glm::value_ptr(lastCameraMVP));
		glBufferSubData(GL_UNIFORM_BUFFER, 320, 16, glm::value_ptr(trans->GetPosition()));
		glm::vec2 offset(i, n);
		i += 1;
		i %= n;
		glBufferSubData(GL_UNIFORM_BUFFER, 336, 16, glm::value_ptr(offset));
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

	void ApplyPostProcess(FrameBufferO& targetBuffer) {
		auto p = postProcess.begin();
		auto last = postProcess.end(); --last;
		(*last)->SetOutFrameBuffer(&targetBuffer);
		(*last)->SetOutTexBuffer(targetBuffer.GetTexture(0));
		(*p)->SetInTexBuffer(*BackFrameBuffer.GetTexture(0));
		for (p; p!=postProcess.end();++p) {

			(*p)->excute();
			
			auto next = p;
			++next;
			if (next == postProcess.end())
				continue;
			if ((*p)->enable)
				(*p)->SendBufferToNext(*next);
			else
				(*next)->SetInTexBuffer((*p)->GetInTexBuffer());
		}
	}

	void RenderGbuffer(Scene* s) {
		auto camaraTransform = mainCamera->object->GetComponent<Transform>();
		gFrameBuffer.Bind();
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
					shader->setMat4("preTrans", transform->lastTransform);
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
		ssao->mainShader->setTexture("gPosition", GetGBuffer(0));
		ssao->mainShader->setTexture("gNormalDepth", GetGBuffer(1));
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
		ssgiShader->setTexture("gFlag", GetGBuffer(3));
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
	
	}

	void BindGBuffer() const {
		defferedShader->use();
		for (auto i = 0; i < GBUFFER_SIZE; i++) {
			defferedShader->setTexture("gBuffer" + std::to_string(i), gColorBuffers[i].id);
		}
	}

	void BindRenderSettings() const{
		glBindBuffer(GL_UNIFORM_BUFFER, RenderSettingsUbo);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, 16, glm::value_ptr(glm::vec2(RenderWidth, RenderHeight)));
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
	Render(unsigned width=SCR_WIDTH,unsigned height=SCR_HEIGHT) :
		RenderWidth(width),RenderHeight(height),
		BackFrameBuffer(width,height,true),gFrameBuffer(width,height,true),
		ColorBuffer(width, height, GL_RGBA32F, GL_RGBA) {
		GenUbos(); 
		BackFrameBuffer.AttachTexture(&ColorBuffer);

		for (int i = 0; i < GBUFFER_SIZE; ++i) {
			gColorBuffers[i] = Texture2D(width, height, GL_RGBA32F, GL_RGBA);
		}
		gFrameBuffer.AttachTexture(gColorBuffers, GBUFFER_SIZE);
			
		auto blit = new Blit();
		auto taa = new TAA();
		AddPostProcess(blit);
		//AddPostProcess(taa);
		
		taaShader = taa->GetShader();
		
		taaShader->use();
		taaShader->setTexture("gWorldPos", GetGBuffer(0));
		taaShader->setTexture("gVelo", GetGBuffer(6));
		taaShader->setTexture("gNormalDepth", GetGBuffer(1));
		cubeMipMapShader = std::make_shared<Shader>("shaders/mipmap.vs", "shaders/mipmap.fs");
		texMipMapShader = std::make_shared<Shader>("shaders/bf.vs", "shaders/mip.fs");
	}

	~Render() {
		
		for (auto& i : postProcess) {
			delete i;
			i = nullptr;
		}
	}

	void operator()(Scene* scene, const FrameBufferO& targetBuffer = TargetOutputFrameBuffer, bool applySky=true) {
		
		if (mainCamera == NULL)
			return;
		auto camaraTransform = mainCamera->object->GetComponent<Transform>();
		if (camaraTransform == NULL)
			return;

		BindRenderSettings();
		BindGBuffer();

		if (firstCall) {
			GenStaticShadowMaps(scene);
			firstCall = false;
		}
		GenShadowMaps(scene);
		auto mvp=ApplyCameraProperties(mainCamera);

		RenderGbuffer(scene);
			
		BackFrameBuffer.Bind();
		glClearColor(0.7, 1, 1, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		if (defferedShader != NULL) {
			defferedShader->use();
			ApplyLightProperties(defferedShader, scene);
			BlitMap(0, *BackFrameBuffer.GetTexture(0), defferedShader.get(), BackFrameBuffer);
			glBindFramebuffer(GL_READ_FRAMEBUFFER, gFrameBuffer.frameBuffer);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, BackFrameBuffer.frameBuffer);
			glBlitFramebuffer(
				0, 0, RenderWidth, RenderHeight, 0, 0, RenderWidth, RenderHeight, GL_DEPTH_BUFFER_BIT, GL_NEAREST
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
			scene->sky->setMat4("projection", mainCamera->GetProjection());
			scene->sky->setMat4("view", mainCamera->GetView());
			glDepthFunc(GL_LEQUAL); 
			glBindVertexArray(scene->skyVAO);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_CUBE_MAP, scene->skyBuffer);
			glDrawArrays(GL_TRIANGLES, 0, 36);
			glBindVertexArray(0);
			glDepthFunc(GL_LESS); // set depth function back to default
		}

		ApplyPostProcess(const_cast<FrameBufferO&>(targetBuffer));

		lastCameraMVP = mvp;
	}

	unsigned GetTexBuffer() const { return ColorBuffer.id; }

	unsigned GetFrameBuffer() const { return BackFrameBuffer.frameBuffer; }

	void AddPostProcess(PostProcess* p) {
		
		if (postProcess.size() > 0) {
			p->GenFrameBuffer();
		}
		else {
			p->SetInTexBuffer(ColorBuffer.id);
		}
		postProcess.push_front(p);
	}

	void SetDefferedShader(std::shared_ptr<Shader> s) { 
		defferedShader = s; 
	}
	
	unsigned GetGBuffer(unsigned i) const { if (i >= 0 && i < GBUFFER_SIZE) return gColorBuffers[i].id; return 0; }

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

#ifdef _DEBUG
	void Capture() {
		SaveFrameBuffer(postProcess.back()->GetOutFrameBuffer());
	}

	void CaptureGBuffer() {
		SaveFrameBuffer(gFrameBuffer);
	}

	void CaptureBackBuffer() {
		SaveFrameBuffer(BackFrameBuffer);
	}

	void CapturePostProcessOutput(unsigned index) {
		auto p = postProcess.begin();
		for (int i = 0; i < index; ++index)
			++p;
		SaveFrameBuffer((*p)->GetOutFrameBuffer());
	}
#endif // _DEBUG

};

#endif // !RENDER_H
