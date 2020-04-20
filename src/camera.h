#pragma once

#include "glm.hpp"


class Camera {
public: 
    int mPlaceholder;

    glm::vec4 mPosition;
    glm::vec4 mViewDirection;
    glm::vec4 mViewUp;
    double mDistanceToViewPlane;
    double mHorizontalViewAngle;

};