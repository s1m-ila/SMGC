#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <array>
#include <string>
#include <vector>

#include "Font.h"

enum class Shape2D : GLint {
    Polygon     = 0,
    Circle      = 1,
    Ellipse     = 2,
    RoundedRect = 3,
    Ring        = 4,
    Line        = 5,
    Text        = 6,
};

struct Vertex2D {
    glm::vec2 pos;
    glm::vec2 local;
    glm::vec4 color;
    glm::vec4 params;
    GLint shape;
};

void Rect2D(std::vector<Vertex2D>& vertices, std::vector<GLuint>& indices, const glm::vec2& pos, const glm::vec2& size, const glm::vec4& color);
void Triangle2D(std::vector<Vertex2D>& vertices, std::vector<GLuint>& indices, const glm::vec2& a, const glm::vec2& b, const glm::vec2& c, const glm::vec4& color);
void Circle2D(std::vector<Vertex2D>& vertices, std::vector<GLuint>& indices, const glm::vec2& center, float radius, const glm::vec4& color);
void Ellipse2D(std::vector<Vertex2D>& vertices, std::vector<GLuint>& indices, const glm::vec2& center, const glm::vec2& radii, const glm::vec4& color);
void RoundedRect2D(std::vector<Vertex2D>& vertices, std::vector<GLuint>& indices, const glm::vec2& pos, const glm::vec2& size, float cornerRadius, const glm::vec4& color);
void Ring2D(std::vector<Vertex2D>& vertices, std::vector<GLuint>& indices, const glm::vec2& center, float radius, float thickness, const glm::vec4& color);
void Line2D(std::vector<Vertex2D>& vertices, std::vector<GLuint>& indices, const glm::vec2& a, const glm::vec2& b, float thickness, const glm::vec4& color);
void Text2D(std::vector<Vertex2D>& vertices, std::vector<GLuint>& indices, const Font& font, const std::string& text, const glm::vec2& pos, float size, const glm::vec4& color, TextAlign align);
