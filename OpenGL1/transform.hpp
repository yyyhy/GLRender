#pragma once


#ifndef TRANSFORM_H
#define TRANSFORM_H
#include"component.hpp"
#include"glm.hpp"


class Transform :public Component{
private:
	glm::vec3 position;
	glm::vec3 scale;
	glm::vec3 rotation;
	
#define ZERO_POINT glm::vec3(0.0f)
#define ZERO_SCALE glm::vec3(1.0f)
#define ZERO_ROTATION glm::vec3(0.0f)
#define X_AXIS glm::vec3(1.f,0.f,0.f)
#define Y_AXIS glm::vec3(0.f,1.f,0.f)
#define Z_AXIS glm::vec3(0.f,0.f,1.f)

public:
	explicit Transform(glm::vec3 pos = ZERO_POINT, glm::vec3 sca = ZERO_SCALE, glm::vec3 rot = ZERO_ROTATION, Object* fa = NULL, bool enable = true)
		:Component(fa, enable), position(pos), scale(sca), rotation(rot),lastTransform(glm::mat4(1)) {
		name = "transform";
	}

	~Transform() {
		
	}

	Transform& operator=(Transform& t) {
		position = t.position;
		rotation = t.rotation;
		scale = t.scale;
		object = NULL;
		SetEnable(t.IsEnable());
	}

	Transform(Transform& t) {
		position = t.position;
		rotation = t.rotation;
		scale = t.scale;
		SetEnable(t.IsEnable());
	}

	void Translate(const glm::vec3& dir,float length) {
		position += glm::normalize(dir) * length;
	}

	void Translate(const glm::vec3& trans) { position += trans; }

	void Translate(float x = 1, float y = 1, float z = 1) {
		position[0] += x;
		position[1] += y;
		position[2] += z;
	}

	void SetPosition(const glm::vec3& pos) {
		position = pos;
	}

	void SetPosition(float x = 1, float y = 1, float z = 1) {
		position[0] = x;
		position[1] = y;
		position[2] = z;
	}

	void Rotate(const glm::vec3& rot) {
		rotation += rot;
	}

	void SetRotation(const glm::vec3& rot) {
		rotation = rot;
	}

	void SetRotation(glm::vec3&& rot) {
		rotation = rot;
	}

	void SetRotation(float x=1, float y=1, float z=1) {
		rotation[0] = x;
		rotation[1] = y;
		rotation[2] = z;
	}

	void RotateAround(const glm::vec3& rot, const glm::vec3& o) {
		glm::mat4 mat(1.f);
		mat = glm::translate(mat, -o);
		if (rotation.x != 0.f)
			mat = glm::rotate(mat, rot.x, X_AXIS);
		if (rotation.y != 0.f)
			mat = glm::rotate(mat, rot.y, Y_AXIS);
		if (rotation.z != 0.f)
			mat = glm::rotate(mat, rot.z, Z_AXIS);

		glm::vec4 v = mat * glm::vec4(position, 1.f);

		position.x = v.x / v.w;
		position.y = v.y / v.w;
		position.z = v.z / v.w;
	}

	void SetScale(const glm::vec3& sca) {
		scale = sca;
	}

	void SetScale(float x = 1, float y = 1, float z = 1) {
		scale[0] = x;
		scale[1] = y;
		scale[2] = z;
	}

	const glm::vec3& GetPosition() const { return position; }

	const glm::vec3& GetRotation() const { return rotation; }

	const glm::vec3& GetScale() const { return scale; }

	glm::mat4 GetTransMat() const {
		glm::mat4 mat(1.0f);
		mat = glm::translate(mat, position);
		if (rotation.x != 0.f)
			mat = glm::rotate(mat, rotation.x * glm::pi<float>() / 180.f, X_AXIS);
		if (rotation.y != 0.f)
			mat = glm::rotate(mat, rotation.y * glm::pi<float>() / 180.f, Y_AXIS);
		if (rotation.z != 0.f)
			mat = glm::rotate(mat, rotation.z * glm::pi<float>() / 180.f, Z_AXIS);
		for (int i = 0; i < 3; i++) {
			mat[i][i] *= scale[i];
		}

		return mat;
	}

	glm::mat4 operator()() {
		auto mat = GetTransMat();
		lastTransform = mat;
		return mat;
	}

	void Start() override {
#ifdef _DEBUG_
		
#endif // DEBUG
	}

	void Update() override {
		
	}
	glm::mat4 lastTransform;
};
#endif // !TRANSFORM_H
