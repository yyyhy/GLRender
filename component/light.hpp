#pragma once
#ifndef LIGHT_H
#define LIGHT_H
#include"glm.hpp"
#include"component.hpp"
#include"photon.hpp"

typedef glm::vec3 Spectrum;
#define WHITE Spectrum(1.f,1.f,1.f)

#define RSM_W 5096
#define RSM_H 5096

class Light:public Component {
protected:
	float intensity;
	Spectrum color;
	unsigned rsmBuffer;
	unsigned frameBuffer;
	unsigned rbo;
	bool genShadowMap = false;
	bool genRsm = false;
	int rsmW;
	int rsmH;
public:
	Light():intensity(0),color(WHITE) { name = "light"; }
	
	Light(const Spectrum& col, float inten, bool genSM = false,bool genRsm=false) :intensity(inten), color(col), genShadowMap(genSM),genRsm(genRsm) { name = "light"; }
	
	~Light() {
		glDeleteBuffers(1, &rsmBuffer);
		glDeleteBuffers(1, &frameBuffer);
#ifdef _DEBUG
		std::cout << "light lose\n";
#endif // _DEBUG

	}

	virtual void initBuffer(int, int) {}

	virtual void LoadToShader(Shader& s){}
	
	virtual Photon samplePhoton() { return {}; }

	virtual glm::mat4 GetLightMat() { return glm::mat4(1.f); }

	bool hasRsm() const { return genShadowMap; }

	uint32_t GetFrameBuffer() const { return frameBuffer; }

	unsigned GetRSMBuffer() const { return rsmBuffer; }

	unsigned GetRSMWidth() const { return rsmW; }

	unsigned GetRSMHeigth() const { return rsmH; }

	void SetRSMBuffer(unsigned buffer) { glDeleteBuffers(1,&rsmBuffer); rsmBuffer = buffer; }

	Spectrum GetSpectrum() const {
		return intensity * color;
	}

	void SetSpectrum(const Spectrum& s) {
		color = s;
	}

	void SetIntensity(float inte) {
		intensity = inte;
	}

	std::shared_ptr<Shader> rsmShader;
	bool isStatic;
};

#endif // !LIGHT_H
