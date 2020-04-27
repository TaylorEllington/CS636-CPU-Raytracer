#include "raytracer.h"
#include "screen.h"
#include "intersectable.h"
#include "sphere.h"
#include "mesh.h"
#include<algorithm>
#include <math.h>  // tan, cos
#include <limits>  // 
#include <chrono>   
#include "gtc/constants.hpp"  //pi half_pi
#include "gtx/scalar_multiplication.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include "ext.hpp"

#include <iostream>

RayTracer::RayTracer(RayTracerSettings init) :
    settings(init)
{
}

void RayTracer::Run()
{
    Screen screen(settings.mWindowWidth , settings.mWindowHeight , settings.mBackgroundColor, settings.mWindowDebugMode);
    std::vector<Pixel>& img = screen.getImage();

    std::vector<Intersectable*> sceneObjects;

    for (SceneObjDesc desc : settings.meshSceneObjects) {
        if (desc.type == "Sphere") {
            sceneObjects.push_back(new Sphere(desc.position, desc.radius, desc.color, desc.mAmbient, desc.mSpecular, desc.mDiffuse, desc.mShinyness));
            continue;
        }

        if (desc.type == "Mesh") {
            sceneObjects.push_back(new Mesh(desc.position, desc.scale, desc.rotate, desc.filename, desc.mAmbient, desc.mSpecular, desc.mDiffuse, desc.mShinyness, desc.color));
            continue;
        }
        
    }


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

   
    // Change size of window for supersampling
    size_t height = settings.mWindowHeight;
    size_t width = settings.mWindowWidth;
    if(settings.mSupersample){
        height = height * 2;
        width = width * 2;
        std::cout << "Raytracer - Supersampling enabled, computations will be done on an " << height << " x " << width << "image" << std::endl;
    }

    std::cout << "Raytracer - Starting raytrace of scene";

    int progressDelta = (height * width) / 10;
    int progressGoal = progressDelta;
    auto startTime = std::chrono::high_resolution_clock::now();

    std::vector<glm::vec3> image(height * width);
    for (int vv = 0; vv < height; ++vv) {
        for (int hh = 0; hh < width; ++hh) {

            //index for pixel array
            int index = (vv * height) + hh;

            //Build ray
            //dj,k = (P0,0 + Sj *(j / (hres - 1))* Xv - Sk * (k / (vres - 1)) * Yv) - O;

            double hOffset = imagePlaneWidth * ((double)hh / (double)(width - 1));
            double vOffset = imagePlaneHeight * ((double)vv / (double)(height - 1));

            glm::vec3 imagePlaneCoord = imagePlaneTopLeft + (hOffset * viewSideDirection) - (vOffset * viewSideUp);
            glm::vec3 dForPixel = imagePlaneCoord - glm::vec3(settings.camera.mPosition);
            glm::vec3 dNormForPixel = glm::normalize(dForPixel);

            float nearestDist = std::numeric_limits<float>::max();


            //per object intersections - TODO: accelerate
            for (Intersectable* object : sceneObjects) {
                Pixel pix;
                float distToIntersect = 0;
                glm::vec3 intersectNorm;
                bool intersect = object->CheckIntersection(settings.camera.mPosition, dNormForPixel, distToIntersect, intersectNorm ,pix);

                glm::vec3 intersectionPoint = glm::vec3(settings.camera.mPosition) + dNormForPixel * distToIntersect;



                //shading happens here
                if (intersect && distToIntersect < nearestDist) {
                    nearestDist = distToIntersect;

                    //wipe out further back objects or background color
                    image[index] = glm::vec3(0.0, 0.0, 0.0);

                    //put colors into a normalized space
                    glm::vec3 matColor = { pix.red / 255.0, pix.green / 255.0, pix.blue / 255.0 };

                    //phong shade diffuse and specular per each light
                    for (auto light : settings.lights) {

                        glm::vec3 lightColor = { light.color.red / 255.0, light.color.green / 255.0, light.color.blue / 255.0 };

                       
                        glm::vec3 toLight = glm::normalize(light.position - intersectionPoint);
                        glm::vec3 toEye = glm::normalize( glm::vec3(settings.camera.mPosition)- intersectionPoint);
                        glm::vec3 r = ((2 * glm::dot(toLight, intersectNorm)) * intersectNorm) - toLight;

                        float diffuseTheta = std::max(glm::dot(toLight, intersectNorm), 0.0f);
                        float specularTheta = std::max(glm::dot(r, toEye), 0.0f);

                        float shinyness = object->getShinyness();

                        float specularThetaToTheN = std::pow(specularTheta, shinyness);

                        float diffuseTermR = std::min(lightColor.r * object->getDiffuse() * matColor.r * diffuseTheta, 1.0f) ;
                        float diffuseTermG = std::min(lightColor.g * object->getDiffuse() * matColor.g * diffuseTheta, 1.0f);
                        float diffuseTermB = std::min(lightColor.b * object->getDiffuse() * matColor.b * diffuseTheta, 1.0f);

                        float specularTermR = std::min(lightColor.r * object->getSpecular() * matColor.r * specularThetaToTheN, 1.0f);
                        float specularTermG = std::min(lightColor.g * object->getSpecular() * matColor.g * specularThetaToTheN, 1.0f);
                        float specularTermB = std::min(lightColor.b * object->getSpecular() * matColor.b * specularThetaToTheN, 1.0f);


                        image[index].r = image[index].r + diffuseTermR + specularTermR;
                        image[index].g = image[index].g + diffuseTermG + specularTermG;
                        image[index].b = image[index].b + diffuseTermB + specularTermB;
                    }

                    float sceneAmbient = 0.01 * object->getAmbient();  //TODO CONFIG ME

                    //ok future me, i need to sleep heres the plan:
                    // 4) get tri-intersect working

                    image[index].r += matColor.r * sceneAmbient;
                    image[index].g += matColor.g * sceneAmbient;
                    image[index].b += matColor.b * sceneAmbient;
                }
            }
            // subtle progress reporting 
            if (index >= progressGoal) {
                progressGoal += progressDelta;
                std::cout << ".";
            }
        }
    } // end rt loops

    auto endRTTime = std::chrono::high_resolution_clock::now();

    //full screen pass to normalize RGB colors
    std::cout << "Raytracer - full screen color norm pass" << std::endl;
    glm::vec3 maxColor = { 0.0,0.0,0.0 };
    for (auto pix : image) {
        if (pix.r > maxColor.r) {
            maxColor.r = pix.r;
        }

        if (pix.g > maxColor.g) {
            maxColor.g = pix.g;
        }

        if (pix.b > maxColor.b) {
            maxColor.b = pix.b;
        }
    }

        for (int ii = 0; ii < image.size(); ii++) {
            image[ii].r = ((image[ii].r * 255.0f) / maxColor.r);
            image[ii].g = ((image[ii].g * 255.0f) / maxColor.g);
            image[ii].b = ((image[ii].b * 255.0f) / maxColor.b);
        }
    std::vector<glm::vec3> finalImage;
    if (!settings.mSupersample) {
        finalImage = image;

    }
    else {
                
        size_t ssHeight = settings.mWindowHeight;
        size_t ssWidth = settings.mWindowWidth;
        std::cout << "Raytracer - Supersampling image to" << ssHeight << " x " << ssWidth << std::endl;
        finalImage.resize(ssHeight* ssWidth);

        for (int vv = 0; vv < ssHeight; ++vv) {
            for (int hh = 0; hh < ssWidth; ++hh) {
                size_t dsIndex = (vv * ssHeight) + hh;
                size_t usIndex = ((vv * 2.0) * height) + (hh * 2.0);

                glm::vec3 aggregate = image[usIndex] + image[usIndex + 1] + image[usIndex + width] + image[usIndex + width + 1];
                finalImage[dsIndex] = aggregate * 0.25;
            
            }
        }

    }

    //transfer everything to screen for write out
    for (int ii = 0; ii < img.size(); ++ii) {
        img[ii].red = (unsigned char)finalImage[ii].r;
        img[ii].green = (unsigned char)finalImage[ii].g;
        img[ii].blue = (unsigned char)finalImage[ii].b;
    }
   


    auto stopTime = std::chrono::high_resolution_clock::now();
    auto RTduration = std::chrono::duration_cast<std::chrono::milliseconds>(endRTTime - startTime).count();
    auto imgProccessduration = std::chrono::duration_cast<std::chrono::milliseconds>(stopTime - endRTTime).count();
    std::cout << "DONE" << std::endl;

    std::cout << "Raytracer - Tracing took           " << RTduration<< "ms" << std::endl;
    std::cout << "Raytracer - image proccessing took " << imgProccessduration << "ms" << std::endl;
    std::cout << "Raytracer - Total time: " << RTduration + imgProccessduration << "ms" << std::endl;

    std::string filename = settings.mOutputFileName;
    if (settings.mSupersample) {
        filename = "ss-" + filename;
    }
    screen.PrintImage(filename);
}

// ugly json parsing methods
void to_json(nlohmann::json& j, const RayTracerSettings& p) {}
void from_json(const nlohmann::json& j, RayTracerSettings& s) {
    j.at("window").at("width").get_to(s.mWindowHeight);
    j.at("window").at("height").get_to(s.mWindowWidth);
    j.at("window").at("debug").get_to(s.mWindowDebugMode);
    j.at("window").at("supersample").get_to(s.mSupersample);
    auto pix = j.at("window").at("background");
    s.mBackgroundColor = { pix[0], pix[1] , pix[2] };
    j.at("outputFile").get_to(s.mOutputFileName);
    j.at("camera").get_to(s.camera);
    j.at("scene").at("intersectables").get_to(s.meshSceneObjects);
    j.at("scene").at("lights").get_to(s.lights);
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
        j.at("diffuse").get_to(m.mDiffuse);
    }

    if (j.count("specular") != 0) {
        j.at("specular").get_to(m.mSpecular);
    }

    if (j.count("ambient") != 0) {
        j.at("ambient").get_to(m.mAmbient);
    }

    j.at("shinyness").get_to(m.mShinyness);


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