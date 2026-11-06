#include <smgc/TextureStore.h>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#define STBI_ONLY_BMP
#include <stb_image.h>

#include <algorithm>
#include <fstream>
#include <iterator>

TextureStore::Entry* TextureStore::Find(uint32_t id) {
    auto it = entries.find(id);
    return it == entries.end() ? nullptr : &it->second;
}

const TextureStore::Entry* TextureStore::Find(uint32_t id) const {
    auto it = entries.find(id);
    return it == entries.end() ? nullptr : &it->second;
}

void TextureStore::Upload(Entry& entry) {
    if (entry.glId == 0) glGenTextures(1, &entry.glId);
    glBindTexture(GL_TEXTURE_2D, entry.glId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, entry.width, entry.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, entry.pixels.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);
}

uint32_t TextureStore::Add(int width, int height, std::vector<unsigned char>&& pixels) {
    const uint32_t id = nextId++;
    Entry entry;
    entry.width = width;
    entry.height = height;
    entry.pixels = std::move(pixels);
    Upload(entry);
    entries.emplace(id, std::move(entry));
    return id;
}

uint32_t TextureStore::Load(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) return kInvalidTexture;
    const std::vector<unsigned char> bytes((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    if (bytes.empty()) return kInvalidTexture;

    int width = 0, height = 0, channels = 0;
    unsigned char* data = stbi_load_from_memory(bytes.data(), static_cast<int>(bytes.size()), &width, &height, &channels, 4);
    if (!data) return kInvalidTexture;

    std::vector<unsigned char> pixels(data, data + size_t(width) * size_t(height) * 4);
    stbi_image_free(data);
    return Add(width, height, std::move(pixels));
}

uint32_t TextureStore::Clone(uint32_t id) {
    const Entry* source = Find(id);
    if (!source) return kInvalidTexture;
    const int width = source->width;
    const int height = source->height;
    std::vector<unsigned char> pixels = source->pixels;
    return Add(width, height, std::move(pixels));
}

uint32_t TextureStore::Merge(uint32_t baseId, uint32_t overlayId, const glm::ivec2& offset) {
    const Entry* base = Find(baseId);
    const Entry* overlay = Find(overlayId);
    if (!base || !overlay) return kInvalidTexture;

    const int width = base->width;
    const int height = base->height;
    std::vector<unsigned char> pixels = base->pixels;

    for (int y = 0; y < overlay->height; y++) {
        const int dy = y + offset.y;
        if (dy < 0 || dy >= height) continue;
        for (int x = 0; x < overlay->width; x++) {
            const int dx = x + offset.x;
            if (dx < 0 || dx >= width) continue;

            const unsigned char* s = &overlay->pixels[(size_t(y) * size_t(overlay->width) + size_t(x)) * 4];
            unsigned char* d = &pixels[(size_t(dy) * size_t(width) + size_t(dx)) * 4];

            const float sa = float(s[3]) / 255.0f;
            const float da = float(d[3]) / 255.0f;
            const float oa = sa + da * (1.0f - sa);
            if (oa <= 0.0f) {
                d[0] = d[1] = d[2] = d[3] = 0;
                continue;
            }
            for (int c = 0; c < 3; c++) {
                const float value = (float(s[c]) * sa + float(d[c]) * da * (1.0f - sa)) / oa;
                d[c] = static_cast<unsigned char>(std::clamp(value + 0.5f, 0.0f, 255.0f));
            }
            d[3] = static_cast<unsigned char>(std::clamp(oa * 255.0f + 0.5f, 0.0f, 255.0f));
        }
    }
    return Add(width, height, std::move(pixels));
}

uint32_t TextureStore::Tint(uint32_t id, uint32_t maskId, const glm::vec3& color) {
    Entry* target = Find(id);
    const Entry* mask = Find(maskId);
    if (!target || !mask) return kInvalidTexture;

    const std::vector<unsigned char> maskPixels = mask->pixels;
    const int maskWidth = mask->width;
    const int maskHeight = mask->height;
    const glm::vec3 tint = glm::clamp(color, 0.0f, 1.0f);

    const int width = std::min(target->width, maskWidth);
    const int height = std::min(target->height, maskHeight);
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            const unsigned char* m = &maskPixels[(size_t(y) * size_t(maskWidth) + size_t(x)) * 4];
            const float brightness = (0.2126f * float(m[0]) + 0.7152f * float(m[1]) + 0.0722f * float(m[2])) / 255.0f;
            if (brightness <= 0.5f) continue;

            unsigned char* p = &target->pixels[(size_t(y) * size_t(target->width) + size_t(x)) * 4];
            for (int c = 0; c < 3; c++)
                p[c] = static_cast<unsigned char>(std::clamp(float(p[c]) * tint[c] + 0.5f, 0.0f, 255.0f));
        }
    }
    Upload(*target);
    return id;
}

bool TextureStore::Free(uint32_t id) {
    auto it = entries.find(id);
    if (it == entries.end()) return false;
    if (it->second.glId != 0) glDeleteTextures(1, &it->second.glId);
    entries.erase(it);
    return true;
}

bool TextureStore::GetSize(uint32_t id, int* width, int* height) const {
    const Entry* entry = Find(id);
    if (!entry) return false;
    *width = entry->width;
    *height = entry->height;
    return true;
}

bool TextureStore::Bind(uint32_t id, GLuint unit) const {
    const Entry* entry = Find(id);
    if (!entry) return false;
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, entry->glId);
    return true;
}

bool TextureStore::Exists(uint32_t id) const {
    return Find(id) != nullptr;
}

void TextureStore::Destroy() {
    for (auto& [id, entry] : entries) {
        if (entry.glId != 0) glDeleteTextures(1, &entry.glId);
    }
    entries.clear();
}
