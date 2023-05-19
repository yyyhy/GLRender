

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
#include"math.hpp"
#include"taa.hpp"
#include"bloom.hpp"
#include<list>
#include"debugTool.hpp"
#include"RenderManager.hpp"

#define CAMERA_UBO_SIZE 208+64+16+64
#define	RENDER_SEETINGS_SIZE 16
#define CAMERA_UBO_POINT 0
#define RENDER_UBO_POINT 1
#define GBUFFER_SIZE 7

static unsigned CameraPropertyUbo;
static unsigned RenderSettingsUbo;

class Render : public RenderPipeline{
private:

	FrameBuffer BackFrameBuffer;
	Texture2D BackColorBuffer;

	FrameBuffer gFrameBuffer;
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
	sp_shader taaShader;
	sp_shader blitShader;
	glm::mat4 lastCameraMVP = glm::mat4(0);
	unsigned i = 0, n = 4;

	std::list<PostProcess*> postProcess;

	bool firstGenShadowMaps = true;
	void GenShadowMaps(Scene* scene) {
		/*glCullFace(GL_FRONT);*/
		for (auto& l : scene->lights)
			if (l && l->hasRsm() && !l->isStatic) {
				auto rsmShader = l->rsmShader;
				if (rsmShader == NULL)
					continue;
				rsmShader->Use();
				auto mats = l->GetLightMat();
				for (unsigned i = 0; i < l->FrameBufferSize; ++i) {
					rsmShader->SetMat4("LightMat", mats.at(i));

					l->GetFrameBuffer(i).Bind();
					glClearColor(0, 0, 0, 1);
					glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

					for (auto& o : scene->objects) {
						rsmShader->SetMat4("trans", (*o->GetComponent<Transform>())());

						for (int i = 0; i < o->GetMeshLength(); ++i) {
							auto shader = o->GetShader(i, Deffered);
							if (shader != nullptr) {
								Texture& albedoMap = shader->GetTexture("albedoMap");
								rsmShader->SetTexture("albedoMap", albedoMap);
								Texture& normalMap = shader->GetTexture("normalMap");
								rsmShader->SetTexture("normalMap", normalMap);
								Texture& roughnessMap = shader->GetTexture("roughnessMap");
								rsmShader->SetTexture("roughnessMap", roughnessMap);
								Texture& metallicMap = shader->GetTexture("metallicMap");
								rsmShader->SetTexture("metallicMap", metallicMap);
							}
							o->draw(i, l->rsmShader.get());
						}

					}
				}

				if (firstGenShadowMaps) {
					firstGenShadowMaps = false;
				}
			}

	}

	void GenStaticShadowMaps(Scene* scene) {
		for (auto& l : scene->lights)
			if (l && l->hasRsm() && l->isStatic) {
				auto rsmShader = l->rsmShader;
				if (rsmShader == NULL)
					continue;
				rsmShader->Use();
				//std::cout << l->GetRSMWidth() << " " << l->GetRSMHeigth() << " " << l->GetFrameBuffer() << " " << l->GetRSMBuffer() << "\n\n";
				auto mats = l->GetLightMat();
				for (unsigned i = 0; i < l->FrameBufferSize; ++i) {
					rsmShader->SetMat4("LightMat", mats.at(i));
					auto fbo = l->GetFrameBuffer(i);
					fbo.Bind();
					glClearColor(0.3, 1, 1, 1);
					glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
					for (auto& o : scene->objects) {
						rsmShader->SetMat4("trans", (*o->GetComponent<Transform>())());
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

	void ApplyLightProperties(std::shared_ptr<Shader> s, Scene* scene) {
		for (int i = 0; i < scene->lights.size(); i++) {
			auto l = scene->lights[i];
			if (l != NULL) {
				//l->
				l->LoadToShader(*s);

			}
		}
	}

	void ApplyPostProcess(FrameBuffer& targetBuffer) {
		auto p = postProcess.begin();
		auto last = postProcess.end(); --last;
		(*last)->SetOutFrameBuffer(&targetBuffer);
		(*last)->SetOutTexBuffer(targetBuffer.GetTexture(0));
		(*p)->SetInTexBuffer(*BackFrameBuffer.GetTexture(0));
		for (p; p != postProcess.end(); ++p) {

			(*p)->Excute();

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
			if (transform == nullptr)
				continue;
			for (int j = 0; j < i->GetMeshLength(); j++) {
				auto shader = i->GetShader(j, Deffered);
				if (shader != nullptr) {
					shader->Use();
					shader->SetMat4("preTrans", transform->lastTransform);
					shader->SetMat4("trans", (*transform)());

				}

			}
			i->drawGBuffer();
		}
	}

	void initSSDO() {
		auto ssdo = new SSDO(RenderWidth, RenderHeight);
		auto ssdoShader = ssdo->mainShader;
		ssdoShader->SetTexture("gPositionRoughness", GetGBuffer(0));
		ssdoShader->SetTexture("gNormalDepth", GetGBuffer(1));
		ssdoShader->SetTexture("gAlbedoMetallic", GetGBuffer(2));

		AddPostProcess(ssdo);
		ssdoOn = true;
	}

	void initSSAO() {
		auto ssao = new SSAO(defferedShader, RenderWidth, RenderHeight);
		ssao->mainShader->SetTexture("gPosition", GetGBuffer(0));
		ssao->mainShader->SetTexture("gNormalDepth", GetGBuffer(1));
		AddPostProcess(ssao);
		ssaoOn = true;
	}

	void initSSGI() {
		auto ssgi = new SSGI(RenderWidth, RenderHeight);
		auto ssgiShader = ssgi->GetShader();
		ssgiShader->Use();
		ssgiShader->SetTexture("gPositionRoughness", GetGBuffer(0));
		ssgiShader->SetTexture("gNormalDepth", GetGBuffer(1));
		ssgiShader->SetTexture("gAlbedoMetallic", GetGBuffer(2));
		ssgiShader->SetTexture("gFlag", GetGBuffer(3));
		AddPostProcess(ssgi);
		ssgiOn = true;
	}

	void initAntiNoise() {
		auto v = new BidirectionVFilter(RenderWidth, RenderHeight);
		auto h = new BidirectionHFilter(RenderWidth, RenderHeight);
		auto vShader = v->GetShader();
		vShader->SetTexture("gNormal", GetGBuffer(1));
		vShader->SetTexture("gPosition", GetGBuffer(0));
		auto hShader = h->GetShader();
		hShader->SetTexture("gNormal", GetGBuffer(1));
		hShader->SetTexture("gPosition", GetGBuffer(0));
		AddPostProcess(v);
		AddPostProcess(h);
		antiNoiseOn = true;
	}

	void updateCurrTBuffer() {
		taaShader->Use();
		taaShader->SetMat4("lastCameraMVP", lastCameraMVP);

	}

	void BindGBuffer() const {
		defferedShader->Use();
		for (auto i = 0; i < GBUFFER_SIZE; i++) {
			defferedShader->SetTexture("gBuffer" + std::to_string(i), gColorBuffers[i].id);
		}
	}

	void BindRenderSettings() const {
		glBindBuffer(GL_UNIFORM_BUFFER, RenderSettingsUbo);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, 16, glm::value_ptr(glm::vec2(RenderWidth, RenderHeight)));
	}

#ifdef _DEBUG
	void initSSSSS() {
		auto s = new SSSSS(RenderWidth, RenderHeight);
		auto shader = s->GetShader();
		shader->use();
		shader->SetTexture("gPositionRoughness", GetGBuffer(0));
		shader->SetTexture("gNormalDepth", GetGBuffer(1));
		shader->SetTexture("gAlbedoMetallic", GetGBuffer(2));
		shader->SetVec3("L", glm::normalize(glm::vec3(-0.5f, -1.0f, 0.3f)));
		shader->SetInt("range", 2);
		AddPostProcess(s);
	}

#endif // _DEBUG

public:
	Render(unsigned width = SCR_WIDTH, unsigned height = SCR_HEIGHT) :
		RenderWidth(width), RenderHeight(height),
		BackFrameBuffer(width, height, true), gFrameBuffer(width, height, true),
		BackColorBuffer(width, height, GL_RGBA32F, GL_RGBA) {
		GenUbos();
		BackFrameBuffer.AttachTexture(&BackColorBuffer);

		for (int i = 0; i < GBUFFER_SIZE; ++i) {
			gColorBuffers[i] = Texture2D(width, height, GL_RGBA32F, GL_RGBA);
		}
		gFrameBuffer.AttachTexture(gColorBuffers, GBUFFER_SIZE);

		auto blit = new Blit(RenderWidth, RenderHeight);

		AddPostProcess(blit);
	}

	~Render() {
		for (auto& i : postProcess) {
			delete i;
			i = nullptr;
		}
		BackColorBuffer.Release();
		for (int i = 0; i < GBUFFER_SIZE; ++i) {
			gColorBuffers[i].Release();
		}
	}

	void MainRender() override{
		
	};

	void operator()(Scene* scene, const FrameBuffer& targetBuffer = TargetOutputFrameBuffer) {

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
		auto mvp = ApplyCameraProperties(mainCamera);

		RenderGbuffer(scene);

		BackFrameBuffer.Bind();
		glClearColor(0.7, 1, 1, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if (defferedShader != NULL) {
			defferedShader->Use();
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
					shader->Use();
					shader->SetMat4("trans", (*transform)());
					ApplyLightProperties(shader, scene);
					i->draw(j);
				}
			}
		}

		//draw sky
		if (scene->sky != NULL) {
			scene->sky->Use();
			scene->sky->SetMat4("projection", mainCamera->GetProjection());
			scene->sky->SetMat4("view", mainCamera->GetView());
			glDepthFunc(GL_LEQUAL);
			glBindVertexArray(scene->skyVAO);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_CUBE_MAP, scene->skyBuffer);
			glDrawArrays(GL_TRIANGLES, 0, 36);
			glBindVertexArray(0);
			glDepthFunc(GL_LESS); // set depth function back to default
		}

		ApplyPostProcess(const_cast<FrameBuffer&>(targetBuffer));

		lastCameraMVP = mvp;
	}

	unsigned GetTexBuffer() const { return BackColorBuffer.id; }

	unsigned GetFrameBuffer() const { return BackFrameBuffer.frameBuffer; }

	void AddPostProcess(PostProcess* p) {

		if (postProcess.size() > 0) {
			p->GenFrameBuffer();
		}
		else {
			p->SetInTexBuffer(BackColorBuffer.id);
		}
		postProcess.push_front(p);
	}

	void SetDefferedShader(std::shared_ptr<Shader> s) {
		defferedShader = s;
	}

	Texture2D& GetGBuffer(unsigned i) const { return const_cast<Texture2D&>(gColorBuffers[i]); }

	void openSSDO()& {
		if (!ssdoOn)
			initSSDO();
	}

	void openSSAO()& {
		if (!ssaoOn)
			initSSAO();
	}

	void openAntiNoise()& {
		if (!antiNoiseOn)
			initAntiNoise();
	}

	void openSSGI()& {
		if (!ssgiOn)
			initSSGI();
	}

	void OpenTAA()& {
		auto taa = new TAA(RenderWidth, RenderHeight);
		taaShader = taa->GetShader();

		taaShader->Use();
		taaShader->SetTexture("gWorldPos", GetGBuffer(0));
		taaShader->SetTexture("gVelo", GetGBuffer(6));
		taaShader->SetTexture("gNormalDepth", GetGBuffer(1));
		AddPostProcess(taa);
	}

	void Capture() {
		SaveFrameBuffer(postProcess.back()->GetOutFrameBuffer());
	}

	void CaptureGBuffer() {
		gFrameBuffer.Bind();
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gColorBuffers[2].id, 0);
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
};

#endif // !RENDER_H
