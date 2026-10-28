#include <smgc/Font.h>

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>

#include <algorithm>
#include <array>
#include <cerrno>
#include <fstream>
#include <iostream>
#include <iterator>
#include <utility>

static constexpr float kBakeSize = 48.0f;
static constexpr int kPadding = 8;
static constexpr unsigned char kOnEdge = 128;
static constexpr float kPixelDistScale = float(kOnEdge) / float(kPadding);
static constexpr int kAtlasWidth = 1024;
static constexpr int kAtlasGap = 1;

static constexpr std::array<std::pair<uint32_t, uint32_t>, 7> kRanges = {{
    {0x0020, 0x007E},
    {0x00A0, 0x00FF},
    {0x0400, 0x045F},
    {0x2010, 0x2027},
    {0x2030, 0x203A},
    {0x20AC, 0x20AC},
    {0x2116, 0x2116},
}};

struct BakedGlyph {
    uint32_t codepoint;
    Glyph glyph;
    int width;
    int height;
    int x;
    int y;
    unsigned char* bitmap;
};

Font::Font() = default;

Font::~Font() = default;

int Font::Load(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        std::cerr << "Error in Font::Load: cannot open \"" << path << "\"" << std::endl;
        return -ENOENT;
    }
    std::vector<unsigned char> newData((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    auto newInfo = std::make_unique<stbtt_fontinfo>();
    const int offset = stbtt_GetFontOffsetForIndex(newData.data(), 0);
    if (offset < 0 || !stbtt_InitFont(newInfo.get(), newData.data(), offset)) {
        std::cerr << "Error in Font::Load: \"" << path << "\" is not a valid TrueType font" << std::endl;
        return -EINVAL;
    }

    const float newScale = stbtt_ScaleForPixelHeight(newInfo.get(), kBakeSize);
    int ascentUnits = 0, descentUnits = 0, lineGapUnits = 0;
    stbtt_GetFontVMetrics(newInfo.get(), &ascentUnits, &descentUnits, &lineGapUnits);

    std::vector<BakedGlyph> baked;
    int penX = 0, penY = 0, rowHeight = 0;
    for (const auto& [first, last] : kRanges) {
        for (uint32_t codepoint = first; codepoint <= last; codepoint++) {
            const int index = stbtt_FindGlyphIndex(newInfo.get(), static_cast<int>(codepoint));
            if (index == 0) continue;

            int advance = 0, leftBearing = 0;
            stbtt_GetGlyphHMetrics(newInfo.get(), index, &advance, &leftBearing);

            BakedGlyph item{codepoint, {}, 0, 0, 0, 0, nullptr};
            int xoff = 0, yoff = 0;
            item.bitmap = stbtt_GetGlyphSDF(newInfo.get(), newScale, index, kPadding, kOnEdge, kPixelDistScale, &item.width, &item.height, &xoff, &yoff);
            item.glyph.index = index;
            item.glyph.advance = float(advance) * newScale;
            item.glyph.offset = glm::vec2(float(xoff), float(yoff));

            if (item.bitmap) {
                if (penX + item.width > kAtlasWidth) {
                    penX = 0;
                    penY += rowHeight + kAtlasGap;
                    rowHeight = 0;
                }
                item.x = penX;
                item.y = penY;
                penX += item.width + kAtlasGap;
                rowHeight = std::max(rowHeight, item.height);
            }
            baked.push_back(item);
        }
    }
    const int atlasHeight = std::max(penY + rowHeight, 1);

    std::vector<unsigned char> pixels(size_t(kAtlasWidth) * size_t(atlasHeight), 0);
    std::unordered_map<uint32_t, Glyph> newGlyphs;
    for (auto& item : baked) {
        if (item.bitmap) {
            for (int row = 0; row < item.height; row++)
                std::copy_n(item.bitmap + row * item.width, item.width, pixels.begin() + (item.y + row) * kAtlasWidth + item.x);
            stbtt_FreeSDF(item.bitmap, nullptr);
            item.glyph.size = glm::vec2(float(item.width), float(item.height));
            item.glyph.uvMin = glm::vec2(float(item.x) / float(kAtlasWidth), float(item.y) / float(atlasHeight));
            item.glyph.uvMax = glm::vec2(float(item.x + item.width) / float(kAtlasWidth), float(item.y + item.height) / float(atlasHeight));
        }
        newGlyphs[item.codepoint] = item.glyph;
    }

    if (ID == 0) glGenTextures(1, &ID);
    glBindTexture(GL_TEXTURE_2D, ID);
    GLint previousAlignment = 4;
    glGetIntegerv(GL_UNPACK_ALIGNMENT, &previousAlignment);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, kAtlasWidth, atlasHeight, 0, GL_RED, GL_UNSIGNED_BYTE, pixels.data());
    glPixelStorei(GL_UNPACK_ALIGNMENT, previousAlignment);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);

    data = std::move(newData);
    info = std::move(newInfo);
    glyphs = std::move(newGlyphs);
    scale = newScale;
    ascent = float(ascentUnits) * newScale;
    lineHeight = float(ascentUnits - descentUnits + lineGapUnits) * newScale;
    return 0;
}

void Font::Bind(GLuint unit) const {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, ID);
}

void Font::Delete() {
    if (ID != 0) glDeleteTextures(1, &ID);
    ID = 0;
    glyphs.clear();
    info = nullptr;
    data.clear();
}

bool Font::IsLoaded() const {
    return info != nullptr;
}

const Glyph* Font::FindGlyph(uint32_t codepoint) const {
    auto it = glyphs.find(codepoint);
    return it == glyphs.end() ? nullptr : &it->second;
}

float Font::Kerning(const Glyph& left, const Glyph& right) const {
    if (!info) return 0.0f;
    return float(stbtt_GetGlyphKernAdvance(info.get(), left.index, right.index)) * scale;
}

float Font::BakeSize() const {
    return kBakeSize;
}

float Font::Ascent() const {
    return ascent;
}

float Font::LineHeight() const {
    return lineHeight;
}

float Font::DistanceScale() const {
    return 255.0f / kPixelDistScale;
}
