#include "bounding-box.h"
#include "face.h"
#include "ray.h"
#include "pixel.h"

#include <algorithm>

BoundingBox::BoundingBox()
{
}

BoundingBox::BoundingBox(const BoundingBox& b2)
{
    mMinVert = b2.mMinVert;
    mMaxVert = b2.mMaxVert;
}

BoundingBox::BoundingBox(const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2)
{
    mMaxVert = { std::max(v0.x, std::max(v1.x, v2.x)),
                std::max(v0.y, std::max(v1.y, v2.y)),
                std::max(v0.z, std::max(v1.z, v2.z)) };

    mMinVert = { std::min(v0.x, std::min(v1.x, v2.x)),
                std::min(v0.y, std::min(v1.y, v2.y)),
                std::min(v0.z, std::min(v1.z, v2.z)) };

}

BoundingBox::BoundingBox(const std::vector<BoundingBox>& boxes)
{
    //Iterate over the whole range of boxes and determine the largest and smallest values of all bounding boxes

    mMaxVert = boxes.back().mMaxVert;
    mMinVert = boxes.back().mMinVert;

    for (const BoundingBox& box : boxes) {
        mMaxVert = { std::max(box.mMaxVert.x, mMaxVert.x), std::max(box.mMaxVert.y, mMaxVert.y) , std::max(box.mMaxVert.z, mMaxVert.z) };
        mMinVert = { std::min(box.mMinVert.x, mMinVert.x), std::min(box.mMinVert.y, mMinVert.y),  std::min(box.mMinVert.z, mMinVert.z) };
    }

}

BoundingBox::BoundingBox(const std::vector<Face>& faces)
{
    //Iterate over the whole range of boxes and determine the largest and smallest values of all bounding boxes
    mMaxVert = faces.back().mBBox.mMaxVert;
    mMinVert = faces.back().mBBox.mMinVert;

    for (const Face & face : faces) {
        mMaxVert = { std::max(face.mBBox.mMaxVert.x, mMaxVert.x), std::max(face.mBBox.mMaxVert.y, mMaxVert.y) , std::max(face.mBBox.mMaxVert.z, mMaxVert.z) };
        mMinVert = { std::min(face.mBBox.mMinVert.x, mMinVert.x), std::min(face.mBBox.mMinVert.y, mMinVert.y),  std::min(face.mBBox.mMinVert.z, mMinVert.z) };
    }
}

BoundingBox::BoundingBox(const glm::vec3& min, const glm::vec3& max) :
    mMinVert(min), mMaxVert(max)
{
}

glm::vec3 BoundingBox::min()
{
    return mMinVert;
}

glm::vec3 BoundingBox::max()
{
    return mMaxVert;
}

bool BoundingBox::CheckIntersection(const Ray & ray, float& distance)
{
    //test for box intersection

     //Do for all 3 axes
     //   n Calculate intersection distance t1 and t2
     //   n Maintain tnear and tfar
     //   n If tnear > tfar, box is missed; Done
     //   n If tfar < 0, box is behind; Done
     // If box survived tests, return intersection at tnear
     //  If tnear is negative, return tfar

    float tnear = 0;
    float tfar = 0;

    float t1x = (mMinVert.x - ray.mOrigin.x) / ray.mNormRayVector.x;
    float t2x = (mMaxVert.x - ray.mOrigin.x) / ray.mNormRayVector.x;

    if (t1x > t2x) {
        std::swap<float>(t1x, t2x);
    }

    float t1y = (mMinVert.y - ray.mOrigin.y) / ray.mNormRayVector.y;
    float t2y = (mMaxVert.y - ray.mOrigin.y) / ray.mNormRayVector.y;

    if (t1y > t2y) {
        std::swap<float>(t1y, t2y);
    }

    float t1z = (mMinVert.z - ray.mOrigin.z) / ray.mNormRayVector.z;
    float t2z = (mMaxVert.z - ray.mOrigin.z) / ray.mNormRayVector.z;

    if (t1z > t2z) {
        std::swap<float>(t1z, t2z);
    }


    // start testing 
    if (t1x > t2y || t1y > t2x) {
        return false;
    }

    if (t1y > t1x) {
        tnear = t1y;
    }
    else {
        tnear = t1x;
    }

    if (t2y < t2x) {
        tfar = t2y;
    }
    else {
        tfar = t2x;
    }

    if (tnear > t2z || t1z > tfar) {
        return false;
    }

    if (t1z > tnear) {
        tnear = t1z;
    }

    if (t2z < tfar) {
        tfar = t2z;
    }


    distance = tnear;

    if (tnear < 0) {
        if (tfar < 0) {
            return false;
        }
        distance = tfar;
    }
    return true;
}