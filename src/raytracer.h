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
    glm::vec3 color;
};

struct PrimitiveDesc {
    glm::vec4 position;
    float radius;
};

struct Fragment {
    glm::vec3 subsamples[25] = { {0.0, 0.0, 0.0} };
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
    
    int SubfragmentRecurse(Fragment frag, int A, glm::vec3 imgPlaneA, int B, glm::vec3 imgPlaneB, int C, glm::vec3 imgPlaneC, int D, glm::vec3 imgPlaneD, glm::vec3 origin, std::vector<Intersectable*> sceneObjects, glm::vec3& outColor, bool skipTop, bool skipLeft, bool stop, float tolerance);
    bool ShootRay(Ray ray, std::vector<Intersectable *> sceneObjects,  Intersectable *& intersectedObject, glm::vec3 & intersectionNormal, float & intersectionDistance, Pixel & pix);
    void PhongShading(glm::vec3& pixel, const Material& mat, const glm::vec3& intersectionPoint, const glm::vec3& intersectNorm, const Camera& camera, const Pixel& pix, const  std::vector<LightDesc>& lights, const float& globalAmbient);
    glm::vec3 HallReflection(const Material& mat, const glm::vec3& intersectionPoint, const glm::vec3& intersectionNorm, const glm::vec3& previousIntersection, const Camera& camera, const std::vector<LightDesc>& lights, const float& globalAmbient, glm::vec3 previousAmbient, int depth);
    void HallShading(glm::vec3& pixel, const Material& mat, const glm::vec3& intersectionPoint, const glm::vec3& intersectionNorm, const Camera& camera, const std::vector<LightDesc>& lights, const float& globalAmbient);

    bool ShootAndShadePrimaryRay(Ray ray, std::vector<Intersectable*> sceneObjects, glm::vec3 & outColor);
    RayTracerSettings settings;
    std::vector<Intersectable*> sceneObjects;
};