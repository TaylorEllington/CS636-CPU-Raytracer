#include <iostream>
#include <iomanip>

#include "raytracer.h"
#include "nlohmann/json.hpp"

#include "CLI11.hpp"

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

int main(int argc, char** argv) {

    // Read in any command line flags 
    CLI::App app{ "CS636 Ray Tracer" };

    std::string filename = "default";
    app.add_option("-f,--file", filename, "A help string");

    CLI11_PARSE(app, argc, argv);


    // Verify that the scene description exists
    RayTracerSettings init = parseSettingsJson(filename);
    
    // Instantiate the ray tracer
    RayTracer rt(init);
    
    // Draw the image
    rt.Run();
}