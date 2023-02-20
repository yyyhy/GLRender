#pragma once
#include"texture.hpp"
#include"AccelStructure.hpp"
#include"BVH.hpp"
#include"basicThread.hpp"
#include<iostream>

class GenerateSDFTask : public Task {
private:
	float* data;
	AccelStructrue* acc;
	Vector3f start;
	Vector3f diag;
	Vector3i startIndex;
	Vector3i endIndex;
	Vector3f resolution;
	std::atomic_int& process;
public:
	GenerateSDFTask(float* data,AccelStructrue* acc,Vector3f s,Vector3f d,Vector3i si,Vector3i ei,Vector3f r,std::atomic_int&);

	virtual void DoTask() override;
};


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
		std::atomic_int process=0;
		BasicThreadsPool pool(12);
		Vector3i delta = resolution / 16.f;
		for (unsigned x = 0; x < resolution.x; x+= delta.x) {
			for (unsigned y = 0; y < resolution.y; y+= delta.y) {
				for (unsigned z = 0; z < resolution.z; z+= delta.z) {
					auto task = std::make_shared<GenerateSDFTask>(data, 
										acc, start, diag, Vector3i(x, y, z), 
										Vector3i(x, y , z)+delta, resolution,process);

					pool.AddTask(task);
				}
			}
		}

		pool.Run();
		pool.Join();
		std::cout << "Process: " << process << "/" << resolution.x * resolution.y * resolution.z / delta.x/delta.y/delta.z << "\r";
		auto t3D= Texture3D(data, resolution.x, resolution.y, resolution.z, GL_R16F,GL_RED);
		t3D.box = Bounds3(start, start + diag);
		delete[] data;
		return t3D;
	}

	glm::vec3 resolution;
	glm::vec3 SDFMin;
	glm::vec3 SDFMax;
};