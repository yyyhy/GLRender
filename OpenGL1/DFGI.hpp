#pragma once


#ifndef DFGI_H
#define DFGI_H
#include"cPostProcess.hpp"
#include"computeShader.hpp"
#include"DFGIRayStruct.hpp"
#include"light.hpp"
#include<random>

const static int DFGI_DOWN_SAMPLE = 2;

#define RESULT_STORE_IN_TEX3D
//#define USE_SH

class DFGI : public PostProcess{
public:
	
	DFGI(int w, int h) :PostProcess(nullptr, nullptr, w, h, 0),
			DFGIFirstRaySparseCS("shaders/cs/dfgi/DFGIFirstRaySparse.comp"),
			DFGIRaysInjectGridCS("shaders/cs/dfgi/DFGIRaysInjectGrid.comp"),
			DFGIRaysBuffer(sizeof(DFGIRay), 32 * 32 * 32 * 16),
			DFGIRayCounterBuffer(sizeof(unsigned), 32 * 32 * 32),
			DFGIGridLightInfosBuffer(sizeof(DFGIGridLightInfo),32*32*32),
			DFGIApplyCS("shaders/cs/dfgi/DFGIApply.comp"),
			sampleOffset(0,0),
			DFGIRayMultBounceCS("shaders/cs/dfgi/DFGIRayMultBounce.comp"),
			DFGIGridCanSparseBuffer(sizeof(unsigned),32*32*32),
			DFGIResult(w/ DFGI_DOWN_SAMPLE,h/ DFGI_DOWN_SAMPLE,GL_RGBA32F,GL_RGBA),
			DFGICompositeResultCS("shaders/cs/dfgi/DFGICompositeResult.comp"),
			DFGIBlurGBufferCS("shaders/cs/dfgi/DFGIBlurGBuffer.comp"),
			BlurGBuffer(w,h,GL_RGBA32F,GL_RGBA),
			DFGIRayLiveDownCS("shaders/cs/dfgi/DFGIRayLiveDown.comp"),
			DFFGIInitBufferCS("shaders/cs/dfgi/DFGIInitBuffer.comp"),
			DFGIWeightBuffer(sizeof(unsigned), 32* 32),
			weight(nullptr),
			DFGIHighQuanlityApplyCS("shaders/cs/dfgi/DFGIHighQuanlityApply.comp"),
			DFGIPackSHCS("shaders/cs/dfgi/DFGIPackSH.comp"),
			DFGISHBuffer(sizeof(DFGISH),32*32*32),
			DFGISHApplyCS("shaders/cs/dfgi/DFGISHApply.comp")
#ifdef RESULT_STORE_IN_TEX3D
			,DFGIGridFlux(32, 32, 32, GL_RGBA32F, GL_RGBA),
			DFGIGridDirection(32, 32, 32, GL_RGBA32F, GL_RGBA),
			DFGIGridPosition(32, 32, 32, GL_RGBA32F, GL_RGBA)
#endif // RESULT_STORE_IN_TEX3D
{
		DFGIRaysBuffer.SetData(new DFGIRay[32 * 32 * 32 * 16]);
		DFGIRayCounterBuffer.SetData(new unsigned[32 * 32 * 32] {0});
		DFGIGridLightInfosBuffer.SetData(new DFGIGridLightInfo[32 * 32 * 32]);
		DFGIGridCanSparseBuffer.SetData(new unsigned[32 * 32 * 32] {0});
		DFGIWeightBuffer.SetData(new unsigned[RSMSampleResolution.x * RSMSampleResolution.y] {0});
		DFGISHBuffer.SetData(new DFGISH[32 * 32 * 32]);
		FirstSparseCounter = FirstSparseFrequency;
		MultBounceCounter = MultBounceFrequency;
		RegenerateCounter = RegenerateWeightFrequency;
		std::cout << "DFGI init\n";
	}

	void excute() override;

	void SendBufferToNext(PostProcess*)  override;

	void Debug();

	//Init buffer
	ComputeShader DFFGIInitBufferCS;
	ComputeBuffer DFGIWeightBuffer;
	unsigned RegenerateWeightFrequency = 256;
	unsigned RegenerateCounter = 0;

	//First ray sparse data
	ComputeShader DFGIFirstRaySparseCS;
	ComputeBuffer DFGIRaysBuffer;
	ComputeBuffer DFGIRayCounterBuffer;
	Texture2D RSMAlbedoFlag[4];
	Texture2D RSMNormalRoughness[4];
	Texture2D RSMPositionMetallic[4];
	Texture2D RSMTangent[4];
	Texture3D gSDF;
	
	glm::vec3 GlobalSDFBoxMin;
	glm::vec3 GlobalSDFBoxMax;
	glm::ivec3 SceneGridsResolution;
	glm::vec3 LightFlux;
	glm::vec2 sampleOffset;
	glm::vec3 LightDirection;

	unsigned FirstSparseFrequency = 1;
	unsigned FirstSparseCounter=0;
	glm::vec2 CurrRSMSampleIndex;
	const glm::vec2 RSMSampleBrickSize = { 80,80 };
	const glm::vec2 RSMSampleResolution = { RSM_W/RSMSampleBrickSize.x/4, RSM_H / RSMSampleBrickSize.y / 4 };
	std::uniform_int_distribution<int> dis;
	std::default_random_engine engine;

	//Compute grid light information
	ComputeShader DFGIRaysInjectGridCS;
	ComputeBuffer DFGIGridLightInfosBuffer;
	unsigned* weight;
	unsigned totolWeight=0;
	std::map<RangePair, glm::vec2> weightTable;

#ifdef RESULT_STORE_IN_TEX3D
	Texture3D DFGIGridFlux;
	Texture3D DFGIGridDirection;
	Texture3D DFGIGridPosition;
#endif

	//SH grid
	ComputeShader DFGIPackSHCS;
	ComputeBuffer DFGISHBuffer;

	//Apply DFGI
	ComputeShader DFGIApplyCS;
	Texture2D gPositionRoughness;
	Texture2D gNormalDepth;
	Texture2D gAlbedoMetallic;
	Texture2D DFGIResult;
	ComputeBuffer DFGIGridCanSparseBuffer;
	
	//Or high quanlity apply
	ComputeShader DFGIHighQuanlityApplyCS;

	//Or sh apply
	ComputeShader DFGISHApplyCS;

	//MultBounce
	ComputeShader DFGIRayMultBounceCS;
	Texture2D gTangent;

	unsigned MultBounceFrequency = 64;
	unsigned MultBounceCounter=0;

	//A sub process
	ComputeShader DFGIBlurGBufferCS;
	Texture2D BlurGBuffer;

	//Composite result
	ComputeShader DFGICompositeResultCS;

	//ray live
	ComputeShader DFGIRayLiveDownCS;

};

#endif // !DFGI_H
