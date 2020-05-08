#pragma once 

#include "pixel.h"
#include "glm.hpp"
#include "ray.h"
#include "material.h"

class Intersectable {
public:
    virtual bool CheckIntersection(const Ray& ray, float& distance, glm::vec3& normAtIntersection, Pixel& pix) = 0;
    virtual Material getMaterial() = 0;

};