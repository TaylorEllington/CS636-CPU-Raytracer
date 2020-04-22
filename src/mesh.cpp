#include "mesh.h"

#include <fstream>
#include <sstream>
#include <iostream>

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
    glm::mat4 r = RotateZ(rotate.rotDegZ) * RotateY(rotate.rotDegY) * RotateX(rotate.rotDegY);
    glm::mat4 t = TranslateTo(glm::vec4(0, 0, 0, 1), pos);
    glm::mat4 transform = t;

    return transform;

}



Mesh::Mesh(glm::vec4 position, Scale scale, Rotate rotate, std::string filename)
{
    translateMat = TranslateTo(glm::vec4(0, 0, 0, 0), position);
    rotateMat = RotateZ(rotate.rotDegZ) * RotateY(rotate.rotDegY) * RotateX(rotate.rotDegY);
    scaleMat = ScaleAll(scale.scaleX, scale.scaleY, scale.scaleX);

    std::fstream file;
    file.open(filename);

    if (!file) {
        throw std::runtime_error("File: " + filename + " does not exist! ");
    }

    std::string line;
    while (std::getline(file, line)) {
        float x = 0, y = 0, z = 0;
        int p1 = 0, p2 = 0, p3 = 0;
        char mode;

        std::stringstream str(line);

        str >> mode;

        if (mode == 'v') {
            str >> x >> y >> z;
            vertices.push_back(Vertex({ x, y, z, 1.0 }));
        }
        else if (mode == 'f') {
            str >> p1 >> p2 >> p3;
            faces.push_back(Face(p1 - 1, p2 - 1, p3 - 1));
        }
    }
    
    std::cout << "Mesh - Loaded mesh: " << filename << " with "<<
        vertices.size() << " verts and " << faces.size() <<" faces" << std::endl;

    file.close();

}


bool Mesh::CheckIntersection(glm::vec3 origin, glm::vec3 normRayVector, Pixel& pix)
{

    glm::mat4 transform = translateMat * rotateMat * scaleMat;

    for (Face f : faces) {


        glm::vec3 vert0 = transform * vertices[f.vert[0]].coord;
        glm::vec3 vert1 = transform * vertices[f.vert[1]].coord;
        glm::vec3 vert2 = transform * vertices[f.vert[2]].coord;
        //std::cout << "x: " << glm::to_string(vertices[f.vert[0]].coord)
        //    << " y: " << glm::to_string(vertices[f.vert[1]].coord)
        //    << " z: " << glm::to_string(vertices[f.vert[2]].coord) << std::endl;


        ////std::cout << "x: " <<glm::to_string(vert0) 
        //<< " y: " << glm::to_string(vert1)
        //    << " z: " << glm::to_string(vert2) << std::endl;

        glm::mat3 matA = { {vert0 - vert1}, {vert0 - vert2}, {normRayVector} };
        float detA = glm::determinant(matA);

        glm::mat3 matBeta = { {vert0 - origin}, {vert0 - vert2}, {normRayVector} };
        glm::mat3 matGamma = { {vert0 - vert1}, {vert0 - origin}, {normRayVector} };
        glm::mat3 matT = { {vert0 - vert1}, {vert0 - vert1}, {vert0 - origin} };

        float detBeta = glm::determinant(matBeta);
        float detGamma = glm::determinant(matGamma);
        float detT = glm::determinant(matT);

        float beta = detBeta / detA;
        float gamma = detGamma / detA;
        float t = detT / detA;

        if (beta >= 0 && gamma >= 0 && gamma + beta < 1 && t >= 0) {
            pix = { 0,0,0 };
            return true;
        }
    }
    return false;


}
