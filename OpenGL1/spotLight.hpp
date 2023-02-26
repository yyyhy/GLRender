

#ifndef SPOT_LIGHT_H
#define SPOT_LIGHT_H
#include"light.hpp"
#include"opengl.hpp"
#include"shader.hpp"

class SpotLight :public Light {
public:
	float radius;
	Vector3f dir;
	Vector3f pos;
public:
	SpotLight(unsigned rsmW, unsigned rsmH,Spectrum color, float r, float inten, Vector3f dir, bool genSM = false)
		:dir(glm::normalize(dir)), radius(r), Light(rsmW,rsmH,1,color, inten, genSM) { if(genSM) initBuffer(2048,2048); }

	void initBuffer(int w, int h) override{
		rsmShader = std::make_shared<Shader>("shaders/RSM.vs", "shaders/RSM.fs");

		/*glGenFramebuffers(bufferSize, frameBuffer);
		glGenTextures(bufferSize, rsmBuffer);
		glGenRenderbuffers(bufferSize, rbo);
		for (unsigned i = 0; i < bufferSize; ++i) {
			glBindTexture(GL_TEXTURE_2D, rsmBuffer[i]);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, w, h, 0, GL_RGB, GL_FLOAT, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer[i]);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, rsmBuffer[i], 0);

			glBindRenderbuffer(GL_RENDERBUFFER, rbo[i]);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, w, h);
			glBindRenderbuffer(GL_RENDERBUFFER, 0);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo[i]);

			glBindTexture(GL_TEXTURE_2D, 0);

			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
				std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!11" << std::endl;
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}*/
	}

	std::vector<glm::mat4> GetLightMat() override {
		glm::mat4 model(1.f);
		model = glm::translate(model, -pos);
		glm::mat4 view = glm::lookAt(pos, pos+dir, glm::vec3(0.f, 1.f, 0.f));
		glm::mat4 projection = glm::perspective(glm::radians(radius), 1.f, 0.01f, 100.f);
		//glm::mat4 projection = glm::ortho(-15.f, 15.f, -15.f, 15.f, 0.1f, 100.f);
		auto tmp = projection *view *model;
		std::vector<glm::mat4> v;
		v.push_back(tmp);
		return v;
	}

	void LoadToShader(Shader& s) override {
		s.Use();
		s.SetVec3("spotLight.uLightColor", GetSpectrum());
		s.SetVec3("spotLight.uLightDir", dir);
		s.SetBool("spotLight.exist", true);
		s.SetFloat("spotLight.radius", radius);
		s.SetVec3("spotLight.pos", pos);
		if (genShadowMap) {
			//s.SetTexture("spotLight.shadowMap0", rsmBuffer[0]);
			s.SetMat4("spotLight.lightMVP", GetLightMat().at(0));
		}
	}

};
#endif // !SPOT_LIGHT_H
