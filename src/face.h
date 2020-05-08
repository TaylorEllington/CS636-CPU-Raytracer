#include "bounding-box.h"
struct Face {
    unsigned int vert[3];
    glm::vec3 mCentroid;
    BoundingBox mBBox;

    Face(unsigned int p0, unsigned int p1, unsigned int p2){
        vert[0] = p0;
        vert[1] = p1;
        vert[2] = p2;
    }

    
};

struct Vertex {
    glm::vec4 coord;
    glm::vec3 normal;

    Vertex(glm::vec4 c) {
        coord = c;
        normal = glm::vec4(0, 0, 0, 0);
    }
};

struct Scale {
    float scaleX;
    float scaleY;
    float scaleZ;
};

struct Rotate {
    float rotDegX;
    float rotDegY;
    float rotDegZ;
};
