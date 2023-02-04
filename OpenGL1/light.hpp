#pragma once
#ifndef LIGHT_H
#define LIGHT_H
#include"glm.hpp"
#include"component.hpp"
#include"photon.hpp"
#include"frameBuffer.hpp"

typedef glm::vec3 Spectrum;
#define WHITE Spectrum(1.f,1.f,1.f)

#define RSM_W 5096
#define RSM_H 5096



class Light:public Component {
protected:
	float flux;
	Spectrum color;

	FrameBuffer* frameBuffers;
	bool genShadowMap = false;
	bool genRsm = false;
	int rsmW;
	int rsmH;

public:
	unsigned FrameBufferSize;

	Light(unsigned rsmW,unsigned rsmH,unsigned frameBufferSize):FrameBufferSize(frameBufferSize),flux(0),color(WHITE),rsmW(rsmW),rsmH(rsmH), frameBuffers(new FrameBuffer[frameBufferSize]) {
		name = "light"; 
	}
	
	Light(unsigned rsmW, unsigned rsmH,unsigned frameBufferSize,const Spectrum& col
		, float inten, bool genSM = false,bool genRsm=false) 
		:FrameBufferSize(frameBufferSize), flux(inten), color(col), genShadowMap(genSM),genRsm(genRsm), rsmW(rsmW), rsmH(rsmH), frameBuffers(new FrameBuffer[frameBufferSize]) {
		name = "light";

	}
	
	virtual ~Light(){
#ifdef _DEBUG
		std::cout << "light lose\n";
#endif // _DEBUG

	}

	virtual void initBuffer(int, int) {}

	virtual void LoadToShader(Shader& s){}
	
	virtual Photon samplePhoton() { return {}; }

	virtual std::vector<glm::mat4> GetLightMat() { return {glm::mat4(1.f)}; }

	bool hasRsm() const { return genShadowMap; }

	const FrameBuffer& GetFrameBuffer(unsigned index) const {  return frameBuffers[index]; }

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

	float GetFlux() const {
		return flux;
	}

	std::shared_ptr<Shader> rsmShader;
	bool isStatic;
};




#endif // !LIGHT_H
