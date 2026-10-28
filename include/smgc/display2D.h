#pragma once

#include <smpb/BaseComponent.h>
#include <smpb/BaseTask.h>

#include <iostream>
#include <memory>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

#include "Layer2D.h"

#include "paletts.h"

#include "taskDefinitions.h"
#include "taskDefinitions2D.h"

#include "KeySubscription.h"

class SMGC2D: public BaseComponent
{
public:
    SMGC2D();
    int init() override;
    void swapShapes();
    void clearShapes();
    int drawRect(const glm::vec2& pos, const glm::vec2& size, const glm::vec4& color);
    int drawTriangle(const glm::vec2& a, const glm::vec2& b, const glm::vec2& c, const glm::vec4& color);
    int drawCircle(const glm::vec2& center, float radius, const glm::vec4& color);
    int drawEllipse(const glm::vec2& center, const glm::vec2& radii, const glm::vec4& color);
    int drawRoundedRect(const glm::vec2& pos, const glm::vec2& size, float cornerRadius, const glm::vec4& color);
    int drawRing(const glm::vec2& center, float radius, float thickness, const glm::vec4& color);
    int drawLine(const glm::vec2& a, const glm::vec2& b, float thickness, const glm::vec4& color);
    int drawText(const std::string& text, const glm::vec2& pos, float size, const glm::vec4& color, TextAlign align);
    int setFont(const std::string& path);
    void setOutline(bool enabled);
    int subscribeKey(int componentIdx, int key);
    int unsubscribeKey(int componentIdx, int key);
    void sendKeyStates();
    int draw();
    int processTask(std::unique_ptr<BaseTask> task) override;
    int tick(long long tickCounter) override;
    void death() override;
private:
    int invalidArgument(const char* function, const char* name, float value);

    GLFWwindow* window = nullptr;

    Layer2D layer{};

    std::vector<KeySubscription> keySubscriptions{};

    bool outlineEnabled = true;

    int windowWidth  = 800;
    int windowHeight = 800;
};
