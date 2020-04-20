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

RayTracer::RayTracer(RayTracerSettings init):
    settings(init)
{
}



void RayTracer::Run()
{
    Screen screen(settings.mWindowWidth, settings.mWindowHeight, settings.mWindowDebugMode);
    std::vector<Pixel> & img = screen.getImage();

    std::vector<Intersectable *> sceneObjects;


    for (MeshDesc m : settings.meshSceneObjects) {
        sceneObjects.push_back(new Mesh(m.position, m.scale, m.rotate, m.filename));
    }

    for (PrimitiveDesc p : settings.primitiveSceneObjects) {
        sceneObjects.push_back(new Sphere(p.position, p.radius));
    }

   
    glm::vec3 viewSideDirection = glm::cross(glm::vec3(settings.camera.mViewDirection) , glm::vec3(settings.camera.mViewUp)); //x_v
    glm::vec3 viewSideUp = glm::cross( viewSideDirection, glm::vec3(settings.camera.mViewDirection)); //y_v

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
        "\tCamera Space - X:" << glm::to_string(viewSideDirection) << "\n"
        "\tCamera Space - Y:" << glm::to_string(viewSideUp) << "\n"
        "\tCamera Space - Z:" << glm::to_string(viewDirection) << "\n\n" <<


        "\tView Plane center at: " << glm::to_string(centerOfViewPlane) << "\n" <<
        "\tView Plane upper left at: " << glm::to_string(imagePlaneTopLeft) << "\n" << std::endl;

    std::cout << "starting raytrace of scene......" << std::endl;
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

            for (Intersectable * object : sceneObjects) {
                Pixel pix;
                bool intersect = object->CheckIntersection(settings.camera.mPosition, dNormForPixel, pix);

                //This is shortsighted, and does not compare intersection distances, this WILL cause issues
                if (intersect) {
                    img[index] = pix;
                    break;
                }
            }
        }
    } // end rt loops


    auto stopTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stopTime - startTime).count();

    std::cout << "Raytracer - tracing took " << duration << "ms" << std::endl;

    screen.PrintImage(settings.mOutputFileName);
}


