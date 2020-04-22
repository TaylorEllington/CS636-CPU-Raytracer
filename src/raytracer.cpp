#include "raytracer.h"
#include "screen.h"
#include "intersectable.h"
#include "sphere.h"
#include "mesh.h"
#include <math.h>  //tan, cos
#include <chrono>
#include "gtc/constants.hpp"  //pi half_pi

#define GLM_ENABLE_EXPERIMENTAL
#include "ext.hpp"

#include <iostream>

RayTracer::RayTracer(RayTracerSettings init) :
    settings(init)
{
}

void RayTracer::Run()
{
    Screen screen(settings.mWindowWidth, settings.mWindowHeight, settings.mWindowDebugMode);
    std::vector<Pixel>& img = screen.getImage();

    std::vector<Intersectable*> sceneObjects;

    for (SceneObjDesc desc : settings.meshSceneObjects) {
        if (desc.type == "Sphere") {
            sceneObjects.push_back(new Sphere(desc.position, desc.radius));
            continue;
        }

        if (desc.type == "Mesh") {
            sceneObjects.push_back(new Mesh(desc.position, desc.scale, desc.rotate, desc.filename));
            continue;
        }
        
    }


    glm::vec3 viewSideDirection = glm::cross(glm::vec3(settings.camera.mViewDirection), glm::vec3(settings.camera.mViewUp)); //x_v
    glm::vec3 viewSideUp = glm::cross(viewSideDirection, glm::vec3(settings.camera.mViewDirection)); //y_v

    // normalize the camera space X Y Z
    glm::vec3 viewDirection = glm::normalize(settings.camera.mViewDirection); //z_v
    viewSideDirection = glm::normalize(viewSideDirection);
    viewSideUp = glm::normalize(viewSideUp);

    double HorizontalViewAngleRadians = settings.camera.mDistanceToViewPlane * glm::half_pi<double>();
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

    std::cout << "Raytracer - Starting raytrace of scene"; 
    int progressDelta = (settings.mWindowHeight * settings.mWindowWidth) / 10;
    int progressGoal = progressDelta;
    auto startTime = std::chrono::high_resolution_clock::now();


    for (int vv = 0; vv < settings.mWindowHeight; ++vv) {
        for (int hh = 0; hh < settings.mWindowWidth; ++hh) {

            //index for pixel array
            int index = (vv * settings.mWindowHeight) + hh;

            //Build ray
            //dj,k = (P0,0 + Sj *(j / (hres - 1))* Xv - Sk * (k / (vres - 1)) * Yv) - O;
            double test = ((double)hh / ((double)settings.mWindowWidth - 1));

            double hOffset = imagePlaneWidth * ((double)hh / (double)(settings.mWindowWidth - 1));
            double vOffset = imagePlaneHeight * ((double)vv / (double)(settings.mWindowHeight - 1));

            glm::vec3 imagePlaneCoord = imagePlaneTopLeft + (hOffset * viewSideDirection) - (vOffset * viewSideUp);
            glm::vec3 dForPixel = imagePlaneCoord - glm::vec3(settings.camera.mPosition);
            glm::vec3 dNormForPixel = glm::normalize(dForPixel);

            for (Intersectable* object : sceneObjects) {
                Pixel pix;
                bool intersect = object->CheckIntersection(settings.camera.mPosition, dNormForPixel, pix);

                //This is shortsighted, and does not compare intersection distances, this WILL cause issues
                if (intersect) {
                    img[index] = pix;
                    break;
                }
            }
            if (index >= progressGoal) {
                progressGoal += progressDelta;
                std::cout << " . ";
            }

        }
    } // end rt loops


    auto stopTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stopTime - startTime).count();
    std::cout << "DONE" << std::endl;

    std::cout << "Raytracer - Tracing took " << duration << "ms" << std::endl;

    screen.PrintImage(settings.mOutputFileName);
}

// ugly json parsing methods
void to_json(nlohmann::json& j, const RayTracerSettings& p) {}
void from_json(const nlohmann::json& j, RayTracerSettings& s) {
    j.at("window").at("width").get_to(s.mWindowHeight);
    j.at("window").at("height").get_to(s.mWindowWidth);
    j.at("window").at("debug").get_to(s.mWindowDebugMode);
    j.at("outputFile").get_to(s.mOutputFileName);
    j.at("camera").get_to(s.camera);
    j.at("scene").at("intersectables").get_to(s.meshSceneObjects);
}
void to_json(nlohmann::json& j, const SceneObjDesc& m) {}
void from_json(const nlohmann::json& j, SceneObjDesc& m) {
    // This must always be first because we need to know what types of types will be available to load
    j.at("type").get_to(m.type); 

    // position is type independant
    auto pos = j.at("position");
    m.position = glm::vec4(pos[0], pos[1], pos[2], 1.0); //point

    if (m.type == "Sphere") {
        j.at("radius").get_to(m.radius);
        return;
    }

    if (m.type =="Mesh") {
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