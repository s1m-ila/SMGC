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

static uint32_t NextCodepoint(const std::string& text, size_t& i) {
    const unsigned char lead = static_cast<unsigned char>(text[i++]);
    if (lead < 0x80) return lead;
    const int extra = (lead & 0xE0) == 0xC0 ? 1 : (lead & 0xF0) == 0xE0 ? 2 : (lead & 0xF8) == 0xF0 ? 3 : -1;
    if (extra < 0) return 0xFFFD;
    uint32_t codepoint = lead & (0x3F >> extra);
    for (int k = 0; k < extra; k++) {
        if (i >= text.size() || (static_cast<unsigned char>(text[i]) & 0xC0) != 0x80) return 0xFFFD;
        codepoint = (codepoint << 6) | (static_cast<unsigned char>(text[i++]) & 0x3F);
    }
    return codepoint;
}

static void TextLine2D(std::vector<Vertex2D>& vertices, std::vector<GLuint>& indices, const Font& font, const std::vector<const Glyph*>& line, float x, float baseline, float scale, const glm::vec4& color, const glm::vec4& params, TextAlign align) {
    float width = 0.0f;
    for (size_t k = 0; k < line.size(); k++) {
        if (k > 0) width += font.Kerning(*line[k - 1], *line[k]);
        width += line[k]->advance;
    }
    if (align == TextAlign::Center) x -= width * scale * 0.5f;
    else if (align == TextAlign::Right) x -= width * scale;

    for (size_t k = 0; k < line.size(); k++) {
        const Glyph& glyph = *line[k];
        if (k > 0) x += font.Kerning(*line[k - 1], glyph) * scale;
        if (glyph.size.x > 0.0f && glyph.size.y > 0.0f) {
            const glm::vec2 topLeft(x + glyph.offset.x * scale, baseline + glyph.offset.y * scale);
            const glm::vec2 bottomRight = topLeft + glyph.size * scale;
            const GLuint base = static_cast<GLuint>(vertices.size());
            const GLint shape = static_cast<GLint>(Shape2D::Text);
            vertices.push_back({topLeft,                              glyph.uvMin,                              color, params, shape});
            vertices.push_back({{bottomRight.x, topLeft.y},           {glyph.uvMax.x, glyph.uvMin.y},           color, params, shape});
            vertices.push_back({bottomRight,                          glyph.uvMax,                              color, params, shape});
            vertices.push_back({{topLeft.x, bottomRight.y},           {glyph.uvMin.x, glyph.uvMax.y},           color, params, shape});
            indices.insert(indices.end(), {base, base + 1, base + 2, base, base + 2, base + 3});
        }
        x += glyph.advance * scale;
    }
}

void Text2D(std::vector<Vertex2D>& vertices, std::vector<GLuint>& indices, const Font& font, const std::string& text, const glm::vec2& pos, float size, const glm::vec4& color, TextAlign align) {
    if (!font.IsLoaded()) return;
    const float scale = size / font.BakeSize();
    const glm::vec4 params(font.DistanceScale() * scale, 0.0f, 0.0f, 0.0f);
    const Glyph* fallback = font.FindGlyph('?');

    std::vector<const Glyph*> line;
    float baseline = pos.y + font.Ascent() * scale;
    size_t i = 0;
    while (true) {
        line.clear();
        while (i < text.size() && text[i] != '\n') {
            const uint32_t codepoint = NextCodepoint(text, i);
            if (codepoint == '\r') continue;
            const Glyph* glyph = font.FindGlyph(codepoint);
            if (!glyph) glyph = fallback;
            if (glyph) line.push_back(glyph);
        }
        TextLine2D(vertices, indices, font, line, pos.x, baseline, scale, color, params, align);
        if (i >= text.size()) break;
        i++;
        baseline += font.LineHeight() * scale;
    }
}
