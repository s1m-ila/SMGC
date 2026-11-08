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

task_SmgcDrawText::task_SmgcDrawText(const std::string& t_text, const glm::vec2& t_pos, float t_size, const glm::vec4& t_color, TextAlign t_align): BaseTask(18), text(t_text), pos(t_pos), size(t_size), color(t_color), align(t_align) {}
bool taskIs_SmgcDrawText(uint32_t taskIdx) {
    return taskIdx == 18;
}

task_SmgcSetFont::task_SmgcSetFont(const std::string& t_path): BaseTask(19), path(t_path) {}
bool taskIs_SmgcSetFont(uint32_t taskIdx) {
    return taskIdx == 19;
}

task_SmgcMeasureText::task_SmgcMeasureText(const std::string& t_text, float t_size, float* t_width, float* t_height)
    : BaseTask(21), text(t_text), size(t_size), width(t_width), height(t_height) {}
bool taskIs_SmgcMeasureText(uint32_t taskIdx) {
    return taskIdx == 21;
}

task_SmgcLoadTexture::task_SmgcLoadTexture(const std::string& t_path, uint32_t* t_id): BaseTask(22), path(t_path), id(t_id) {}
bool taskIs_SmgcLoadTexture(uint32_t taskIdx) {
    return taskIdx == 22;
}

task_SmgcFreeTexture::task_SmgcFreeTexture(uint32_t t_id): BaseTask(23), id(t_id) {}
bool taskIs_SmgcFreeTexture(uint32_t taskIdx) {
    return taskIdx == 23;
}

task_SmgcGetTextureSize::task_SmgcGetTextureSize(uint32_t t_id, int* t_width, int* t_height): BaseTask(24), id(t_id), width(t_width), height(t_height) {}
bool taskIs_SmgcGetTextureSize(uint32_t taskIdx) {
    return taskIdx == 24;
}

task_SmgcCloneTexture::task_SmgcCloneTexture(uint32_t t_id, uint32_t* t_result): BaseTask(25), id(t_id), result(t_result) {}
bool taskIs_SmgcCloneTexture(uint32_t taskIdx) {
    return taskIdx == 25;
}

task_SmgcTintTexture::task_SmgcTintTexture(uint32_t t_id, uint32_t t_maskId, const glm::vec3& t_color, uint32_t* t_result): BaseTask(26), id(t_id), maskId(t_maskId), color(t_color), result(t_result) {}
bool taskIs_SmgcTintTexture(uint32_t taskIdx) {
    return taskIdx == 26;
}

task_SmgcMergeTextures::task_SmgcMergeTextures(uint32_t t_baseId, uint32_t t_overlayId, const glm::ivec2& t_offset, uint32_t* t_result): BaseTask(27), baseId(t_baseId), overlayId(t_overlayId), offset(t_offset), result(t_result) {}
bool taskIs_SmgcMergeTextures(uint32_t taskIdx) {
    return taskIdx == 27;
}

task_SmgcDrawTexture::task_SmgcDrawTexture(uint32_t t_id, const glm::vec2& t_pos, const glm::vec2& t_size, const glm::vec4& t_color): BaseTask(28), id(t_id), pos(t_pos), size(t_size), color(t_color) {}
task_SmgcDrawTexture::task_SmgcDrawTexture(uint32_t t_id, const glm::vec2& t_pos, const glm::vec2& t_size, const glm::vec4& t_color, const glm::vec4& t_srcRect): BaseTask(28), id(t_id), pos(t_pos), size(t_size), color(t_color), srcRect(t_srcRect) {}
bool taskIs_SmgcDrawTexture(uint32_t taskIdx) {
    return taskIdx == 28;
}
