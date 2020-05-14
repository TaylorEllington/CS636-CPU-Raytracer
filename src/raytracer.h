#pragma once
#include "camera.h"
#include "mesh.h"

#include <string>
#include <vector>
#include "nlohmann/json.hpp"

// init object for meshes
struct SceneObjDesc {
    glm::vec4 position;
    std::string type;
    Pixel color;
    Material material;

    //sphere only
    float radius;

    //mesh only
    BVHSettings settings;
    std::string filename;
    Scale scale;
    Rotate rotate;
};

struct LightDesc {
    glm::vec3 position; 
    float intensity;
    Pixel color;
};

struct PrimitiveDesc {
    glm::vec4 position;
    float radius;
};

//init object for the whole scene
struct RayTracerSettings {
    // setting up the window
    int mWindowHeight = 1;
    int mWindowWidth = 1;
    bool mSupersample = false;
    bool mWindowDebugMode = false;
    Pixel mBackgroundColor;

    //where to dump the file
    std::string mOutputFileName;

    //scene objects and associated data
    Camera camera;
    std::vector<SceneObjDesc> meshSceneObjects;
    std::vector<LightDesc> lights;
    float mSceneAmbient = 0.01;
};
void to_json(nlohmann::json& j, const RayTracerSettings& p);
void from_json(const nlohmann::json& j, RayTracerSettings& p);
void to_json(nlohmann::json& j, const SceneObjDesc& p);
void from_json(const nlohmann::json& j, SceneObjDesc& p);
void to_json(nlohmann::json& j, const PrimitiveDesc& p);
void from_json(const nlohmann::json& j, PrimitiveDesc& p);
void to_json(nlohmann::json& j, const LightDesc& l);
void from_json(const nlohmann::json& j, LightDesc& l);



class RayTracer {
public:
    RayTracer(RayTracerSettings init);
    void Run();

private:
    bool ShootRay(Ray ray, std::vector<Intersectable *> sceneObjects,  Intersectable *& intersectedObject, glm::vec3 & intersectionNormal, float & intersectionDistance, Pixel & pix);
    RayTracerSettings settings;
};