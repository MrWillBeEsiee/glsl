#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <errno.h>
#include <algorithm>
#include "../include/imgui.h"
#include "../include/imgui_impl_glfw.h"
#include "../include/imgui_impl_opengl3.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

// Inclure stb_image.h et définir STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "../include/stb_image.h"

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

// Variable globale pour le FOV
float fov = 55.0f; // Initialiser avec un FOV par défaut de 45 degrés

// Variable globale pour la position de l'objet
glm::vec3 objectPosition(0.0f, 0.5f, -1.0f);

// Variable globale pour la rotation de l'objet
float objectRotationX = 0.0f; // Initialiser avec une rotation de 0 degrés autour de X
float objectRotationY = 0.0f; // Initialiser avec une rotation de 0 degrés autour de Y
float objectRotationZ = 0.0f; // Initialiser avec une rotation de 0 degrés autour de Z

// Variables pour contrôler les post-traitements
bool vignetteEnabled = true;
bool gammaCorrectionEnabled = true;
bool sepiaEnabled = false;
bool hueShiftEnabled = false;

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

    // Créer une fenêtre de taille fixe
    GLFWwindow* window = glfwCreateWindow(800, 600, "OpenGL Shader Example", nullptr, nullptr);
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

    // Initialisation IMGUI
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // Définir la fonction de rappel pour les événements clavier
    glfwSetKeyCallback(window, keyCallback);

    // Lire les shaders depuis les fichiers
    std::string vertexShader = readFile("../src/shaders/vertex_shader.glsl");
    std::string fragmentShader = readFile("../src/shaders/fragment_shader.glsl");

    GLuint shaderProgram = createShaderProgram(vertexShader, fragmentShader);

    float vertices[] = {
        // positions          // texture coords
        -1.0f, -1.0f, 0.0f,  0.0f, 0.0f,
         1.0f, -1.0f, 0.0f,  1.0f, 0.0f,
         1.0f,  1.0f, 0.0f,  1.0f, 1.0f,
        -1.0f,  1.0f, 0.0f,  0.0f, 1.0f
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

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Texture coord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Charger la texture
    int width, height, nrChannels;
    unsigned char *data = stbi_load("../src/ressources/texture/pierre.jpg", &width, &height, &nrChannels, 0);
    if (!data) {
        std::cerr << "Failed to load texture" << std::endl;
        return -1;
    }

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    // Définir les paramètres de la texture
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Charger les données de l'image
    if (nrChannels == 3) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    } else if (nrChannels == 4) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    }

    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);

    // Obtenir les locations des uniformes
    GLint iResolutionLocation = glGetUniformLocation(shaderProgram, "iResolution");
    GLint iTimeLocation = glGetUniformLocation(shaderProgram, "iTime");
    GLint iMouseLocation = glGetUniformLocation(shaderProgram, "iMouse");
    GLint fovLocation = glGetUniformLocation(shaderProgram, "fov");
    GLint objectPositionLocation = glGetUniformLocation(shaderProgram, "objectPosition");
    GLint objectRotationXLocation = glGetUniformLocation(shaderProgram, "objectRotationX");
    GLint objectRotationYLocation = glGetUniformLocation(shaderProgram, "objectRotationY");
    GLint objectRotationZLocation = glGetUniformLocation(shaderProgram, "objectRotationZ");

    // Locations des uniformes pour les post-traitements
    GLint vignetteEnabledLocation = glGetUniformLocation(shaderProgram, "vignetteEnabled");
    GLint gammaCorrectionEnabledLocation = glGetUniformLocation(shaderProgram, "gammaCorrectionEnabled");
    GLint sepiaEnabledLocation = glGetUniformLocation(shaderProgram, "sepiaEnabled");
    GLint hueShiftEnabledLocation = glGetUniformLocation(shaderProgram, "hueShiftEnabled");

    float timeOffset = 0.0f;

    while (!glfwWindowShouldClose(window)) {
        if (!paused) {
            // Obtenir les coordonnées de la souris
            glfwGetCursorPos(window, &mouseX, &mouseY);

            // Limiter la coordonnée Y de la souris
            mouseY = std::max(0.1 * 600, std::min(mouseY, 0.9 * 600));
        } else {
            // Utiliser les coordonnées de la souris lors de la pause
            mouseX = pausedMouseX;
            mouseY = pausedMouseY;
        }

        // Rendu de la scène OpenGL
        glUseProgram(shaderProgram);
        glUniform2f(iResolutionLocation, 800, 600);
        if (!paused) {
            glUniform1f(iTimeLocation, (float)glfwGetTime() - timeOffset);
        } else {
            timeOffset += (float)glfwGetTime() - timeOffset;
        }
        glUniform2f(iMouseLocation, (float)mouseX, (float)(600 - mouseY)); // Coordonnées de la souris avec origine en bas à gauche
        glUniform1f(fovLocation, glm::radians(fov)); // Envoyer le FOV au shader
        glUniform3fv(objectPositionLocation, 1, glm::value_ptr(objectPosition)); // Envoyer la position de l'objet au shader
        glUniform1f(objectRotationXLocation, glm::radians(objectRotationX)); // Envoyer la rotation de l'objet autour de X au shader
        glUniform1f(objectRotationYLocation, glm::radians(objectRotationY)); // Envoyer la rotation de l'objet autour de Y au shader
        glUniform1f(objectRotationZLocation, glm::radians(objectRotationZ)); // Envoyer la rotation de l'objet autour de Z au shader

        // Envoyer les états des post-traitements aux shaders
        glUniform1i(vignetteEnabledLocation, vignetteEnabled);
        glUniform1i(gammaCorrectionEnabledLocation, gammaCorrectionEnabled);
        glUniform1i(sepiaEnabledLocation, sepiaEnabled);
        glUniform1i(hueShiftEnabledLocation, hueShiftEnabled);

        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // Rendu ImGui
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Créer une fenêtre ImGui pour contrôler le FOV, la position de l'objet et les post-traitements
        ImGui::Begin("Contrôles de la scène");
        ImGui::SliderFloat("FOV", &fov, 30.0f, 120.0f);
        ImGui::SliderFloat3("Position de l'objet", glm::value_ptr(objectPosition), -1.5f, 1.5f);
        ImGui::SliderFloat("Rotation de l'objet autour de X", &objectRotationX, 0.0f, 360.0f); // Ajouter un slider pour la rotation de l'objet autour de X
        ImGui::SliderFloat("Rotation de l'objet autour de Y", &objectRotationY, 0.0f, 360.0f); // Ajouter un slider pour la rotation de l'objet autour de Y
        ImGui::SliderFloat("Rotation de l'objet autour de Z", &objectRotationZ, 0.0f, 360.0f); // Ajouter un slider pour la rotation de l'objet autour de Z
        ImGui::Checkbox("Vignettage", &vignetteEnabled);
        ImGui::Checkbox("Correction Gamma", &gammaCorrectionEnabled);
        ImGui::Checkbox("Sepia", &sepiaEnabled);
        ImGui::Checkbox("Changement de Teinte", &hueShiftEnabled);
        ImGui::End();

        // Rendu ImGui
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);
    glDeleteTextures(1, &texture);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
