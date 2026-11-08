#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace lunasvg {
class Document;
}

inline constexpr uint32_t kInvalidTexture = 0xFFFFFFFFu;

class TextureStore {
public:
    uint32_t Load(const std::string& path);
    uint32_t Clone(uint32_t id);
    uint32_t Merge(uint32_t baseId, uint32_t overlayId, const glm::ivec2& offset);
    uint32_t Tint(uint32_t id, uint32_t maskId, const glm::vec3& color);
    bool Free(uint32_t id);
    bool GetSize(uint32_t id, int* width, int* height) const;
    bool IsVector(uint32_t id) const;
    bool Acquire(uint32_t id, const glm::vec2& scale, uint32_t* variant);
    bool Bind(uint32_t id, uint32_t variant, GLuint unit) const;
    bool Exists(uint32_t id) const;
    void NextFrame();
    void Destroy();
private:
    struct Raster {
        uint32_t serial = 0;
        int width = 0;
        int height = 0;
        GLuint glId = 0;
        uint64_t lastUsed = 0;
    };

    struct Entry {
        GLuint glId = 0;
        int width = 0;
        int height = 0;
        std::vector<unsigned char> pixels{};
        std::shared_ptr<lunasvg::Document> document{};
        std::vector<Raster> rasters{};
    };

    Entry* Find(uint32_t id);
    const Entry* Find(uint32_t id) const;
    uint32_t Add(Entry&& entry);
    const std::vector<unsigned char>* NativePixels(const Entry& entry, std::vector<unsigned char>& scratch) const;
    void DeleteRasters(Entry& entry);
    void EvictRasters();

    std::unordered_map<uint32_t, Entry> entries{};
    uint32_t nextId = 0;
    uint32_t nextSerial = 1;
    uint64_t generation = 0;
    size_t rasterBytes = 0;
};
