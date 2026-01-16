#include <Debugger.h>
#include <iostream>
#include <iomanip>
#include <glm/glm.hpp>

void GLClearError()
{
    while (glGetError() != GL_NO_ERROR);
}

bool GLLogCall(const char* function, const char* file, int line)
{
    while (GLenum error = glGetError())
    {
        std::cout << "[OpenGL Error] (" << error << "): " << function << " " << file << ":" << line << std::endl;
        return false;
    }
    return true;
}

void PrintMatrix(const char* label, const glm::mat4& matrix)
{
    std::cout << "--- Matrix: " << label << " ---" << std::endl;
    for (int i = 0; i < 4; i++)
    {
        std::cout << "| ";
        for (int j = 0; j < 4; j++)
            std::cout << std::setw(10) << std::fixed << std::setprecision(4) << matrix[j][i] << " ";
        std::cout << "|" << std::endl;
    }
    std::cout << "---------------------------------------------" << std::endl;
}