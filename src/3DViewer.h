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

    C3DFigure* m_currentModel = nullptr;
    glm::vec3 indexToColor(int index);
    int colorToIndex(unsigned char r, unsigned char g, unsigned char b);
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

    void setupBoundingBox(BoundingBox box);
    void renderNormals(const SubMesh& mesh);

    void performPicking(int x, int y); 
    void updateCameraVectors();

protected:
    int width = 720;
    int height = 480;
    vector<RGBA> m_buffer;
    RGBA m_background_color = { 84, 84, 84, 255};
    GLFWwindow* m_window = nullptr;
    GLuint m_vao = 0;
    GLuint m_vbo = 0;
    GLuint m_shaderProgram = 0;
    double lastTime = 0.0;
    GLuint m_bboxVAO = 0, m_bboxVBO = 0;
    RGBA bbColor = {46, 204, 113, 255};
    bool m_showBBox = false;

    bool m_enableDepthTest = true;
    bool m_enableCullFace = false;
    bool m_enableLineSmooth = false;
    
    bool m_showFPS = false;
    float m_fps = 0.0f;
    float m_timeAccumulator = 0.0f;
    int m_frameCounter = 0;

    GLuint m_normalVAO = 0;
    GLuint m_normalVBO = 0;

    bool mouseButtonsDown[3] = { false, false, false };
    
    glm::vec3 m_modelPos = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::quat m_rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    float scale_factor = 1.0f;
    glm::vec3 m_userScale = glm::vec3(1.0f, 1.0f, 1.0f);

    glm::vec3 m_camPos   = glm::vec3(0.0f, 0.0f, 2.5f);
    glm::vec3 m_camFront = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 m_camUp    = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 m_camRight = glm::vec3(1.0f, 0.0f, 0.0f);
    
    float m_yaw   = -90.0f;
    float m_pitch = 0.0f;

    float m_deltaTime = 0.0f;
    float m_lastFrame = 0.0f;

    float panelWidth = 300.0f;
    char saveFileName[256] = "modelo_exportado";
    char loadFileName[256] = "cube.obj";
    bool m_ownsModel = false;

    bool isDragging = false;
    bool isRotating = false;
    double lastMouseX, lastMouseY;
    int selectedSubMeshIndex = -1;
    
    bool m_requestLoad = false;
    bool m_requestSave = false;
    
    const char* vertexShaderSrc = R"glsl(
        #version 330 core
        layout(location = 0) in vec3 aPos;
        layout(location = 1) in vec3 aColor;
        
        uniform mat4 u_mvp;
        uniform vec3 u_elementOffset;

        out vec3 vColor;
        void main() 
        {
            gl_Position = u_mvp * vec4(aPos + u_elementOffset, 1.0);
            vColor = aColor;
        }
    )glsl";

    const char* fragmentShaderSrc = R"glsl(
        #version 330 core
        in vec3 vColor;
        out vec4 FragColor;

        uniform vec3 u_pickingColor; 
        uniform bool u_isPicking;    
        uniform int u_selectedIndex; 
        uniform int u_currentMeshID; 
        uniform vec3 u_elementColor;
        uniform bool u_suppressHighlight;

        void main() {
            if (u_isPicking) {
                FragColor = vec4(u_pickingColor, 1.0);
            } else {
                FragColor = vec4(u_elementColor, 1.0); 
            }
        }
    )glsl";
};
