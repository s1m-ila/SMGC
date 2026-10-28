#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

struct stbtt_fontinfo;

enum class TextAlign : int {
    Left   = 0,
    Center = 1,
    Right  = 2,
};

struct Glyph {
    int index = 0;
    glm::vec2 size{0.0f};
    glm::vec2 offset{0.0f};
    glm::vec2 uvMin{0.0f};
    glm::vec2 uvMax{0.0f};
    float advance = 0.0f;
};

class Font {
public:
    GLuint ID = 0;

    Font();
    ~Font();

    int Load(const std::string& path);
    void Bind(GLuint unit) const;
    void Delete();

    bool IsLoaded() const;
    const Glyph* FindGlyph(uint32_t codepoint) const;
    float Kerning(const Glyph& left, const Glyph& right) const;

    float BakeSize() const;
    float Ascent() const;
    float LineHeight() const;
    float DistanceScale() const;
private:
    std::vector<unsigned char> data{};
    std::unique_ptr<stbtt_fontinfo> info;
    std::unordered_map<uint32_t, Glyph> glyphs{};
    float scale = 0.0f;
    float ascent = 0.0f;
    float lineHeight = 0.0f;
};
