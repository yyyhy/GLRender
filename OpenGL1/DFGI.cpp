#include "DFGI.hpp"

void DFGI::excute()
{
	//
	if (RegenerateCounter >= RegenerateWeightFrequency) {
		DFFGIInitBufferCS.Use();
		DFFGIInitBufferCS.SetBuffer(0, DFGIWeightBuffer);
		DFFGIInitBufferCS.SetVec2("RSMSampleResolution", RSMSampleResolution);
		DFFGIInitBufferCS.Dispath(RSMSampleResolution.x, RSMSampleResolution.y, 1);
		RegenerateCounter = 0;
		CurrRSMSampleIndex = { -1,0 };
	}

	//
	if (FirstSparseCounter >= FirstSparseFrequency) {

		if (RegenerateCounter == 0) {
			CurrRSMSampleIndex.x++;
			if (CurrRSMSampleIndex.x >= RSMSampleResolution.x) {
				CurrRSMSampleIndex.y++;
				CurrRSMSampleIndex.x = 0;
				if (CurrRSMSampleIndex.y >= RSMSampleResolution.y) {
#ifdef IMPORTANCE_SAMPLE_RSM
					RegenerateCounter = 1;
					weight = (unsigned*)DFGIWeightBuffer.ReadData();
					unsigned totol = 0;
					weightTable.clear();
					for (int y = 0; y < RSMSampleResolution.y; ++y) {
						for (int x = 0; x < RSMSampleResolution.x; ++x) {
							int index = y * RSMSampleResolution.x + x;
							if (weight[index] != 0) {
								weightTable[{totol, totol + weight[index]}] = glm::vec2(x, y);
								totol += weight[index];
							}
						}
					}
					totolWeight = totol;
					dis = std::uniform_int_distribution<int>(0, totolWeight);
					engine = std::default_random_engine(time(0));
					std::cout << "IS generate: \n ";
#else
					CurrRSMSampleIndex = { 0,0 };
#endif // IMPORTANCE_SAMPLE_RSM

					
				}
			}
		}
		if (RegenerateCounter != 0) {
			int random = dis(engine);
			auto it = weightTable.find(random);
			CurrRSMSampleIndex = it->second;
		}

		DFGIRaySparseCS.Use();
		DFGIRaySparseCS.SetInt("GridContainsRays", GridContainsRays);
		DFGIRaySparseCS.SetVec3("GlobalSDFBoxMin", GlobalSDFBoxMin);
		DFGIRaySparseCS.SetVec3("GlobalSDFBoxMax", GlobalSDFBoxMax);
		DFGIRaySparseCS.SetVec3("SceneGridsResolution", SceneGridsResolution);
		DFGIRaySparseCS.SetVec3("LightFlux", LightFlux);
		DFGIRaySparseCS.SetVec3("LightDirection", LightDirection);
		DFGIRaySparseCS.SetVec2("CurrRSMSampleIndex", CurrRSMSampleIndex);
		DFGIRaySparseCS.SetVec2("RSMSampleResolution", RSMSampleResolution);
		DFGIRaySparseCS.SetVec2("RSMSampleBrickSize", RSMSampleBrickSize);
		DFGIRaySparseCS.SetVec2("RSMDownSample", RSMDownSample);
		DFGIRaySparseCS.SetVec2("RSMSize", glm::vec2(RSM_W/4, RSM_H/4));
		DFGIRaySparseCS.SetBuffer(0, DFGIRaysBuffer);
		DFGIRaySparseCS.SetBuffer(1, DFGIWeightBuffer);
		DFGIRaySparseCS.SetTexture("GlobalSDF", gSDF);

		for (int i = 1; i < 2; ++i) {

			DFGIRaySparseCS.SetTexture("RSMAlbedoFlag", RSMAlbedoFlag[i]);
			DFGIRaySparseCS.SetTexture("RSMNormalRoughness", RSMNormalRoughness[i]);
			DFGIRaySparseCS.SetTexture("RSMPositionMetallic", RSMPositionMetallic[i]);
			DFGIRaySparseCS.SetTexture("RSMTangent", RSMTangent[i]);
			DFGIRaySparseCS.Dispath(RSMSampleBrickSize.x/RSMDownSample.x, RSMSampleBrickSize.y/RSMDownSample.y, 1);
		}

		FirstSparseCounter = 0;
	}
	
	//
#ifdef  PACK_SH
	DFGIPackSHCS.Use();
	DFGIPackSHCS.SetInt("GridContainsRays", GridContainsRays);
	DFGIPackSHCS.SetVec3("GlobalSDFBoxMin", GlobalSDFBoxMin);
	DFGIPackSHCS.SetVec3("GlobalSDFBoxMax", GlobalSDFBoxMax);
	DFGIPackSHCS.SetVec3("SceneGridsResolution", SceneGridsResolution);

	DFGIPackSHCS.SetBuffer(0, DFGIRaysBuffer);
	DFGIPackSHCS.SetBuffer(1, DFGISHBuffer);
	DFGIPackSHCS.Dispath(SceneGridsResolution.x, SceneGridsResolution.y, SceneGridsResolution.z);
#else

#ifndef PER_RAY_INTEGRATE_APPLY
	DFGIRaysInjectGridCS.Use();
	DFGIRaysInjectGridCS.SetInt("GridContainsRays", GridContainsRays);
	DFGIRaysInjectGridCS.SetVec3("GlobalSDFBoxMin", GlobalSDFBoxMin);
	DFGIRaysInjectGridCS.SetVec3("GlobalSDFBoxMax", GlobalSDFBoxMax);
	DFGIRaysInjectGridCS.SetVec3("SceneGridsResolution", SceneGridsResolution);
	DFGIRaysInjectGridCS.SetBuffer(0, DFGIRaysBuffer);
#ifdef RESULT_STORE_IN_TEX3D
	DFGIRaysInjectGridCS.SetBindingImage(1, DFGIGridFlux);
	DFGIRaysInjectGridCS.SetBindingImage(2, DFGIGridDirection);
	DFGIRaysInjectGridCS.SetBindingImage(3, DFGIGridPosition);
#else
	DFGIRaysInjectGridCS.SetBuffer(1, DFGIGridLightInfosBuffer);
#endif 
	DFGIRaysInjectGridCS.Dispath(SceneGridsResolution.x, SceneGridsResolution.y, SceneGridsResolution.z);
#endif // !HIGH_QUANLITY_APPLY

#endif //  USE_SH

	//default quanlity
#ifndef PER_RAY_INTEGRATE_APPLY
	DFGIApplyCS.Use();
	DFGIApplyCS.SetVec3("GlobalSDFBoxMin", GlobalSDFBoxMin);
	DFGIApplyCS.SetVec3("GlobalSDFBoxMax", GlobalSDFBoxMax);
	DFGIApplyCS.SetVec3("SceneGridsResolution", SceneGridsResolution);
	DFGIApplyCS.SetTexture("gPositionRoughness", gPositionRoughness);
	DFGIApplyCS.SetTexture("gAlbedoMetallic", gAlbedoMetallic);
	DFGIApplyCS.SetTexture("gNormalDepth", gNormalDepth);
#ifdef RESULT_STORE_IN_TEX3D
	DFGIApplyCS.SetTexture("DFGIGridFlux", DFGIGridFlux);
	DFGIApplyCS.SetTexture("DFGIGridImportanceDirection", DFGIGridDirection);
	DFGIApplyCS.SetTexture("DFGIGridImportancePosition", DFGIGridPosition);
#else
	DFGIApplyCS.SetBuffer(2, DFGIGridLightInfosBuffer);
#endif
	DFGIApplyCS.SetBindingImage(1, DFGIResult);
	DFGIApplyCS.Dispath(width / DFGIIngegrateDownSample, height / DFGIIngegrateDownSample, 1);
#else
	DFGIPerRayIntegrateApplyCS.Use();
	DFGIPerRayIntegrateApplyCS.SetInt("GridContainsRays", GridContainsRays);
	DFGIPerRayIntegrateApplyCS.SetVec3("GlobalSDFBoxMin", GlobalSDFBoxMin);
	DFGIPerRayIntegrateApplyCS.SetVec3("GlobalSDFBoxMax", GlobalSDFBoxMax);
	DFGIPerRayIntegrateApplyCS.SetVec3("SceneGridsResolution", SceneGridsResolution);
	DFGIPerRayIntegrateApplyCS.SetTexture("gPositionRoughness", gPositionRoughness);
	DFGIPerRayIntegrateApplyCS.SetTexture("gAlbedoMetallic", gAlbedoMetallic);
	DFGIPerRayIntegrateApplyCS.SetTexture("gNormalDepth", gNormalDepth);
	DFGIPerRayIntegrateApplyCS.SetBindingImage(1, DFGIResult);
	DFGIPerRayIntegrateApplyCS.SetBuffer(2, DFGISHBuffer);
	DFGIPerRayIntegrateApplyCS.Dispath(width / DFGIIngegrateDownSample, height / DFGIIngegrateDownSample, 1);
#endif // !HIGH_QUANLITY_APPLY


	//
#ifdef MULT_BOUNCE_ON
	if (MultBounceCounter >= MultBounceFrequency) {
		DFGIRayMultBounceCS.Use();
		DFGIRayMultBounceCS.SetInt("GridContainsRays", GridContainsRays);
		DFGIRayMultBounceCS.SetVec3("GlobalSDFBoxMin", GlobalSDFBoxMin);
		DFGIRayMultBounceCS.SetVec3("GlobalSDFBoxMax", GlobalSDFBoxMax);
		DFGIRayMultBounceCS.SetVec3("SceneGridsResolution", SceneGridsResolution);
		DFGIRayMultBounceCS.SetVec2("MultBounceDownSample", MultBounceDownSample);
		DFGIRayMultBounceCS.SetVec2("Resolution", glm::vec2(width, height));
		DFGIRayMultBounceCS.SetTexture("gPositionRoughness", gPositionRoughness);
		DFGIRayMultBounceCS.SetTexture("gAlbedoMetallic", gAlbedoMetallic);
		DFGIRayMultBounceCS.SetTexture("gNormalDepth", gNormalDepth);
		DFGIRayMultBounceCS.SetTexture("GlobalSDF", gSDF);
		DFGIRayMultBounceCS.SetBuffer(1, DFGIRaysBuffer);
		DFGIRayMultBounceCS.Dispath(width/ MultBounceDownSample.x, height/ MultBounceDownSample.y, 1);
		MultBounceCounter = 0;
	}
#endif // MULT_BOUNCE_ON

	//
	DFGICompositeResultCS.Use();
	DFGICompositeResultCS.SetTexture("DFGIResult", DFGIResult);
	DFGICompositeResultCS.SetBindingImage(0, GetInTexBuffer());
	DFGICompositeResultCS.SetVec2("Resolution", glm::vec2(SCR_WIDTH,SCR_HEIGHT));
	DFGICompositeResultCS.Dispath(width, height, 1);

	//
	DFGIRayLiveDownCS.Use();
	DFGIRayLiveDownCS.SetVec3("SceneGridsResolution", glm::vec3(32, 32, 32));
	DFGIRayLiveDownCS.SetBuffer(0, DFGIRaysBuffer);
	//DFGIRayLiveDownCS.Dispath(32, 32, 32);

	
	/*RSMDownSample = RSMDownSample+glm::vec2{3, 3};
	RSMDownSample.x = std::max(3, int(RSMDownSample.x) % 8);
	RSMDownSample.y = std::max(3, int(RSMDownSample.y) % 8);
	RSMDownSample =  glm::vec2{ 1, 1 };*/
#ifdef MULT_BOUNCE_ON
	MultBounceCounter++;
#endif
	FirstSparseCounter++;
	if (RegenerateCounter != 0)
		RegenerateCounter++;
}

void DFGI::SendBufferToNext(PostProcess* p)
{
	p->SetInTexBuffer(GetInTexBuffer());
}

void DFGI::Debug()
{
	/*DFGIRay* rays = (DFGIRay*)DFGIRaysBuffer.ReadData();
	for (int i = 0; i < 4; ++i) {
		std::cout << rays[i].hitPosition.x << " "<<rays[i].direction.x<<"  ";
	}*/
	/*unsigned* cntrs = (unsigned*)DFGIRayCounterBuffer.ReadData();
	for (int i = 0; i < 4; ++i) {
		std::cout << cntrs[i] << " ";
	}*/

	DFGIGridLightInfo* infos = (DFGIGridLightInfo*)DFGIGridLightInfosBuffer.ReadData();
	for (int i = 0; i < 32 * 32 * 32; ++i) {
		if (infos[i].importanceDirection.x != 0|| infos[i].importanceDirection.y != 0|| infos[i].importanceDirection.z != 0) {
			std::cout << infos[i].importancePosition.x << " " << infos[i].importancePosition.y << " " << infos[i].importancePosition.z << "\n";
			return;
		}
	}
	std::cout << "\n";
}
