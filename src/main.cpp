#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <Debugger.h>
#include <VertexBuffer.h>
#include <VertexBufferLayout.h>
#include <IndexBuffer.h>
#include <VertexArray.h>
#include <Shader.h>
#include <Texture.h>
#include <Camera.h>

#include <iostream>

//added
#include "CubeFaceRotations.h"
/* Window size */
const unsigned int width = 800;
const unsigned int height = 800;
// const float FOVdegree = 45.0f;  // Field Of View Angle
const float near = 0.1f;
const float far = 100.0f;

/* Cube vertices: 24 vertices (4 per face) to allow distinct colors/textures per face */
float vertices[] = {
    // positions          // colors           // texCoords
    // Front face (Red)
    -0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,   0.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,   1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,   1.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,   0.0f, 1.0f,
    // Back face (Orange)
    -0.5f, -0.5f, -0.5f,  1.0f, 0.5f, 0.0f,   0.0f, 0.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 0.5f, 0.0f,   1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 0.5f, 0.0f,   1.0f, 1.0f,
    -0.5f,  0.5f, -0.5f,  1.0f, 0.5f, 0.0f,   0.0f, 1.0f,
    // Top face (White)
    -0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f,   0.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f,   1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f,   1.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f,   0.0f, 1.0f,
    // Bottom face (Yellow)
    -0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 0.0f,   0.0f, 0.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 0.0f,   1.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 0.0f,   1.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 0.0f,   0.0f, 1.0f,
    // Right face (Blue)
     0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 1.0f,   0.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  0.0f, 0.0f, 1.0f,   1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f,   1.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 1.0f,   0.0f, 1.0f,
    // Left face (Green)
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f,   0.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,   1.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 0.0f,   1.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 1.0f, 0.0f,   0.0f, 1.0f
};

/* Indices for vertices order */
unsigned int indices[] = {
    0, 1, 2, 2, 3, 0,       // Front
    4, 5, 6, 6, 7, 4,       // Back
    8, 9, 10, 10, 11, 8,    // Top
    12, 13, 14, 14, 15, 12, // Bottom
    16, 17, 18, 18, 19, 16, // Right
    20, 21, 22, 22, 23, 20  // Left
};

// Struct to hold rotation state for each face
CubeFaceRotations g_cubeFaceRotations;

// Store the 27 cubies and their current transformation matrices
std::vector<glm::mat4> g_cubieMatrices;

void RotateFace(glm::vec3 axis, int axisIndex, float posValue, float angle) {
    glm::mat4 rot = glm::rotate(glm::mat4(1.0f), angle, axis);
    for (auto& matrix : g_cubieMatrices) {
        // Check if the cubie is on the face by looking at its current translation
        // The translation is stored in the 4th column (index 3) of the matrix
        if (glm::abs(matrix[3][axisIndex] - posValue) < 0.1f) {
            // Orbit the cubie around the center of the cube
            matrix = rot * matrix;
        }
    }
}

int main(int argc, char* argv[])
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
    {
        return -1;
    }
    
    /* Set OpenGL to Version 3.3.0 */
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(width, height, "OpenGL", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    /* Load GLAD so it configures OpenGL */
    gladLoadGL();

    /* Control frame rate */
    glfwSwapInterval(1);

    /* Print OpenGL version after completing initialization */
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;

    /* Set scope so that on widow close the destructors will be called automatically */
    {
        /* Blend to fix images with transperancy */
        GLCall(glEnable(GL_BLEND));
        GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

        /* Generate VAO, VBO, EBO and bind them */
        VertexArray va;
        VertexBuffer vb(vertices, sizeof(vertices));
        IndexBuffer ib(indices, sizeof(indices));

        VertexBufferLayout layout;
        layout.Push<float>(3);  // positions
        layout.Push<float>(3);  // colors
        layout.Push<float>(2);  // texCoords
        va.AddBuffer(vb, layout);

        /* World Axes vertices (X=Magenta, Y=Cyan, Z=White) */
        float axisVertices[] = {
            // positions          // colors           // texCoords
             0.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f,   0.5f, 0.5f, // X-axis (Magenta)
            10.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f,   0.5f, 0.5f,
             0.0f,  0.0f,  0.0f,  0.0f, 1.0f, 1.0f,   0.5f, 0.5f, // Y-axis (Cyan)
             0.0f, 10.0f,  0.0f,  0.0f, 1.0f, 1.0f,   0.5f, 0.5f,
             0.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f,   0.5f, 0.5f, // Z-axis (White)
             0.0f,  0.0f, 10.0f,  1.0f, 1.0f, 1.0f,   0.5f, 0.5f
        };
        VertexArray vaAxis;
        VertexBuffer vbAxis(axisVertices, sizeof(axisVertices));
        // Reuse the same layout as the cube
        vaAxis.AddBuffer(vbAxis, layout);

        /* Create texture */
        Texture texture("res/textures/plane.png");
        texture.Bind();
         
        /* Create shaders */
        Shader shader("res/shaders/basic.shader");
        shader.Bind();

        /* Unbind all to prevent accidentally modifying them */
        va.Unbind();
        vb.Unbind();
        ib.Unbind();
        shader.Unbind();

        /* Enables the Depth Buffer */
    	GLCall(glEnable(GL_DEPTH_TEST));

        /* Create camera */
        Camera camera(width, height);
        camera.SetPerspective(45.0f, near, far);
        camera.SetPosition(glm::vec3(0.0f, 0.0f, 10.0f));
        camera.EnableInputs(window);

        // Initialize the 27 cubies at their starting positions in a 3x3x3 grid
        g_cubieMatrices.clear();
        for (int x = -1; x <= 1; x++)
            for (int y = -1; y <= 1; y++)
                for (int z = -1; z <= 1; z++)
                    g_cubieMatrices.push_back(glm::translate(glm::mat4(1.0f), glm::vec3((float)x, (float)y, (float)z)));
        
        /*creates variables  */
        /* Loop until the user closes the window */
        while (!glfwWindowShouldClose(window))
        {
            /* Set white background color */
            GLCall(glClearColor(0.0f, 0.0f, 0.0f, 1.0f));

            /* Render here */
            GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

            /* Initialize uniform color */
            glm::vec4 color = glm::vec4(1.0);

            /* Create a global rotation for the whole Rubik's Cube */
            glm::mat4 globalRotation = glm::rotate(glm::mat4(1.0f), glm::radians(camera.m_RotationX), glm::vec3(1.0f, 0.0f, 0.0f));
            globalRotation = glm::rotate(globalRotation, glm::radians(camera.m_RotationY), glm::vec3(0.0f, 1.0f, 0.0f));

            glm::mat4 view = camera.GetViewMatrix();
            glm::mat4 proj = camera.GetProjectionMatrix();

            /* Bind shader and common uniforms once per frame */
            shader.Bind();
            shader.SetUniform4f("u_Color", color);
            shader.SetUniform1i("u_Texture", 0);

            /* Disable depth test so axes are drawn on top of everything */
            GLCall(glDisable(GL_DEPTH_TEST));

            /* Draw World Axes (Fixed in space) */
            shader.SetUniformMat4f("u_MVP", proj * view);
            vaAxis.Bind();
            GLCall(glLineWidth(2.0f));
            GLCall(glDrawArrays(GL_LINES, 0, 6));

            /* Draw Local Axes (Rotating with the cube) */
            shader.SetUniformMat4f("u_MVP", proj * view * globalRotation);
            GLCall(glLineWidth(5.0f));
            GLCall(glDrawArrays(GL_LINES, 0, 6));
            GLCall(glLineWidth(1.0f)); // Reset line width

            /* Re-enable depth test for the cube */
            GLCall(glEnable(GL_DEPTH_TEST));

            /* Draw the 27 cubies using their stored matrices */
            va.Bind();
            ib.Bind();
            for (const auto& cubieMatrix : g_cubieMatrices)
            {
                glm::mat4 mvp = proj * view * globalRotation * cubieMatrix;
                shader.SetUniformMat4f("u_MVP", mvp);
                GLCall(glDrawElements(GL_TRIANGLES, ib.GetCount(), GL_UNSIGNED_INT, nullptr));
            }

            /* Swap front and back buffers */
            glfwSwapBuffers(window);

            /* Poll for and process events */
            glfwPollEvents();
        }
    }

    glfwTerminate();
    return 0;
}