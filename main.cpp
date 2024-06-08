#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <errno.h>  // Utilisez <errno.h> au lieu de <cerrno>
#include <algorithm> // Pour std::max et std::min

// Fonction pour lire un fichier shader
std::string readFile(const char* filePath) {
    std::ifstream file(filePath);
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

// Fonction pour compiler un shader
GLuint compileShader(GLenum type, const std::string& source) {
    GLuint shader = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    int result;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE) {
        int length;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
        char* message = new char[length];
        glGetShaderInfoLog(shader, length, &length, message);
        std::cerr << "Failed to compile shader!" << std::endl;
        std::cerr << message << std::endl;
        delete[] message;
        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

// Fonction pour créer un programme shader
GLuint createShaderProgram(const std::string& vertexShader, const std::string& fragmentShader) {
    GLuint program = glCreateProgram();
    GLuint vs = compileShader(GL_VERTEX_SHADER, vertexShader);
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, fragmentShader);

    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glValidateProgram(program);

    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;
}

// Variables pour stocker les coordonnées de la souris
double mouseX, mouseY;

// Variables pour stocker les coordonnées de la souris lors de la pause
double pausedMouseX, pausedMouseY;

// Variable pour suivre l'état de pause
bool paused = false;

// Fonction de rappel pour les événements clavier
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
        paused = !paused;
        if (paused) {
            // Stocker les coordonnées de la souris lors de la pause
            glfwGetCursorPos(window, &pausedMouseX, &pausedMouseY);
        }
    }
}

int main() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // Obtenir le moniteur principal
    GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);

    // Créer une fenêtre en plein écran
    GLFWwindow* window = glfwCreateWindow(mode->width, mode->height, "OpenGL Shader Example", primaryMonitor, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return -1;
    }

    // Définir la fonction de rappel pour les événements clavier
    glfwSetKeyCallback(window, keyCallback);

    // Lire les shaders depuis les fichiers
    std::string vertexShader = readFile("shaders/vertex_shader.glsl");
    std::string fragmentShader = readFile("shaders/fragment_shader.glsl");

    GLuint shaderProgram = createShaderProgram(vertexShader, fragmentShader);

    float vertices[] = {
        -1.0f, -1.0f, 0.0f,
         1.0f, -1.0f, 0.0f,
         1.0f,  1.0f, 0.0f,
        -1.0f,  1.0f, 0.0f
    };

    GLuint indices[] = {
        0, 1, 2,
        2, 3, 0
    };

    GLuint vao, vbo, ebo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Obtenir les locations des uniformes
    GLint iResolutionLocation = glGetUniformLocation(shaderProgram, "iResolution");
    GLint iTimeLocation = glGetUniformLocation(shaderProgram, "iTime");
    GLint iMouseLocation = glGetUniformLocation(shaderProgram, "iMouse");

    float timeOffset = 0.0f;

    while (!glfwWindowShouldClose(window)) {
        if (!paused) {
            // Obtenir les coordonnées de la souris
            glfwGetCursorPos(window, &mouseX, &mouseY);

            // Limiter la coordonnée Y de la souris
            mouseY = std::max(0.1 * mode->height, std::min(mouseY, 0.9 * mode->height));
        } else {
            // Utiliser les coordonnées de la souris lors de la pause
            mouseX = pausedMouseX;
            mouseY = pausedMouseY;
        }

        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);

        // Envoyer les uniformes
        glUniform2f(iResolutionLocation, mode->width, mode->height);

        if (!paused) {
            glUniform1f(iTimeLocation, (float)glfwGetTime() - timeOffset);
        } else {
            timeOffset += (float)glfwGetTime() - timeOffset;
        }

        glUniform2f(iMouseLocation, (float)mouseX, (float)(mode->height - mouseY)); // Coordonnées de la souris avec origine en bas à gauche

        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
