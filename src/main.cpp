#include <iostream>
#include <iomanip>

#include "raytracer.h"
#include "nlohmann/json.hpp"

#include <fstream>

RayTracerSettings parseSettingsJson(std::string filename) {
    std::fstream f(filename);

    if (!f.is_open()) {
        throw std::runtime_error("File: " + filename + " does not exist! ");
    }

    nlohmann::json j;
    f >> j;
    RayTracerSettings settings = j.get<RayTracerSettings>();
    return settings;
}

void main(int argc, char** argv) {

    //read in any command line flags
    //verify that the scene description exists
    //instantiate the ray tracer
    //draw the image

    //RayTracerSettings init = parseSettingsJson("assets/models-and-spheres.json");
    //RayTracerSettings init = parseSettingsJson("assets/single-supertoroid.json");
    //RayTracerSettings init = parseSettingsJson("assets/spheres-scene.json");
    //RayTracerSettings init = parseSettingsJson("assets/halo-ring-scene.json");
    RayTracerSettings init = parseSettingsJson("assets/dragon-scene.json");
    
    //RayTracerSettings init = parseSettingsJson("assets/solarized-sphere-and-supertoroid-scene.json");
    
    RayTracer rt(init);
    rt.Run();
}