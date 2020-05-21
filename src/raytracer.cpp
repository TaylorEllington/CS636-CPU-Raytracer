#include "raytracer.h"
#include "screen.h"
#include "intersectable.h"
#include "sphere.h"
#include "mesh.h"
#include "ray.h"
#include "material.h"

#include<algorithm>
#include <math.h>  // tan, cos
#include <limits>  // 
#include <chrono> 

#include "gtc/constants.hpp"  //pi half_pi
#include "gtx/scalar_multiplication.hpp"
#include "gtx/hash.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include "ext.hpp"

#include <iostream>

struct pair_hash
{
    template <class T1, class T2>
    std::size_t operator() (const std::pair<T1, T2>& pair) const
    {
        return std::hash<T1>()(pair.first) ^ std::hash<T2>()(pair.second);
    }
};

Ray BuildRay(glm::vec3 origin, glm::vec3 target) {
    glm::vec3 direction = target - origin;
    glm::vec3 normDirection = glm::normalize(direction);

    return { origin, normDirection };
}



bool WithinTolerance(glm::vec3 c1, glm::vec3 c2, float tolerance) {
    return (std::abs(c1.r - c2.r) < tolerance) && (std::abs(c1.g - c2.g) < tolerance) && (std::abs(c1.b - c2.b) < tolerance);
}

bool FragmentInTolerance(glm::vec3 A, glm::vec3 B, glm::vec3 C, glm::vec3 D, float tolerance) {
   return ( WithinTolerance(A, B, tolerance) && WithinTolerance(A, C, tolerance) && WithinTolerance(D, B, tolerance) && WithinTolerance(D, C, tolerance));
}


RayTracer::RayTracer(RayTracerSettings init) :
    settings(init)
{


    for (SceneObjDesc desc : settings.meshSceneObjects) {
        if (desc.type == "Sphere") {
            sceneObjects.push_back(new Sphere(desc.position, desc.radius, desc.color, desc.material));
            continue;
        }

        if (desc.type == "Mesh") {
            sceneObjects.push_back(new Mesh(desc.position, desc.scale, desc.rotate, desc.filename, desc.color, desc.material, desc.settings));
            continue;
        }

    }

}

void RayTracer::PhongShading(glm::vec3& pixel, const Material& mat, const glm::vec3& intersectionPoint, const glm::vec3& intersectNorm, const Camera& camera, const Pixel& pix, const  std::vector<LightDesc>& lights, const float& globalAmbient) {

    //wipe out further back objects or background color
    pixel = glm::vec3(0.0, 0.0, 0.0);

    //put colors into a normalized space
    glm::vec3 matColor = { pix.red / 255.0, pix.green / 255.0, pix.blue / 255.0 };

    //phong shade diffuse and specular per each light
    for (auto& const light : lights) {

        Intersectable* objectPtr = nullptr;
        glm::vec3 intNormal;
        float intDistance;
        Pixel intPix;

        // Shadows
        glm::vec3 shadowCheckPoint = intersectionPoint + (intersectNorm);
        float distToLight = glm::distance(light.position, shadowCheckPoint);
        if (ShootRay(BuildRay(shadowCheckPoint, light.position), sceneObjects, objectPtr, intNormal, intDistance, intPix) && intDistance < distToLight) {
            continue;
        }

        glm::vec3 lightColor = { light.color.red / 255.0, light.color.green / 255.0, light.color.blue / 255.0 };

        glm::vec3 toLight = glm::normalize(light.position - intersectionPoint);
        glm::vec3 toEye = glm::normalize(glm::vec3(camera.mPosition) - intersectionPoint);
        glm::vec3 r = ((2 * glm::dot(toLight, intersectNorm)) * intersectNorm) - toLight;

        float diffuseTheta = std::max(glm::dot(toLight, intersectNorm), 0.0f);
        float specularTheta = std::max(glm::dot(r, toEye), 0.0f);

        float specularThetaToTheN = std::pow(specularTheta, mat.mShinyness);

        float diffuseTermR = std::min(lightColor.r * mat.mDiffuse * matColor.r * diffuseTheta, 1.0f);
        float diffuseTermG = std::min(lightColor.g * mat.mDiffuse * matColor.g * diffuseTheta, 1.0f);
        float diffuseTermB = std::min(lightColor.b * mat.mDiffuse * matColor.b * diffuseTheta, 1.0f);

        float specularTermR = std::min(lightColor.r * mat.mSpecular * matColor.r * specularThetaToTheN, 1.0f);
        float specularTermG = std::min(lightColor.g * mat.mSpecular * matColor.g * specularThetaToTheN, 1.0f);
        float specularTermB = std::min(lightColor.b * mat.mSpecular * matColor.b * specularThetaToTheN, 1.0f);

        pixel.r = pixel.r + diffuseTermR + specularTermR;
        pixel.g = pixel.g + diffuseTermG + specularTermG;
        pixel.b = pixel.b + diffuseTermB + specularTermB;
    }

    float sceneAmbient = globalAmbient * mat.mAmbient;

    pixel.r += matColor.r * sceneAmbient;
    pixel.g += matColor.g * sceneAmbient;
    pixel.b += matColor.b * sceneAmbient;
}

int RayTracer::SubfragmentRecurse(Fragment frag, int A, glm::vec3 imgPlaneA, int B, glm::vec3 imgPlaneB, int C, glm::vec3 imgPlaneC, int D, glm::vec3 imgPlaneD, glm::vec3 origin , std::vector<Intersectable*> sceneObjects, glm::vec3& outColor, bool skipTop, bool skipLeft, bool stop, float tolerance) {
    int shotRays = 0;
    //compute midpoints along edges, and dead center
    glm::vec3 topMid = (imgPlaneA + imgPlaneB) / 2;
    glm::vec3 bottomMid = (imgPlaneC + imgPlaneD) / 2;
    glm::vec3 leftMid = (imgPlaneA + imgPlaneC) / 2;
    glm::vec3 rightMid = (imgPlaneB + imgPlaneD) / 2;

    //compute indexes for midpoints
    int topIndex = (A + B) / 2;
    int bottomIndex = (C + D) / 2;
    int leftIndex = (A + C) / 2;
    int rightIndex = (B + D) / 2;

    glm::vec3 center = (imgPlaneA + imgPlaneB + imgPlaneC + imgPlaneD) / 4;
    int centerIndex = (A + D) / 2;

    if (!skipTop) {
        Ray t = BuildRay(origin, topMid);
        ShootAndShadePrimaryRay(t, sceneObjects, frag.subsamples[topIndex]);
        ++shotRays;
    }

    if (!skipLeft) {
        Ray l = BuildRay(origin, leftMid);
        ShootAndShadePrimaryRay(l, sceneObjects, frag.subsamples[leftIndex]);
        ++shotRays;
    }

    ShootAndShadePrimaryRay(BuildRay(origin, rightMid), sceneObjects, frag.subsamples[rightIndex]);
    ShootAndShadePrimaryRay(BuildRay(origin, bottomMid), sceneObjects, frag.subsamples[bottomIndex]);
    ShootAndShadePrimaryRay(BuildRay(origin, center), sceneObjects, frag.subsamples[centerIndex]);
    shotRays += 3;

    auto s = frag.subsamples;

    glm::vec3 upperLeftColor = (s[A] + s[topIndex] + s[centerIndex] + s[leftIndex]) /4;
    glm::vec3 upperRightColor = (s[B] + s[topIndex] + s[centerIndex] + s[rightIndex])/4;
    glm::vec3 lowerLeftColor = (s[C] + s[bottomIndex] + s[centerIndex] + s[leftIndex])/4;
    glm::vec3 lowerRightColor = (s[D] + s[bottomIndex] + s[centerIndex] + s[rightIndex])/4;

    if (!FragmentInTolerance(s[A], s[topIndex], s[centerIndex], s[leftIndex], tolerance) && !stop) {
        shotRays += SubfragmentRecurse(frag, A, imgPlaneA, topIndex, topMid, leftIndex, leftMid, centerIndex, center, origin, sceneObjects, upperLeftColor, skipTop, skipLeft, true, tolerance);
    }

    if (!FragmentInTolerance(s[B], s[topIndex] , s[centerIndex] , s[rightIndex], tolerance) && !stop) {
        shotRays += SubfragmentRecurse(frag, topIndex, topMid, B, imgPlaneB, centerIndex, center, rightIndex, rightMid, origin, sceneObjects, upperRightColor, skipTop, false, true, tolerance);
    }

    if (!FragmentInTolerance(s[C] , s[bottomIndex] , s[centerIndex] , s[leftIndex], tolerance) && !stop) {
        shotRays += SubfragmentRecurse(frag, leftIndex, leftMid, centerIndex, center, C, imgPlaneC, bottomIndex, bottomMid, origin, sceneObjects, lowerLeftColor, skipTop, skipLeft, true, tolerance);
    }

    if (!FragmentInTolerance(s[D], s[bottomIndex] , s[centerIndex] , s[rightIndex], tolerance) && !stop) {
        shotRays += SubfragmentRecurse(frag, centerIndex, center, rightIndex, rightMid, bottomIndex, bottomMid, D, imgPlaneD, origin, sceneObjects, lowerRightColor, true, true, true, tolerance);
    }

    outColor = (upperLeftColor + upperRightColor + lowerLeftColor + lowerRightColor) / 4;

    return shotRays;
}
void RayTracer::Run()
{
    Screen screen(settings.mWindowWidth, settings.mWindowHeight, settings.mBackgroundColor, settings.mWindowDebugMode);
    Screen heatmap(settings.mWindowWidth, settings.mWindowHeight, settings.mBackgroundColor, settings.mWindowDebugMode);
    std::vector<Pixel>& hmap = heatmap.getImage();
    std::vector<Pixel>& img = screen.getImage();

    float tolerance = 0.05;

    glm::vec3 viewSideDirection = glm::cross(glm::vec3(settings.camera.mViewDirection), glm::vec3(settings.camera.mViewUp)); //x_v
    glm::vec3 viewSideUp = glm::cross(viewSideDirection, glm::vec3(settings.camera.mViewDirection)); //y_v

    // normalize the camera space X Y Z
    glm::vec3 viewDirection = glm::normalize(settings.camera.mViewDirection); //z_v
    viewSideDirection = glm::normalize(viewSideDirection);
    viewSideUp = glm::normalize(viewSideUp);

    double HorizontalViewAngleRadians = settings.camera.mHorizontalViewAngle * (glm::pi<double>() / 180.0);
    double imagePlaneWidth = (2 * settings.camera.mDistanceToViewPlane) * glm::tan(HorizontalViewAngleRadians / 2.0);

    //this could misbehave if int division acts up, watch this
    double imagePlaneHeight = imagePlaneWidth * (settings.mWindowHeight / settings.mWindowWidth);

    glm::vec3 centerOfViewPlane = glm::vec3(settings.camera.mPosition) + glm::vec3(settings.camera.mDistanceToViewPlane * viewDirection);

    glm::vec3 imagePlaneTopLeft = glm::vec3(settings.camera.mPosition) +
        (settings.camera.mDistanceToViewPlane * viewDirection) -
        (imagePlaneWidth / 2) *
        viewSideDirection +
        (imagePlaneHeight / 2) *
        viewSideUp;

    std::cout << "Raytracer - Performing inital setup with: \n" <<
        "\tCamera at " << glm::to_string(settings.camera.mPosition) << "\n" <<
        "\tCamera Space - X: " << glm::to_string(viewSideDirection) << "\n"
        "\tCamera Space - Y: " << glm::to_string(viewSideUp) << "\n"
        "\tCamera Space - Z: " << glm::to_string(viewDirection) << "\n\n" <<

        "\tDist to view plane " << settings.camera.mDistanceToViewPlane << "\n\n" <<
        "\tView angle (DEG) " << settings.camera.mHorizontalViewAngle << "\n\n" <<

        "\tView Plane center at: " << glm::to_string(centerOfViewPlane) << "\n" <<
        "\tView Plane upper left at: " << glm::to_string(imagePlaneTopLeft) << "\n" << std::endl;



    size_t height = settings.mWindowHeight;
    size_t width = settings.mWindowWidth;

    //tracking some data as we work, for progress reporting and pixel data reporting
    int progressDelta = (height * width) / 10;
    int progressGoal = progressDelta;
    size_t primaryRayCounter = 0;
   
    std::vector<glm::vec3> image(height * width);
    glm::vec3 origin = settings.camera.mPosition;
    std::vector<size_t> perPixelRayCount(height * width);
    size_t maxRaysPerPixel = 0;



    //contains the previous bottom row, and current row 's results
    std::unordered_map<std::pair<int, int>, Fragment, pair_hash> fragmentCache;

    std::cout << "Raytracer - Starting raytrace of scene";
    auto startTime = std::chrono::high_resolution_clock::now();


    for (int vv = 0; vv < height; ++vv) {
        for (int hh = 0; hh < width; ++hh) {
            size_t raysThisPixel = 0;

            //index for pixel array
            int index = (vv * height) + hh;


            double hOffset = imagePlaneWidth * ((double)hh / (double)(width - 1));
            double hOffsetNext = imagePlaneWidth * (((double)hh + 1) / (double)(width - 1));
            double vOffset = imagePlaneHeight * ((double)vv / (double)(height - 1));
            double vOffsetNext = imagePlaneHeight * (((double)vv + 1) / (double)(height - 1));


            // shoot rays for the four corners of our pixel
            //   A --- B
            //   |     |
            //   C --- D

            glm::vec3 imagePlaneCoordA = imagePlaneTopLeft + (hOffset * viewSideDirection) - (vOffset * viewSideUp);
            glm::vec3 imagePlaneCoordB = imagePlaneTopLeft + (hOffsetNext * viewSideDirection) - (vOffset * viewSideUp);
            glm::vec3 imagePlaneCoordC = imagePlaneTopLeft + (hOffset * viewSideDirection) - (vOffsetNext * viewSideUp);
            glm::vec3 imagePlaneCoordD = imagePlaneTopLeft + (hOffsetNext * viewSideDirection) - (vOffsetNext * viewSideUp);

            Fragment currFrag;
            bool rayTraceTop = true;
            bool rayTraceLeft = true;

            //look for pixel above, reuse any sub samples on the shared border
            if (fragmentCache.find(std::make_pair(hh, vv - 1)) != fragmentCache.end()) {
                Fragment above = fragmentCache.at(std::make_pair(hh, vv - 1));

                currFrag.subsamples[0] = above.subsamples[20];
                currFrag.subsamples[1] = above.subsamples[21];
                currFrag.subsamples[2] = above.subsamples[22];
                currFrag.subsamples[3] = above.subsamples[23];
                currFrag.subsamples[4] = above.subsamples[24];

                rayTraceTop = false; // we dont need to re-shoot A - B
            }

            //look for pixel to the left, reuse any sub samples on the shared border
            if (fragmentCache.find(std::make_pair(hh - 1, vv)) != fragmentCache.end()) {
                Fragment left = fragmentCache.at(std::make_pair(hh - 1, vv));

                currFrag.subsamples[0] = left.subsamples[4];
                currFrag.subsamples[5] = left.subsamples[9];
                currFrag.subsamples[10] = left.subsamples[14];
                currFrag.subsamples[15] = left.subsamples[19];
                currFrag.subsamples[20] = left.subsamples[24];

                rayTraceLeft = false; // we dont need to re-shoot A - C
            }

            // now shoot the rays needed for the basic fragment
            if (rayTraceLeft && rayTraceTop) {
                // shoot A
                ShootAndShadePrimaryRay(BuildRay(origin, imagePlaneCoordA), sceneObjects, currFrag.subsamples[0]);
                ++raysThisPixel;
            }

            if (rayTraceTop) {
                // shoot B
                ShootAndShadePrimaryRay(BuildRay(origin, imagePlaneCoordB), sceneObjects, currFrag.subsamples[4]);
                ++raysThisPixel;
            }

            if (rayTraceLeft) {
                // shoot C
                ShootAndShadePrimaryRay(BuildRay(origin, imagePlaneCoordC), sceneObjects, currFrag.subsamples[20]);
                ++raysThisPixel;
            }

            // Always shoot D
            ShootAndShadePrimaryRay(BuildRay(origin, imagePlaneCoordD), sceneObjects, currFrag.subsamples[24]);
            ++raysThisPixel;


           
            //check tolerances
            if (!FragmentInTolerance(currFrag.subsamples[0], currFrag.subsamples[4], currFrag.subsamples[20], currFrag.subsamples[24], tolerance)) {
                //recurse into sub fragments
                raysThisPixel += SubfragmentRecurse(currFrag,0,imagePlaneCoordA,4, imagePlaneCoordB,20, imagePlaneCoordC ,24, imagePlaneCoordD, origin,sceneObjects, image[index], rayTraceTop, rayTraceLeft, false, tolerance);
            }
            else {
                image[index] = (currFrag.subsamples[0] + currFrag.subsamples[4] + currFrag.subsamples[20] + currFrag.subsamples[24]) / 4;
            }

            fragmentCache.emplace(std::make_pair(hh, vv), currFrag); // cache this fragment for the next one
            perPixelRayCount[index] = raysThisPixel;
            primaryRayCounter += raysThisPixel;
            maxRaysPerPixel = std::max(maxRaysPerPixel, raysThisPixel);

            // subtle progress reporting 
            if (index >= progressGoal) {
                progressGoal += progressDelta;
                std::cout << ".";
            }
        }
    } // end rt loops

    auto endRTTime = std::chrono::high_resolution_clock::now();
    std::cout << "DONE" << std::endl;


    glm::vec3 maxColor = { 1.0,1.0,1.0 };
    for (int ii = 0; ii < image.size(); ii++) {

        hmap[ii].red = perPixelRayCount[ii] /(float)maxRaysPerPixel * 255.0f;
        hmap[ii].green = (perPixelRayCount[ii]) / (float)maxRaysPerPixel * 255.0f;
        hmap[ii].blue = (perPixelRayCount[ii]) / (float)maxRaysPerPixel * 255.0f;

        image[ii].r = std::min((image[ii].r * 255.0f), 255.0f);
        image[ii].g = std::min((image[ii].g * 255.0f), 255.0f);
        image[ii].b = std::min((image[ii].b * 255.0f), 255.0f);
    }


    //transfer everything to screen for write out
    for (int ii = 0; ii < img.size(); ++ii) {
        img[ii].red = (unsigned char)image[ii].r;
        img[ii].green = (unsigned char)image[ii].g;
        img[ii].blue = (unsigned char)image[ii].b;

    }



    auto stopTime = std::chrono::high_resolution_clock::now();
    auto RTduration = std::chrono::duration_cast<std::chrono::milliseconds>(endRTTime - startTime).count();
    auto imgProccessduration = std::chrono::duration_cast<std::chrono::milliseconds>(stopTime - endRTTime).count();


    std::cout << "Raytracer - Tracing took           " << RTduration << "ms" << std::endl;
    std::cout << "Raytracer - image proccessing took " << imgProccessduration << "ms" << std::endl;
    std::cout << "Raytracer - Total time: " << RTduration + imgProccessduration << "ms" << std::endl;
    std::cout << "Raytracer - Ray Stats: " << std::endl;
    std::cout << "    Primary Rays:       " << primaryRayCounter << std::endl;
    std::cout << "    Max Ray Per Pixel:  " << maxRaysPerPixel << std::endl;


    std::string filename = settings.mOutputFileName;
    heatmap.PrintImage("hm-" + std::to_string(tolerance)+ "-" + filename);
    screen.PrintImage(filename);
}

bool RayTracer::ShootRay(Ray ray, std::vector<Intersectable*> sceneObjects, Intersectable*& intersectedObject, glm::vec3& intersectionNormal, float& intersectionDistance, Pixel& pix)
{
    intersectionDistance = std::numeric_limits<float>::max();
    bool didHit = false;

    Pixel p;
    glm::vec3 norm;
    for (Intersectable* object : sceneObjects) {

        float distToIntersect = std::numeric_limits<float>::max();
        glm::vec3 intersectNorm;

        bool intersect = object->CheckIntersection(ray, distToIntersect, norm, p);
        if (intersect && distToIntersect < intersectionDistance) {
            didHit = true;
            intersectionDistance = distToIntersect;
            intersectedObject = object;
            pix = p;
            intersectionNormal = norm;
        }
    }

    return didHit;
}

bool RayTracer::ShootAndShadePrimaryRay(Ray ray, std::vector<Intersectable*> sceneObjects, glm::vec3& outColor)
{
    Intersectable* obj = nullptr;
    glm::vec3 normal;
    float dist;
    Pixel pix = { 0.0,0.0,0.0 };

    bool didIntersect = ShootRay(ray, sceneObjects, obj, normal, dist, pix);
    if (didIntersect) {
        glm::vec3 intersectionPoint = ray.mOrigin + ray.mNormRayVector * dist;

        PhongShading(outColor, obj->getMaterial(), intersectionPoint, normal, settings.camera, pix, settings.lights, settings.mSceneAmbient);
    }
    return didIntersect;
}

// ugly json parsing methods
void to_json(nlohmann::json& j, const RayTracerSettings& p) {}
void from_json(const nlohmann::json& j, RayTracerSettings& s) {
    j.at("window").at("width").get_to(s.mWindowHeight);
    j.at("window").at("height").get_to(s.mWindowWidth);
    j.at("outputFile").get_to(s.mOutputFileName);
    j.at("camera").get_to(s.camera);
    j.at("scene").at("intersectables").get_to(s.meshSceneObjects);
    j.at("scene").at("lights").get_to(s.lights);
    j.at("scene").at("ambient").get_to(s.mSceneAmbient);
}
void to_json(nlohmann::json& j, const SceneObjDesc& m) {}
void from_json(const nlohmann::json& j, SceneObjDesc& m) {
    // This must always be first because we need to know what types of types will be available to load
    j.at("type").get_to(m.type);

    // position is type independant
    auto pos = j.at("position");
    m.position = glm::vec4(pos[0], pos[1], pos[2], 1.0); //point
    if (j.count("color") != 0) {
        auto pix = j.at("color");
        m.color = { pix[0], pix[1] , pix[2] };
    }

    if (j.count("diffuse") != 0) {
        j.at("diffuse").get_to(m.material.mDiffuse);
    }

    if (j.count("specular") != 0) {
        j.at("specular").get_to(m.material.mSpecular);
    }

    if (j.count("ambient") != 0) {
        j.at("ambient").get_to(m.material.mAmbient);
    }

    j.at("shinyness").get_to(m.material.mShinyness);


    if (m.type == "Sphere") {
        j.at("radius").get_to(m.radius);
        return;
    }

    if (m.type == "Mesh") {
        j.at("BVHmaxDepth").get_to(m.settings.maxDepth);
        j.at("BVHtriThreshold").get_to(m.settings.triangleThreshold);

        j.at("filename").get_to(m.filename);

        auto rot = j.at("rotation");
        m.rotate = { rot[0],rot[1], rot[2] };

        auto scale = j.at("scale");
        m.scale = { scale[0],scale[1], scale[2] };

        return;
    }
}
void to_json(nlohmann::json& j, const PrimitiveDesc& p) {}
void from_json(const nlohmann::json& j, PrimitiveDesc& p) {}
void to_json(nlohmann::json& j, const LightDesc& l) {}
void from_json(const nlohmann::json& j, LightDesc& l) {
    /* glm::vec3 position;
     float intensity;
     Pixel color;*/

    auto pos = j.at("position");
    l.position = glm::vec3(pos[0], pos[1], pos[2]);

    j.at("intensity").get_to(l.intensity);

    auto c = j.at("color");
    l.color = { c[0], c[1], c[2] };
}