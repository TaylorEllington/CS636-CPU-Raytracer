#include "sphere.h"
#define GLM_ENABLE_EXPERIMENTAL
#include "ext.hpp"

#include <iostream>


Sphere::Sphere(glm::vec4 position, float radius, Pixel color, float ambient, float specular, float diffuse, float shinyness):
    pos(position), r(radius), mColor(color), mAmbient(ambient), mSpecular(specular), mDiffuse(diffuse), mShinyness(shinyness)
{
    std::cout << "Sphere - Set up sphere at: [" << pos.x << ", " << pos.y << ", " << pos.z << "] with radius: "<< radius << std::endl;
}

bool Sphere::CheckIntersection(glm::vec3 origin, glm::vec3 normRayVector, float & distance, glm::vec3 & normAtIntersection, Pixel& pix)
{
    float A = glm::dot(normRayVector, normRayVector);
    float B = 2 * glm::dot(normRayVector, origin - glm::vec3(pos));
    float C = glm::dot(origin - glm::vec3(pos), origin - glm::vec3(pos)) - (r * r);

    float discrimiant  = B * B - 4 * A * C;

    float t0; 
    float t1;

    //early out if discriminant shows no good intersection
    if (discrimiant < 0) {
        return false;
    }

    if (discrimiant == 0) {
        //both t values are the same
        t0 = -(B / (2 * A));
        t1 = t0;
    }
    else {
        // different t values
        t0 = (-B) + (sqrt(discrimiant));
        t1 = (-B) - (sqrt(discrimiant));

        t0 = t0 / (2 * A);
        t1 = t1 / (2 * A);
    }

    //make sure t0 is the smaller of the two
    if (t0 > t1) {
        std::swap(t0, t1);
    }

    //early out if both are less than zero
    if (t0 < 0 && t1 < 0) {
        return false;
    }
    distance = t0;
    glm::vec3 intersectionPoint = origin + (normRayVector * t0);
    normAtIntersection = glm::normalize(intersectionPoint - glm::vec3(pos));

    pix = mColor;
    return true;
}


float Sphere::getAmbient() {
    return mAmbient;
}
float Sphere::getSpecular() {
    return mSpecular;
}
float Sphere::getDiffuse() {
    return mDiffuse;
}

float Sphere::getShinyness() {
    return mShinyness;
}
