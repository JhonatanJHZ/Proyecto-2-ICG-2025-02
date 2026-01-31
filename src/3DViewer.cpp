#include "3DViewer.h"
#include <iostream>
#include "utils/3DFigure.h"
#include "tinyfiledialogs.h"

C3DViewer::C3DViewer()
{
}

C3DViewer::~C3DViewer()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    
    if (m_vbo) glDeleteBuffers(1, &m_vbo);
    if (m_vao) glDeleteVertexArrays(1, &m_vao);
    if (m_shaderProgram) glDeleteProgram(m_shaderProgram);
    if (m_window) glfwDestroyWindow(m_window);
    glfwTerminate();
}

bool C3DViewer::setup()
{
    if (!glfwInit()) 
        return false;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);

    m_window = glfwCreateWindow(width, height, "C3DViewer Window: 3D Object Render Modifier", NULL, NULL);
    if (!m_window) 
    {
        glfwTerminate();
        return false;
    }

    glfwGetFramebufferSize(m_window, &width, &height);
    glfwMakeContextCurrent(m_window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) 
    {
        glfwDestroyWindow(m_window);
        glfwTerminate();
        return false;
    }
    
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(m_window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");
    
    glfwSetWindowUserPointer(m_window, this);
    glfwSetFramebufferSizeCallback(m_window, [](GLFWwindow* window, int w, int h) {
        auto ptr = reinterpret_cast<C3DViewer*>(glfwGetWindowUserPointer(window));
        if (ptr) 
            ptr->resize(w, h);
    });

    if (!setupShader()) return false;

    setupTriangle();

    glEnable(GL_DEPTH_TEST);
    glViewport(0, 0, width, height);

    glfwSetWindowUserPointer(m_window, this);
    glfwSetKeyCallback(m_window, keyCallbackStatic);
    glfwSetMouseButtonCallback(m_window, mouseButtonCallbackStatic);
    glfwSetCursorPosCallback(m_window, cursorPosCallbackStatic);

    return true;
}

void C3DViewer::update()
{
    float currentFrame = (float)glfwGetTime();
    m_deltaTime = currentFrame - m_lastFrame;
    m_lastFrame = currentFrame;

    float cameraSpeed = 2.5f * m_deltaTime;

    if (glfwGetKey(m_window, GLFW_KEY_UP) == GLFW_PRESS)
        m_camPos += cameraSpeed * m_camFront;
    if (glfwGetKey(m_window, GLFW_KEY_DOWN) == GLFW_PRESS)
        m_camPos -= cameraSpeed * m_camFront;

    float rotSpeed = 100.0f * m_deltaTime; 
    if (glfwGetKey(m_window, GLFW_KEY_LEFT) == GLFW_PRESS)
    {
        m_yaw -= rotSpeed;
        updateCameraVectors();
    }
    if (glfwGetKey(m_window, GLFW_KEY_RIGHT) == GLFW_PRESS)
    {
        m_yaw += rotSpeed;
        updateCameraVectors();
    }
}

void C3DViewer::mainLoop() 
{
    while (!glfwWindowShouldClose(m_window)) 
    {
        glfwPollEvents();

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        render();

        glfwSwapBuffers(m_window);
    }
}

void C3DViewer::onKey(int key, int scancode, int action, int mods) 
{
    if (action == GLFW_PRESS || action == GLFW_REPEAT)
    {
        if (key == GLFW_KEY_ESCAPE)
            glfwSetWindowShouldClose(m_window, GLFW_TRUE);
    }
}

void C3DViewer::onMouseButton(int button, int action, int mods) 
{
    if (ImGui::GetIO().WantCaptureMouse) return;
    
    if (button >= 0 && button < 3)
    {
        if (action == GLFW_PRESS)
        {
            mouseButtonsDown[button] = true;
            glfwGetCursorPos(m_window, &lastMouseX, &lastMouseY);

            if (button == GLFW_MOUSE_BUTTON_LEFT) 
            {
                if (lastMouseX >= panelWidth) 
                {
                    performPicking((int)lastMouseX, (int)lastMouseY);
                }
            }
        }
        else if (action == GLFW_RELEASE)
        {
            mouseButtonsDown[button] = false;
        }
    }
}

void C3DViewer::performPicking(int x, int y) 
{
    glViewport((int)panelWidth, 0, width - (int)panelWidth, height);
    
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(m_shaderProgram);
    
    float aspect = (float)(width - panelWidth) / (float)height;
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);
    glm::mat4 view = glm::lookAt(m_camPos, m_camPos + m_camFront, m_camUp);
    
    glm::mat4 model = glm::translate(glm::mat4(1.0f), m_modelPos);
    model = model * glm::mat4_cast(m_rotation);
    model = glm::scale(model, m_userScale * scale_factor);
    
    glm::mat4 mvp = projection * view * model;

    GLuint mvpLoc = glGetUniformLocation(m_shaderProgram, "u_mvp");
    glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(mvp));

    GLint isPickingLoc = glGetUniformLocation(m_shaderProgram, "u_isPicking");
    GLint pickColorLoc = glGetUniformLocation(m_shaderProgram, "u_pickingColor");
    
    glUniform1i(isPickingLoc, 1);

    if (m_currentModel) {
        glBindVertexArray(m_vao);
        
        const std::vector<SubMesh>& subMeshes = m_currentModel->getSubMeshes();
        GLuint offsetLoc = glGetUniformLocation(m_shaderProgram, "u_elementOffset");

        for (int i = 0; i < (int)subMeshes.size(); ++i) {
            glm::vec3 pickColor = indexToColor(i); 
            glUniform3fv(pickColorLoc, 1, glm::value_ptr(pickColor));
            glUniform3fv(offsetLoc, 1, glm::value_ptr(subMeshes[i].offset));
            
            glDrawArrays(GL_TRIANGLES, subMeshes[i].startVertex, subMeshes[i].vertexCount);
        }
        glBindVertexArray(0);
    }

    unsigned char pixel[4];
    glReadPixels(x, height - y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);

    int pickedID = colorToIndex(pixel[0], pixel[1], pixel[2]);

    if (pickedID != -1 && pickedID < (int)m_currentModel->getSubMeshes().size()) {
        selectedSubMeshIndex = pickedID;
        m_showBBox = true;
    } else {
        selectedSubMeshIndex = -1;
        m_showBBox = false;
    }

    glUniform1i(isPickingLoc, 0);
    
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 
    glUniform1i(glGetUniformLocation(m_shaderProgram, "u_isPicking"), 0); 
    glUniform1i(glGetUniformLocation(m_shaderProgram, "u_selectedIndex"), selectedSubMeshIndex);
}

void C3DViewer::onCursorPos(double xpos, double ypos) 
{
    if (mouseButtonsDown[0] || mouseButtonsDown[1] || mouseButtonsDown[2]) 
    {
        cout << "Mouse Drag at " << xpos << ", " << ypos << "\n";
    }

    if (mouseButtonsDown[0]) 
    {
        float deltaX = (float)(xpos - lastMouseX);
        float deltaY = (float)(ypos - lastMouseY);
        
        float dist = glm::length(m_camPos - m_modelPos);
        float fov = glm::radians(45.0f); 
        float visibleHeight = 2.0f * dist * tan(fov / 2.0f);
        float sensitivity = visibleHeight / height;

        m_modelPos.x += deltaX * sensitivity;
        m_modelPos.y -= deltaY * sensitivity;
        lastMouseX = xpos;
        lastMouseY = ypos;
    }
    else if(mouseButtonsDown[1]){
        float deltaX = (float)(xpos - lastMouseX);
        float deltaY = (float)(ypos - lastMouseY);
        float sensitivty = 0.005f; 

        glm::quat rotY = glm::angleAxis(deltaX * sensitivty, glm::vec3(0.0f, 1.0f, 0.0f));
        glm::quat rotX = glm::angleAxis(deltaY * sensitivty, glm::vec3(1.0f, 0.0f, 0.0f));
        
        m_rotation = (rotX * rotY) * m_rotation;

        lastMouseX = xpos;
        lastMouseY = ypos;
    }
    else if (mouseButtonsDown[2]) 
    {
        float xoffset = (float)(xpos - lastMouseX);
        float yoffset = (float)(lastMouseY - ypos); 
        lastMouseX = xpos;
        lastMouseY = ypos;

        float sensitivity = 0.1f;
        m_yaw   += xoffset * sensitivity;
        m_pitch += yoffset * sensitivity;

        if (m_pitch > 89.0f)  m_pitch = 89.0f;
        if (m_pitch < -89.0f) m_pitch = -89.0f;

        updateCameraVectors();
    }
}

void C3DViewer::render() {
    update();
    
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f); 
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 
    glViewport((int)panelWidth, 0, width - (int)panelWidth, height);

    glUseProgram(m_shaderProgram);

    if (height == 0) height = 1; 
    float aspect = (float)(width - panelWidth) / (float)height;
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);
    glm::mat4 view = glm::lookAt(m_camPos, m_camPos + m_camFront, m_camUp);
    glm::mat4 model = glm::translate(glm::mat4(1.0f), m_modelPos);
    model = model * glm::mat4_cast(m_rotation); 
    model = glm::scale(model, m_userScale * scale_factor);

    glm::mat4 mvp = projection * view * model;

    GLuint mvpLoc = glGetUniformLocation(m_shaderProgram, "u_mvp");
    if (mvpLoc != -1) {
        glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(mvp));
    }
    glUniform1i(glGetUniformLocation(m_shaderProgram, "u_selectedIndex"), selectedSubMeshIndex);

    if (m_currentModel) {
        glBindVertexArray(m_vao);
        const auto& meshes = m_currentModel->getSubMeshes();
        GLuint meshIdLoc = glGetUniformLocation(m_shaderProgram, "u_currentMeshID");
        GLuint offsetLoc = glGetUniformLocation(m_shaderProgram, "u_elementOffset");
        GLuint colorLoc  = glGetUniformLocation(m_shaderProgram, "u_elementColor");
        glUniform1i(glGetUniformLocation(m_shaderProgram, "u_suppressHighlight"), m_isEditingColor);

        for (int i = 0; i < (int)meshes.size(); ++i) {
            glUniform1i(meshIdLoc, i);
            glUniform3fv(offsetLoc, 1, glm::value_ptr(meshes[i].offset));
            glUniform3fv(colorLoc, 1, glm::value_ptr(meshes[i].material.kd));
            
            if (meshes[i].vertexCount > 0) {
                glDrawArrays(GL_TRIANGLES, meshes[i].startVertex, meshes[i].vertexCount);
            }
        }
    }

    if (m_showBBox && m_bboxVAO != 0 && selectedSubMeshIndex != -1 && selectedSubMeshIndex < m_currentModel->getSubMeshes().size()) {
        const SubMesh& sm = m_currentModel->getSubMeshes()[selectedSubMeshIndex];
        
        setupBoundingBox(sm.bbox);

        GLuint offsetLoc = glGetUniformLocation(m_shaderProgram, "u_elementOffset");
        GLuint colorLoc = glGetUniformLocation(m_shaderProgram, "u_elementColor");

        glUniform1i(glGetUniformLocation(m_shaderProgram, "u_currentMeshID"), -1);
        glUniform3fv(offsetLoc, 1, glm::value_ptr(sm.offset));
        glUniform3fv(colorLoc, 1, bbColor);

        glBindVertexArray(m_bboxVAO);
        
        glDisableVertexAttribArray(1); 
        glVertexAttrib3fv(1, bbColor); 

        glLineWidth(2.0f); 
        glDrawArrays(GL_LINES, 0, 24); 
        glLineWidth(1.0f);

        glEnableVertexAttribArray(1);
    }

    glViewport(0, 0, width, height);
    drawInterface();
}

void C3DViewer::drawInterface()
{
    double currentTime = glfwGetTime();
    double deltaTime = currentTime - lastTime;

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    
    ImGui::SetNextWindowPos(viewport->Pos); 
    ImGui::SetNextWindowSize(ImVec2(panelWidth, viewport->Size.y));

    ImGuiWindowFlags window_flags = 0;
    window_flags |= ImGuiWindowFlags_NoMove;          
    window_flags |= ImGuiWindowFlags_NoResize;        
    window_flags |= ImGuiWindowFlags_NoCollapse;      
    window_flags |= ImGuiWindowFlags_NoSavedSettings; 
    window_flags |= ImGuiWindowFlags_NoTitleBar;      

    ImGui::Begin("Control Panel", nullptr, window_flags);
    ImGui::Text("Menú Principal");

    ImGui::Separator();
    ImGui::Text("Transformaciones");
    
    ImGui::Text("Escala por Eje:");
    ImGui::SliderFloat("X", &m_userScale.x, 0.1f, 5.0f);
    ImGui::SliderFloat("Y", &m_userScale.y, 0.1f, 5.0f);
    ImGui::SliderFloat("Z", &m_userScale.z, 0.1f, 5.0f);

    if (ImGui::Button("Reiniciar Escala")) {
        m_userScale = vec3(1.0f, 1.0f, 1.0f);
    }

    if(ImGui::Button("Centrar objeto")){
        m_modelPos = glm::vec3(0.0f, 0.0f, 0.0f); 
        m_rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
        m_camPos   = glm::vec3(0.0f, 0.0f, 3.0f);
        m_yaw      = -90.0f; 
        m_pitch    = 0.0f;
        updateCameraVectors();
    }

    ImGui::Text("Bounding Box:");
    ImGui::ColorEdit3("Color", bbColor);

    if (selectedSubMeshIndex != -1 && m_currentModel) {
        ImGui::Separator();
        ImGui::Text("Sub-Malla Seleccionada");
        
        vector<SubMesh>& meshes = m_currentModel->getSubMeshesModifiable();
        if (selectedSubMeshIndex < meshes.size()) {
            SubMesh& mesh = meshes[selectedSubMeshIndex];
            ImGui::Text("ID: %d - %s", selectedSubMeshIndex, mesh.groupName.c_str());
            
            ImGui::ColorEdit3("Color SM", glm::value_ptr(mesh.material.kd));
            
            if (ImGui::IsItemActive()) {
                m_isEditingColor = true;
            } else {
                m_isEditingColor = false;
            }

            ImGui::DragFloat3("Traslacion SM", glm::value_ptr(mesh.offset), 0.01f);
            
            if (ImGui::Button("Eliminar Sub-malla")) {
                m_currentModel->deleteSubMesh(selectedSubMeshIndex);
                setupModel(m_currentModel);
                selectedSubMeshIndex = -1;
                m_showBBox = false;
            }
        }
    }

    ImGui::Separator();
    ImGui::Text("Guardar Modelo OBJ/MTL");
    if (ImGui::Button("Guardar OBJ")) {
        const char* filterPatterns[] = { "*.obj" };
        const char* savePath = tinyfd_saveFileDialog(
            "Guardar Modelo", 
            "modelo_exportado.obj", 
            1, 
            filterPatterns, 
            "Archivos Wavefront OBJ"
        );

        if (savePath) {
            m_currentModel->saveObject(string(savePath), m_modelPos, m_rotation, m_userScale * scale_factor);
        }
    }

    ImGui::Separator();
    ImGui::Text("Cargar Modelo OBJ");
    if (ImGui::Button("Cargar OBJ")) {
        const char* filterPatterns[] = { "*.obj" };
        const char* openPath = tinyfd_openFileDialog(
            "Cargar Modelo", 
            "", 
            1, 
            filterPatterns, 
            "Archivos Wavefront OBJ", 
            0
        );

        if (openPath) {
            C3DFigure* newModel = new C3DFigure();
            if (newModel->loadObject(string(openPath))) {
                if (m_ownsModel && m_currentModel) {
                    delete m_currentModel;
                }
                setupModel(newModel);
                m_ownsModel = true;
                
                m_modelPos = glm::vec3(0.0f);
                m_rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
                m_userScale = glm::vec3(1.0f); 
                m_currentModel->normalization(); 
                setupModel(m_currentModel); 
            } else {
                std::cerr << "Error cargando: " << openPath << std::endl;
                delete newModel;
            }
        }
    }
    
    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    if (deltaTime >= 1.0) 
    {
        lastTime = currentTime;
    }
}

void C3DViewer::resize(int new_width, int new_height) 
{
    width = new_width;
    height = new_height;

    glViewport(0, 0, width, height);
}

bool C3DViewer::setupShader() 
{
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSrc, nullptr);
    glCompileShader(vertexShader);
    if (!checkCompileErrors(vertexShader, "VERTEX")) return false;

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSrc, nullptr);
    glCompileShader(fragmentShader);
    if (!checkCompileErrors(fragmentShader, "FRAGMENT")) return false;

    m_shaderProgram = glCreateProgram();
    glAttachShader(m_shaderProgram, vertexShader);
    glAttachShader(m_shaderProgram, fragmentShader);
    glLinkProgram(m_shaderProgram);
    if (!checkCompileErrors(m_shaderProgram, "PROGRAM")) return false;

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    return true;
}

bool C3DViewer::checkCompileErrors(GLuint shader, const char* type) 
{
    GLint success;
    GLchar infoLog[1024];
    if (strcmp(type, "PROGRAM") != 0) 
    {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) 
        {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            fprintf(stderr, "ERROR::SHADER_COMPILATION_ERROR of type: %s\n%s\n", type, infoLog);
            return false;
        }
    }
    else 
    {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            fprintf(stderr, "ERROR::PROGRAM_LINKING_ERROR of type: %s\n%s\n", type, infoLog);
            return false;
        }
    }
    return true;
}

void C3DViewer::setupModel(C3DFigure* obj)
{
    m_currentModel = obj;
    vector<float> vertices = obj->flatten();
    
    m_vertexCount = static_cast<int>(vertices.size() / 6);

    if (m_vao == 0) glGenVertexArrays(1, &m_vao);
    if (m_vbo == 0) glGenBuffers(1, &m_vbo);

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    setupBoundingBox(obj->getBoundingBox());
}

void C3DViewer::setupTriangle()
{
    float vertices[] = 
    {
        -1.0f,  1.0f,  0.0f, 1.0f, 0.0f, 0.0f, 
         1.0f,  1.0f,  0.0f, 0.0f, 1.0f, 0.0f, 
         1.0f, -1.0f,  0.0f, 0.0f, 0.0f, 1.0f
    };

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);

    glBindVertexArray(m_vao);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
}

void C3DViewer::keyCallbackStatic(GLFWwindow* window, int key, int scancode, int action, int mods) 
{
    ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
    C3DViewer* self = (C3DViewer*)glfwGetWindowUserPointer(window);
    if (self) 
        self->onKey(key, scancode, action, mods);
}

void C3DViewer::mouseButtonCallbackStatic(GLFWwindow* window, int button, int action, int mods) 
{
    ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
    C3DViewer* self = (C3DViewer*)glfwGetWindowUserPointer(window);
    if (self)
        self->onMouseButton(button, action, mods);
}

void C3DViewer::cursorPosCallbackStatic(GLFWwindow* window, double xpos, double ypos) 
{
    ImGui_ImplGlfw_CursorPosCallback(window, xpos, ypos);
    C3DViewer* self = (C3DViewer*)glfwGetWindowUserPointer(window);
    if (self) 
        self->onCursorPos(xpos, ypos);
}

glm::vec3 C3DViewer::indexToColor(int index) {
    float r = (index + 1) / 255.0f; 
    return glm::vec3(r, 0.0f, 0.0f);
}

int C3DViewer::colorToIndex(unsigned char r, unsigned char g, unsigned char b) {
    if (r == 255 && g == 255 && b == 255) return -1;
    return (int)r - 1;
}

void C3DViewer::updateCameraVectors() 
{
    glm::vec3 front;
    front.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    front.y = sin(glm::radians(m_pitch));
    front.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    
    m_camFront = glm::normalize(front);
    m_camRight = glm::normalize(glm::cross(m_camFront, glm::vec3(0.0f, 1.0f, 0.0f)));
    m_camUp    = glm::normalize(glm::cross(m_camRight, m_camFront));
}

void C3DViewer::setupBoundingBox(BoundingBox box) {
    float eps = 0.005f;
    float minX = box.min.x - eps, minY = box.min.y - eps, minZ = box.min.z - eps;
    float maxX = box.max.x + eps, maxY = box.max.y + eps, maxZ = box.max.z + eps;

    float bboxVertices[] = {
        minX, minY, minZ,  maxX, minY, minZ,
        maxX, minY, minZ,  maxX, maxY, minZ,
        maxX, maxY, minZ,  minX, maxY, minZ,
        minX, maxY, minZ,  minX, minY, minZ,

        minX, minY, maxZ,  maxX, minY, maxZ,
        maxX, minY, maxZ,  maxX, maxY, maxZ,
        maxX, maxY, maxZ,  minX, maxY, maxZ,
        minX, maxY, maxZ,  minX, minY, maxZ,

        minX, minY, minZ,  minX, minY, maxZ,
        maxX, minY, minZ,  maxX, minY, maxZ,
        maxX, maxY, minZ,  maxX, maxY, maxZ,
        minX, maxY, minZ,  minX, maxY, maxZ
    };

    if (m_bboxVAO == 0) glGenVertexArrays(1, &m_bboxVAO);
    if (m_bboxVBO == 0) glGenBuffers(1, &m_bboxVBO);
    glBindVertexArray(m_bboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_bboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(bboxVertices), bboxVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
}
