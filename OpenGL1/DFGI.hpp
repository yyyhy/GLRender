#pragma once


#ifndef DFGI_H
#define DFGI_H
#include"cPostProcess.hpp"
#include"computeShader.hpp"
#include"DFGIRayStruct.hpp"
#include"light.hpp"
#include<random>

const static int DFGIIngegrateDownSample = 2;
const static glm::vec3 SceneGridsResolution = { 16,16,16 };
const static int SceneGrids = SceneGridsResolution.x*SceneGridsResolution.y*SceneGridsResolution.z;
const static int GridContainsRays = 25;

const static glm::vec2 RSMSampleBrickSize = { 512,512 };
const static glm::vec2 RSMSampleResolution = { RSM_W / RSMSampleBrickSize.x / 4, RSM_H / RSMSampleBrickSize.y / 4 };
const static glm::vec2 RSMDownSample = { 2,2 };

const static glm::vec3 PackSHGroupSize = { 32,32,32 };

const static glm::vec2 MultBounceDownSample = { 4,4 };
//#define IMPORTANCE_SAMPLE_RSM
//#define RESULT_STORE_IN_TEX3D
#define PER_RAY_INTEGRATE_APPLY
#define PACK_SH
#define MULT_BOUNCE_ON

class DFGI : public PostProcess{
public:
	
	DFGI(int w, int h) :PostProcess(nullptr, nullptr, w, h, 0),
			DFGIRaySparseCS("shaders/cs/dfgi/DFGIRaySparse.comp"),
			DFGIRaysInjectGridCS("shaders/cs/dfgi/DFGIRaysInjectGrid.comp"),
			DFGIRaysBuffer(sizeof(DFGIRay), SceneGrids * GridContainsRays),
			DFGIGridLightInfosBuffer(sizeof(DFGIGridLightInfo), SceneGrids),
			DFGIApplyCS("shaders/cs/dfgi/DFGIApply.comp"),
			DFGIRayMultBounceCS("shaders/cs/dfgi/DFGIMultBounce.comp"),
			DFGIResult(w/ DFGIIngegrateDownSample,h/ DFGIIngegrateDownSample,GL_RGBA32F,GL_RGBA),
			DFGICompositeResultCS("shaders/cs/dfgi/DFGICompositeResult.comp"),
			DFGIRayLiveDownCS("shaders/cs/dfgi/DFGIRayLiveDown.comp"),
			DFFGIInitBufferCS("shaders/cs/dfgi/DFGIInitBuffer.comp"),
			DFGIWeightBuffer(sizeof(unsigned), 32* 32),
			weight(nullptr),
			DFGIPerRayIntegrateApplyCS("shaders/cs/dfgi/DFGIPerRayIntegrateApply.comp"),
			DFGIPackSHCS("shaders/cs/dfgi/DFGIPackSH.comp"),
			SH0(SceneGridsResolution.x, SceneGridsResolution.y, SceneGridsResolution.z, GL_RGBA32F, GL_RGBA),
			SH10(SceneGridsResolution.x, SceneGridsResolution.y, SceneGridsResolution.z, GL_RGBA32F, GL_RGBA),
			SH11(SceneGridsResolution.x, SceneGridsResolution.y, SceneGridsResolution.z, GL_RGBA32F, GL_RGBA),
			SH12(SceneGridsResolution.x, SceneGridsResolution.y, SceneGridsResolution.z, GL_RGBA32F, GL_RGBA)/*,
			GridFluxAtlas(9*32*6,9*32*6,GL_RGB32F,GL_RGB)*/
#ifdef RESULT_STORE_IN_TEX3D
			,DFGIGridFlux(SceneGridsResolution.x, SceneGridsResolution.y, SceneGridsResolution.z, GL_RGBA32F, GL_RGBA),
			DFGIGridDirection(SceneGridsResolution.x, SceneGridsResolution.y, SceneGridsResolution.z, GL_RGBA32F, GL_RGBA),
			DFGIGridPosition(SceneGridsResolution.x, SceneGridsResolution.y, SceneGridsResolution.z, GL_RGBA32F, GL_RGBA)
#endif // RESULT_STORE_IN_TEX3D
{
		DFGIRaysBuffer.SetData(new DFGIRay[SceneGrids * GridContainsRays]);
		DFGIGridLightInfosBuffer.SetData(new DFGIGridLightInfo[SceneGrids]);
		DFGIWeightBuffer.SetData(new unsigned[RSMSampleResolution.x * RSMSampleResolution.y] {0});
		FirstSparseCounter = FirstSparseFrequency;
		MultBounceCounter = MultBounceFrequency;
		RegenerateCounter = RegenerateWeightFrequency;
		std::cout <<"DFGI init\n";
	}

	void Excute() override;

	void SendBufferToNext(PostProcess*)  override;

	void Debug();

	//Init buffer
	ComputeShader DFFGIInitBufferCS;
	ComputeBuffer DFGIWeightBuffer;
	unsigned RegenerateWeightFrequency = 128;
	unsigned RegenerateCounter = 0;

	//First ray sparse data
	ComputeShader DFGIRaySparseCS;
	ComputeBuffer DFGIRaysBuffer;
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
	Texture3D SH0;
	Texture3D SH10;
	Texture3D SH11;
	Texture3D SH12;

	//Apply DFGI
	ComputeShader DFGIApplyCS;
	Texture2D gPositionRoughness;
	Texture2D gNormalDepth;
	Texture2D gAlbedoMetallic;
	Texture2D DFGIResult;
	
	//Or integrate all rays to apply
	ComputeShader DFGIPerRayIntegrateApplyCS;

	//MultBounce
	ComputeShader DFGIRayMultBounceCS;

	unsigned MultBounceFrequency = 1;
	unsigned MultBounceCounter=0;

	//Composite result
	ComputeShader DFGICompositeResultCS;

	//ray live
	ComputeShader DFGIRayLiveDownCS;

};

#endif // !DFGI_H
