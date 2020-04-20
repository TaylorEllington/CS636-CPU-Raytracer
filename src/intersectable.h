#pragma once 

#include "pixel.h"
#include "glm.hpp"

class Intersectable {
public:
    virtual bool CheckIntersection(glm::vec3 origin, glm::vec3 normRayVector, Pixel& pix) = 0;
};