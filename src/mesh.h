#pragma once
#include <string>
#include <vector>

#include "intersectable.h"

struct Face {
    unsigned int vert[3];

    Face(unsigned int p0, unsigned int p1, unsigned int p2) {
        vert[0] = p0;
        vert[1] = p1;
        vert[2] = p2;
    }
};

struct Vertex {
    glm::vec4 coord;
    glm::vec3 normal;

    Vertex(glm::vec4 c) {
        coord = c;
        normal = glm::vec4(0, 0, 0, 0);
    }
};

struct Scale {
    float scaleX;
    float scaleY;
    float scaleZ;
};

struct Rotate {
    float rotDegX;
    float rotDegY;
    float rotDegZ;
};



class Mesh : public Intersectable {
public:
    Mesh(glm::vec4 position, Scale scale, Rotate rotate,  std::string filename, float ambient, float specular, float diffuse, float shinyness, Pixel color);

    bool CheckIntersection(glm::vec3 origin, glm::vec3 normRayVector, float& distance, glm::vec3 & normAtIntersection, Pixel& pix) override;

    float getAmbient() override;
    float getSpecular() override;
    float getDiffuse() override;
    float getShinyness() override;


private:
    glm::mat4 translateMat;
    glm::mat4 rotateMat;
    glm::mat4 scaleMat;
    
    std::vector<Face> faces;
    std::vector<Vertex> vertices;

    Pixel mColor;

    float mAmbient;
    float mSpecular;
    float mDiffuse;
    float mShinyness;

};