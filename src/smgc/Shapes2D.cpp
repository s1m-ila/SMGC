#include <smgc/Shapes2D.h>

#include <algorithm>

static constexpr float kAntialiasPadding = 1.0f;

static void Quad2D(std::vector<Vertex2D>& vertices, std::vector<GLuint>& indices, const std::array<glm::vec2, 4>& corners, const glm::vec2& origin, const glm::vec4& color, const glm::vec4& params, Shape2D shape) {
    const GLuint base = static_cast<GLuint>(vertices.size());
    for (const auto& corner : corners)
        vertices.push_back({corner, corner - origin, color, params, static_cast<GLint>(shape)});
    indices.insert(indices.end(), {base, base + 1, base + 2, base, base + 2, base + 3});
}

static void CenteredQuad2D(std::vector<Vertex2D>& vertices, std::vector<GLuint>& indices, const glm::vec2& center, const glm::vec2& halfSize, const glm::vec4& color, const glm::vec4& params, Shape2D shape) {
    const glm::vec2 h = halfSize + kAntialiasPadding;
    Quad2D(vertices, indices, {
        center + glm::vec2(-h.x, -h.y),
        center + glm::vec2( h.x, -h.y),
        center + glm::vec2( h.x,  h.y),
        center + glm::vec2(-h.x,  h.y),
    }, center, color, params, shape);
}

void Rect2D(std::vector<Vertex2D>& vertices, std::vector<GLuint>& indices, const glm::vec2& pos, const glm::vec2& size, const glm::vec4& color) {
    Quad2D(vertices, indices, {
        pos,
        pos + glm::vec2(size.x, 0.0f),
        pos + size,
        pos + glm::vec2(0.0f, size.y),
    }, pos, color, glm::vec4(0.0f), Shape2D::Polygon);
}

void Triangle2D(std::vector<Vertex2D>& vertices, std::vector<GLuint>& indices, const glm::vec2& a, const glm::vec2& b, const glm::vec2& c, const glm::vec4& color) {
    const GLuint base = static_cast<GLuint>(vertices.size());
    for (const auto& p : {a, b, c})
        vertices.push_back({p, p - a, color, glm::vec4(0.0f), static_cast<GLint>(Shape2D::Polygon)});
    indices.insert(indices.end(), {base, base + 1, base + 2});
}

void Circle2D(std::vector<Vertex2D>& vertices, std::vector<GLuint>& indices, const glm::vec2& center, float radius, const glm::vec4& color) {
    CenteredQuad2D(vertices, indices, center, glm::vec2(radius), color, glm::vec4(radius, 0.0f, 0.0f, 0.0f), Shape2D::Circle);
}

void Ellipse2D(std::vector<Vertex2D>& vertices, std::vector<GLuint>& indices, const glm::vec2& center, const glm::vec2& radii, const glm::vec4& color) {
    if (radii.x <= 0.0f || radii.y <= 0.0f) return;
    CenteredQuad2D(vertices, indices, center, radii, color, glm::vec4(radii, 0.0f, 0.0f), Shape2D::Ellipse);
}

void RoundedRect2D(std::vector<Vertex2D>& vertices, std::vector<GLuint>& indices, const glm::vec2& pos, const glm::vec2& size, float cornerRadius, const glm::vec4& color) {
    const glm::vec2 halfSize = size * 0.5f;
    const float radius = std::min(cornerRadius, std::min(halfSize.x, halfSize.y));
    CenteredQuad2D(vertices, indices, pos + halfSize, halfSize, color, glm::vec4(halfSize, radius, 0.0f), Shape2D::RoundedRect);
}

void Ring2D(std::vector<Vertex2D>& vertices, std::vector<GLuint>& indices, const glm::vec2& center, float radius, float thickness, const glm::vec4& color) {
    CenteredQuad2D(vertices, indices, center, glm::vec2(radius), color, glm::vec4(radius, std::min(thickness, radius), 0.0f, 0.0f), Shape2D::Ring);
}

void Line2D(std::vector<Vertex2D>& vertices, std::vector<GLuint>& indices, const glm::vec2& a, const glm::vec2& b, float thickness, const glm::vec4& color) {
    const glm::vec2 ab = b - a;
    const float length = glm::length(ab);
    const glm::vec2 dir = length > 0.0f ? ab / length : glm::vec2(1.0f, 0.0f);
    const glm::vec2 normal(-dir.y, dir.x);
    const float halfThickness = thickness * 0.5f;
    const float h = halfThickness + kAntialiasPadding;
    Quad2D(vertices, indices, {
        a - dir * h - normal * h,
        b + dir * h - normal * h,
        b + dir * h + normal * h,
        a - dir * h + normal * h,
    }, a, color, glm::vec4(ab, halfThickness, 0.0f), Shape2D::Line);
}
