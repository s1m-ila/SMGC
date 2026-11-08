#pragma once

#include <smpb/BaseTask.h>

#include <glm/glm.hpp>

#include <cstdint>
#include <optional>
#include <string>

#include "Font.h"
#include "TextureStore.h"

class task_SmgcDrawRect: public BaseTask {
public:
    task_SmgcDrawRect(const glm::vec2& t_pos, const glm::vec2& t_size, const glm::vec4& t_color);
    glm::vec2 pos{};
    glm::vec2 size{};
    glm::vec4 color{};
};
bool taskIs_SmgcDrawRect(uint32_t taskIdx);

class task_SmgcDrawTriangle: public BaseTask {
public:
    task_SmgcDrawTriangle(const glm::vec2& t_a, const glm::vec2& t_b, const glm::vec2& t_c, const glm::vec4& t_color);
    glm::vec2 a{};
    glm::vec2 b{};
    glm::vec2 c{};
    glm::vec4 color{};
};
bool taskIs_SmgcDrawTriangle(uint32_t taskIdx);

class task_SmgcDrawCircle: public BaseTask {
public:
    task_SmgcDrawCircle(const glm::vec2& t_center, float t_radius, const glm::vec4& t_color);
    glm::vec2 center{};
    float radius = 0.0f;
    glm::vec4 color{};
};
bool taskIs_SmgcDrawCircle(uint32_t taskIdx);

class task_SmgcDrawEllipse: public BaseTask {
public:
    task_SmgcDrawEllipse(const glm::vec2& t_center, const glm::vec2& t_radii, const glm::vec4& t_color);
    glm::vec2 center{};
    glm::vec2 radii{};
    glm::vec4 color{};
};
bool taskIs_SmgcDrawEllipse(uint32_t taskIdx);

class task_SmgcDrawRoundedRect: public BaseTask {
public:
    task_SmgcDrawRoundedRect(const glm::vec2& t_pos, const glm::vec2& t_size, float t_cornerRadius, const glm::vec4& t_color);
    glm::vec2 pos{};
    glm::vec2 size{};
    float cornerRadius = 0.0f;
    glm::vec4 color{};
};
bool taskIs_SmgcDrawRoundedRect(uint32_t taskIdx);

class task_SmgcDrawRing: public BaseTask {
public:
    task_SmgcDrawRing(const glm::vec2& t_center, float t_radius, float t_thickness, const glm::vec4& t_color);
    glm::vec2 center{};
    float radius = 0.0f;
    float thickness = 0.0f;
    glm::vec4 color{};
};
bool taskIs_SmgcDrawRing(uint32_t taskIdx);

class task_SmgcDrawLine: public BaseTask {
public:
    task_SmgcDrawLine(const glm::vec2& t_a, const glm::vec2& t_b, float t_thickness, const glm::vec4& t_color);
    glm::vec2 a{};
    glm::vec2 b{};
    float thickness = 0.0f;
    glm::vec4 color{};
};
bool taskIs_SmgcDrawLine(uint32_t taskIdx);

class task_SmgcDrawText: public BaseTask {
public:
    task_SmgcDrawText(const std::string& t_text, const glm::vec2& t_pos, float t_size, const glm::vec4& t_color, TextAlign t_align = TextAlign::Left);
    std::string text{};
    glm::vec2 pos{};
    float size = 0.0f;
    glm::vec4 color{};
    TextAlign align = TextAlign::Left;
};
bool taskIs_SmgcDrawText(uint32_t taskIdx);

class task_SmgcSetFont: public BaseTask {
public:
    task_SmgcSetFont(const std::string& t_path);
    std::string path{};
};
bool taskIs_SmgcSetFont(uint32_t taskIdx);

class task_SmgcMeasureText: public BaseTask {
public:
    task_SmgcMeasureText(const std::string& t_text, float t_size, float* t_width, float* t_height);
    std::string text{};
    float size = 0.0f;
    float* width  = nullptr;
    float* height = nullptr;
};
bool taskIs_SmgcMeasureText(uint32_t taskIdx);

class task_SmgcLoadTexture: public BaseTask {
public:
    task_SmgcLoadTexture(const std::string& t_path, uint32_t* t_id);
    std::string path{};
    uint32_t* id = nullptr;
};
bool taskIs_SmgcLoadTexture(uint32_t taskIdx);

class task_SmgcFreeTexture: public BaseTask {
public:
    explicit task_SmgcFreeTexture(uint32_t t_id);
    uint32_t id = kInvalidTexture;
};
bool taskIs_SmgcFreeTexture(uint32_t taskIdx);

class task_SmgcGetTextureSize: public BaseTask {
public:
    task_SmgcGetTextureSize(uint32_t t_id, int* t_width, int* t_height);
    uint32_t id = kInvalidTexture;
    int* width = nullptr;
    int* height = nullptr;
};
bool taskIs_SmgcGetTextureSize(uint32_t taskIdx);

class task_SmgcCloneTexture: public BaseTask {
public:
    task_SmgcCloneTexture(uint32_t t_id, uint32_t* t_result);
    uint32_t id = kInvalidTexture;
    uint32_t* result = nullptr;
};
bool taskIs_SmgcCloneTexture(uint32_t taskIdx);

class task_SmgcTintTexture: public BaseTask {
public:
    task_SmgcTintTexture(uint32_t t_id, uint32_t t_maskId, const glm::vec3& t_color, uint32_t* t_result);
    uint32_t id = kInvalidTexture;
    uint32_t maskId = kInvalidTexture;
    glm::vec3 color{1.0f};
    uint32_t* result = nullptr;
};
bool taskIs_SmgcTintTexture(uint32_t taskIdx);

class task_SmgcMergeTextures: public BaseTask {
public:
    task_SmgcMergeTextures(uint32_t t_baseId, uint32_t t_overlayId, const glm::ivec2& t_offset, uint32_t* t_result);
    uint32_t baseId = kInvalidTexture;
    uint32_t overlayId = kInvalidTexture;
    glm::ivec2 offset{0};
    uint32_t* result = nullptr;
};
bool taskIs_SmgcMergeTextures(uint32_t taskIdx);

class task_SmgcDrawTexture: public BaseTask {
public:
    task_SmgcDrawTexture(uint32_t t_id, const glm::vec2& t_pos, const glm::vec2& t_size, const glm::vec4& t_color = glm::vec4{1.0f});
    task_SmgcDrawTexture(uint32_t t_id, const glm::vec2& t_pos, const glm::vec2& t_size, const glm::vec4& t_color, const glm::vec4& t_srcRect);
    uint32_t id = kInvalidTexture;
    glm::vec2 pos{};
    glm::vec2 size{};
    glm::vec4 color{1.0f};
    std::optional<glm::vec4> srcRect = std::nullopt;
};
bool taskIs_SmgcDrawTexture(uint32_t taskIdx);
