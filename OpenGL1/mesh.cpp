

#include"mesh.hpp"
#include"component.hpp"
#include"opengl.hpp"
#include<fstream>
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
	Intersection inter = Intersection();

	
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
	if (t_tmp <= 0.001)
		return inter;
	inter.coords = ray(t_tmp);
	inter.distance = t_tmp;
	inter.obj = this;
	inter.normal = (1 - u - v) * v0.Normal + u * v1.Normal + v * v2.Normal;
	//this->normal;
	inter.happened = true;
	inter.tcoords = (1 - u - v) * (v0.TexCoords - glm::floor(v0.TexCoords))
		+ u * (v1.TexCoords - glm::floor(v1.TexCoords)) +
		v * (v2.TexCoords - glm::floor(v2.TexCoords));
	return inter;
}

Vector3d Triangle::evalDiffuseColor(const Vector2d& uv) const
{
	unsigned u = uv.x * albedo->w;
	unsigned v = uv.y * albedo->h;
	Vector3i pos = { 4 * (albedo->w * v + u) ,4 * (albedo->w * v + u) + 1 ,4 * (albedo->w * v + u) + 2 };
	Vector3d col = { albedo->albedo[pos.x],
					albedo->albedo[pos.y],
					albedo->albedo[pos.z]
	};
	return col / 255.;
}

void MeshTriangle::setupMesh(std::vector<Vertex>& vertices, std::vector<unsigned>& indices)
{
	glGenVertexArrays(1, VAO.get());
	glGenBuffers(1, VBO.get());
	glGenBuffers(1, EBO.get());

	glBindVertexArray(*VAO);
	glBindBuffer(GL_ARRAY_BUFFER, *VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *EBO);
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
	//light map

	//---------------

	auto min = Vector3d{ std::numeric_limits<float>::infinity(),
						std::numeric_limits<float>::infinity(),
						std::numeric_limits<float>::infinity() };
	auto max = Vector3d{ -std::numeric_limits<float>::infinity(),
						-std::numeric_limits<float>::infinity(),
						-std::numeric_limits<float>::infinity() };
	area = 0;
	for (uint32_t i = 0; i < indices.size() / 3; i++) {
		auto v0 = vertices[indices[i * 3]];
		auto v1 = vertices[indices[i * 3 + 1]];
		auto v2 = vertices[indices[i * 3 + 2]];
		min = Min(min, Min(v2.Position, Min(v0.Position, v1.Position)));
		max = Max(max, Max(v2.Position, Max(v0.Position, v1.Position)));
		triangles.emplace_back(v0, v1, v2, &albedo);
		area += triangles[i].area;
	}

	box = Bounds3(min, max);

}

MeshTriangle::MeshTriangle() {
	glGenVertexArrays(1, VAO.get());
	glGenBuffers(1, VBO.get());
	glGenBuffers(1, EBO.get());
	area = 0;
	bvh = nullptr;
}

MeshTriangle::~MeshTriangle() {
	delete bvh;
#ifdef _DEBUG
	std::cout << VAO << " mesh lose\n";
#endif // _DEBUG


}

void MeshTriangle::draw() const {
	// bind appropriate textures
	if (!forwardShader)
		return;
	else
		forwardShader->InitTexture();

	// draw mesh
	glBindVertexArray(*VAO);
	glDrawElements(GL_TRIANGLES, indicesCount, GL_UNSIGNED_INT, 0);
	//glBindVertexArray(0);

}

void MeshTriangle::draw(Shader* s) const
{
	s->Use();
	s->InitTexture();

	glBindVertexArray(*VAO);
	glDrawElements(GL_TRIANGLES, indicesCount, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

void MeshTriangle::drawGBuffer() const
{
	if (defferedShader != NULL) {
		defferedShader->InitTexture();
		glBindVertexArray(*VAO);
		glDrawElements(GL_TRIANGLES, indicesCount, GL_UNSIGNED_INT, 0);
	}
}

void MeshTriangle::drawInstance(unsigned size) const
{
	glBindVertexArray(*VAO);
	glDrawElementsInstanced(GL_TRIANGLES, indicesCount, GL_UNSIGNED_INT, 0, size);
}


