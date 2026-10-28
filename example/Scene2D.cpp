#include "Scene2D.h"

#include <smpb/Core.h>

#include <array>
#include <cmath>
#include <format>

static constexpr std::array<int, 6> kKeys = {
    GLFW_KEY_W, GLFW_KEY_A, GLFW_KEY_S, GLFW_KEY_D,
    GLFW_KEY_O, GLFW_KEY_R,
};

Scene2D::Scene2D() : BaseComponent("Scene2D") {
}

int Scene2D::init() {
    ID_SMGC = Core_getComponentIdxByName("SMGC2D");
    for (int key : kKeys) {
        int errorCode = sendTask(std::make_unique<task_SmgcSubscribeKey>(key), ID_SMGC);
        if (errorCode != 0) return errorCode;
    }
    return sendTask(std::make_unique<task_SmgcSetOutline>(outlineEnabled), ID_SMGC);
}

int Scene2D::processTask(std::unique_ptr<BaseTask> task) {
    if (taskIs_SmgcKeyState(task->getIdx())) {
        auto* typed = static_cast<task_SmgcKeyState*>(task.get());
        pressedKeys[typed->key] = typed->pressed;
        return 0;
    }
    return -EINVAL;
}

bool Scene2D::isPressed(int key) const {
    auto it = pressedKeys.find(key);
    return it != pressedKeys.end() && it->second;
}

void Scene2D::keyboard() {
    float step = playerSpeed / float(Core_getTPS());

    if (isPressed(GLFW_KEY_W)) playerPos.y -= step;
    if (isPressed(GLFW_KEY_S)) playerPos.y += step;
    if (isPressed(GLFW_KEY_A)) playerPos.x -= step;
    if (isPressed(GLFW_KEY_D)) playerPos.x += step;

    if (isPressed(GLFW_KEY_R)) playerPos = standartPlayerPos;

    bool outlineKeyPressed = isPressed(GLFW_KEY_O);
    if (outlineKeyPressed && !outlineKeyWasPressed) {
        outlineEnabled = !outlineEnabled;
        sendTask(std::make_unique<task_SmgcSetOutline>(outlineEnabled), ID_SMGC);
    }
    outlineKeyWasPressed = outlineKeyPressed;
}

int Scene2D::tick(long long tickCounter) {
    keyboard();

    sendTask(std::make_unique<task_SmgcClearObjects>(), ID_SMGC);

    float time = float(tickCounter) / float(Core_getTPS());
    float m = (sin(time * 2.0f)) * 0.5f + 0.5f;
    glm::vec3 col = PaletteCosinka(m);
    glm::vec3 col2 = PaletteCosinka(std::fmod(time, 1.0f));
    glm::vec3 col3 = PaletteCosinka2(m);

    sendTask(std::make_unique<task_SmgcDrawRect>(
        glm::vec2{50.0f, 50.0f}, glm::vec2{180.0f, 120.0f}, glm::vec4{col, 1.0f}), ID_SMGC);

    sendTask(std::make_unique<task_SmgcDrawRoundedRect>(
        glm::vec2{310.0f, 50.0f}, glm::vec2{180.0f, 120.0f}, 10.0f + 40.0f * m, glm::vec4{col2, 1.0f}), ID_SMGC);

    sendTask(std::make_unique<task_SmgcDrawEllipse>(
        glm::vec2{660.0f, 110.0f}, glm::vec2{40.0f + 50.0f * m, 90.0f - 50.0f * m}, glm::vec4{col3, 1.0f}), ID_SMGC);

    sendTask(std::make_unique<task_SmgcDrawCircle>(
        glm::vec2{140.0f, 330.0f}, 50.0f + 30.0f * m, glm::vec4{col2, 1.0f}), ID_SMGC);

    sendTask(std::make_unique<task_SmgcDrawRing>(
        glm::vec2{400.0f, 330.0f}, 85.0f, 5.0f + 30.0f * m, glm::vec4{col, 1.0f}), ID_SMGC);

    sendTask(std::make_unique<task_SmgcDrawTriangle>(
        glm::vec2{570.0f, 410.0f}, glm::vec2{660.0f, 250.0f}, glm::vec2{750.0f, 410.0f}, glm::vec4{col3, 1.0f}), ID_SMGC);

    glm::vec2 lineCenter{400.0f, 540.0f};
    glm::vec2 lineDir{std::cos(time), std::sin(time)};
    sendTask(std::make_unique<task_SmgcDrawLine>(
        lineCenter - lineDir * 90.0f, lineCenter + lineDir * 90.0f, 12.0f, glm::vec4{col, 1.0f}), ID_SMGC);
    for (int i = 0; i < 5; i++) {
        float x = 50.0f + float(i) * 45.0f;
        sendTask(std::make_unique<task_SmgcDrawLine>(
            glm::vec2{x, 470.0f}, glm::vec2{x + 30.0f, 610.0f}, float(i) * 2.0f, glm::vec4{1.0f, 1.0f, 1.0f, 1.0f}), ID_SMGC);
    }

    sendTask(std::make_unique<task_SmgcDrawCircle>(
        glm::vec2{620.0f, 540.0f}, 70.0f, glm::vec4{col2, 1.0f}), ID_SMGC);
    sendTask(std::make_unique<task_SmgcDrawRect>(
        glm::vec2{560.0f, 500.0f}, glm::vec2{180.0f, 80.0f}, glm::vec4{col3, 0.5f}), ID_SMGC);

    const glm::vec4 white{1.0f, 1.0f, 1.0f, 1.0f};
    const glm::vec4 gray{0.7f, 0.7f, 0.7f, 1.0f};
    sendTask(std::make_unique<task_SmgcDrawText>(
        "SMGC2D: текст / text, Ёё", glm::vec2{400.0f, 6.0f}, 28.0f, white, TextAlign::Center), ID_SMGC);

    const std::array<std::pair<glm::vec2, const char*>, 9> labels = {{
        {{140.0f, 205.0f}, "Rect"},     {{400.0f, 205.0f}, "RoundedRect"}, {{660.0f, 205.0f}, "Ellipse"},
        {{140.0f, 422.0f}, "Circle"},   {{400.0f, 422.0f}, "Ring"},        {{660.0f, 422.0f}, "Triangle"},
        {{155.0f, 625.0f}, "Line"},     {{400.0f, 635.0f}, "Line"},        {{650.0f, 625.0f}, "Alpha"},
    }};
    for (const auto& [pos, label] : labels)
        sendTask(std::make_unique<task_SmgcDrawText>(label, pos, 16.0f, gray, TextAlign::Center), ID_SMGC);

    sendTask(std::make_unique<task_SmgcDrawText>(
        "WASD — движение\nO — линии\nR — сброс", glm::vec2{790.0f, 715.0f}, 18.0f, white, TextAlign::Right), ID_SMGC);
    sendTask(std::make_unique<task_SmgcDrawText>(
        std::format("tick: {}\nx: {:.0f}  y: {:.0f}", tickCounter, playerPos.x, playerPos.y), glm::vec2{10.0f, 740.0f}, 18.0f, white, TextAlign::Left), ID_SMGC);
    sendTask(std::make_unique<task_SmgcDrawText>(
        "Привет!", glm::vec2{155.0f, 650.0f}, 20.0f + 20.0f * m, glm::vec4{col3, 1.0f}, TextAlign::Center), ID_SMGC);

    sendTask(std::make_unique<task_SmgcDrawCircle>(
        playerPos, 25.0f, glm::vec4{1.0f, 1.0f, 1.0f, 1.0f}), ID_SMGC);
    sendTask(std::make_unique<task_SmgcDrawRing>(
        playerPos, 35.0f, 3.0f, glm::vec4{1.0f, 1.0f, 1.0f, 0.6f}), ID_SMGC);

    sendTask(std::make_unique<task_SmgcSwapObjects>(), ID_SMGC);
    return 0;
}

void Scene2D::death() {
    for (int key : kKeys) sendTask(std::make_unique<task_SmgcUnsubscribeKey>(key), ID_SMGC);
}
