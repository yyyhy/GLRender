#pragma once
#include"texture.hpp"
#include"AccelStructure.hpp"
#include"BVH.hpp"

class SDF {

private:
	AccelStructrue* acc;
	Vector3f start;
	Vector3f diag;
public:
	SDF(AccelStructrue* acc, glm::vec3 accuracy) :resolution(accuracy),acc(acc){
		Bounds3 box = acc->GetBounds();
		start = box.pMin;
		start = start - glm::vec3(1, 1, 1);
		diag = box.Diagonal();
		diag = diag + glm::vec3(2, 2, 2);

		SDFMin = start;
		SDFMax = start + diag;
	}

	Texture3D GenerateSDF() {
		float* data=new float[resolution.x * resolution.y * resolution.z];
		for (unsigned x = 0; x < resolution.x; ++x) {
			for (unsigned y = 0; y < resolution.y; ++y) {
				for (unsigned z = 0; z < resolution.z; ++z) {
					
					Vector3f pos = start + diag * Vector3f(( x + 0.5 ) / resolution.x, (y + 0.5) / resolution.y, 
						(z + 0.5) / resolution.z);

					float pi = glm::pi<float>();
					unsigned t = 16;
					float sdfValue = 99999;
					int inside = -1;
					for (float the = 0; the < pi; the += pi / t) {
						for (float phi = 0; phi < 2 * pi; phi += 2 * pi / t) {
							Vector3f dir(sin(the) * cos(phi),  sin(the) * sin(phi), cos(the));
							Ray r(pos,dir);
							auto inter = acc->Intersect(r);
							sdfValue = std::abs(sdfValue) > std::abs(inter.distance) && inter.happened ? std::abs(inter.distance) : std::abs(sdfValue);
							if (!inter.happened)
								inside = 1; 
							
						}
					}
					int p = z * resolution.x * resolution.y + y * resolution.x + x;
					data[p] = sdfValue*inside;
					if (sdfValue == 99999)
						data[p] = 0;
					std::cout << "sdf generate: " << x * resolution.y * resolution.z + y * resolution.x + z << "/" << resolution.x * resolution.y * resolution.z << "\r";
				}
			}
		}
		auto t3D= Texture3D(data, resolution.x, resolution.y, resolution.z, GL_R16F,GL_RED);
		t3D.box = Bounds3(start, start + diag);
		delete[] data;
		
		return t3D;
	}

	glm::vec3 resolution;
	glm::vec3 SDFMin;
	glm::vec3 SDFMax;
};