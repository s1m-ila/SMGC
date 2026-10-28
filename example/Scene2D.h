#pragma once

#include <smpb/BaseComponent.h>
#include <smpb/BaseTask.h>

#include <smgc/display2D.h>

#include <unordered_map>

class Scene2D: public BaseComponent
{
public:
    Scene2D();
    int init() override;
    int processTask(std::unique_ptr<BaseTask> task) override;
    int tick(long long tickCounter) override;
    void death() override;
private:
    int ID_SMGC;

    bool isPressed(int key) const;
    void keyboard();

    std::unordered_map<int, bool> pressedKeys{};

    glm::vec2 standartPlayerPos{400.0f, 700.0f};
    glm::vec2 playerPos = standartPlayerPos;
    float playerSpeed = 300.0f;

    bool outlineEnabled = true;
    bool outlineKeyWasPressed = false;
};
