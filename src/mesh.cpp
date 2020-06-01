#include "mesh.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <limits>
#include <algorithm>

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



Mesh::Mesh(glm::vec4 position, Scale scale, Rotate rotate, std::string filename, Material mat, BVHSettings settings) :
    mMaterial(mat)
{
    glm::mat4 translateMat = TranslateTo(glm::vec4(0, 0, 0, 0), position);
    glm::mat4 rotateMat = RotateZ(rotate.rotDegZ) * RotateY(rotate.rotDegY) * RotateX(rotate.rotDegX);
    glm::mat4 scaleMat = ScaleAll(scale.scaleX, scale.scaleY, scale.scaleZ);
    glm::mat4 transform = translateMat * rotateMat * scaleMat;
    std::fstream file;
    file.open(filename);

    if (!file) {
        throw std::runtime_error("Mesh File: " + filename + " does not exist! ");
    }
    //dummy vert to maintain 1 -indexing 
    vertices.push_back(Vertex({ 0, 0, 0, 1.0 }));
    std::string line;
    while (std::getline(file, line)) {
        float x = 0, y = 0, z = 0;
        int p0 = 0, p1 = 0, p2 = 0;
        char mode;

        std::stringstream str(line);
        
        str >> mode;
        if (mode == 'v') {
            str >> x >> y >> z;
            vertices.push_back(Vertex(transform * glm::vec4(x, y, z, 1.0)));
        }
        else if (mode == 'f') {
            str >> p0 >> p1 >> p2;
            faces.push_back(Face(p0, p1, p2));
        }
    }

    for (Face& f : faces) {
        glm::vec3 p0 = glm::vec3(vertices[f.vert[0]].coord);
        glm::vec3 p1 = glm::vec3(vertices[f.vert[1]].coord);
        glm::vec3 p2 = glm::vec3(vertices[f.vert[2]].coord);

        // calcualte norms, add them to the norms for that vert
        glm::vec3 norm = glm::normalize(glm::cross((p1 - p0), (p2 - p0)));

        vertices[f.vert[0]].normal += norm;
        vertices[f.vert[1]].normal += norm;
        vertices[f.vert[2]].normal += norm;

        //calculate centroid of the triangle
        f.mCentroid = (p0 + p1 + p2) / 3;

        // get per face bounding boxes
        f.mBBox = BoundingBox(p0, p1, p2);
    }

    for (Vertex& v : vertices) {
        // normalize summed norms to get
        v.normal = glm::normalize(v.normal);
    }

    size_t maxDepth = 25;
    size_t triangleThreshold = 5;
    BuildBVH(settings.triangleThreshold, settings.maxDepth);

    glm::vec3 minVert = headNode.bbox.min();
    glm::vec3 maxVert = headNode.bbox.max();

    std::cout << "Mesh - Loaded mesh: " << filename << " with " <<
        vertices.size() - 1 << " verts and " << faces.size() << " faces" << std::endl;
    std::cout << "\t bounded at min [" << minVert.x << ", " << minVert.y << ", " << minVert.z << "] max [" << maxVert.x << " " << maxVert.y << " " << maxVert.z << "]" << std::endl;

    file.close();

}

struct CentroidX
{
    inline bool operator() (const Face& f1, const Face& f2)
    {
        return (f1.mCentroid.x < f2.mCentroid.x);
    }
};
struct CentroidY
{
    inline bool operator() (const Face& f1, const Face& f2)
    {
        return (f1.mCentroid.y < f2.mCentroid.y);
    }
};
struct CentroidZ
{
    inline bool operator() (const Face& f1, const Face& f2)
    {
        return (f1.mCentroid.z < f2.mCentroid.z);
    }
};

void Mesh::BuildBVH(size_t triangleThreshold, size_t maxDepth) {
    
    std::sort(faces.begin(), faces.end(), CentroidX());
    size_t faceDiv = faces.size() / 2;
    headNode.bbox = BoundingBox(faces);

    //distribute the faces among sub nodes
    headNode.left = std::unique_ptr<BVHNode>(new BVHNode);
    headNode.left->faces = std::vector<Face>(faces.begin(), faces.begin() + faceDiv);
    headNode.left->bbox = BoundingBox(headNode.left->faces);

    headNode.right = std::unique_ptr<BVHNode>(new BVHNode);
    headNode.right->faces = std::vector<Face>(faces.begin() + faceDiv, faces.end());
    headNode.right->bbox = BoundingBox(headNode.right->faces);

    RecursiveBuildBVH(triangleThreshold, maxDepth, 1, headNode.left.get());
    RecursiveBuildBVH(triangleThreshold, maxDepth, 1, headNode.right.get());
}

void Mesh::RecursiveBuildBVH(size_t triangleThreshold, size_t maxDepth, size_t currentDepth, BVHNode* nodePtr) {
    if (currentDepth > maxDepth || nodePtr->faces.size() < triangleThreshold) {
        return;
    }

    size_t sortIndex = currentDepth % 3;

    switch (sortIndex)
    {
    case 0:
        std::sort(nodePtr->faces.begin(), nodePtr->faces.end(), CentroidX());
        break;
    case 1:
        std::sort(nodePtr->faces.begin(), nodePtr->faces.end(), CentroidY());
        break;
    case 2:
        std::sort(nodePtr->faces.begin(), nodePtr->faces.end(), CentroidZ());
        break;
    default:
        std::cout << "Mesh - Recursive Build BVH - This should never happen" << std::endl;
        break;
    }

    size_t faceDiv = nodePtr->faces.size() / 2;

    nodePtr->left = std::unique_ptr<BVHNode>(new BVHNode);
    nodePtr->left->faces = std::vector<Face>(nodePtr->faces.begin(), nodePtr->faces.begin() + faceDiv);
    nodePtr->left->bbox = BoundingBox(nodePtr->left->faces);

    nodePtr->right = std::unique_ptr<BVHNode>(new BVHNode);
    nodePtr->right->faces = std::vector<Face>(nodePtr->faces.begin() + faceDiv, nodePtr->faces.end());
    nodePtr->right->bbox = BoundingBox(nodePtr->right->faces);

    RecursiveBuildBVH(triangleThreshold, maxDepth, currentDepth + 1, nodePtr->right.get());
    RecursiveBuildBVH(triangleThreshold, maxDepth, currentDepth + 1, nodePtr->left.get());
}

bool Mesh::IntersectCollectionOfFaces(const Ray& ray, float& distance, glm::vec3& normAtIntersection, Pixel& pix,  std::vector<Face> const & collectionOfFaces) {
    
    float dis = std::numeric_limits<float>::max();
    bool foundIntersection = false;
    for (Face const & f : collectionOfFaces) {

        glm::vec3 vert0 = vertices[f.vert[0]].coord;
        glm::vec3 vert1 = vertices[f.vert[1]].coord;
        glm::vec3 vert2 = vertices[f.vert[2]].coord;

        glm::vec3 norm0 = vertices[f.vert[0]].normal;
        glm::vec3 norm1 = vertices[f.vert[1]].normal;
        glm::vec3 norm2 = vertices[f.vert[2]].normal;


        glm::mat3 matA = { {vert0 - vert1}, {vert0 - vert2}, {ray.mNormRayVector} };
        float detA = glm::determinant(matA);

        glm::mat3 matBeta = { {vert0 - ray.mOrigin}, {vert0 - vert2}, {ray.mNormRayVector} };
        glm::mat3 matGamma = { {vert0 - vert1}, {vert0 - ray.mOrigin}, {ray.mNormRayVector} };
        glm::mat3 matT = { {vert0 - vert1}, {vert0 - vert2}, {vert0 - ray.mOrigin} };

        float detBeta = glm::determinant(matBeta);
        float detGamma = glm::determinant(matGamma);
        float detT = glm::determinant(matT);

        float beta = detBeta / detA;
        float gamma = detGamma / detA;
        float alpha = 1.0 - (beta + gamma);
        float t = detT / detA;

        //do checks, including distance 
        if (beta >= 0 && gamma >= 0 && gamma + beta < 1 && t > 0 && t < dis) {
            pix = mColor;
            foundIntersection = true;
            dis = t;
            distance = t;
            normAtIntersection = (norm0 * alpha) + (norm1 * beta) + (norm2 * gamma);
        }
    }

    return foundIntersection;
}

bool Mesh::CheckIntersectionBVHRec(const Ray& ray, float& distance, glm::vec3& normAtIntersection, Pixel& pix, BVHNode * node) {
    // sanity null check
    if (node == nullptr) {
        return false;
    }

    //does the ray intersect this node or its children?
    if (!node->bbox.CheckIntersection(ray, distance)){
        return false;
    }

    // if there are no child nodes, this is a leaf, run intersection against 
    // the faces 
    if (node->left == nullptr && node->right == nullptr) {
        return IntersectCollectionOfFaces(ray, distance, normAtIntersection, pix, node->faces);
    }

    if (node->left == nullptr || node->right == nullptr) {
        std::cout << "Mesh - CheckIntersectionBVHRec - This should never happen" << std::endl;
    }

    //if there are child nodes, recurse down the tree, "return" smallest distance
    float leftDist = std::numeric_limits<float>::max();
    float rightDist = std::numeric_limits<float>::max();

    glm::vec3 leftNorm;
    glm::vec3 rightNorm;

    Pixel leftPix;
    Pixel rightPix;

    bool intersectLeft = CheckIntersectionBVHRec(ray, leftDist, leftNorm, leftPix, node->left.get());
    bool intersectRight = CheckIntersectionBVHRec(ray, rightDist, rightNorm, rightPix, node->right.get());

    // This is obnoxiously verbose, but when I tried to condense everything down into clever little blocks of code
    // I ended up with false negatives, or inverted geometry where the farther pixel was reported over the nearer one
    // Ill trade elegance for readability and correctness anyday. 
    if (intersectRight && intersectLeft) {
        if (rightDist < leftDist) {
            distance = rightDist;
            normAtIntersection = rightNorm;
            pix = rightPix;;
        }
        else {
            distance = leftDist;
            normAtIntersection = leftNorm;
            pix = leftPix;
        }
        return true;
    }

    if (intersectRight ) {
        distance = rightDist;
        normAtIntersection = rightNorm;
        pix = rightPix;;
        return true;
    }
    
    if (intersectLeft ) {
        distance = leftDist;
        normAtIntersection = leftNorm;
        pix = leftPix;
        return true;
    }

    return false;
}

bool Mesh::CheckIntersection(const Ray & ray, float& distance, glm::vec3& normAtIntersection, Pixel& pix)
{
    bool didHit = CheckIntersectionBVHRec(ray, distance, normAtIntersection, pix, &headNode);
    pix = mColor;
    return didHit;
}

Material Mesh::getMaterial()
{
    return mMaterial;
}




