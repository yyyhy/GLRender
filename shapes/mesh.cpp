
#ifndef MESH_H
#include"mesh.hpp"
#endif // !mesh_H
#include"component.hpp"
#include"opengl.hpp"
#include<string>
#include<iostream>

bool Triangle::intersect(const Ray& ray) { return true; }

bool Triangle::intersect(const Ray& ray, float& tnear, uint32_t& index) const
{
	return false;
}

Bounds3 Triangle::getBounds() { return Union(Bounds3(v0.Position, v1.Position), v2.Position); }

Intersection Triangle::getIntersection(Ray ray)
{
	Intersection inter;

	if (glm::dot(ray.direction, normal) > 0)
		return inter;
	float u, v, t_tmp = 0;
	Vector3d pvec = glm::cross(ray.direction, e2);
	double det = glm::dot(e1, pvec);
	if (fabs(det) < EPSILON)
		return inter;
	double det_inv = 1. / det;
	Vector3d tvec = ray.origin - Vector3d(v0.Position);
	u = glm::dot(tvec, pvec) * det_inv;
	if (u < 0.f || u > 1.f)
		return inter;
	Vector3d qvec = glm::cross(tvec, e1);
	v = glm::dot(ray.direction, qvec) * det_inv;
	if (v < 0.f || u + v > 1.f)
		return inter;
	t_tmp = glm::dot(e2, qvec) * det_inv;

	inter.coords = ray(t_tmp);
	inter.distance = t_tmp;
	inter.obj = this;
	inter.normal = //(1 - u - v) * v0.Normal + u * v1.Normal + v * v2.Normal;
		this->normal;
	inter.happened = true;
	inter.tcoords = (1 - u - v) * (v0.TexCoords-glm::floor(v0.TexCoords))
					+ u * (v1.TexCoords - glm::floor(v1.TexCoords)) + 
					v * (v2.TexCoords - glm::floor(v2.TexCoords));
	return inter;
}

Vector3d Triangle::evalDiffuseColor(const Vector2d& uv) const
{
	unsigned u = uv.x * albedo->w;
	unsigned v = uv.y * albedo->h;
	Vector3i pos = { 3 * (albedo->w * v + u) ,3 * (albedo->w * v + u)+1 ,3 * (albedo->w * v + u)+2 };
	Vector3d col = { albedo->albedo[pos.x],
					albedo->albedo[pos.y],
					albedo->albedo[pos.z]
						};
	return col / 255.;
}

void MeshTriangle::setupMesh(bool genBVH)
{

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),
		&indices[0], GL_STATIC_DRAW);

	// 顶点位置
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	// 顶点法线
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
	// 顶点纹理坐标
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
	//tangent
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));
	//bitangent
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));
	glBindVertexArray(0);
	if (!genBVH)
		return;
	auto min= Vector3d{ std::numeric_limits<float>::infinity(),
						std::numeric_limits<float>::infinity(),
						std::numeric_limits<float>::infinity() };
	auto max = Vector3d{-std::numeric_limits<float>::infinity(),
						-std::numeric_limits<float>::infinity(),
						-std::numeric_limits<float>::infinity() };
	area = 0;
	for (uint32_t i = 0; i < indices.size() / 3; i++) {
		auto v0 = vertices[indices[i]];
		auto v1 = vertices[indices[i + 1]];
		auto v2 = vertices[indices[i + 2]];
		min = Min(min, Min(v0.Position, v1.Position));
		max = Max(max, Max(v0.Position, v1.Position));
		triangles.emplace_back(v0, v1, v2,&albedo);
		area += triangles[i].area;
	}

	std::vector<Shape*> ptrs;
	for (int i = 0; i < triangles.size(); i++)
		ptrs.push_back(&triangles[i]);

	box = Bounds3(min, max);
	std::cout << "Box:" << box.pMin.x << " " << box.pMin.y << " " << box.pMin.z <<
		", " << box.pMax.x << " " << box.pMax.y << " " << box.pMax.z << "\n";
	bvh = new BVH(ptrs);
}

MeshTriangle::MeshTriangle() {
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
	area = 0;
	bvh = nullptr;
}

MeshTriangle::MeshTriangle(const MeshTriangle& o) :vertices(o.vertices), indices(o.indices), albedo(o.albedo) {
	setupMesh(false);
};

MeshTriangle::MeshTriangle(MeshTriangle&& o) noexcept :vertices(o.vertices), indices(o.indices), albedo(o.albedo) {
	o.clear();
	setupMesh(false);
};

MeshTriangle::~MeshTriangle() {
	clear();
	delete bvh;
#ifdef _DEBUG
	std::cout << VAO << " mesh lose\n";
#endif // _DEBUG

	
}

MeshTriangle& MeshTriangle::operator=(MeshTriangle& o)
{
	indices = o.indices;
	albedo = o.albedo;
	vertices = o.vertices;

	// TODO: 在此处插入 return 语句
	return *this;
}

MeshTriangle& MeshTriangle::operator=(MeshTriangle&& o) noexcept
{
	o.indices.swap(o.indices);
	o.vertices.swap(o.vertices);
	o.albedo=o.albedo;
	o.clear();

	return *this;
}

void MeshTriangle::clear()
{
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
	glDeleteVertexArrays(1, &VAO);
}

void MeshTriangle::draw() const {
	// bind appropriate textures
	if (!forwardShader)
		;
	else
		forwardShader->initTexture();

	// draw mesh
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
	//glBindVertexArray(0);

}

void MeshTriangle::draw(Shader* s) const
{
	s->use();
	s->initTexture();

	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

void MeshTriangle::drawGBuffer() const
{
	if (defferedShader != NULL) {
		defferedShader->initTexture();
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
	}
}


