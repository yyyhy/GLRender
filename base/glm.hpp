
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
#define dotProduct(x,y)  glm::dot(x,y)
#define crossProduct(x,y)  glm::cross(x,y)


#endif // !GLM_H
