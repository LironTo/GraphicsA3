#include <Camera.h>
#include <string>

void PrintCameraMapping(Camera* camera) {
    glm::mat4 globalRotation = glm::rotate(glm::mat4(1.0f), glm::radians(camera->m_RotationX), glm::vec3(1.0f, 0.0f, 0.0f));
    globalRotation = glm::rotate(globalRotation, glm::radians(camera->m_RotationY), glm::vec3(0.0f, 1.0f, 0.0f));

    auto getMapping = [&](glm::vec3 worldDir) {
        int bestIdx = 0;
        float maxAbsDot = -1.0f;
        float bestSign = 1.0f;
        for (int i = 0; i < 3; i++) {
            float d = glm::dot(glm::vec3(globalRotation[i]), worldDir);
            if (glm::abs(d) > maxAbsDot) {
                maxAbsDot = glm::abs(d);
                bestIdx = i;
                bestSign = (d > 0.0f) ? 1.0f : -1.0f;
            }
        }
        const char* names[] = { "X", "Y", "Z" };
        return std::string(names[bestIdx]) + (bestSign > 0 ? "+" : "-");
    };

    std::cout << "\n[ Camera-Relative Controls ]" << std::endl;
    std::cout << " F: " << getMapping(glm::vec3(0, 0, 1)) << "  |  B: " << getMapping(glm::vec3(0, 0, -1)) << std::endl;
    std::cout << " R: " << getMapping(glm::vec3(1, 0, 0)) << "  |  L: " << getMapping(glm::vec3(-1, 0, 0)) << std::endl;
    std::cout << " U: " << getMapping(glm::vec3(0, 1, 0)) << "  |  D: " << getMapping(glm::vec3(0, -1, 0)) << std::endl;
}

void Camera::UpdateViewMatrix()
{
    m_View = glm::lookAt(m_Position, m_Position + m_Orientation, m_Up);
}

void Camera::SetOrthographic(float near, float far)
{
    m_Near = near;
    m_Far = far;

    m_Projection = glm::ortho(m_Left, m_Right, m_Bottom, m_Top, near, far);
    UpdateViewMatrix();
}

void Camera::SetPerspective(float fovDegree, float near, float far)
{
    m_Near = near;
    m_Far = far;

    m_Projection = glm::perspective(glm::radians(fovDegree), (float)m_Width / (float)m_Height, near, far);
    UpdateViewMatrix();
}

/////////////////////
// Input Callbacks //
/////////////////////

void KeyCallback(GLFWwindow* window, int key, int scanCode, int action, int mods)
{
    Camera* camera = (Camera*) glfwGetWindowUserPointer(window);
    if (!camera) {
        std::cout << "Warning: Camera wasn't set as the Window User Pointer! KeyCallback is skipped" << std::endl;
        return;
    }

    // Prevent overlapping animations
    extern RotationAnimation g_rotationAnimation;
    if (g_rotationAnimation.active) return;

    if (action == GLFW_PRESS || action == GLFW_REPEAT)
    {
        // Access the global cube face rotation struct
        float baseAngle = (mods & GLFW_MOD_SHIFT) ? glm::radians(-90.0f) : glm::radians(90.0f);

        // 1. Reconstruct the current global rotation of the cube
        glm::mat4 globalRotation = glm::rotate(glm::mat4(1.0f), glm::radians(camera->m_RotationX), glm::vec3(1.0f, 0.0f, 0.0f));
        globalRotation = glm::rotate(globalRotation, glm::radians(camera->m_RotationY), glm::vec3(0.0f, 1.0f, 0.0f));

        // 2. Define the world-space direction associated with the key
        glm::vec3 worldDir(0.0f);
        switch (key)
        {
            case GLFW_KEY_F: worldDir = glm::vec3(0, 0, 1);  break; // Front (toward camera)
            case GLFW_KEY_B: worldDir = glm::vec3(0, 0, -1); break; // Back
            case GLFW_KEY_R: worldDir = glm::vec3(1, 0, 0);  break; // Right
            case GLFW_KEY_L: worldDir = glm::vec3(-1, 0, 0); break; // Left
            case GLFW_KEY_U: worldDir = glm::vec3(0, 1, 0);  break; // Up
            case GLFW_KEY_D: worldDir = glm::vec3(0, -1, 0); break; // Down
            default:
                return; // Not a rotation key
        }

        // 3. Find which local cube axis is most aligned with the target world direction
        int bestIdx = 0;
        float maxAbsDot = -1.0f;
        float bestSign = 1.0f;

        for (int i = 0; i < 3; i++)
        {
            // The i-th column of globalRotation is the i-th local axis in world space
            glm::vec3 localAxisInWorld = glm::vec3(globalRotation[i]);
            float d = glm::dot(localAxisInWorld, worldDir);

            if (glm::abs(d) > maxAbsDot)
            {
                maxAbsDot = glm::abs(d);
                bestIdx = i;
                bestSign = (d > 0.0f) ? 1.0f : -1.0f;
            }
        }

        // 4. Apply the rotation to the identified local face
        // We rotate around the local axis (X, Y, or Z)
        glm::vec3 localRotationAxis(0.0f);
        localRotationAxis[bestIdx] = 1.0f;

        // Start the animation
        g_rotationAnimation.axis = localRotationAxis;
        g_rotationAnimation.axisIndex = bestIdx;
        g_rotationAnimation.posValue = bestSign;
        g_rotationAnimation.targetAngle = baseAngle * bestSign;
        g_rotationAnimation.currentAngle = 0.0f;
        g_rotationAnimation.movingCubieIndices.clear();
        for (size_t i = 0; i < g_cubieMatrices.size(); i++) {
            if (glm::abs(g_cubieMatrices[i][3][bestIdx] - bestSign) < 0.1f) {
                g_rotationAnimation.movingCubieIndices.push_back(i);
            }
        }
        g_rotationAnimation.active = true;

    }
}

void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    Camera* camera = (Camera*)glfwGetWindowUserPointer(window);
    if (!camera) return;

    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        if (action == GLFW_PRESS) {
            double x, y;
            glfwGetCursorPos(window, &x, &y);
            camera->m_OldMouseX = x;
            camera->m_OldMouseY = y;
        }
        else if (action == GLFW_RELEASE) {
            PrintCameraMapping(camera);
        }
    }
}

void CursorPosCallback(GLFWwindow* window, double currMouseX, double currMouseY)
{
    Camera* camera = (Camera*) glfwGetWindowUserPointer(window);
    if (!camera) {
        std::cout << "Warning: Camera wasn't set as the Window User Pointer! KeyCallback is skipped" << std::endl;
        return;
    }

    camera->m_NewMouseX = camera->m_OldMouseX - currMouseX;
    camera->m_NewMouseY = camera->m_OldMouseY - currMouseY;
    camera->m_OldMouseX = currMouseX;
    camera->m_OldMouseY = currMouseY;

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
    {
        camera->m_RotationY -= (float)camera->m_NewMouseX * 0.2f;
        camera->m_RotationX -= (float)camera->m_NewMouseY * 0.2f;
    }
}

void ScrollCallback(GLFWwindow* window, double scrollOffsetX, double scrollOffsetY)
{
    Camera* camera = (Camera*) glfwGetWindowUserPointer(window);
    if (!camera) {
        std::cout << "Warning: Camera wasn't set as the Window User Pointer! ScrollCallback is skipped" << std::endl;
        return;
    }

    std::cout << "SCROLL Motion" << std::endl;
}

void Camera::EnableInputs(GLFWwindow* window)
{
    // Set camera as the user pointer for the window
    glfwSetWindowUserPointer(window, this);

    // Handle key inputs
    glfwSetKeyCallback(window, (void(*)(GLFWwindow *, int, int, int, int)) KeyCallback);

    // Handle cursor buttons
    glfwSetMouseButtonCallback(window, MouseButtonCallback);

    // Handle cursor position and inputs on motion
    glfwSetCursorPosCallback(window , (void(*)(GLFWwindow *, double, double)) CursorPosCallback);

    // Handle scroll inputs
    glfwSetScrollCallback(window, (void(*)(GLFWwindow *, double, double)) ScrollCallback);
}