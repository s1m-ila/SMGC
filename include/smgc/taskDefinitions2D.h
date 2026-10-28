#pragma once

#include <smpb/BaseTask.h>

#include <glm/glm.hpp>

#include <string>

#include "Font.h"

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
