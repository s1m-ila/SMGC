#pragma once

// Base task
#include <smpb/BaseTask.h>

// Object class
#include "ObjectsBuffer.h"
#include "Camera.h"
#include "VBO.h"

// For unique_ptr
#include <memory>

class task_SmgcSwapObjects: public BaseTask {
public:
    task_SmgcSwapObjects();
};
bool taskIs_SmgcSwapObjects(uint32_t taskIdx);

class task_SmgcClearObjects: public BaseTask {
public:
    task_SmgcClearObjects();
};
bool taskIs_SmgcClearObjects(uint32_t taskIdx);

class task_SmgcPushObject: public BaseTask {
public:
    task_SmgcPushObject(std::unique_ptr<Object> t_obj, const VertexAttributes& t_attributes = {});
    std::unique_ptr<Object> obj = nullptr;
    VertexAttributes attributes{};
};
bool taskIs_SmgcPushObject(uint32_t taskIdx);

class task_SmgcSetOutline: public BaseTask {
public:
    task_SmgcSetOutline(bool t_enabled);
    bool enabled = true;
};
bool taskIs_SmgcSetOutline(uint32_t taskIdx);

class task_SmgcSetMaxDist: public BaseTask {
public:
    task_SmgcSetMaxDist(float t_maxDist);
    float maxDist = 0.0f;
};
bool taskIs_SmgcSetMaxDist(uint32_t taskIdx);

class task_SmgcSetCamera: public BaseTask {
public:
    task_SmgcSetCamera(const Camera& t_camera);
    Camera camera{};
};
bool taskIs_SmgcSetCamera(uint32_t taskIdx);

class task_SmgcSubscribeKey: public BaseTask {
public:
    task_SmgcSubscribeKey(int t_key);
    int key = 0;
};
bool taskIs_SmgcSubscribeKey(uint32_t taskIdx);

class task_SmgcUnsubscribeKey: public BaseTask {
public:
    task_SmgcUnsubscribeKey(int t_key);
    int key = 0;
};
bool taskIs_SmgcUnsubscribeKey(uint32_t taskIdx);

class task_SmgcKeyState: public BaseTask {
public:
    task_SmgcKeyState(int t_key, bool t_pressed);
    int key = 0;
    bool pressed = false;
};
bool taskIs_SmgcKeyState(uint32_t taskIdx);

class task_SmgcSetShadows: public BaseTask {
public:
    task_SmgcSetShadows(bool t_enabled);
    bool enabled = true;
};
bool taskIs_SmgcSetShadows(uint32_t taskIdx);
