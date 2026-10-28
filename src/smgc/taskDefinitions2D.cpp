#include <smgc/taskDefinitions2D.h>

task_SmgcDrawRect::task_SmgcDrawRect(const glm::vec2& t_pos, const glm::vec2& t_size, const glm::vec4& t_color): BaseTask(11), pos(t_pos), size(t_size), color(t_color) {}
bool taskIs_SmgcDrawRect(uint32_t taskIdx) {
    return taskIdx == 11;
}

task_SmgcDrawTriangle::task_SmgcDrawTriangle(const glm::vec2& t_a, const glm::vec2& t_b, const glm::vec2& t_c, const glm::vec4& t_color): BaseTask(12), a(t_a), b(t_b), c(t_c), color(t_color) {}
bool taskIs_SmgcDrawTriangle(uint32_t taskIdx) {
    return taskIdx == 12;
}

task_SmgcDrawCircle::task_SmgcDrawCircle(const glm::vec2& t_center, float t_radius, const glm::vec4& t_color): BaseTask(13), center(t_center), radius(t_radius), color(t_color) {}
bool taskIs_SmgcDrawCircle(uint32_t taskIdx) {
    return taskIdx == 13;
}

task_SmgcDrawEllipse::task_SmgcDrawEllipse(const glm::vec2& t_center, const glm::vec2& t_radii, const glm::vec4& t_color): BaseTask(14), center(t_center), radii(t_radii), color(t_color) {}
bool taskIs_SmgcDrawEllipse(uint32_t taskIdx) {
    return taskIdx == 14;
}

task_SmgcDrawRoundedRect::task_SmgcDrawRoundedRect(const glm::vec2& t_pos, const glm::vec2& t_size, float t_cornerRadius, const glm::vec4& t_color): BaseTask(15), pos(t_pos), size(t_size), cornerRadius(t_cornerRadius), color(t_color) {}
bool taskIs_SmgcDrawRoundedRect(uint32_t taskIdx) {
    return taskIdx == 15;
}

task_SmgcDrawRing::task_SmgcDrawRing(const glm::vec2& t_center, float t_radius, float t_thickness, const glm::vec4& t_color): BaseTask(16), center(t_center), radius(t_radius), thickness(t_thickness), color(t_color) {}
bool taskIs_SmgcDrawRing(uint32_t taskIdx) {
    return taskIdx == 16;
}

task_SmgcDrawLine::task_SmgcDrawLine(const glm::vec2& t_a, const glm::vec2& t_b, float t_thickness, const glm::vec4& t_color): BaseTask(17), a(t_a), b(t_b), thickness(t_thickness), color(t_color) {}
bool taskIs_SmgcDrawLine(uint32_t taskIdx) {
    return taskIdx == 17;
}
