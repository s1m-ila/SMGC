#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "shaderClass.h"
#include "VAO.h"
#include "VBO.h"
#include "EBO.h"
#include "Font.h"
#include "Shapes2D.h"
#include "TextureStore.h"

struct DrawBatch {
    uint32_t texture;
    uint32_t variant;
    size_t indexStart;
    size_t indexCount;
};

class Layer2D {
public:
    int init();
    void clear();
    void swap();
    void pushRect(const glm::vec2& pos, const glm::vec2& size, const glm::vec4& color);
    void pushTriangle(const glm::vec2& a, const glm::vec2& b, const glm::vec2& c, const glm::vec4& color);
    void pushCircle(const glm::vec2& center, float radius, const glm::vec4& color);
    void pushEllipse(const glm::vec2& center, const glm::vec2& radii, const glm::vec4& color);
    void pushRoundedRect(const glm::vec2& pos, const glm::vec2& size, float cornerRadius, const glm::vec4& color);
    void pushRing(const glm::vec2& center, float radius, float thickness, const glm::vec4& color);
    void pushLine(const glm::vec2& a, const glm::vec2& b, float thickness, const glm::vec4& color);
    void pushTexture(uint32_t id, const glm::vec2& pos, const glm::vec2& size, const glm::vec4& color, const std::optional<glm::vec4>& srcRect = std::nullopt);
    TextMetrics measureText(const std::string& text, float size) const;
    void pushText(const std::string& text, const glm::vec2& pos, float size, const glm::vec4& color, TextAlign align);
    int setFont(const std::string& path);
    TextureStore& textureStore();
    void draw(int width, int height, bool outline);
    void destroy();
private:
    void track(uint32_t texture, uint32_t variant, size_t indexStart);

    std::unique_ptr<Shader> shaderProgram = nullptr;

    std::unique_ptr<VAO> vao = nullptr;
    std::unique_ptr<VBO> vbo = nullptr;
    std::unique_ptr<EBO> ebo = nullptr;

    Font font{};
    TextureStore textures{};

    std::vector<Vertex2D> Vertices_A{};
    std::vector<Vertex2D> Vertices_B{};
    std::vector<GLuint> Indices_A{};
    std::vector<GLuint> Indices_B{};
    std::vector<DrawBatch> Batches_A{};
    std::vector<DrawBatch> Batches_B{};
};
