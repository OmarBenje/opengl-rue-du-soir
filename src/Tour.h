#pragma once
#include "Camera.h"
#include <vector>

struct CameraPose { glm::vec3 position, target; };
class TourCamera : public Camera {
public:
    static constexpr float duration = 60.f;
    TourCamera();
    void sample(float seconds);
    static CameraPose pose(float seconds);
    static bool verify();
};
