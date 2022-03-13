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

struct Vertex
{
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec2 TexCoords;
	glm::vec3 Tangent;
	glm::vec3 Bitangent;
	glm::vec3 IndLight;
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
		normal = glm::normalize(glm::cross(e1, e2));
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
//bool rayTriangleIntersect(const Vector3d& v0, const Vector3d& v1,
//	const Vector3d& v2, const Vector3d& orig,
//	const Vector3d& dir, float& tnear, float& u, float& v)
//{
//	Vector3d edge1 = v1 - v0;
//	Vector3d edge2 = v2 - v0;
//	Vector3d pvec = glm::cross(dir, edge2);
//	float det = glm::dot(edge1, pvec);
//	if (det == 0 || det < 0)
//		return false;
//
//	Vector3d tvec = orig - v0;
//	u = glm::dot(tvec, pvec);
//	if (u < 0 || u > det)
//		return false;
//
//	Vector3d qvec = glm::cross(tvec, edge1);
//	v = glm::dot(dir, qvec);
//	if (v < 0 || u + v > det)
//		return false;
//
//	float invDet = 1 / det;
//
//	tnear = glm::dot(edge2, qvec) * invDet;
//	u *= invDet;
//	v *= invDet;
//
//	return true;
//}

class MeshTriangle: public Shape {
	
public:
	unsigned VAO;
	unsigned VBO;
	unsigned EBO;
	std::vector<Vertex> vertices;
	std::vector<unsigned> indices;
	Texture albedo;
	std::vector<Triangle> triangles;
	unsigned verticesCount=0;
	std::shared_ptr<Shader> forwardShader;
	std::shared_ptr<Shader> defferedShader;
	BVH* bvh;
	Bounds3 box;
	float area;
	void setupMesh(bool genBVH);
public:
	
	MeshTriangle();

	MeshTriangle(const MeshTriangle& o);

	MeshTriangle(MeshTriangle&& o) noexcept;

	MeshTriangle(std::vector<Vertex>& vertices, std::vector<unsigned>& indices,bool genBVH=false)
		:vertices(std::move(vertices)), indices(std::move(indices)){
		setupMesh(genBVH);
	};

	~MeshTriangle();

	void SetShader(std::shared_ptr<Shader> s,RenderPass p=RenderPass::Forward) { 
		if (p == Forward)
			forwardShader = (s);
		else if (p == Deffered)
			defferedShader = s;

		albedo = s->GetTexture("albedoMap");
	}

	const unsigned GetVAO() const { return VAO; }

	MeshTriangle& operator=(MeshTriangle&);

	MeshTriangle& operator=(MeshTriangle&&) noexcept;

	void clear();

	void draw() const;

	void draw(Shader* s) const;

	void drawGBuffer() const;

	bool intersect(const Ray& ray)  override { return true; }

	bool intersect(const Ray& ray, float& tnear, uint32_t& index) const override
	{
		bool intersect = false;
		/*for (uint32_t k = 0; k < indices.size()/3; ++k) {
			const Vector3d& v0 = vertices[indices[k * 3]].Position;
			const Vector3d& v1 = vertices[indices[k * 3 + 1]].Position;
			const Vector3d& v2 = vertices[indices[k * 3 + 2]].Position;
			float t, u, v;
			if (rayTriangleIntersect(v0, v1, v2, ray.origin, ray.direction, t,
				u, v) &&
				t < tnear) {
				tnear = t;
				index = k;
				intersect |= true;
			}
		}*/

		return intersect;
	}

	Bounds3 getBounds()  override { return box; }

	void getSurfaceProperties(const Vector3d& P, const Vector3d& I,
		const uint32_t& index, const Vector2d& uv,
		Vector3d& N, Vector2d& st) const override
	{
		const Vector3d& v0 = vertices[indices[index * 3]].Position;
		const Vector3d& v1 = vertices[indices[index * 3 + 1]].Position;
		const Vector3d& v2 = vertices[indices[index * 3 + 2]].Position;
		Vector3d e0 = normalize(v1 - v0);
		Vector3d e1 = normalize(v2 - v1);
		N = glm::normalize(glm::cross(e0, e1));
		const Vector2d& st0 = vertices[indices[index * 3]].TexCoords;
		const Vector2d& st1 = vertices[indices[index * 3]+1].TexCoords;
		const Vector2d& st2 = vertices[indices[index * 3]+2].TexCoords;
		st = st0 * (1 - uv.x - uv.y) + st1 * uv.x + st2 * uv.y;
	}

	Vector3d evalDiffuseColor(const Vector2d& st) const override
	{
		float scale = 5;
		double pattern =
			(fmodf(st.x * scale, 1) > 0.5) ^ (fmodf(st.y * scale, 1) > 0.5);
		return Vector3d(0.815, 0.235, 0.031)*(1-pattern)+
			Vector3d(0.937, 0.937, 0.231)*pattern;
	}

	Intersection getIntersection(Ray ray) override
	{
		Intersection intersec;
		if (bvh) {
			intersec = bvh->Intersect(ray);
		}
		return intersec;
	}

	void Sample(Intersection& pos, float& pdf) override {
		bvh->Sample(pos, pdf);
	}

	float getArea() override {
		return area;
	}

	friend class Scene;
};

#endif // !mesh_H
