#pragma once 
#include "glm.hpp"
#include <string>

class Material {

public:
    glm::vec3 mSpecularColor;
    glm::vec3 mDiffuseColor;
    glm::vec3 mTransmissionColor;
    float mAmbient = 0;
    float mSpecular = 0;
    float mDiffuse = 0;
    float mReflection = 0;
    float mShinyness = 0;
    float mTransmission = 0;
    float mRefractionIndex = 0;
    std::string comment;
};