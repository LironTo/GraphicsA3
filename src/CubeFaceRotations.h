#pragma once
#include <glm/glm.hpp>
#include <vector>

extern std::vector<glm::mat4> g_cubieMatrices;

struct RotationAnimation {
    bool active = false;
    glm::vec3 axis;
    int axisIndex;
    float posValue;
    float currentAngle = 0.0f;
    float targetAngle = 0.0f;
    float speed = 10.0f; // Radians per second
    std::vector<size_t> movingCubieIndices;
};
extern RotationAnimation g_rotationAnimation;

void RotateFace(glm::vec3 axis, int axisIndex, float posValue, float angle);