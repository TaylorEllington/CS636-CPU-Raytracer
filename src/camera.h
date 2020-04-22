#pragma once

#include "glm.hpp"
#include "nlohmann/json.hpp"


class Camera {
public:
    glm::vec4 mPosition;
    glm::vec4 mViewDirection;
    glm::vec4 mViewUp;
    double mDistanceToViewPlane;
    double mHorizontalViewAngle;

};

//void to_json(nlohmann::json& j, const Camera& c) {
//
//};
void from_json(const nlohmann::json& j, Camera& c);