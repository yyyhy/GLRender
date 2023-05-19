#pragma once


#ifndef SCENE_H
#define SCENE_H
#include"object.hpp"
#include "camera.hpp"
#include"light.hpp"
#include"commons.hpp"
#include"photon_mapping.hpp"
#include"SDF.hpp"

class Render;
class Scene;

class Scene {
private:
	std::vector<std::shared_ptr<Object>> objects;
	std::vector<Light*> lights;
	unsigned skyBuffer;
	Shader* sky;
	BVH* bvh;
	unsigned skyVAO;
	PhotonMapping* photonMappingEngine;
public:

	~Scene() {
		delete sky;
		delete bvh;
	}

	Scene() :skyBuffer(0), bvh(nullptr) {
		sky = new Shader("shaders/sky.vs", "shaders/sky.fs");
		sky->SetInt("skybox", 0);
		glGenVertexArrays(1, &skyVAO);
		glBindVertexArray(skyVAO);
		unsigned VBO;
		glGenBuffers(1, &VBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(boxVertices), &boxVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		photonMappingEngine = PhotonMapping::GetPMInstance();
		//readPhotons();
	}

	void AddObject(std::shared_ptr<Object> o) {
		objects.push_back(std::shared_ptr<Object>(o));
	}

	void AddLight(Light* i) {
		lights.push_back(i);
	}

	int GetObjectsLength() const { return objects.size(); }

	void Update() {
		for (auto& o : objects) {
			if (o) {
				for (auto& c : o->GetComponents())
					c->Update();
			}
		}
	}

	unsigned GetSkyBox() const { return skyBuffer; }

	void SetSkyBox(unsigned box) { glDeleteTextures(1, &skyBuffer); skyBuffer = box; }

	void SetSkyBox(const std::vector<std::string>& faces) {
		glGenTextures(1, &skyBuffer);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skyBuffer);
		int width, height, nrChannels;

		for (int i = 0; i < 6; i++) {
			unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
			if (data)
			{
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
					0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
				);
				stbi_image_free(data);
			}
			else
			{
				std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
				stbi_image_free(data);
			}
		}

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
	}

	friend class Render;

	Intersection intersect(const Ray& ray) const {
		return this->bvh->Intersect(ray);
	}

	void buildBVH() {
		printf("Generate scene BVH...");
		std::vector<Shape*> meshs;
		for (auto& i : objects) {
			if (!i->isStatic)
				continue;
			for (int j = 0; j < i->GetMeshLength(); j++) {
				auto m = i->meshes[j];
				meshs.push_back(m.get());
			}
		}
		this->bvh = new BVH(meshs);
		printf("  ok\n");
	}

	void mappingPhotons(Light* l, int maxPhotons = PHOTON_MOUNT) {
		photonMappingEngine->mappingPhotons(bvh, l, maxPhotons);
	}

	void printPhotons() const {
		photonMappingEngine->print();
	}

	void savePhotons(const std::string& path) const {
		photonMappingEngine->saveFile(path);
	}

	void readPhotons(const std::string& path) {
		photonMappingEngine->readFile(path);
	}

#if _DEBUG
	void genLightMap(Object* o, int index, const std::string& path) {
		if (index == -1) {
			for (unsigned i = 0; i < o->meshes.size(); ++i) {
				photonMappingEngine->genLightMap(o->meshes[i]->vertices, path + std::to_string(i) + ".txt");
				//pm.startGenLightMap(o->meshs[i]->vertices, "./baking/lightMap/bk" + std::to_string(i) + ".txt");
			}

			return;
		}
		if (index >= o->meshes.size())
			return;
		auto m = o->meshes.at(index);
		if (m != NULL)
			photonMappingEngine->genLightMap(m->vertices, path + std::to_string(index) + ".txt");
	}

	void genLightMapIR(Object* o, int index, const std::string& path) {
		if (!bvh)
			buildBVH();
		if (index == -1) {
			for (unsigned i = 0; i < o->meshes.size(); ++i) {
				photonMappingEngine->genLightMap(bvh, o->meshes[i]->vertices, path + std::to_string(i) + ".txt");
			}

			return;
		}
		if (index >= o->meshes.size())
			return;
		auto m = o->meshes.at(index);
		if (m != NULL)
			photonMappingEngine->genLightMap(bvh, m->vertices, path + std::to_string(index) + ".txt");
	}
#endif
	glm::vec3 GetSceneBoundMax() const {
		Bounds3 box = bvh->GetBounds();
		return box.pMax;
	}

	glm::vec3 GetSceneBoundMin() const {
		Bounds3 box = bvh->GetBounds();
		return box.pMin;
	}

	SDF GetGlobalSDFGenerator(glm::vec3 resolution) {
		SDF s(bvh, resolution);
		return s;
	}

	const std::vector<Photon>& getPhotons() const { return photonMappingEngine->getPhotons(); }
};

#endif // SCENE_H
