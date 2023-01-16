#pragma once
#include"texture.hpp"
#include"AccelStructure.hpp"
#include"BVH.hpp"

class SDF {

public:
	SDF(float accuracy = 16) :accuracy(accuracy){}

	Texture3D GenerateSDF(AccelStructrue* acc) {
		Bounds3 box = acc->GetBounds();
		Vector3f start = box.pMin;
		//start = { -19,-3,-9 };
		Vector3f end(19, 17, 9);
		Vector3f diag = box.Diagonal();
		//diag = end - start;
		float* data=new float[accuracy * accuracy * accuracy];
		for (unsigned x = 0; x < accuracy; ++x) {
			for (unsigned y = 0; y < accuracy; ++y) {
				for (unsigned z = 0; z < accuracy; ++z) {
					
					Vector3f pos = start + diag * Vector3f(( x + 0.5 ) / accuracy, (y + 0.5) / accuracy, 
						(z + 0.5) / accuracy);

					float pi = glm::pi<float>();
					unsigned t = 4;
					float sdfValue = 99999;
					for (float the = 0; the < pi; the += pi / t) {
						for (float phi = 0; phi < 2 * pi; phi += 2 * pi / t) {
							Vector3f dir(sin(the) * cos(phi), cos(the), sin(the) * sin(phi));
							Ray r(pos,dir);
							auto inter = acc->Intersect(r);
							sdfValue = sdfValue > inter.distance ? inter.distance : sdfValue;
							/*r = Ray(pos, -dir);
							inter = acc->Intersect(r);*/
							//sdfValue = sdfValue > inter.distance ? inter.distance : sdfValue;
						}
					}
					int p = z * accuracy * accuracy + y * accuracy + x;
					data[p] = sdfValue;
					std::cout << "sdf... " << x * accuracy * accuracy + y * accuracy + z << "/" << accuracy * accuracy * accuracy << "\r";
				}
			}
		}
		auto t3D= Texture3D(data, accuracy, accuracy, accuracy, GL_RED);
		t3D.box = box;
		delete[] data;
		return t3D;
	}

	float accuracy;
};