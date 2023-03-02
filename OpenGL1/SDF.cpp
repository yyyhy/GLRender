#include "SDF.hpp"

GenerateSDFTask::GenerateSDFTask(float* data, AccelStructrue* acc, Vector3f s, Vector3f d, Vector3i si, Vector3i ei, Vector3f r, std::atomic_int* p)
	:data(data)
	,acc(acc)
	,start(s)
	,diag(d)
	,startIndex(si)
	,endIndex(ei)
	,resolution(r)
	,process(p)
{
}

void GenerateSDFTask::DoTask()
{
	for (unsigned x = startIndex.x; x < endIndex.x; ++x) {
		for (unsigned y = startIndex.y; y < endIndex.y; ++y) {
			for (unsigned z = startIndex.z; z < endIndex.z; ++z) {

				Vector3f pos = start + diag * Vector3f((x + 0.5) / resolution.x, (y + 0.5) / resolution.y,
					(z + 0.5) / resolution.z);

				float pi = glm::pi<float>();
				unsigned t = 16;
				float sdfValue = 99999;
				int inside = -1;
				for (float the = 0; the < pi; the += pi / t) {
					for (float phi = 0; phi < 2 * pi; phi += 2 * pi / t) {
						Vector3f dir(sin(the) * cos(phi), sin(the) * sin(phi), cos(the));
						Ray r(pos, dir);
						auto inter = acc->Intersect(r);
						sdfValue = std::abs(sdfValue) > std::abs(inter.distance) && inter.happened ? std::abs(inter.distance) : std::abs(sdfValue);
						if (!inter.happened)
							inside = 1;

					}
				}
				int p = z * resolution.x * resolution.y + y * resolution.x + x;
				data[p] = sdfValue * inside;
				if (sdfValue == 99999)
					data[p] = 0;
			}
		}
	}

	(*process)++;
	if((*process)%20==0)
		std::cout << "process: " << *process << "/4096\r";
}
