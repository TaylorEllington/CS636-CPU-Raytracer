#pragma once
#include "intersectable.h"


class Sphere : public Intersectable {
public:
    Sphere(glm::vec4 position, float radius);

    bool CheckIntersection(glm::vec3 origin, glm::vec3 normRayVector, Pixel& pix) override;

private:
    glm::vec4 pos;
    float r;

};