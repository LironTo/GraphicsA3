#pragma once

#define GLM_ENABLE_EXPERIMENTAL

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <Debugger.h>
#include <Shader.h>

//added 
#include "CubeFaceRotations.h"

class Camera
{
    private:
        // View and Projection
        glm::mat4 m_View = glm::mat4(1.0f);
        glm::mat4 m_Projection = glm::mat4(1.0f);

        // View matrix paramters
        glm::vec3 m_Position = glm::vec3(0.0f, 0.0f, 0.0f);
        glm::vec3 m_Orientation = glm::vec3(0.0f, 0.0f, -1.0f);
        glm::vec3 m_Up = glm::vec3(0.0f, 1.0f, 0.0f);

        // Projection matrix parameters
        float m_Near = 0.1f; 
        float m_Far = 100.0f;
        int m_Width;
        int m_Height;

        // Orthographic Projection parameters
        float m_Left = -1.0f;
        float m_Right = 1.0f;
        float m_Bottom = -1.0f; 
        float m_Top = 1.0f;

        // Rotation and mouse state
        double m_OldMouseX = 0.0;
        double m_OldMouseY = 0.0;
        float m_RotationX = 45.0f;
        float m_RotationY = 45.0f;





    public:

        bool isLocked(int axisIndex, bool axisLocked[3][2]);
        void SwitchLockedAxisState(int axisIndex, int sign, bool axisLocked[3][2]);
     struct AxisMapping {
            int index; // 0 for X, 1 for Y, 2 for Z
            int sign; // 1 or -1
        };
        Camera(int width, int height)
            : m_Width(width), m_Height(height) {};

        // Update View matrix based on current position and orientation
        void UpdateViewMatrix();

        // Update Projection matrix for Orthographic mode
        void SetOrthographic(float near, float far);

        // Update Projection matrix for Perspective mode
        void SetPerspective(float fovDegree, float near, float far);

        // Handle camera inputs
        void EnableInputs(GLFWwindow* window);

        // Get the rotation matrix based on current angles
        glm::mat4 GetRotationMatrix() const;
        // Handle mouse movement deltas and update rotation
        void HandleMouseMovement(double xpos, double ypos, bool leftButtonPressed);

        // Find which local cube axis is most aligned with a world direction
        AxisMapping GetWorldToLocalMapping(glm::vec3 worldDir) const;

        inline void SetPosition(glm::vec3 position) { m_Position = position; UpdateViewMatrix(); }
        inline void SetMousePosition(double x, double y) { m_OldMouseX = x; m_OldMouseY = y; }
        inline glm::mat4 GetViewMatrix() const { return m_View; }
        inline glm::mat4 GetProjectionMatrix() const { return m_Projection; }
};