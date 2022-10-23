

#ifndef INTERSECTION_H
#define INTERSECTION_H
#include"glm.hpp"
class Shape;

struct Intersection
{
    Intersection() {
        happened = false;
        coords = { 0,0,0 };
        normal = { 0,0,0 };
        distance = std::numeric_limits<double>::max();
        obj = nullptr;
    }
    bool happened;
    glm::vec3 coords;
    glm::vec2 tcoords;
    glm::vec3 normal;
    glm::vec3 emit;
    double distance;
    Shape* obj;
    
};
#endif // !INTERSECTION_H
