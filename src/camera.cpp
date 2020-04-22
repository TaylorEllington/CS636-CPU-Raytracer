#include "camera.h"

void from_json(const nlohmann::json& j, Camera& c) {
    auto pos = j.at("position");
    c.mPosition = glm::vec4(pos[0], pos[1], pos[2], 1.0); //point

    auto view = j.at("lookAtPoint");
    c.mViewDirection =  glm::vec4(view[0], view[1], view[2], 1.0) -  c.mPosition; // vector (point - point)

    auto up = j.at("up");
    c.mViewUp = glm::vec4(up[0], up[1], up[2], 0.0);  // vector

    j.at("viewPlaneDist").get_to(c.mDistanceToViewPlane);
    j.at("hViewAngle").get_to(c.mHorizontalViewAngle);

}