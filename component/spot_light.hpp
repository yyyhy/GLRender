

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
	SpotLight(Spectrum color, float r, float inten, Vector3f dir, bool genSM = false) :dir(glm::normalize(dir)), radius(r), Light(color, inten, genSM) { if(genSM) initBuffer(2048,2048); }

	void initBuffer(int w, int h) override{
		rsmShader = std::make_shared<Shader>("shaders/RSM.vs", "shaders/RSM_z.fs");
		
		rsmH = h;
		rsmW = w;
		glGenFramebuffers(1, &frameBuffer);

		glGenTextures(1, &rsmBuffer);
		glBindTexture(GL_TEXTURE_2D, rsmBuffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, w, h, 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, rsmBuffer, 0);

		glGenRenderbuffers(1, &rbo);
		glBindRenderbuffer(GL_RENDERBUFFER, rbo);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, w, h);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

		glBindTexture(GL_TEXTURE_2D, 0);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!11" << std::endl;
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	glm::mat4 GetLightMat() override {
		/*glm::mat4 model(1.f);
		model = glm::translate(model, -pos);
		glm::mat4 projection = glm::ortho(-5.f, 5.f, -5.f, 5.f, 0.1f, 100.f);
		glm::mat4 view = glm::lookAt(pos, pos+dir, glm::vec3(0.f, 1.f, 0.f));
		glm::mat4 tmp = projection * view*model;
		return tmp;*/

		glm::mat4 model(1.f);
		model = glm::translate(model, -pos);
		glm::mat4 view = glm::lookAt(pos, pos+dir, glm::vec3(0.f, 1.f, 0.f));
		glm::mat4 projection = glm::perspective(glm::radians(radius), 1.f, 0.01f, 100.f);
		//glm::mat4 projection = glm::ortho(-15.f, 15.f, -15.f, 15.f, 0.1f, 100.f);
		auto tmp = projection *view *model;
		
		return tmp;
	}

	void LoadToShader(Shader& s) override {
		s.use();
		s.setVec3("spotLight.uLightColor", GetSpectrum());
		s.setVec3("spotLight.uLightDir", dir);
		s.setBool("spotLight.exist", true);
		s.setFloat("spotLight.radius", radius);
		s.setVec3("spotLight.pos", pos);
		if (genShadowMap) {
			s.setTexture("spotLight.shadowMap0", rsmBuffer);
			s.setMat4("spotLight.lightMVP", GetLightMat());
		}
	}

};
#endif // !SPOT_LIGHT_H
