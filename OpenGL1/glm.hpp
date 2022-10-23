
#ifndef GLM_H
#define GLM_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using Vector3d = glm::dvec3;
using Vector2d = glm::dvec2;
using Vector3f = glm::vec3;
using Vector2f = glm::vec2;
using Vector3i = glm::ivec3;
using Vector2i = glm::ivec2;
using Vector4i = glm::ivec4;
using Vector4f = glm::vec4;
#define dotProduct(x,y)  glm::dot(x,y)
#define crossProduct(x,y)  glm::cross(x,y)


inline void setRGB(int x, int y, Vector3f alpha, unsigned resolution, unsigned char* data) {
	data[3 * (resolution * x + y) + 0] = uint8_t(alpha.x);
	data[3 * (resolution * x + y) + 1] = uint8_t(alpha.y);
	data[3 * (resolution * x + y) + 2] = uint8_t(alpha.z);
}

inline Vector3f getRGB(int x, int y, unsigned resolution, unsigned char* data) {
	return Vector3f(data[3 * (resolution * x + y) + 0],
		data[3 * (resolution * x + y) + 1],
		data[3 * (resolution * x + y) + 2]);
}

inline void setRGB(int x, int y, Vector3f alpha, unsigned resolution, char* data) {
	data[3 * (resolution * x + y) + 0] = uint8_t(alpha.x);
	data[3 * (resolution * x + y) + 1] = uint8_t(alpha.y);
	data[3 * (resolution * x + y) + 2] = uint8_t(alpha.z);
}

inline Vector3f getRGB(int x, int y, unsigned resolution, char* data) {
	return Vector3f(data[3 * (resolution * x + y) + 0],
		data[3 * (resolution * x + y) + 1],
		data[3 * (resolution * x + y) + 2]);
}


#endif // !GLM_H
