#include <smgc/ObjectsGenerator.h>

static glm::mat3 ObjectRotation(const glm::vec3& angle) {
    float cx = std::cos(angle.x), sx = std::sin(angle.x);
    float cy = std::cos(angle.y), sy = std::sin(angle.y);
    float cz = std::cos(angle.z), sz = std::sin(angle.z);

    glm::mat3 Rx({1.0f, 0.0f, 0.0f}, {0.0f, cx, -sx}, {0.0f, sx, cx});
    glm::mat3 Ry({cy, 0.0f, sy}, {0.0f, 1.0f, 0.0f}, {-sy, 0.0f, cy});
    glm::mat3 Rz({cz, -sz, 0.0f}, {sz, cz, 0.0f}, {0.0f, 0.0f, 1.0f});

    return Rx * Ry * Rz;
}

static glm::vec3 ObjectToWorld(const Object& obj, const glm::mat3& rot, const glm::vec3& local) {
    return glm::vec3(obj.data1.y, obj.data1.z, obj.data1.w) + rot * local;
}

static void RoundMeshLocal(std::vector<glm::vec3>& positions, std::vector<GLuint>& indices,
                           float radius, float halfLength, float centerY) {
    constexpr int kStacks = 16;
    constexpr int kSlices = 32;

    std::vector<std::pair<float, float>> rings;
    for (int i = 0; i <= kStacks; i++) {
        float theta = float(M_PI) * float(i) / float(kStacks);
        if (halfLength > 0.0f && i == kStacks / 2) {
            rings.push_back({theta,  halfLength});
            rings.push_back({theta, -halfLength});
        } else {
            rings.push_back({theta, (i < kStacks / 2) ? halfLength : -halfLength});
        }
    }

    for (auto& [theta, offsetY] : rings) {
        for (int j = 0; j < kSlices; j++) {
            float phi = 2.0f * float(M_PI) * float(j) / float(kSlices);
            positions.push_back({
                radius * std::sin(theta) * std::cos(phi),
                radius * std::cos(theta) + offsetY + centerY,
                radius * std::sin(theta) * std::sin(phi),
            });
        }
    }

    indices.reserve(indices.size() + (rings.size() - 1) * kSlices * 6);
    for (GLuint r = 0; r + 1 < rings.size(); r++) {
        for (GLuint j = 0; j < kSlices; j++) {
            GLuint j1 = (j + 1) % kSlices;
            GLuint a  = r * kSlices + j;
            GLuint a1 = r * kSlices + j1;
            GLuint b  = (r + 1) * kSlices + j;
            GLuint b1 = (r + 1) * kSlices + j1;
            if (r + 2 < rings.size()) indices.insert(indices.end(), {a, b, b1});
            if (r > 0)                indices.insert(indices.end(), {b1, a1, a});
        }
    }
}

static void RoundMesh(std::vector<Vertex>& vertices, std::vector<GLuint>& indices, const Object& obj, float attached,
                      float radius, float halfLength, float centerY) {
    static const float kEnclose = [] {
        std::vector<glm::vec3> p;
        std::vector<GLuint> i;
        RoundMeshLocal(p, i, 1.0f, 0.0f, 0.0f);
        float minDist = 1.0f;
        for (size_t k = 0; k < i.size(); k += 3) {
            glm::vec3 n = glm::normalize(glm::cross(p[i[k + 1]] - p[i[k]], p[i[k + 2]] - p[i[k]]));
            minDist = std::min(minDist, std::abs(glm::dot(n, p[i[k]])));
        }
        return 1.0f / minDist;
    }();

    std::vector<glm::vec3> positions;
    std::vector<GLuint> localIndices;
    RoundMeshLocal(positions, localIndices, radius * kEnclose, halfLength, centerY);

    const glm::mat3 rot = ObjectRotation(glm::vec3(obj.data4));
    const auto base = static_cast<GLuint>(vertices.size());

    vertices.reserve(vertices.size() + positions.size());
    for (auto& p : positions) vertices.push_back({ObjectToWorld(obj, rot, p), attached});

    indices.reserve(indices.size() + localIndices.size());
    for (GLuint i : localIndices) indices.push_back(base + i);
}

void Quad(std::vector<Vertex>& vertices, std::vector<GLuint>& indices, float attached) {
    const auto base = static_cast<GLuint>(vertices.size());

    static constexpr std::array<glm::vec3, 4> kQuad = {{
        {-1.0f, -1.0f, 0.0f},
        { 1.0f, -1.0f, 0.0f},
        { 1.0f,  1.0f, 0.0f},
        {-1.0f,  1.0f, 0.0f},
    }};
    for (auto& p : kQuad) vertices.push_back({p, attached});

    indices.reserve(indices.size() + 6);
    indices.insert(indices.end(), {
        base + 0, base + 1, base + 2,
        base + 2, base + 3, base + 0,
    });
}
void Cube(std::vector<Vertex>& vertices, std::vector<GLuint>& indices, const Object& obj, float attached) {
    const auto base = static_cast<GLuint>(vertices.size());

    static constexpr std::array<glm::vec3, 8> kCube = {{
        {-1.0f, -1.0f, -1.0f},
        { 1.0f, -1.0f, -1.0f},
        { 1.0f,  1.0f, -1.0f},
        {-1.0f,  1.0f, -1.0f},
        {-1.0f, -1.0f, 1.0f},
        { 1.0f, -1.0f, 1.0f},
        { 1.0f,  1.0f, 1.0f},
        {-1.0f,  1.0f, 1.0f}
    }};
    const float radius = obj.data2.x;
    const glm::mat3 rot = ObjectRotation(glm::vec3(obj.data4));
    for (auto& p : kCube) vertices.push_back({ObjectToWorld(obj, rot, p * radius), attached});

    indices.reserve(indices.size() + 36);
    indices.insert(indices.end(), {
        base + 2, base + 3, base + 0,
        base + 0, base + 1, base + 2,

        base + 6, base + 5, base + 4,
        base + 4, base + 7, base + 6,

        base + 7, base + 4, base + 0,
        base + 0, base + 3, base + 7,

        base + 6, base + 2, base + 1,
        base + 1, base + 5, base + 6,

        base + 5, base + 1, base + 0,
        base + 0, base + 4, base + 5,

        base + 6, base + 7, base + 3,
        base + 3, base + 2, base + 6
    });
}
void Sphere(std::vector<Vertex>& vertices, std::vector<GLuint>& indices, const Object& obj, float attached) {
    RoundMesh(vertices, indices, obj, attached, obj.data2.x, 0.0f, 0.0f);
}
void Capsule(std::vector<Vertex>& vertices, std::vector<GLuint>& indices, const Object& obj, float attached) {
    const float radius = obj.data2.x;
    const float length = obj.data2.w;
    RoundMesh(vertices, indices, obj, attached, radius, length * 0.5f, length - radius);
}
void Plane(std::vector<Vertex>& vertices, std::vector<GLuint>& indices, const glm::vec3& center, float halfSize, float height, float halfThickness, float attached) {
    const auto base = static_cast<GLuint>(vertices.size());

    static constexpr std::array<glm::vec3, 8> kCube = {{
        {-1.0f, -1.0f, -1.0f},
        { 1.0f, -1.0f, -1.0f},
        { 1.0f,  1.0f, -1.0f},
        {-1.0f,  1.0f, -1.0f},
        {-1.0f, -1.0f, 1.0f},
        { 1.0f, -1.0f, 1.0f},
        { 1.0f,  1.0f, 1.0f},
        {-1.0f,  1.0f, 1.0f}
    }};
    const glm::vec3 scale = {halfSize, halfThickness, halfSize};
    const glm::vec3 offset = {center.x, height, center.z};
    for (auto& p : kCube) vertices.push_back({offset + p * scale, attached});

    indices.reserve(indices.size() + 36);
    indices.insert(indices.end(), {
        base + 2, base + 3, base + 0,
        base + 0, base + 1, base + 2,

        base + 6, base + 5, base + 4,
        base + 4, base + 7, base + 6,

        base + 7, base + 4, base + 0,
        base + 0, base + 3, base + 7,

        base + 6, base + 2, base + 1,
        base + 1, base + 5, base + 6,

        base + 5, base + 1, base + 0,
        base + 0, base + 4, base + 5,

        base + 6, base + 7, base + 3,
        base + 3, base + 2, base + 6
    });
}
bool GenerateObject(std::vector<Vertex>& vertices, std::vector<GLuint>& indices, const Object& obj, float attached) {
    const int type = int(std::round(obj.data1.x));
    if (type == -1) {
        Object proxy = obj;
        proxy.data2.x *= 1.05f;
        Sphere(vertices, indices, proxy, attached);
        return true;
    }
    switch (std::abs(type)) {
        case 1: Sphere(vertices, indices, obj, attached);  return true;
        case 2: Cube(vertices, indices, obj, attached);    return true;
        case 3: Capsule(vertices, indices, obj, attached); return true;
        default: return false;
    }
}
float ObjectBoundingRadius(const Object& obj) {
    const int type = int(std::round(obj.data1.x));
    const float radius = obj.data2.x;
    switch (std::abs(type)) {
        case 1: return (type < 0) ? radius * 1.05f : radius;
        case 2: return radius * std::sqrt(3.0f);
        case 3: return std::abs(obj.data2.w - radius) + obj.data2.w * 0.5f + radius;
        default: return -1.0f;
    }
}
GpuObject ToGpuObject(const Object& obj, const VertexAttributes& attributes) {
    const glm::mat3 invRot = glm::transpose(ObjectRotation(glm::vec3(obj.data4)));
    return {obj.data1, obj.data2, obj.data3, {
        glm::vec4(invRot[0], attributes.polygonal ? 1.0f : 0.0f),
        glm::vec4(invRot[1], attributes.attached ? 1.0f : 0.0f),
        glm::vec4(invRot[2], 0.0f),
    }};
}
glm::mat3 CameraRotation(const glm::vec3& deg) {
    const glm::vec3 r = glm::radians(deg);
    float cp = std::cos(r.x), sp = std::sin(r.x);
    float cy = std::cos(r.y), sy = std::sin(r.y);
    float cr = std::cos(r.z), sr = std::sin(r.z);

    glm::mat3 Rx({1.0f, 0.0f, 0.0f}, {0.0f, cp, sp}, {0.0f, -sp, cp});
    glm::mat3 Ry({cy, 0.0f, -sy}, {0.0f, 1.0f, 0.0f}, {sy, 0.0f, cy});
    glm::mat3 Rz({cr, sr, 0.0f}, {-sr, cr, 0.0f}, {0.0f, 0.0f, 1.0f});

    return Rz * Rx * Ry;
}
GpuObject AttachedToWorld(const GpuObject& obj, const glm::vec3& cameraPos, const glm::mat3& cameraRot) {
    GpuObject world = obj;
    const glm::vec3 pos = cameraPos + glm::transpose(cameraRot) * glm::vec3(obj.data1.y, obj.data1.z, obj.data1.w);
    world.data1 = glm::vec4(obj.data1.x, pos);
    const glm::mat3 invRot = glm::mat3(glm::vec3(obj.invRot[0]), glm::vec3(obj.invRot[1]), glm::vec3(obj.invRot[2])) * cameraRot;
    for (int i = 0; i < 3; i++) world.invRot[i] = glm::vec4(invRot[i], obj.invRot[i].w);
    return world;
}
void SetObjectAttributes(std::vector<Vertex>& vertices, size_t vertexStart, GLint objectIdx, float polygonal) {
    for (size_t i = vertexStart; i < vertices.size(); i++) {
        vertices[i].objectIdx = objectIdx;
        vertices[i].polygonal = polygonal;
    }
}
void CullFarTriangles(std::vector<Vertex>& vertices, std::vector<GLuint>& indices, size_t vertexStart, size_t indexStart, const glm::vec3& center, float maxDist) {
    const float maxDist2 = maxDist * maxDist;
    auto isFar = [&](GLuint i) {
        glm::vec3 d = vertices[i].pos - center;
        return glm::dot(d, d) > maxDist2;
    };

    std::vector<GLuint> remap(vertices.size() - vertexStart, GLuint(-1));
    std::vector<Vertex> kept;
    size_t indexEnd = indexStart;

    for (size_t k = indexStart; k + 2 < indices.size(); k += 3) {
        const GLuint tri[3] = {indices[k], indices[k + 1], indices[k + 2]};
        if (isFar(tri[0]) && isFar(tri[1]) && isFar(tri[2])) continue;
        for (GLuint i : tri) {
            GLuint& r = remap[i - vertexStart];
            if (r == GLuint(-1)) {
                r = static_cast<GLuint>(vertexStart + kept.size());
                kept.push_back(vertices[i]);
            }
            indices[indexEnd++] = r;
        }
    }

    indices.resize(indexEnd);
    vertices.resize(vertexStart);
    vertices.insert(vertices.end(), kept.begin(), kept.end());
}
void AntiCube(std::vector<Vertex>& vertices, std::vector<GLuint>& indices, float radius, float attached) {
    const auto base = static_cast<GLuint>(vertices.size());

    static constexpr std::array<glm::vec3, 8> kCube = {{
        {-1.0f, -1.0f, -1.0f},
        { 1.0f, -1.0f, -1.0f},
        { 1.0f,  1.0f, -1.0f},
        {-1.0f,  1.0f, -1.0f},
        {-1.0f, -1.0f, 1.0f},
        { 1.0f, -1.0f, 1.0f},
        { 1.0f,  1.0f, 1.0f},
        {-1.0f,  1.0f, 1.0f}
    }};
    for (auto& p : kCube) vertices.push_back({p * radius, attached});

    indices.reserve(indices.size() + 12);
    indices.insert(indices.end(), {
        base + 0, base + 3, base + 2,
        base + 2, base + 1, base + 0,

        base + 4, base + 5, base + 6,
        base + 6, base + 7, base + 4,

        base + 0, base + 4, base + 7,
        base + 7, base + 3, base + 0,

        base + 1, base + 2, base + 6,
        base + 6, base + 5, base + 1,

        base + 0, base + 1, base + 5,
        base + 5, base + 4, base + 0,

        base + 3, base + 7, base + 6,
        base + 6, base + 2, base + 3
    });
}
