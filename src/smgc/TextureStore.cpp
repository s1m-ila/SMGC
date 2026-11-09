#include <smgc/TextureStore.h>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#define STBI_ONLY_BMP
#include <stb_image.h>

#include <lunasvg.h>

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <limits>

static constexpr int kMaxSvgSide = 16384;
static constexpr int kMaxRasterSide = 8192;
static constexpr size_t kMaxRasterBytes = size_t(128) * 1024 * 1024;

static bool IsSvgPath(const std::string& path) {
    std::string extension = std::filesystem::path(path).extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return extension == ".svg";
}

static void UploadTexture(GLuint& glId, int width, int height, const unsigned char* data, GLint filter) {
    if (glId == 0) glGenTextures(1, &glId);
    glBindTexture(GL_TEXTURE_2D, glId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);
}

static bool RasterizeToPixels(const lunasvg::Document& document, int width, int height, std::vector<unsigned char>& pixels) {
    lunasvg::Bitmap bitmap = document.renderToBitmap(width, height);
    if (bitmap.isNull() || bitmap.width() != width || bitmap.height() != height) return false;
    bitmap.convertToRGBA();

    pixels.resize(size_t(width) * size_t(height) * 4);
    for (int y = 0; y < height; y++)
        std::memcpy(&pixels[size_t(y) * size_t(width) * 4], bitmap.data() + size_t(y) * size_t(bitmap.stride()), size_t(width) * 4);
    return true;
}

TextureStore::Entry* TextureStore::Find(uint32_t id) {
    auto it = entries.find(id);
    return it == entries.end() ? nullptr : &it->second;
}

const TextureStore::Entry* TextureStore::Find(uint32_t id) const {
    auto it = entries.find(id);
    return it == entries.end() ? nullptr : &it->second;
}

uint32_t TextureStore::Add(Entry&& entry) {
    const uint32_t id = nextId++;
    entries.emplace(id, std::move(entry));
    return id;
}

const std::vector<unsigned char>* TextureStore::NativePixels(const Entry& entry, std::vector<unsigned char>& scratch) const {
    if (!entry.document) return &entry.pixels;
    if (!RasterizeToPixels(*entry.document, entry.width, entry.height, scratch)) return nullptr;
    return &scratch;
}

void TextureStore::DeleteRasters(Entry& entry) {
    for (auto& raster : entry.rasters) {
        if (raster.glId != 0) glDeleteTextures(1, &raster.glId);
        rasterBytes -= size_t(raster.width) * size_t(raster.height) * 4;
    }
    entry.rasters.clear();
}

void TextureStore::EvictRasters() {
    while (rasterBytes > kMaxRasterBytes) {
        Entry* owner = nullptr;
        size_t index = 0;
        uint64_t oldest = std::numeric_limits<uint64_t>::max();
        for (auto& [id, entry] : entries) {
            for (size_t i = 0; i < entry.rasters.size(); i++) {
                const Raster& raster = entry.rasters[i];
                if (generation < raster.lastUsed + 2) continue;
                if (raster.lastUsed < oldest) {
                    oldest = raster.lastUsed;
                    owner = &entry;
                    index = i;
                }
            }
        }
        if (!owner) break;

        Raster& raster = owner->rasters[index];
        if (raster.glId != 0) glDeleteTextures(1, &raster.glId);
        rasterBytes -= size_t(raster.width) * size_t(raster.height) * 4;
        owner->rasters.erase(owner->rasters.begin() + static_cast<std::ptrdiff_t>(index));
    }
}

uint32_t TextureStore::Load(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) return kInvalidTexture;
    const std::vector<unsigned char> bytes((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    if (bytes.empty()) return kInvalidTexture;

    Entry entry;

    if (IsSvgPath(path)) {
        std::shared_ptr<lunasvg::Document> document = lunasvg::Document::loadFromData(reinterpret_cast<const char*>(bytes.data()), bytes.size());
        if (!document) return kInvalidTexture;

        const float documentWidth = static_cast<float>(document->width());
        const float documentHeight = static_cast<float>(document->height());
        if (!std::isfinite(documentWidth) || !std::isfinite(documentHeight)) return kInvalidTexture;

        entry.width = static_cast<int>(std::ceil(documentWidth));
        entry.height = static_cast<int>(std::ceil(documentHeight));
        if (entry.width <= 0 || entry.height <= 0 || entry.width > kMaxSvgSide || entry.height > kMaxSvgSide) return kInvalidTexture;

        entry.document = std::move(document);
        return Add(std::move(entry));
    }

    int width = 0, height = 0, channels = 0;
    unsigned char* data = stbi_load_from_memory(bytes.data(), static_cast<int>(bytes.size()), &width, &height, &channels, 4);
    if (!data) return kInvalidTexture;

    entry.width = width;
    entry.height = height;
    entry.pixels.assign(data, data + size_t(width) * size_t(height) * 4);
    stbi_image_free(data);
    UploadTexture(entry.glId, entry.width, entry.height, entry.pixels.data(), GL_NEAREST);
    return Add(std::move(entry));
}

uint32_t TextureStore::Clone(uint32_t id) {
    const Entry* source = Find(id);
    if (!source) return kInvalidTexture;

    Entry entry;
    entry.width = source->width;
    entry.height = source->height;
    if (source->document) {
        entry.document = source->document;
    } else {
        entry.pixels = source->pixels;
        UploadTexture(entry.glId, entry.width, entry.height, entry.pixels.data(), GL_NEAREST);
    }
    return Add(std::move(entry));
}

uint32_t TextureStore::Merge(uint32_t baseId, uint32_t overlayId, const glm::ivec2& offset) {
    const Entry* base = Find(baseId);
    const Entry* overlay = Find(overlayId);
    if (!base || !overlay) return kInvalidTexture;

    std::vector<unsigned char> baseScratch;
    std::vector<unsigned char> overlayScratch;
    const std::vector<unsigned char>* basePixels = NativePixels(*base, baseScratch);
    const std::vector<unsigned char>* overlayPixels = NativePixels(*overlay, overlayScratch);
    if (!basePixels || !overlayPixels) return kInvalidTexture;

    const int width = base->width;
    const int height = base->height;
    const int overlayWidth = overlay->width;
    const int overlayHeight = overlay->height;
    std::vector<unsigned char> pixels = *basePixels;

    for (int y = 0; y < overlayHeight; y++) {
        const int dy = y + offset.y;
        if (dy < 0 || dy >= height) continue;
        for (int x = 0; x < overlayWidth; x++) {
            const int dx = x + offset.x;
            if (dx < 0 || dx >= width) continue;

            const unsigned char* s = &(*overlayPixels)[(size_t(y) * size_t(overlayWidth) + size_t(x)) * 4];
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

    Entry entry;
    entry.width = width;
    entry.height = height;
    entry.pixels = std::move(pixels);
    UploadTexture(entry.glId, entry.width, entry.height, entry.pixels.data(), GL_NEAREST);
    return Add(std::move(entry));
}

uint32_t TextureStore::Tint(uint32_t id, uint32_t maskId, const glm::vec3& color) {
    Entry* target = Find(id);
    const Entry* mask = Find(maskId);
    if (!target || !mask) return kInvalidTexture;

    std::vector<unsigned char> maskScratch;
    const std::vector<unsigned char>* maskSource = NativePixels(*mask, maskScratch);
    if (!maskSource) return kInvalidTexture;
    const std::vector<unsigned char> maskPixels = *maskSource;
    const int maskWidth = mask->width;
    const int maskHeight = mask->height;

    if (target->document) {
        std::vector<unsigned char> flat;
        if (!RasterizeToPixels(*target->document, target->width, target->height, flat)) return kInvalidTexture;
        DeleteRasters(*target);
        target->document.reset();
        target->pixels = std::move(flat);
    }

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
    UploadTexture(target->glId, target->width, target->height, target->pixels.data(), GL_NEAREST);
    return id;
}

bool TextureStore::Free(uint32_t id) {
    auto it = entries.find(id);
    if (it == entries.end()) return false;
    DeleteRasters(it->second);
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

bool TextureStore::IsVector(uint32_t id) const {
    const Entry* entry = Find(id);
    return entry && entry->document;
}

bool TextureStore::Acquire(uint32_t id, const glm::vec2& scale, uint32_t* variant) {
    Entry* entry = Find(id);
    if (!entry) return false;
    if (!entry->document) {
        *variant = 0;
        return true;
    }
    if (!std::isfinite(scale.x) || !std::isfinite(scale.y)) return false;

    const int width = static_cast<int>(std::clamp(std::round(double(entry->width) * double(scale.x)), 1.0, double(kMaxRasterSide)));
    const int height = static_cast<int>(std::clamp(std::round(double(entry->height) * double(scale.y)), 1.0, double(kMaxRasterSide)));

    for (auto& raster : entry->rasters) {
        if (raster.width == width && raster.height == height) {
            raster.lastUsed = generation;
            *variant = raster.serial;
            return true;
        }
    }

    lunasvg::Bitmap bitmap = entry->document->renderToBitmap(width, height);
    if (bitmap.isNull() || bitmap.width() != width || bitmap.height() != height) return false;
    bitmap.convertToRGBA();

    Raster raster;
    raster.serial = nextSerial++;
    raster.width = width;
    raster.height = height;
    raster.lastUsed = generation;

    const unsigned char* uploadData = bitmap.data();
    std::vector<unsigned char> packedPixels;
    if (bitmap.stride() != static_cast<std::size_t>(width * 4)) {
        packedPixels.resize(size_t(width) * size_t(height) * 4);
        for (int y = 0; y < height; y++) {
            std::memcpy(&packedPixels[size_t(y) * size_t(width) * 4],
                        bitmap.data() + size_t(y) * size_t(bitmap.stride()),
                        size_t(width) * 4);
        }
        uploadData = packedPixels.data();
    }
    UploadTexture(raster.glId, width, height, uploadData, GL_LINEAR);

    entry->rasters.push_back(raster);
    rasterBytes += size_t(width) * size_t(height) * 4;
    *variant = raster.serial;
    EvictRasters();
    return true;
}

bool TextureStore::Bind(uint32_t id, uint32_t variant, GLuint unit) const {
    const Entry* entry = Find(id);
    if (!entry) return false;

    GLuint glId = 0;
    if (!entry->document) {
        glId = entry->glId;
    } else {
        for (const auto& raster : entry->rasters) {
            if (raster.serial == variant) {
                glId = raster.glId;
                break;
            }
        }
    }
    if (glId == 0) return false;

    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, glId);
    return true;
}

bool TextureStore::Exists(uint32_t id) const {
    return Find(id) != nullptr;
}

void TextureStore::NextFrame() {
    generation++;
}

void TextureStore::Destroy() {
    for (auto& [id, entry] : entries) {
        DeleteRasters(entry);
        if (entry.glId != 0) glDeleteTextures(1, &entry.glId);
    }
    entries.clear();
    rasterBytes = 0;
}
