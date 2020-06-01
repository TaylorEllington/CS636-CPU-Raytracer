#pragma once 
#include "glm.hpp"

class Material {

public:
    glm::vec3 mSpecularColor;
    glm::vec3 mDiffuseColor;
    float mAmbient;
    float mSpecular;
    float mDiffuse;
    float mReflection;
    float mShinyness;
};