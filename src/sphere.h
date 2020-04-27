#pragma once
#include "intersectable.h"


class Sphere : public Intersectable {
public:
    Sphere(glm::vec4 position, float radius, Pixel color, float ambient, float specular, float diffuse, float shinyness);

    bool CheckIntersection(glm::vec3 origin, glm::vec3 normRayVector, float& distance, glm::vec3& normAtIntersection, Pixel& pix) override;

    float getAmbient() override;
    float getSpecular() override ;
    float getDiffuse() override;
    float getShinyness() override;
    

private:

    float mAmbient;
    float mSpecular;
    float mDiffuse;
    float mShinyness;

    Pixel mColor;
    
    glm::vec4 pos;
    float r;

};