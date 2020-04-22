#include <iostream>

#include "raytracer.h"

void main(int argc, char** argv) {

    //read in any command line flags
    //verify that the scene description exists
    //instantiate the ray tracer
    //draw the image

    RayTracerSettings init;
    init.mWindowWidth = 512;
    init.mWindowHeight = 512;
    init.mWindowDebugMode = true;  // this gives us the purple background
    init.mOutputFileName = "cube.png";

    init.camera.mPosition = { 0.0, 0.0, 6.0, 1.0 };
    init.camera.mViewDirection = glm::vec4(0,0,0, 1) - init.camera.mPosition;
    init.camera.mViewUp = { 0.0, 1.0, 0.0, 0.0 };

    init.camera.mDistanceToViewPlane = 1;
    init.camera.mHorizontalViewAngle = 56.0;

    MeshDesc desc;
    desc.filename = "assets/cube.smf";
    desc.position = { 1.0, 2.0, 1.0, 1.0 };
    desc.rotate = { 0,45,0 };
    desc.scale = { 1,1,1};

    init.meshSceneObjects.push_back(desc);

     PrimitiveDesc pDesc0;
     pDesc0.position = { 4, 6, 0.0, 1.0 };
     pDesc0.radius = 0.5;
     //init.primitiveSceneObjects.push_back(pDesc0);





    // Supertoroid scene
    // MeshDesc desc;
    // desc.filename = "assets/supertoroid.smf";
    // desc.position = { 0.0, 0.0, 0.0, 1.0 };
    // desc.rotate = { 0,0,45 };
    // desc.scale = { 1,1,1 };

    // PrimitiveDesc pDesc0;
    // pDesc0.position = { -3, 0.0, 0.0, 1.0 };
    // pDesc0.radius = 0.5;

    // PrimitiveDesc pDesc1;
    // pDesc1.position = { 3, 0.0, 0.0, 1.0 };
    // pDesc1.radius = 0.5;

    // PrimitiveDesc pDesc2;
    // pDesc2.position = { 0.0, -3.0, 0.0, 1.0 };
    // pDesc2.radius = 0.5;

    // PrimitiveDesc pDesc3;
    // pDesc3.position = { 0.0, 3.0, 0.0, 1.0 };
    // pDesc3.radius = 0.5;

    // init.meshSceneObjects.push_back(desc);

    // init.primitiveSceneObjects.push_back(pDesc0);
    // init.primitiveSceneObjects.push_back(pDesc1);
    // init.primitiveSceneObjects.push_back(pDesc2);
    // init.primitiveSceneObjects.push_back(pDesc3);




    RayTracer rt(init);
    rt.Run();

}