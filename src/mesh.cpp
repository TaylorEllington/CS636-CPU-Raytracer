#include "mesh.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <limits>

#define _USE_MATH_DEFINES // for pi
#include <math.h>

#define GLM_ENABLE_EXPERIMENTAL
#define GLM_SWIZZLE
#include "ext.hpp"

glm::mat4 ScaleAll(float sx, float sy, float sz) {
    glm::mat4 scaleMat = glm::mat4(1.0f);
    scaleMat[0][0] = sx;
    scaleMat[1][1] = sy;
    scaleMat[2][2] = sz;
    return scaleMat;
}

glm::mat4 Translate(float dx, float dy, float dz) {
    glm::mat4 transMat = glm::mat4(1.0f);
    transMat[3][0] = dx;
    transMat[3][1] = dy;
    transMat[3][2] = dz;
    return transMat;
}

glm::mat4 TranslateTo(glm::vec4 origin, glm::vec4 target) {
    glm::vec4 delta = target - origin;
    return Translate(delta.x, delta.y, delta.z);
}

glm::mat4 RotateX(float theta) {
    glm::mat4 rot = glm::mat4(1.0f);
    float thetaRad = (M_PI * theta) / 180;
    rot[1][1] = cos(thetaRad);
    rot[1][2] = sin(thetaRad);
    rot[2][1] = -(sin(thetaRad));
    rot[2][2] = cos(thetaRad);
    return rot;
}

glm::mat4 RotateY(float theta) {
    glm::mat4 rot = glm::mat4(1.0f);
    float thetaRad = (M_PI * theta) / 180;
    rot[0][0] = cos(thetaRad);
    rot[0][2] = -sin(thetaRad);
    rot[2][0] = sin(thetaRad);
    rot[2][2] = cos(thetaRad);
    return rot;
}
glm::mat4 RotateZ(float theta) {
    glm::mat4 rot = glm::mat4(1.0f);
    float thetaRad = (M_PI * theta) / 180;
    rot[0][0] = cos(thetaRad);
    rot[0][1] = sin(thetaRad);
    rot[1][0] = -sin(thetaRad);
    rot[1][1] = cos(thetaRad);
    return rot;
}

glm::mat4 BulidTransform(glm::vec4 pos, Scale scale, Rotate rotate) {

    glm::mat4 s = ScaleAll(scale.scaleX, scale.scaleY, scale.scaleX);
    glm::mat4 r = RotateZ(rotate.rotDegZ) * RotateY(rotate.rotDegY) * RotateX(rotate.rotDegX);
    glm::mat4 t = TranslateTo(glm::vec4(0, 0, 0, 1), pos);
    glm::mat4 transform = t * r * s;

    return transform;

}



Mesh::Mesh(glm::vec4 position, Scale scale, Rotate rotate, std::string filename, float ambient, float specular, float diffuse, float shinyness, Pixel color):
    mAmbient(ambient), mSpecular(specular), mDiffuse(diffuse), mShinyness(shinyness), mColor(color)
{
    translateMat = TranslateTo(glm::vec4(0, 0, 0, 0), position);
    rotateMat = RotateZ(rotate.rotDegZ) * RotateY(rotate.rotDegY) * RotateX(rotate.rotDegX);
    scaleMat = ScaleAll(scale.scaleX, scale.scaleY, scale.scaleZ);
    glm::mat4 transform = translateMat * rotateMat * scaleMat;
    std::fstream file;
    file.open(filename);

    if (!file) {
        throw std::runtime_error("File: " + filename + " does not exist! ");
    }
    vertices.push_back(Vertex({ 0, 0, 0, 1.0 }));
    std::string line;
    while (std::getline(file, line)) {
        float x = 0, y = 0, z = 0;
        int p0 = 0, p1 = 0, p2 = 0;
        char mode;

        std::stringstream str(line);

        str >> mode;
        //dummy vert to maintain 1 -indexing 

        if (mode == 'v') {
            str >> x >> y >> z;
            vertices.push_back(Vertex(transform * glm::vec4( x, y, z, 1.0 ) ));
        }
        else if (mode == 'f') {
            str >> p0 >> p1 >> p2;
            faces.push_back(Face(p0, p1, p2));
        }
    }

    for(Face const f : faces) {
        glm::vec3 p0 = glm::vec3(vertices[f.vert[0]].coord);
        glm::vec3 p1 = glm::vec3(vertices[f.vert[1]].coord);
        glm::vec3 p2 = glm::vec3(vertices[f.vert[2]].coord);

        
        glm::vec3 norm = glm::normalize(glm::cross((p1 - p0), (p2 - p0)));
               
        vertices[f.vert[0]].normal += norm;
        vertices[f.vert[1]].normal += norm;
        vertices[f.vert[2]].normal += norm;

    }

    for (Vertex& v : vertices) {
        v.normal = glm::normalize(v.normal);
    }
    
    std::cout << "Mesh - Loaded mesh: " << filename << " with "<<
        vertices.size()-1 << " verts and " << faces.size() <<" faces" << std::endl;

    file.close();

}




bool Mesh::CheckIntersection(glm::vec3 origin, glm::vec3 normRayVector, float & distance, glm::vec3 & normAtIntersection, Pixel& pix)
{

    glm::mat4 transform = translateMat * rotateMat * scaleMat;

    float dis = std::numeric_limits<float>::max();
    bool foundIntersection = false;

    for (Face f : faces) {

        glm::vec3 vert0 = vertices[f.vert[0]].coord;
        glm::vec3 vert1 = vertices[f.vert[1]].coord;
        glm::vec3 vert2 = vertices[f.vert[2]].coord;

        glm::vec3 norm0 =  vertices[f.vert[0]].normal;
        glm::vec3 norm1 =  vertices[f.vert[1]].normal;
        glm::vec3 norm2 =  vertices[f.vert[2]].normal;


        glm::mat3 matA = { {vert0 - vert1}, {vert0 - vert2}, {normRayVector} };
        float detA = glm::determinant(matA);

        glm::mat3 matBeta = { {vert0 - origin}, {vert0 - vert2}, {normRayVector} };
        glm::mat3 matGamma = { {vert0 - vert1}, {vert0 - origin}, {normRayVector} };
        glm::mat3 matT = { {vert0 - vert1}, {vert0 - vert2}, {vert0 - origin} };

        float detBeta = glm::determinant(matBeta);
        float detGamma = glm::determinant(matGamma);
        float detT = glm::determinant(matT);

        float beta = detBeta / detA;
        float gamma = detGamma / detA;
        float alpha = 1.0 - (beta + gamma);
        float t = detT / detA;

       /* glm::vec3 edge1 = vert1 - vert0;
        glm::vec3 edge2 = vert2 - vert0;

        glm::vec3 perp = glm::cross(origin - vert0, edge1);
        t = glm::dot(edge2, perp) / detA;*/

        //bool test = glm::intersectRayTriangle(origin, normRayVector, vert0, vert1, vert2, bary, dist);
        //if (test) {
        //    glm::vec3 norm = (alpha * norm0) + (beta * norm1) + (gamma + norm2);
        //    pix = { (unsigned char)(255 * norm.x),(unsigned char)(255 * norm.y),(unsigned char)(255 * norm.z) };
 
        //}

        if (beta >= 0 && gamma >= 0 && gamma + beta < 1 && t > 0 && t < dis ) {
            pix = mColor;
            foundIntersection = true;
            dis = t;
            distance = t;
            normAtIntersection = (norm0 * alpha ) + ( norm1 * beta ) + (norm2 * gamma); 
        }
    }


    return foundIntersection;


}

float Mesh::getAmbient() {
    return mAmbient;
}
float Mesh::getSpecular() {
    return mSpecular;
}
float Mesh::getDiffuse() {
    return mDiffuse;
}

float Mesh::getShinyness() {
    return mShinyness;
}

