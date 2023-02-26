#pragma once

#ifndef MESH_H

#define MESH_H
#include"glm.hpp"
#include<string>
#include"assimp.hpp"
#include"texture.hpp"
#include"component.hpp"
#include<vector>
#include"shader.hpp"
#include"BVH.hpp"
#include"shape.hpp"
#include<random>
#include"kd_tree.hpp"


struct Vertex
{
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec2 TexCoords;
	glm::vec3 Tangent;
	glm::vec3 Bitangent;
	glm::vec3 IndLight;
	glm::vec3 IndLightDir;
};

class Triangle : public Shape
{
public:
	Vertex v0, v1, v2; 
	Vector3d e1, e2;     // 2 edges v1-v0, v2-v0;
	Vector3d normal;
	float area;
	Texture* albedo;
	Triangle(Vertex _v0, Vertex _v1, Vertex _v2,Texture* t)
		: v0(_v0), v1(_v1), v2(_v2),albedo(t)
	{
		e1 = v1.Position - v0.Position;
		e2 = v2.Position - v0.Position;
		normal = glm::normalize(v0.Normal+v1.Normal+v2.Normal);
		area = glm::length(glm::cross(e1, e2)) * 0.5f;
	}

	bool intersect(const Ray& ray) override;

	bool intersect(const Ray& ray, float& tnear,uint32_t& index) const override;

	Intersection getIntersection(Ray ray) override;

	void getSurfaceProperties(const Vector3d& P, const Vector3d& I,
		const uint32_t& index, const Vector2d& uv,
		Vector3d& N, Vector2d& st) const override
	{
		N = normal;
		//        throw std::runtime_error("triangle::getSurfaceProperties not
		//        implemented.");
	}

	Vector3d evalDiffuseColor(const Vector2d&) const override;

	Bounds3 getBounds() override;

	void Sample(Intersection& pos, float& pdf) override {
		std::default_random_engine engine;
		std::uniform_real_distribution<float> dis;
		float x = std::sqrt(dis(engine)), y = dis(engine);
		pos.coords = v0.Position * (1.0f - x) + v1.Position * (x * (1.0f - y)) + v2.Position * (x * y);
		pos.normal = this->normal;
		pdf = 1.0f / area;
	}

	float getArea() override {
		return area;
	}
};

inline bool rayTriangleIntersect(const Vector3d& v0, const Vector3d& v1,
	const Vector3d& v2, const Vector3d& orig,
	const Vector3d& dir, float& tnear, float& u, float& v)
{
	Vector3d edge1 = v1 - v0;
	Vector3d edge2 = v2 - v0;
	Vector3d pvec = glm::cross(dir, edge2);
	float det = glm::dot(edge1, pvec);
	if (det == 0 || det < 0)
		return false;

	Vector3d tvec = orig - v0;
	u = glm::dot(tvec, pvec);
	if (u < 0 || u > det)
		return false;

	Vector3d qvec = glm::cross(tvec, edge1);
	v = glm::dot(dir, qvec);
	if (v < 0 || u + v > det)
		return false;

	float invDet = 1 / det;

	tnear = glm::dot(edge2, qvec) * invDet;
	u *= invDet;
	v *= invDet;

	return true;
}

class MeshTriangle: public Shape {
	
public:
	std::shared_ptr<unsigned> VAO;
	std::shared_ptr<unsigned> VBO;
	std::shared_ptr<unsigned> EBO;
	Texture albedo;
	std::vector<Triangle> triangles;
	unsigned indicesCount=0;
	std::shared_ptr<Shader> forwardShader;
	std::shared_ptr<Shader> defferedShader;
	union
	{
		AccelStructrue* bvh;
		AccelStructrue* kdTree;
	};
	
	Bounds3 box;
	float area;
	void setupMesh(std::vector<Vertex>& vertices, std::vector<unsigned>& indices);
public:
	
	MeshTriangle();

	MeshTriangle(std::vector<Vertex>& vertices, std::vector<unsigned>& indices) :
		VAO(new unsigned()), VBO(new unsigned()), EBO(new unsigned()), indicesCount(indices.size()) {
		setupMesh(vertices, indices);
	};

	~MeshTriangle();

	void SetShader(std::shared_ptr<Shader> s,RenderPass p=RenderPass::Forward) { 
		if (p == Forward)
			forwardShader = (s);
		else if (p == Deffered)
			defferedShader = s;

		albedo = s->GetTexture("albedoMap");
		//stbi_write_png("0.png", 1024, 1024, 3, albedo.albedo, 0);
	}

	const std::shared_ptr<unsigned> GetVAO() const { return VAO; }

	void draw() const;

	void draw(Shader* s) const;

	void drawGBuffer() const;

	void drawInstance(unsigned) const;

	bool intersect(const Ray& ray)  override { return true; }

	bool intersect(const Ray& ray, float& tnear, uint32_t& index) const override
	{
		bool intersect = false;
		for (uint32_t k = 0; k < triangles.size(); ++k) {
			const Vector3d& v0 = triangles[index].v0.Position;
			const Vector3d& v1 = triangles[index].v1.Position;
			const Vector3d& v2 = triangles[index].v2.Position;
			float t, u, v;
			if (rayTriangleIntersect(v0, v1, v2, ray.origin, ray.direction, t,
				u, v) &&
				t < tnear) {
				tnear = t;
				index = k;
				intersect |= true;
			}
		}

		return intersect;
	}

	Bounds3 getBounds()  override { return box; }

	void getSurfaceProperties(const Vector3d& P, const Vector3d& I,
		const uint32_t& index, const Vector2d& uv,
		Vector3d& N, Vector2d& st) const override
	{
		const Vector3d& v0 = triangles[index].v0.Position;
		const Vector3d& v1 = triangles[index].v1.Position;
		const Vector3d& v2 = triangles[index].v2.Position;
		Vector3d e0 = normalize(v1 - v0);
		Vector3d e1 = normalize(v2 - v1);
		N = glm::normalize(glm::cross(e0, e1));
		const Vector2d& st0 = triangles[index].v0.TexCoords;
		const Vector2d& st1 = triangles[index].v1.TexCoords;
		const Vector2d& st2 = triangles[index].v2.TexCoords;
		st = st0 * (1 - uv.x - uv.y) + st1 * uv.x + st2 * uv.y;
	}

	Vector3d evalDiffuseColor(const Vector2d& st) const override
	{
		throw "MeshTriangel is't supported evalDiffuseColor!\n";
		return { 0,0,0 };
	}

	Intersection getIntersection(Ray ray) override
	{
		Intersection intersec;
		if (bvh) {
			intersec = bvh->Intersect(ray);
		}
		if (!bvh) {
			if (kdTree)
				kdTree->Intersect(ray,&intersec);
			else {
				if(*VAO!=0)
					std::cerr << "No Accel-Structure attach to this obj!"<<VAO<<"\n";
			}
		}
		return intersec;
	}

	void Sample(Intersection& pos, float& pdf) override {
		bvh->Sample(pos, pdf);
	}

	float getArea() override {
		return area;
	}

	void LoadLightMapData() {
		/*auto v = vertices.at(0);
		if (v.IndLight.x + v.IndLight.y + v.IndLight.z < 0.1) {
			return;
		}
		
		glBindVertexArray(*VAO);
		glBindBuffer(GL_ARRAY_BUFFER, *VBO);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesCount * sizeof(unsigned int),
			&indices[0], GL_STATIC_DRAW);*/

		glEnableVertexAttribArray(5);
		glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, IndLight));
		glEnableVertexAttribArray(6);
		glVertexAttribPointer(6, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, IndLightDir));
		glBindVertexArray(0);
	}

	void buildKdTree() {
		std::vector<Shape*> ptrs;
		for (int i = 0; i < triangles.size(); i++)
			ptrs.push_back(&triangles[i]);

		kdTree = new KdTree(ptrs);
	}

	void buildBVH() {
		std::vector<Shape*> ptrs;
		for (int i = 0; i < triangles.size(); i++)
			ptrs.push_back(&triangles[i]);

		bvh = new BVH(ptrs);
		/*std::cout << box.pMin.x << " " << box.pMin.y << " " << box.pMin.z << " ; " <<
			box.pMax.x << " " << box.pMax.y << " " << box.pMax.z << "\n";*/
	}

	friend class Scene;
};

#endif // !mesh_H
