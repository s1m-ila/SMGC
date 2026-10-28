#pragma once

// Base component
#include <smpb/BaseComponent.h>

// Base task
#include <smpb/BaseTask.h>

// Для работы со строками, вывода в консоль и других базовых операций
#include <iostream>
#include <format>

// For unique_ptr
#include <memory>

// Графика
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// Vec math
#include <glm/glm.hpp>

// Opengl classes
#include "shaderClass.h"
#include "VAO.h"
#include "VBO.h"
#include "EBO.h"

// ObjectsBuffer and Object classes
#include "ObjectsBuffer.h"

// BEATIFUL!
#include "paletts.h"

// Task definitions
#include "taskDefinitions.h"
#include "taskDefinitions2D.h"

#include "Layer2D.h"

// Objects Generator
#include "ObjectsGenerator.h"

#include "Camera.h"

#include "KeySubscription.h"

class SMGC: public BaseComponent
{
public:
    SMGC();
    int init() override;
    void windowResize();
    void swapObjects();
    void clearObjects();
    void pushObject(std::unique_ptr<Object> obj, const VertexAttributes& attributes);
    void setOutline(bool enabled);
    void setShadows(bool enabled);
    int setMaxDist(float newMaxDist);
    void setCamera(const Camera& newCamera);
    int drawText(const std::string& text, const glm::vec2& pos, float size, const glm::vec4& color, TextAlign align);
    int setFont(const std::string& path);
    int subscribeKey(int componentIdx, int key);
    int unsubscribeKey(int componentIdx, int key);
    void sendKeyStates();
    int draw();
    int processTask(std::unique_ptr<BaseTask> task) override;
    int tick(long long tickCounter) override;
    void death() override;
private:
    GLFWwindow* window = nullptr;
    long long localTickCounter = 0;
    
    std::unique_ptr<Shader> shaderProgram = nullptr;

    std::unique_ptr<VAO> vao = nullptr;
    std::unique_ptr<VBO> vbo = nullptr;
    std::unique_ptr<EBO> ebo = nullptr;

    std::vector<GpuObject> Objects_A{};
    std::vector<GpuObject> Objects_B{};
    std::vector<GpuObject> objectsUpload{};

    std::vector<Vertex> Vertices_A{};
    std::vector<Vertex> Vertices_B{};
    std::vector<GLuint> Indices_A{};
    std::vector<GLuint> Indices_B{};

    std::unique_ptr<ObjectsBuffer> objectsBuffer = nullptr;

    Layer2D overlay{};

    Camera camera{{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}};

    std::vector<KeySubscription> keySubscriptions{};

    bool outlineEnabled = true;
    bool shadowsEnabled = true;
    float maxDist = 50.0f;
    float epsilon = 0.0005f;

    int windowWidth  = 800;
    int windowHeight = 800;
};