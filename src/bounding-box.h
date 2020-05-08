#pragma once
#include "glm.hpp"
#include "ray.h"

#include <vector>

//forward declare face
struct Face;


class BoundingBox {
public:
    BoundingBox();
    BoundingBox(const BoundingBox& b2);
    BoundingBox(const glm::vec3 & v0, const glm::vec3 & v1, const glm::vec3 & v2);
    BoundingBox(const std::vector<BoundingBox> & boxes);
    BoundingBox(const std::vector<Face>& faces);
    BoundingBox(const glm::vec3& min, const glm::vec3& max);
    glm::vec3 min();
    glm::vec3 max();

    bool CheckIntersection(const Ray & ray, float & distance);

private:
    glm::vec3 mMaxVert;
    glm::vec3 mMinVert;
};