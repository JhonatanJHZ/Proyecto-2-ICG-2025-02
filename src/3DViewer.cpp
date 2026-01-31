#include "3DViewer.h"
#include <iostream>
#include "utils/3DFigure.h"

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

    // Inicializar glad
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) 
    {
        glfwDestroyWindow(m_window);
        glfwTerminate();
        return false;
    }
    
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Opcional

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(m_window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");
    
    glfwSetWindowUserPointer(m_window, this);
    glfwSetFramebufferSizeCallback(m_window, [](GLFWwindow* window, int w, int h) {
        auto ptr = reinterpret_cast<C3DViewer*>(glfwGetWindowUserPointer(window));
        if (ptr) 
            ptr->resize(w, h);
    });

    // Setup shader
    if (!setupShader()) return false;

    // Setup VAO y VBO para el tringulo
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

    // Movimiento adelante/atras con UP/DOWN
    if (glfwGetKey(m_window, GLFW_KEY_UP) == GLFW_PRESS)
        m_camPos += cameraSpeed * m_camFront;
    if (glfwGetKey(m_window, GLFW_KEY_DOWN) == GLFW_PRESS)
        m_camPos -= cameraSpeed * m_camFront;

    // Rotacin Cmara LEFT/RIGHT (Yaw)
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

        // color de borrado
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

        // borrando bferes
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        render();

        glfwSwapBuffers(m_window);
    }
}

// Debugging Inputs
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
            if (button == 0 || button == 1 || button == 2) {
                glfwGetCursorPos(m_window, &lastMouseX, &lastMouseY);
            }
        }
        else if (action == GLFW_RELEASE)
        {
            mouseButtonsDown[button] = false;
        }
    }
}

void C3DViewer::onCursorPos(double xpos, double ypos) 
{
    if (mouseButtonsDown[0] || mouseButtonsDown[1] || mouseButtonsDown[2]) 
    {
        // std::cout << "[DEBUG] Mouse Drag at " << xpos << ", " << ypos << "\n";
    }

    if (mouseButtonsDown[0]) 
    {
        // Traslacin Modelo
        float deltaX = (float)(xpos - lastMouseX);
        float deltaY = (float)(ypos - lastMouseY);
        float sensitivity = 0.005f; 

        m_modelPos.x += deltaX * sensitivity;
        m_modelPos.y -= deltaY * sensitivity;
        lastMouseX = xpos;
        lastMouseY = ypos;
    }
    else if(mouseButtonsDown[1]){
        // Rotacin Modelo (Quaternion)
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
        // Rotacin Cmara (Mouse Look)
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
    // 1. Preparación del frame
    update();
    
    // ESENCIAL: Limpiar el fondo Y el buffer de profundidad (Z-buffer)
    // Si no limpias GL_DEPTH_BIT, el 3D se verá "encimado" o plano.
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f); 
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 

    // --- AJUSTE DEL VIEWPORT ---
    // Usamos la variable miembro panelWidth
    // El dibujo comienza en x = panelWidth, ocupando el resto del ancho
    glViewport((int)panelWidth, 0, width - (int)panelWidth, height);

    // 2. Activar Shaders
    glUseProgram(m_shaderProgram);

    // 3. Configuración de Matrices (MVP)
    // --- AJUSTE DEL ASPECT RATIO ---
    if (height == 0) height = 1; 
    float aspect = (float)(width - panelWidth) / (float)height;
    mat4 projection = perspective(radians(45.0f), aspect, 0.1f, 100.0f);

    // VISTA: Cámara en el origen mirando hacia el infinito negativo (-Z)
    mat4 view = lookAt(m_camPos, m_camPos + m_camFront, m_camUp);

    // MODELO: Trasladar el cochino (ya normalizado y centrado en 0) a Z = -3
    mat4 model = translate(mat4(1.0f), m_modelPos);
    
    // Rotacion Quat
    model = model * mat4_cast(m_rotation); 
    
    // Escala
    model = scale(model, m_userScale * scale_factor);

    // Combinación de matrices
    mat4 mvp = projection * view * model;

    // 4. Envío de la matriz al Shader
    GLuint mvpLoc = glGetUniformLocation(m_shaderProgram, "u_mvp");
    if (mvpLoc != -1) {
        glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, value_ptr(mvp));
    }

    // 5. Dibujo del Objeto
    glBindVertexArray(m_vao);
    
    /* TRUCO PARA VER EL 3D: 
       Si el objeto se ve de un solo color sólido, activa esta línea 
       para ver la malla de triángulos (Wireframe).
    */
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); 
    
    glDrawArrays(GL_TRIANGLES, 0, m_vertexCount);
    
    // glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // Regresar a sólido
    
    // Restaurar viewport para ImGui (pantalla completa)
    glViewport(0, 0, width, height);

    // 6. Interfaz (ImGui u otros)
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

    // Usamos la variable miembro panelWidth
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
    
    ImGui::End();

    ImGui::Render();
    // Rendirizar ImGui con OpenGL
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

    // actualizamos el viewport cada vez que haya un resize
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
    vector<float> vertices = obj->flatten();
    m_vertexCount = static_cast<int>(vertices.size() / 6);

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);

    glBindVertexArray(m_vao);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

void C3DViewer::setupTriangle()
{
    float vertices[] = 
    {
        // x      y      z     r     g     b 
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
