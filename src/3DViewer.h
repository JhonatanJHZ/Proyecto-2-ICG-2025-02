#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>
#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_opengl3.h"
#include "utils/3DFigure.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "../glm/mat4x4.hpp"
#include "../glm/gtc/matrix_transform.hpp"
#include "../glm/gtc/type_ptr.hpp"
#include "../glm/gtc/quaternion.hpp"
#include "../glm/gtx/quaternion.hpp"


class C3DViewer 
{

public:
    C3DViewer();

    bool setup();
    void setupModel(C3DFigure* model);

    void mainLoop();

    virtual ~C3DViewer();

private:

    int m_vertexCount;
    virtual void onKey(int key, int scancode, int action, int mods);

    virtual void onMouseButton(int button, int action, int mods);

    virtual void onCursorPos(double xpos, double ypos);

    virtual void update();

    virtual void render();

    virtual void drawInterface();

    void resize(int new_width, int new_height);

    bool setupShader();

    bool checkCompileErrors(GLuint shader, const char* type);

    void setupTriangle();

    static void keyCallbackStatic(GLFWwindow* window, int key, int scancode, int action, int mods);

    static void mouseButtonCallbackStatic(GLFWwindow* window, int button, int action, int mods);

    static void cursorPosCallbackStatic(GLFWwindow* window, double xpos, double ypos);

    void updateCameraVectors();

protected:
    int width = 720;
    int height = 480;
    GLFWwindow* m_window = nullptr;
    GLuint m_vao = 0;
    GLuint m_vbo = 0;
    GLuint m_shaderProgram = 0;
    double lastTime = 0.0;
    bool mouseButtonsDown[3] = { false, false, false };
    
    // Model State
    glm::vec3 m_modelPos = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::quat m_rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    float scale_factor = 1.0f;
    glm::vec3 m_userScale = glm::vec3(1.0f, 1.0f, 1.0f);

    // Camera State
    glm::vec3 m_camPos   = glm::vec3(0.0f, 0.0f, 2.5f);
    glm::vec3 m_camFront = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 m_camUp    = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 m_camRight = glm::vec3(1.0f, 0.0f, 0.0f);
    
    float m_yaw   = -90.0f;
    float m_pitch = 0.0f;

    // Timing
    float m_deltaTime = 0.0f;
    float m_lastFrame = 0.0f;

    float panelWidth = 200.0f;

    bool isDragging = false;
    bool isRotating = false;
    double lastMouseX, lastMouseY;
    int selectedSubMeshIndex = -1;

    const char* vertexShaderSrc = R"glsl(
        #version 330 core
        layout(location = 0) in vec3 aPos;
        layout(location = 1) in vec3 aColor;
        
        uniform mat4 u_mvp;

        out vec3 vColor;
        void main() 
        {
            gl_Position = u_mvp * vec4(aPos, 1.0);
            vColor = aColor;
        }
    )glsl";

    const char* fragmentShaderSrc = R"glsl(
        #version 330 core
        in vec3 vColor;
        out vec4 FragColor;
        void main() {
            FragColor = vec4(vColor, 1.0);
        }
    )glsl";
};
