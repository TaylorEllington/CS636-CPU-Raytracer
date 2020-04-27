#pragma once 

#include "pixel.h"
#include "glm.hpp"

class Intersectable {
public:
    virtual bool CheckIntersection(glm::vec3 origin, glm::vec3 normRayVector, float & distance, glm::vec3 & normAtIntersection, Pixel& pix) = 0;

    virtual float getAmbient() = 0;
    virtual float getSpecular() = 0;
    virtual float getDiffuse()= 0;
    virtual float getShinyness() = 0;

};