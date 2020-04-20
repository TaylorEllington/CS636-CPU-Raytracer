#pragma once
#include "camera.h"
#include "mesh.h"

#include <string>
#include <vector>

// init object for meshes
struct MeshDesc {
    std::string filename;
    glm::vec4 position;
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
    int mWindowHeight = 0;
    int mWindowWidth = 0;
    bool mWindowDebugMode = 0;

    //where to dump the file
    std::string mOutputFileName;

    //scene objects and associated data
    Camera camera;
    std::vector<MeshDesc> meshSceneObjects;
    std::vector<PrimitiveDesc> primitiveSceneObjects;
};


class RayTracer {
public:
    RayTracer(RayTracerSettings init);
    void Run();

private:
    RayTracerSettings settings;
};