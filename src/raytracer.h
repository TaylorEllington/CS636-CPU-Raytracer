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

    //sphere only
    float radius;

    //mesh only
    std::string filename;
    Scale scale;
    Rotate rotate;
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
    bool mWindowDebugMode = false;

    //where to dump the file
    std::string mOutputFileName;

    //scene objects and associated data
    Camera camera;
    std::vector<SceneObjDesc> meshSceneObjects;
};
void to_json(nlohmann::json& j, const RayTracerSettings& p);
void from_json(const nlohmann::json& j, RayTracerSettings& p);
void to_json(nlohmann::json& j, const SceneObjDesc& p);
void from_json(const nlohmann::json& j, SceneObjDesc& p);
void to_json(nlohmann::json& j, const PrimitiveDesc& p);
void from_json(const nlohmann::json& j, PrimitiveDesc& p);



class RayTracer {
public:
    RayTracer(RayTracerSettings init);
    void Run();

private:
    RayTracerSettings settings;
};