#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <filesystem>
#include "../include/tiny_obj_loader.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace fs = std::filesystem;

// Shader sources
const char* vertexShaderSource = R"(
#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 FragPos;
out vec3 Normal;

void main() {
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;
    gl_Position = projection * view * vec4(FragPos, 1.0);
}
)";

const char* fragmentShaderSource = R"(
#version 330 core
in vec3 FragPos;
in vec3 Normal;

out vec4 FragColor;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;

// Material properties
uniform vec3 ambientColor;
uniform vec3 diffuseColor;
uniform vec3 specularColor;
uniform float shininess;

// Default material colors
uniform vec3 defaultAmbientColor = vec3(0.1, 0.1, 0.1);
uniform vec3 defaultDiffuseColor = vec3(0.8, 0.8, 0.8);
uniform vec3 defaultSpecularColor = vec3(0.5, 0.5, 0.5);
uniform float defaultShininess = 32.0;

void main() {
    vec3 ambient = ambientColor;
    vec3 diffuse = diffuseColor;
    vec3 specular = specularColor;
    float shiny = shininess;

    if (ambient == vec3(0.0) && diffuse == vec3(0.0) && specular == vec3(0.0) && shiny == 0.0) {
        ambient = defaultAmbientColor;
        diffuse = defaultDiffuseColor;
        specular = defaultSpecularColor;
        shiny = defaultShininess;
    }

    // Ambient
    vec3 ambientLight = ambient * lightColor;

    // Diffuse
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuseLight = diff * diffuse * lightColor;

    // Specular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shiny);
    vec3 specularLight = spec * specular * lightColor;

    vec3 result = ambientLight + diffuseLight + specularLight;
    FragColor = vec4(result, 1.0);
}
)";

// Utility function to compile a shader
GLuint compileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "Error compiling shader: " << infoLog << std::endl;
    }

    return shader;
}

// Utility function to create a shader program
GLuint createShaderProgram(const char* vertexSource, const char* fragmentSource) {
    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexSource);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentSource);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    int success;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cerr << "Error linking shader program: " << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

// Load OBJ file using TinyObjLoader
bool loadOBJ(const char* path, std::vector<float>& vertices, std::vector<float>& normals, std::vector<tinyobj::material_t>& materials) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::string warn, err;

    // Ajout des messages de débogage
    std::cout << "Loading OBJ file: " << path << std::endl;

    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path, fs::path(path).parent_path().string().c_str());
    if (!warn.empty()) std::cout << "WARN: " << warn << std::endl;
    if (!err.empty()) std::cerr << "ERR: " << err << std::endl;
    if (!ret) return false;

    if (materials.empty()) {
        std::cerr << "WARN: No materials found. Default material will be used." << std::endl;
    } else {
        std::cout << "Materials loaded: " << materials.size() << std::endl;
        for (const auto& material : materials) {
            std::cout << "Material name: " << material.name << std::endl;
        }
    }

    for (const auto& shape : shapes) {
        for (const auto& index : shape.mesh.indices) {
            vertices.push_back(attrib.vertices[3 * index.vertex_index + 0]);
            vertices.push_back(attrib.vertices[3 * index.vertex_index + 1]);
            vertices.push_back(attrib.vertices[3 * index.vertex_index + 2]);
            normals.push_back(attrib.normals[3 * index.normal_index + 0]);
            normals.push_back(attrib.normals[3 * index.normal_index + 1]);
            normals.push_back(attrib.normals[3 * index.normal_index + 2]);
        }
    }

    return true;
}

// Camera control variables
bool firstMouse = true;
float lastX = 400, lastY = 300;
float yaw = -90.0f, pitch = 0.0f;
float radius = 3.0f;
float cameraX = 0.0f, cameraY = 0.0f, cameraZ = radius;

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    static float lastX = 400, lastY = 300;
    static bool firstMouse = true;
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    cameraX = radius * cos(glm::radians(pitch)) * cos(glm::radians(yaw));
    cameraY = radius * sin(glm::radians(pitch));
    cameraZ = radius * cos(glm::radians(pitch)) * sin(glm::radians(yaw));
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    radius -= yoffset;
    if (radius < 1.0f) radius = 1.0f;

    cameraX = radius * cos(glm::radians(pitch)) * cos(glm::radians(yaw));
    cameraY = radius * sin(glm::radians(pitch));
    cameraZ = radius * cos(glm::radians(pitch)) * sin(glm::radians(yaw));
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <OBJ file name>" << std::endl;
        return -1;
    }

    const char* objFileName = argv[1];
    std::string basePath = "../src/ressources/obj/";
    std::string objPath = basePath + objFileName;

    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // Create a windowed mode window and its OpenGL context
    GLFWwindow* window = glfwCreateWindow(800, 600, "OpenGL OBJ Loader", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Make the window's context current
    glfwMakeContextCurrent(window);

    // Set the mouse callback
    glfwSetCursorPosCallback(window, mouse_callback);

    // Set the scroll callback
    glfwSetScrollCallback(window, scroll_callback);

    // Capture the mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Initialize GLEW
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return -1;
    }

    // Build and compile our shader program
    GLuint shaderProgram = createShaderProgram(vertexShaderSource, fragmentShaderSource);

    // Load the OBJ file
    std::vector<float> vertices;
    std::vector<float> normals;
    std::vector<tinyobj::material_t> materials;
    if (!loadOBJ(objPath.c_str(), vertices, normals, materials)) {
        std::cerr << "Failed to load OBJ file" << std::endl;
        return -1;
    }

    // Create VAO, VBO
    GLuint VAO, VBO, NBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &NBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, NBO);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(float), normals.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Configure global OpenGL state
    glEnable(GL_DEPTH_TEST);

    // Render loop
    while (!glfwWindowShouldClose(window)) {
        // Input
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        // Render
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Activate shader
        glUseProgram(shaderProgram);

        // Create transformations
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::rotate(model, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)); // Rotate 180 degrees around the X axis
        glm::mat4 view = glm::lookAt(glm::vec3(cameraX, cameraY, cameraZ), glm::vec3(0.0f, 0.2f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);

        // Set uniforms
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

        glUniform3f(glGetUniformLocation(shaderProgram, "lightPos"), 1.2f, 1.0f, 2.0f);
        glUniform3f(glGetUniformLocation(shaderProgram, "viewPos"), cameraX, cameraY, cameraZ);
        glUniform3f(glGetUniformLocation(shaderProgram, "lightColor"), 1.0f, 1.0f, 1.0f);

        // Set material properties if available, otherwise use default
        if (!materials.empty()) {
            tinyobj::material_t mat = materials[0];
            glUniform3f(glGetUniformLocation(shaderProgram, "ambientColor"), mat.ambient[0], mat.ambient[1], mat.ambient[2]);
            glUniform3f(glGetUniformLocation(shaderProgram, "diffuseColor"), mat.diffuse[0], mat.diffuse[1], mat.diffuse[2]);
            glUniform3f(glGetUniformLocation(shaderProgram, "specularColor"), mat.specular[0], mat.specular[1], mat.specular[2]);
            glUniform1f(glGetUniformLocation(shaderProgram, "shininess"), mat.shininess);
        } else {
            glUniform3f(glGetUniformLocation(shaderProgram, "ambientColor"), 0.1f, 0.1f, 0.1f);
            glUniform3f(glGetUniformLocation(shaderProgram, "diffuseColor"), 0.8f, 0.8f, 0.8f);
            glUniform3f(glGetUniformLocation(shaderProgram, "specularColor"), 0.5f, 0.5f, 0.5f);
            glUniform1f(glGetUniformLocation(shaderProgram, "shininess"), 32.0f);
        }

        // Render the OBJ
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, vertices.size() / 3);
        glBindVertexArray(0);

        // Swap buffers
        glfwSwapBuffers(window);

        // Poll for and process events
        glfwPollEvents();
    }

    // Cleanup
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &NBO);

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
