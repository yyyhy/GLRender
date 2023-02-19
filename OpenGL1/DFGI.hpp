#pragma once


#ifndef DFGI_H
#define DFGI_H
#include"cPostProcess.hpp"
#include"computeShader.hpp"
#include"DFGIRayStruct.hpp"
#include"light.hpp"
#include<random>

const static int DFGIIngegrateDownSample = 2;
const static glm::vec3 SceneGridsResolution = { 32,32,32 };
const static int SceneGrids = SceneGridsResolution.x*SceneGridsResolution.y*SceneGridsResolution.z;
const static int GridContainsRays = 16;

const static glm::vec2 RSMSampleBrickSize = { 512,512 };
const static glm::vec2 RSMSampleResolution = { RSM_W / RSMSampleBrickSize.x / 4, RSM_H / RSMSampleBrickSize.y / 4 };


//#define IMPORTANCE_SAMPLE_RSM
#define RESULT_STORE_IN_TEX3D
#define HIGH_QUANLITY_APPLY
//#define USE_SH
#define MULT_BOUNCE_ON

class DFGI : public PostProcess{
public:
	
	DFGI(int w, int h) :PostProcess(nullptr, nullptr, w, h, 0),
			DFGIFirstRaySparseCS("shaders/cs/dfgi/DFGIFirstRaySparse.comp"),
			DFGIRaysInjectGridCS("shaders/cs/dfgi/DFGIRaysInjectGrid.comp"),
			DFGIRaysBuffer(sizeof(DFGIRay), SceneGrids * GridContainsRays),
			DFGIRayCounterBuffer(sizeof(unsigned), SceneGrids),
			DFGIGridLightInfosBuffer(sizeof(DFGIGridLightInfo), SceneGrids),
			DFGIApplyCS("shaders/cs/dfgi/DFGIApply.comp"),
			DFGIRayMultBounceCS("shaders/cs/dfgi/DFGIRayMultBounce.comp"),
			DFGIGridCanSparseBuffer(sizeof(unsigned), SceneGrids),
			DFGIResult(w/ DFGIIngegrateDownSample,h/ DFGIIngegrateDownSample,GL_RGBA32F,GL_RGBA),
			DFGICompositeResultCS("shaders/cs/dfgi/DFGICompositeResult.comp"),
			DFGIBlurGBufferCS("shaders/cs/dfgi/DFGIBlurGBuffer.comp"),
			BlurGBuffer(w,h,GL_RGBA32F,GL_RGBA),
			DFGIRayLiveDownCS("shaders/cs/dfgi/DFGIRayLiveDown.comp"),
			DFFGIInitBufferCS("shaders/cs/dfgi/DFGIInitBuffer.comp"),
			DFGIWeightBuffer(sizeof(unsigned), 32* 32),
			weight(nullptr),
			DFGIHighQuanlityApplyCS("shaders/cs/dfgi/DFGIHighQuanlityApply.comp"),
			DFGIPackSHCS("shaders/cs/dfgi/DFGIPackSH.comp"),
			DFGISHBuffer(sizeof(DFGISH), SceneGrids),
			DFGISHApplyCS("shaders/cs/dfgi/DFGISHApply.comp")
#ifdef RESULT_STORE_IN_TEX3D
			,DFGIGridFlux(SceneGridsResolution.x, SceneGridsResolution.y, SceneGridsResolution.z, GL_RGBA32F, GL_RGBA),
			DFGIGridDirection(SceneGridsResolution.x, SceneGridsResolution.y, SceneGridsResolution.z, GL_RGBA32F, GL_RGBA),
			DFGIGridPosition(SceneGridsResolution.x, SceneGridsResolution.y, SceneGridsResolution.z, GL_RGBA32F, GL_RGBA)
#endif // RESULT_STORE_IN_TEX3D
{
		DFGIRaysBuffer.SetData(new DFGIRay[SceneGrids * GridContainsRays]);
		DFGIRayCounterBuffer.SetData(new unsigned[SceneGrids] {0});
		DFGIGridLightInfosBuffer.SetData(new DFGIGridLightInfo[SceneGrids]);
		DFGIGridCanSparseBuffer.SetData(new unsigned[SceneGrids] {0});
		DFGIWeightBuffer.SetData(new unsigned[RSMSampleResolution.x * RSMSampleResolution.y] {0});
		DFGISHBuffer.SetData(new DFGISH[SceneGrids]);
		FirstSparseCounter = FirstSparseFrequency;
		MultBounceCounter = MultBounceFrequency;
		RegenerateCounter = RegenerateWeightFrequency;
		std::cout <<"\nRayStructSize" << sizeof(DFGIRay) << "    DFGI init\n";
	}

	void excute() override;

	void SendBufferToNext(PostProcess*)  override;

	void Debug();

	//Init buffer
	ComputeShader DFFGIInitBufferCS;
	ComputeBuffer DFGIWeightBuffer;
	unsigned RegenerateWeightFrequency = 128;
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
	glm::vec3 LightFlux;
	glm::vec3 LightDirection;

	unsigned FirstSparseFrequency = 1;
	unsigned FirstSparseCounter=0;
	glm::vec2 CurrRSMSampleIndex;
	glm::vec2 RSMDownSample = { 8,8 };
	
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

	unsigned MultBounceFrequency = 2;
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
