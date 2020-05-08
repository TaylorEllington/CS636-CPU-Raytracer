add_executable(ray-tracer
    bounding-box.h
    camera.h
    face.h
    intersectable.h
    mesh.h
    raytracer.h
    scene.h
    screen.h
    sphere.h

    bounding-box.cpp
    camera.cpp
    face.cpp
    mesh.cpp
    raytracer.cpp
    screen.cpp
    sphere.cpp
    
    main.cpp)

target_link_libraries(ray-tracer
    CLI11
    stb-image-write
    header-only
    )


set_target_properties(
   ray-tracer PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/")
