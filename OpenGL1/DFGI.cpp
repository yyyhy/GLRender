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

		DFGIFirstRaySparseCS.Use();
		DFGIFirstRaySparseCS.SetInt("GridContainsRays", GridContainsRays);
		DFGIFirstRaySparseCS.SetVec3("GlobalSDFBoxMin", GlobalSDFBoxMin);
		DFGIFirstRaySparseCS.SetVec3("GlobalSDFBoxMax", GlobalSDFBoxMax);
		DFGIFirstRaySparseCS.SetVec3("SceneGridsResolution", SceneGridsResolution);
		DFGIFirstRaySparseCS.SetVec3("LightFlux", LightFlux);
		DFGIFirstRaySparseCS.SetVec3("LightDirection", LightDirection);
		DFGIFirstRaySparseCS.SetVec2("CurrRSMSampleIndex", CurrRSMSampleIndex);
		DFGIFirstRaySparseCS.SetVec2("RSMSampleResolution", RSMSampleResolution);
		DFGIFirstRaySparseCS.SetVec2("RSMSampleBrickSize", RSMSampleBrickSize);
		DFGIFirstRaySparseCS.SetVec2("RSMDownSample", RSMDownSample);

		DFGIFirstRaySparseCS.SetBuffer(0, DFGIRaysBuffer);
		DFGIFirstRaySparseCS.SetBuffer(1, DFGIRayCounterBuffer);
		DFGIFirstRaySparseCS.SetBuffer(2, DFGIWeightBuffer);
		DFGIFirstRaySparseCS.SetTexture("GlobalSDF", gSDF);

		for (int i = 1; i < 3; ++i) {

			DFGIFirstRaySparseCS.SetTexture("RSMAlbedoFlag", RSMAlbedoFlag[i]);
			DFGIFirstRaySparseCS.SetTexture("RSMNormalRoughness", RSMNormalRoughness[i]);
			DFGIFirstRaySparseCS.SetTexture("RSMPositionMetallic", RSMPositionMetallic[i]);
			DFGIFirstRaySparseCS.SetTexture("RSMTangent", RSMTangent[i]);
			DFGIFirstRaySparseCS.Dispath(RSMSampleBrickSize.x/RSMDownSample.x, RSMSampleBrickSize.y/RSMDownSample.y, 1);
		}

		FirstSparseCounter = 0;
	}
	
	//
#ifdef  USE_SH
	DFGIPackSHCS.Use();
	DFGIPackSHCS.SetInt("GridContainsRays", GridContainsRays);
	DFGIPackSHCS.SetVec3("GlobalSDFBoxMin", GlobalSDFBoxMin);
	DFGIPackSHCS.SetVec3("GlobalSDFBoxMax", GlobalSDFBoxMax);
	DFGIPackSHCS.SetVec3("SceneGridsResolution", SceneGridsResolution);
	DFGIPackSHCS.SetBuffer(0, DFGIRaysBuffer);
	DFGIPackSHCS.SetBuffer(1, DFGISHBuffer);
	DFGIPackSHCS.Dispath(32, 32, 32);
#else

#ifndef HIGH_QUANLITY_APPLY
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
#ifdef USE_SH
	DFGISHApplyCS.Use();

	DFGISHApplyCS.SetVec3("GlobalSDFBoxMin", GlobalSDFBoxMin);
	DFGISHApplyCS.SetVec3("GlobalSDFBoxMax", GlobalSDFBoxMax);
	DFGISHApplyCS.SetVec3("SceneGridsResolution", SceneGridsResolution);
	DFGISHApplyCS.SetTexture("gPositionRoughness", gPositionRoughness);
	DFGISHApplyCS.SetTexture("gAlbedoMetallic", gAlbedoMetallic);
	DFGISHApplyCS.SetTexture("gNormalDepth", gNormalDepth);
	DFGISHApplyCS.SetBindingImage(1, DFGIResult);
	DFGISHApplyCS.SetBuffer(2, DFGISHBuffer);
	DFGISHApplyCS.SetBuffer(3, DFGIGridCanSparseBuffer);
	if (MultBounceCounter >= MultBounceFrequency)
		DFGISHApplyCS.SetBool("NeedMarkGridCanMultBounce", true);
	else
		DFGISHApplyCS.SetBool("NeedMarkGridCanMultBounce", false);
	DFGISHApplyCS.Dispath(width / DFGI_DOWN_SAMPLE, height / DFGI_DOWN_SAMPLE, 1);

#else
#ifndef HIGH_QUANLITY_APPLY
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
#ifdef RESULT_STORE_IN_TEX3D
	DFGIApplyCS.SetBuffer(2, DFGIGridCanSparseBuffer);
#else
	DFGIApplyCS.SetBuffer(3, DFGIGridCanSparseBuffer);
#endif
	if (MultBounceCounter >= MultBounceFrequency)
		DFGIApplyCS.SetBool("NeedMarkGridCanMultBounce", true);
	else
		DFGIApplyCS.SetBool("NeedMarkGridCanMultBounce", false);
	DFGIApplyCS.Dispath(width / DFGIIngegrateDownSample, height / DFGIIngegrateDownSample, 1);
#else
	DFGIHighQuanlityApplyCS.Use();
	DFGIHighQuanlityApplyCS.SetInt("GridContainsRays", GridContainsRays);
	DFGIHighQuanlityApplyCS.SetVec3("GlobalSDFBoxMin", GlobalSDFBoxMin);
	DFGIHighQuanlityApplyCS.SetVec3("GlobalSDFBoxMax", GlobalSDFBoxMax);
	DFGIHighQuanlityApplyCS.SetVec3("SceneGridsResolution", SceneGridsResolution);
	DFGIHighQuanlityApplyCS.SetTexture("gPositionRoughness", gPositionRoughness);
	DFGIHighQuanlityApplyCS.SetTexture("gAlbedoMetallic", gAlbedoMetallic);
	DFGIHighQuanlityApplyCS.SetTexture("gNormalDepth", gNormalDepth);
	DFGIHighQuanlityApplyCS.SetBindingImage(1, DFGIResult);
	DFGIHighQuanlityApplyCS.SetBuffer(2, DFGIRaysBuffer);
	DFGIHighQuanlityApplyCS.SetBuffer(3, DFGIGridCanSparseBuffer);
	if (MultBounceCounter >= MultBounceFrequency)
		DFGIHighQuanlityApplyCS.SetBool("NeedMarkGridCanMultBounce", true);
	else
		DFGIHighQuanlityApplyCS.SetBool("NeedMarkGridCanMultBounce", false);
	DFGIHighQuanlityApplyCS.Dispath(width / DFGIIngegrateDownSample, height / DFGIIngegrateDownSample, 1);
#endif // !HIGH_QUANLITY_APPLY

	
#endif // USE_SH

	//
#ifdef MULT_BOUNCE_ON
	if (MultBounceCounter >= MultBounceFrequency) {
		DFGIRayMultBounceCS.Use();
		DFGIRayMultBounceCS.SetInt("GridContainsRays", GridContainsRays);
		DFGIRayMultBounceCS.SetVec3("GlobalSDFBoxMin", GlobalSDFBoxMin);
		DFGIRayMultBounceCS.SetVec3("GlobalSDFBoxMax", GlobalSDFBoxMax);
		DFGIRayMultBounceCS.SetVec3("SceneGridsResolution", SceneGridsResolution);
		DFGIRayMultBounceCS.SetTexture("gPositionRoughness", gPositionRoughness);
		DFGIRayMultBounceCS.SetTexture("gAlbedoMetallic", gAlbedoMetallic);
		DFGIRayMultBounceCS.SetTexture("gNormalDepth", gNormalDepth);
		DFGIRayMultBounceCS.SetTexture("gTangent", gTangent);
		DFGIRayMultBounceCS.SetTexture("GlobalSDF", gSDF);
		DFGIRayMultBounceCS.SetBuffer(1, DFGIRaysBuffer);
		DFGIRayMultBounceCS.SetBuffer(2, DFGIRayCounterBuffer);
		DFGIRayMultBounceCS.SetBuffer(3, DFGIGridCanSparseBuffer);
		DFGIRayMultBounceCS.Dispath(SceneGridsResolution.x, SceneGridsResolution.y, SceneGridsResolution.z);
		MultBounceCounter = 0;
	}
#endif // MULT_BOUNCE_ON

	//
	DFGICompositeResultCS.Use();
	DFGICompositeResultCS.SetTexture("DFGIResult", DFGIResult);
	DFGICompositeResultCS.SetBindingImage(0, GetInTexBuffer());
	DFGICompositeResultCS.Dispath(width, height, 1);

	//
	DFGIRayLiveDownCS.Use();
	DFGIRayLiveDownCS.SetVec3("SceneGridsResolution", glm::vec3(32, 32, 32));
	DFGIRayLiveDownCS.SetBuffer(0, DFGIRaysBuffer);
	//DFGIRayLiveDownCS.Dispath(32, 32, 32);

	
	RSMDownSample = RSMDownSample+glm::vec2{3, 3};
	RSMDownSample.x = int(RSMDownSample.x) % 8;
	RSMDownSample.y = int(RSMDownSample.y) % 8;
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
