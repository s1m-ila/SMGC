#include <smgc/taskDefinitions.h>

task_SmgcSwapObjects::task_SmgcSwapObjects(): BaseTask(0) {}
bool taskIs_SmgcSwapObjects(uint32_t taskIdx) {
    return taskIdx == 0;
}

task_SmgcClearObjects::task_SmgcClearObjects(): BaseTask(1) {}
bool taskIs_SmgcClearObjects(uint32_t taskIdx) {
    return taskIdx == 1;
}

task_SmgcPushObject::task_SmgcPushObject(std::unique_ptr<Object> t_obj, const VertexAttributes& t_attributes): BaseTask(2), attributes(t_attributes) {
    obj = std::move(t_obj);
}
bool taskIs_SmgcPushObject(uint32_t taskIdx) {
    return taskIdx == 2;
}

task_SmgcSetOutline::task_SmgcSetOutline(bool t_enabled): BaseTask(3), enabled(t_enabled) {}
bool taskIs_SmgcSetOutline(uint32_t taskIdx) {
    return taskIdx == 3;
}

task_SmgcSetMaxDist::task_SmgcSetMaxDist(float t_maxDist): BaseTask(4), maxDist(t_maxDist) {}
bool taskIs_SmgcSetMaxDist(uint32_t taskIdx) {
    return taskIdx == 4;
}

task_SmgcSetCamera::task_SmgcSetCamera(const Camera& t_camera): BaseTask(5), camera(t_camera) {}
bool taskIs_SmgcSetCamera(uint32_t taskIdx) {
    return taskIdx == 5;
}

task_SmgcSubscribeKey::task_SmgcSubscribeKey(int t_key): BaseTask(6), key(t_key) {}
bool taskIs_SmgcSubscribeKey(uint32_t taskIdx) {
    return taskIdx == 6;
}

task_SmgcUnsubscribeKey::task_SmgcUnsubscribeKey(int t_key): BaseTask(7), key(t_key) {}
bool taskIs_SmgcUnsubscribeKey(uint32_t taskIdx) {
    return taskIdx == 7;
}

task_SmgcKeyState::task_SmgcKeyState(int t_key, bool t_pressed): BaseTask(8), key(t_key), pressed(t_pressed) {}
bool taskIs_SmgcKeyState(uint32_t taskIdx) {
    return taskIdx == 8;
}

task_SmgcSetShadows::task_SmgcSetShadows(bool t_enabled): BaseTask(9), enabled(t_enabled) {}
bool taskIs_SmgcSetShadows(uint32_t taskIdx) {
    return taskIdx == 9;
}
