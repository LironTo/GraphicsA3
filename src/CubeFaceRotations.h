#pragma once
#include <glm/glm.hpp>
#include <vector>

struct CubeFaceRotations {
    float front = 0.0f;
    float back = 0.0f;
    float left = 0.0f;
    float right = 0.0f;
    float top = 0.0f;
    float bottom = 0.0f;
};

extern CubeFaceRotations g_cubeFaceRotations;
extern std::vector<glm::mat4> g_cubieMatrices;

void RotateFace(glm::vec3 axis, int axisIndex, float posValue, float angle);