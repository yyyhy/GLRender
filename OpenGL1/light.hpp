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
	float flux;
	Spectrum color;
	unsigned *rsmBuffer;
	unsigned *frameBuffer;
	unsigned *rbo;
	bool genShadowMap = false;
	bool genRsm = false;
	int rsmW;
	int rsmH;

public:
	const unsigned bufferSize;
	Light(unsigned bufferSize=1):flux(0),color(WHITE),bufferSize(bufferSize) { 
		name = "light"; 
		rsmBuffer = new unsigned[bufferSize];
		frameBuffer = new unsigned[bufferSize];
		rbo = new unsigned[bufferSize];
	}
	
	Light(const Spectrum& col, float inten, bool genSM = false,bool genRsm=false, unsigned bufferSize = 1) :flux(inten), color(col), genShadowMap(genSM),genRsm(genRsm), bufferSize(bufferSize) { name = "light"; rsmBuffer = new unsigned[bufferSize];
		frameBuffer = new unsigned[bufferSize];
		rbo = new unsigned[bufferSize];
	}
	
	virtual ~Light(){
		glDeleteBuffers(bufferSize, rsmBuffer);
		glDeleteBuffers(bufferSize, frameBuffer);
		delete[] frameBuffer;
		delete[] rsmBuffer;
		delete[] rbo;
#ifdef _DEBUG
		std::cout << "light lose\n";
#endif // _DEBUG

	}

	virtual void initBuffer(int, int) {}

	virtual void LoadToShader(Shader& s){}
	
	virtual Photon samplePhoton() { return {}; }

	virtual std::vector<glm::mat4> GetLightMat() { return {glm::mat4(1.f)}; }

	bool hasRsm() const { return genShadowMap; }

	uint32_t GetFrameBuffer(unsigned index) const { if (index < bufferSize) return frameBuffer[index]; return 0; }

	unsigned GetRSMBuffer(unsigned index) const { if (index < bufferSize) return rsmBuffer[index]; return 0; }

	unsigned GetRSMWidth() const { return rsmW; }

	unsigned GetRSMHeigth() const { return rsmH; }

	//void SetRSMBuffer(unsigned buffer) { glDeleteBuffers(1,&rsmBuffer); rsmBuffer = buffer; }

	virtual Spectrum GetSpectrum() const {
		return flux * color;
	}

	void SetSpectrum(const Spectrum& s) {
		color = s;
	}

	void SetIntensity(float inte) {
		flux = inte;
	}

	std::shared_ptr<Shader> rsmShader;
	bool isStatic;
};




#endif // !LIGHT_H
