#include <Camera.h>
#include <string>
#include <vector>
#include <iostream>

extern std::vector<glm::mat4> g_cubieMatrices;

void PrintCameraMapping(Camera* camera) {
    auto getMapping = [&](glm::vec3 worldDir) {
        Camera::AxisMapping mapping = camera->GetWorldToLocalMapping(worldDir);
        const char* names[] = { "X", "Y", "Z" };
        return std::string(names[mapping.index]) + (mapping.sign > 0 ? "+" : "-");
    };

    std::cout << "\n[ Camera-Relative Controls ]" << std::endl;
    std::cout << " F: " << getMapping(glm::vec3(0, 0, 1)) << "  |  B: " << getMapping(glm::vec3(0, 0, -1)) << std::endl;
    std::cout << " R: " << getMapping(glm::vec3(1, 0, 0)) << "  |  L: " << getMapping(glm::vec3(-1, 0, 0)) << std::endl;
    std::cout << " U: " << getMapping(glm::vec3(0, 1, 0)) << "  |  D: " << getMapping(glm::vec3(0, -1, 0)) << std::endl;
}

glm::mat4 Camera::GetRotationMatrix() const
{
    glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(m_RotationX), glm::vec3(1.0f, 0.0f, 0.0f));
    return glm::rotate(rotation, glm::radians(m_RotationY), glm::vec3(0.0f, 1.0f, 0.0f));
}

Camera::AxisMapping Camera::GetWorldToLocalMapping(glm::vec3 worldDir) const
{
    glm::mat4 globalRotation = GetRotationMatrix();
    int bestIdx = 0;
    float maxAbsDot = -1.0f;
    int bestSign = 1;

    for (int i = 0; i < 3; i++) {
        float d = glm::dot(glm::vec3(globalRotation[i]), worldDir);
        if (glm::abs(d) > maxAbsDot) {
            maxAbsDot = glm::abs(d);
            bestIdx = i;
            bestSign = (d > 0.0f) ? 1 : -1;
        }
    }
    return { bestIdx, bestSign};
}

void Camera::HandleMouseMovement(double xpos, double ypos, bool leftButtonPressed)
{
    double dx = m_OldMouseX - xpos;
    double dy = m_OldMouseY - ypos;
    m_OldMouseX = xpos;
    m_OldMouseY = ypos;

    if (leftButtonPressed)
    {
        m_RotationY -= (float)dx * 0.2f;
        m_RotationX -= (float)dy * 0.2f;
        UpdateViewMatrix();
    }
}

void Camera::HandleScroll(double yoffset)
{
    // Adjust the Z position. Scrolling up (positive yoffset) moves the camera closer.
    m_Position.z -= (float)yoffset * 0.5f;

    // Clamp the zoom distance to keep the cube visible and prevent flipping the view
    if (m_Position.z < 2.0f) m_Position.z = 2.0f;
    if (m_Position.z > 50.0f) m_Position.z = 50.0f;

    UpdateViewMatrix();
}

void Camera::UpdateViewMatrix()
{
    m_View = glm::lookAt(m_Position, m_Position + m_Orientation, m_Up) * GetRotationMatrix();
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

void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    Camera* camera = (Camera*) glfwGetWindowUserPointer(window);
    if (!camera) {
        std::cout << "Warning: Camera wasn't set as the Window User Pointer! KeyCallback is skipped" << std::endl;
        return;
    }

    // Persistent turn angle state
    static float s_TurnAngle = 90.0f;
    static bool axisLocked[3][2];// {{X, -X}, Y, -Y, Z, -Z
    if (action == GLFW_PRESS)
    {
        if (key == GLFW_KEY_Z && s_TurnAngle < 180.0f) {
            s_TurnAngle *= 2.0f;
            std::cout << "Current Turn Angle: " << s_TurnAngle << " degrees" << std::endl;
            return;
        }
        if (key == GLFW_KEY_A && s_TurnAngle  > 45.0f ) {
            s_TurnAngle /= 2.0f;
            std::cout << "Current Turn Angle: " << s_TurnAngle << " degrees" << std::endl;
            return;
        }
    }

    // Prevent overlapping animations
    extern RotationAnimation g_rotationAnimation;
    if (g_rotationAnimation.active) return;
    
    
    if (action == GLFW_PRESS || action == GLFW_REPEAT)
    {
        // Use Space key state for reverse rotation
        bool reverse = (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS);
        float baseAngle = reverse ? glm::radians(-s_TurnAngle) : glm::radians(s_TurnAngle);
        
        
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
        Camera::AxisMapping mapping = camera->GetWorldToLocalMapping(worldDir);
        int bestIdx = mapping.index;
        int bestSign = mapping.sign;

        //check for axis lock toggles
        if(camera->isLocked(bestIdx, axisLocked)) {std::cout << "Locked wall:" << (bestIdx == 0 ? "X" : bestIdx == 1 ? "Y" : "Z") <<  std::endl;   return ;} else if(s_TurnAngle == 45.0f) camera->SwitchLockedAxisState(bestIdx, bestSign, axisLocked);
        

        // 4. Apply the rotation to the identified local face
        // We rotate around the local axis (X, Y, or Z)
        glm::vec3 localRotationAxis(0.0f);
        localRotationAxis[bestIdx] = 1.0f;


        //turns all



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
//checks if axis is locked
bool Camera::isLocked(int axisIndex, bool axisLocked[3][2]) {
    return axisLocked[(axisIndex + 1) % 3][0] ||
    axisLocked[(axisIndex + 2) % 3][0] ||
    axisLocked[(axisIndex + 1) % 3][1] ||
    axisLocked[(axisIndex + 2) % 3][1];
}
void Camera::SwitchLockedAxisState(int axisIndex, int sign, bool axisLocked[3][2]) {
    axisLocked[axisIndex][sign > 0 ? 0 : 1] = !axisLocked[axisIndex][sign > 0 ? 0 : 1];
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
            camera->SetMousePosition(x, y);
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

    bool leftPressed = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
    camera->HandleMouseMovement(currMouseX, currMouseY, leftPressed);
}

void ScrollCallback(GLFWwindow* window, double scrollOffsetX, double scrollOffsetY)
{
    Camera* camera = (Camera*) glfwGetWindowUserPointer(window);
    if (!camera) {
        std::cout << "Warning: Camera wasn't set as the Window User Pointer! ScrollCallback is skipped" << std::endl;
        return;
    }
    camera->HandleScroll(scrollOffsetY);
}

void Camera::EnableInputs(GLFWwindow* window)
{
    // Set camera as the user pointer for the window
    glfwSetWindowUserPointer(window, this);

    // Handle key inputs
    glfwSetKeyCallback(window, KeyCallback);

    // Handle cursor buttons
    glfwSetMouseButtonCallback(window, MouseButtonCallback);

    // Handle cursor position and inputs on motion
    glfwSetCursorPosCallback(window, CursorPosCallback);

    // Handle scroll inputs
    glfwSetScrollCallback(window, ScrollCallback);
}