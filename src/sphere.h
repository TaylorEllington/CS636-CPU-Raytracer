#pragma once
#include "intersectable.h"


class Sphere : public Intersectable {
public:
    Sphere(glm::vec4 position, float radius, Pixel color, Material mat);
    bool CheckIntersection(const Ray & ray,  float& distance, glm::vec3& normAtIntersection, Pixel& pix) override;
    Material getMaterial() override;

private:
    Material mMaterial;
    Pixel mColor;
    glm::vec4 pos;
    float r;
};