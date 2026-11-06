#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

inline constexpr uint32_t kInvalidTexture = 0xFFFFFFFFu;

class TextureStore {
public:
    uint32_t Load(const std::string& path);
    uint32_t Clone(uint32_t id);
    uint32_t Merge(uint32_t baseId, uint32_t overlayId, const glm::ivec2& offset);
    uint32_t Tint(uint32_t id, uint32_t maskId, const glm::vec3& color);
    bool Free(uint32_t id);
    bool GetSize(uint32_t id, int* width, int* height) const;
    bool Bind(uint32_t id, GLuint unit) const;
    bool Exists(uint32_t id) const;
    void Destroy();
private:
    struct Entry {
        GLuint glId = 0;
        int width = 0;
        int height = 0;
        std::vector<unsigned char> pixels{};
    };

    Entry* Find(uint32_t id);
    const Entry* Find(uint32_t id) const;
    uint32_t Add(int width, int height, std::vector<unsigned char>&& pixels);
    static void Upload(Entry& entry);

    std::unordered_map<uint32_t, Entry> entries{};
    uint32_t nextId = 0;
};
