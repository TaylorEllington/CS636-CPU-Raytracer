#pragma once
#include <string>
#include <vector>
#include <memory>
#include "bounding-box.h"
#include "face.h"
#include "intersectable.h"

struct BVHNode {
    BoundingBox bbox;
    std::vector<Face> faces;
    std::unique_ptr<BVHNode> left;
    std::unique_ptr<BVHNode> right;
};

struct BVHSettings {
    size_t maxDepth;
    size_t triangleThreshold;
};

class Mesh : public Intersectable {
public:
    Mesh(glm::vec4 position, Scale scale, Rotate rotate,  std::string filename, Material mat, BVHSettings settings);

    bool CheckIntersection(const Ray& ray, float& distance, glm::vec3 & normAtIntersection, Pixel& pix) override;
    Material getMaterial() override;

private:
    void BuildBVH(size_t triangleThreshold, size_t maxDepth);
    void RecursiveBuildBVH(size_t triangleThreshold, size_t maxDepth, size_t currentDepth, BVHNode * next);
    bool IntersectCollectionOfFaces(const Ray& ray, float& distance, glm::vec3& normAtIntersection, Pixel& pix, const std::vector<Face> & faces);
    bool CheckIntersectionBVHRec(const Ray& ray, float& distance, glm::vec3& normAtIntersection, Pixel& pix, BVHNode* node);

    
    std::vector<Face> faces;
    std::vector<Vertex> vertices;

    BVHNode headNode;

    Pixel mColor;

    Material mMaterial;
};