#include "Scene3D.h"

// Core class
#include <smpb/Core.h>

#include <array>
#include <format>

static constexpr std::array<int, 12> kKeys = {
    GLFW_KEY_LEFT, GLFW_KEY_RIGHT, GLFW_KEY_UP, GLFW_KEY_DOWN,
    GLFW_KEY_W, GLFW_KEY_A, GLFW_KEY_S, GLFW_KEY_D,
    GLFW_KEY_Q, GLFW_KEY_E, GLFW_KEY_C, GLFW_KEY_R,
};

Scene3D::Scene3D() : BaseComponent("Scene3D") {
    // Constructor
}

int Scene3D::init() {
    ID_SMGC = Core_getComponentIdxByName("SMGC");
    for (int key : kKeys) {
        int errorCode = sendTask(std::make_unique<task_SmgcSubscribeKey>(key), ID_SMGC);
        if (errorCode != 0) return errorCode;
    }
    return 0;
}

int Scene3D::processTask(std::unique_ptr<BaseTask> task) {
    if (taskIs_SmgcKeyState(task->getIdx())) {
        auto* typed = static_cast<task_SmgcKeyState*>(task.get());
        pressedKeys[typed->key] = typed->pressed;
        return 0;
    }
    return -EINVAL;
}

bool Scene3D::isPressed(int key) const {
    auto it = pressedKeys.find(key);
    return it != pressedKeys.end() && it->second;
}

void Scene3D::keyboard(long long tickCounter) {
    float dt = 1.0f / float(Core_getTPS());

    if (isPressed(GLFW_KEY_LEFT))  camera.rot.y += rotSpeed * dt;
    if (isPressed(GLFW_KEY_RIGHT)) camera.rot.y -= rotSpeed * dt;
    if (isPressed(GLFW_KEY_UP))    camera.rot.x += rotSpeed * dt;
    if (isPressed(GLFW_KEY_DOWN))  camera.rot.x -= rotSpeed * dt;
    
    // ограничение pitch, чтобы не было переворота (gimbal-lock)
    camera.rot.x = std::clamp(camera.rot.x, -89.0f, 89.0f);
    
    // ограничение yaw чтобы не было бесконечного увеличения числа
    camera.rot.y = std::fmod(camera.rot.y, 360.0f);

    // пересчитываем углы
    float yaw   = glm::radians(camera.rot.y);
    float pitch = glm::radians(camera.rot.x);

    // --- направления ---
    glm::vec3 forward = {0.0f, 0.0f, 0.0f};
    forward.x = -cos(pitch) * sin(yaw);
    forward.y =  sin(pitch);
    forward.z =  cos(pitch) * cos(yaw);

    glm::vec3 right;
    right.x = cos(yaw);
    right.y = 0.0f;
    right.z = sin(yaw);

    // --- ДВИЖЕНИЕ (WASD) ---
    float step = moveSpeed * dt;

    if (isPressed(GLFW_KEY_W)) camera.pos += forward * step;
    if (isPressed(GLFW_KEY_S)) camera.pos -= forward * step;
    if (isPressed(GLFW_KEY_D)) camera.pos += right   * step;
    if (isPressed(GLFW_KEY_A)) camera.pos -= right   * step;


    if (isPressed(GLFW_KEY_C)) {
        camera.rot.x = round(camera.rot.x / 45.0f) * 45.0f;
        camera.rot.y = round(camera.rot.y / 45.0f) * 45.0f;
        camera.rot.z = round(camera.rot.z / 45.0f) * 45.0f;
        camera.pos.x = floor(camera.pos.x) + 0.5;
        camera.pos.y = floor(camera.pos.y) + 0.5;
        camera.pos.z = floor(camera.pos.z) + 0.5;
    }

    if (isPressed(GLFW_KEY_R)) {
        camera.rot = {0.0f, 0.0f, 0.0f};
        camera.pos = {0.0f, 0.0f, 0.0f};
        moveSpeed = standartMoveSpeed;
        rotSpeed = standartRotSpeed;
    }

    if (tickCounter % 10 == 0) {
        if (isPressed(GLFW_KEY_Q)) {
            rotSpeed /= 2.0f;
            moveSpeed /= 2.0f;
        }
        if (isPressed(GLFW_KEY_E)) {
            rotSpeed *= 2.0f;
            moveSpeed *= 2.0f;
        }
    }
}

int Scene3D::tick(long long tickCounter) {
    keyboard(tickCounter);
    sendTask(std::make_unique<task_SmgcSetCamera>(camera), ID_SMGC);

    sendTask(std::make_unique<task_SmgcClearObjects>(), ID_SMGC);
    
    float time = float(tickCounter) / float(Core_getTPS());
    float m = (sin(time * 2.0f)) * 0.5f + 0.5f;
    glm::vec3 col = PaletteCosinka(m);
    glm::vec3 col2 = PaletteCosinka(std::fmod(time, 1.0));
    float rot = std::fmod(time, 360.0f);
    sendTask(
        std::make_unique<task_SmgcPushObject>(
            std::make_unique<Object>(
                glm::vec4{1.0f, m, m, 0.75f},
                glm::vec4{0.5f, 0.0f, 0.0f, 0.0f},
                glm::vec4{col.x, col.y, col.z, 1.0f},
                glm::vec4{0.0f, 0.0f, 0.0f, 0.0f}
            )
        ),
        ID_SMGC
    );
    for (int i = -2; i < 2; i += 2) {
        sendTask(
            std::make_unique<task_SmgcPushObject>(
                std::make_unique<Object>(
                    glm::vec4{2.0f, float(i), 0.0f, 2.25f},
                    glm::vec4{0.5f, 0.0f, 0.0f, 0.0f},
                    glm::vec4{col2.x, col2.y, col2.z, 1.0f},
                    glm::vec4{rot, rot, rot, 0.0f}
                )
            ),
            ID_SMGC
        );
    }
    sendTask(
        std::make_unique<task_SmgcPushObject>(
            std::make_unique<Object>(
                glm::vec4{3.0f, 0.0f, 0.75f, 3.75f},
                glm::vec4{0.5f, 0.0f, 0.0f, 1.0f},
                glm::vec4{col2.x, col2.y, col2.z, 1.0f},
                glm::vec4{rot, rot, rot, 0.0f}
            )
        ),
        ID_SMGC
    );
    sendTask(
        std::make_unique<task_SmgcPushObject>(
            std::make_unique<Object>(
                glm::vec4{-1.0f, 0.0f, 0.0f, -0.75f},
                glm::vec4{0.5f, 0.0f, 0.0f, 0.0f},
                glm::vec4{0.75f, 0.75f, 0.75f, 1.0f},
                glm::vec4{0.0f, 0.0f, 0.0f, 0.0f}
            )
        ),
        ID_SMGC
    );
    sendTask(
        std::make_unique<task_SmgcPushObject>(
            std::make_unique<Object>(
                glm::vec4{-2.0f, 0.0f, 0.0f, -2.25f},
                glm::vec4{0.5f, 0.0f, 0.0f, 0.0f},
                glm::vec4{1.0f, 1.0f, 0.4f, 1.0f},
                glm::vec4{0.0f, 0.0f, 0.0f, 0.0f}
            )
        ),
        ID_SMGC
    );
    
    sendTask(
        std::make_unique<task_SmgcDrawText>(
            std::format("pos: {:.2f} {:.2f} {:.2f}\nrot: {:.1f} {:.1f} {:.1f}\nскорость: {:.2f}",
                camera.pos.x, camera.pos.y, camera.pos.z, camera.rot.x, camera.rot.y, camera.rot.z, moveSpeed),
            glm::vec2{10.0f, 10.0f}, 18.0f, glm::vec4{1.0f, 1.0f, 1.0f, 1.0f}
        ),
        ID_SMGC
    );

    sendTask(std::make_unique<task_SmgcSwapObjects>(), ID_SMGC);
    return 0;
}

void Scene3D::death() {
    for (int key : kKeys) sendTask(std::make_unique<task_SmgcUnsubscribeKey>(key), ID_SMGC);
}