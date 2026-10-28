#include <smgc/Layer2D.h>

#include <iostream>

static constexpr const char* kVertexShaderPath   = "files/display2D.vert";
static constexpr const char* kFragmentShaderPath = "files/display2D.frag";
static constexpr const char* kDefaultFontPath    = "files/DejaVuSans.ttf";

int Layer2D::init() {
    shaderProgram = std::make_unique<Shader>(kVertexShaderPath, kFragmentShaderPath);
    int errorCode = shaderProgram->getStatus();
    if (errorCode != 0) {std::cerr << "Critical Error in Layer2D::init: shaderProgram status code is " << std::to_string(errorCode) << std::endl; return errorCode;}

    errorCode = font.Load(kDefaultFontPath);
    if (errorCode != 0) {std::cerr << "Critical Error in Layer2D::init: font.Load return error code " << std::to_string(errorCode) << std::endl; return errorCode;}

    vao = std::make_unique<VAO>();
    vbo = std::make_unique<VBO>();
    ebo = std::make_unique<EBO>();

    vao->Bind();
    ebo->Bind();

    vao->LinkAttrib (*vbo, 0, 2, GL_FLOAT, sizeof(Vertex2D), (void*)offsetof(Vertex2D, pos));
    vao->LinkAttrib (*vbo, 1, 2, GL_FLOAT, sizeof(Vertex2D), (void*)offsetof(Vertex2D, local));
    vao->LinkAttrib (*vbo, 2, 4, GL_FLOAT, sizeof(Vertex2D), (void*)offsetof(Vertex2D, color));
    vao->LinkAttrib (*vbo, 3, 4, GL_FLOAT, sizeof(Vertex2D), (void*)offsetof(Vertex2D, params));
    vao->LinkAttribI(*vbo, 4, 1, GL_INT,   sizeof(Vertex2D), (void*)offsetof(Vertex2D, shape));

    vao->Unbind();
    vbo->Unbind();
    ebo->Unbind();

    return 0;
}

void Layer2D::clear() {
    Vertices_B.clear();
    Indices_B.clear();
}

void Layer2D::swap() {
    std::swap(Vertices_A, Vertices_B);
    std::swap(Indices_A, Indices_B);

    vao->Unbind();
    vbo->Upload(Vertices_A.data(), sizeof(Vertex2D) * Vertices_A.size());
    ebo->Upload(Indices_A);
}

void Layer2D::pushRect(const glm::vec2& pos, const glm::vec2& size, const glm::vec4& color) {
    Rect2D(Vertices_B, Indices_B, pos, size, color);
}
void Layer2D::pushTriangle(const glm::vec2& a, const glm::vec2& b, const glm::vec2& c, const glm::vec4& color) {
    Triangle2D(Vertices_B, Indices_B, a, b, c, color);
}
void Layer2D::pushCircle(const glm::vec2& center, float radius, const glm::vec4& color) {
    Circle2D(Vertices_B, Indices_B, center, radius, color);
}
void Layer2D::pushEllipse(const glm::vec2& center, const glm::vec2& radii, const glm::vec4& color) {
    Ellipse2D(Vertices_B, Indices_B, center, radii, color);
}
void Layer2D::pushRoundedRect(const glm::vec2& pos, const glm::vec2& size, float cornerRadius, const glm::vec4& color) {
    RoundedRect2D(Vertices_B, Indices_B, pos, size, cornerRadius, color);
}
void Layer2D::pushRing(const glm::vec2& center, float radius, float thickness, const glm::vec4& color) {
    Ring2D(Vertices_B, Indices_B, center, radius, thickness, color);
}
void Layer2D::pushLine(const glm::vec2& a, const glm::vec2& b, float thickness, const glm::vec4& color) {
    Line2D(Vertices_B, Indices_B, a, b, thickness, color);
}
void Layer2D::pushText(const std::string& text, const glm::vec2& pos, float size, const glm::vec4& color, TextAlign align) {
    Text2D(Vertices_B, Indices_B, font, text, pos, size, color, align);
}

int Layer2D::setFont(const std::string& path) {
    return font.Load(path);
}

void Layer2D::draw(int width, int height, bool outline) {
    if (Indices_A.empty()) return;

    const GLboolean depthTest = glIsEnabled(GL_DEPTH_TEST);
    const GLboolean cullFace = glIsEnabled(GL_CULL_FACE);
    const GLboolean blend = glIsEnabled(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    shaderProgram->Activate();
    shaderProgram->setVec2("iResolution", float(width), float(height));
    font.Bind(0);

    vao->Bind();

    shaderProgram->setFloat("iOutline", 0.0f);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDrawElements(GL_TRIANGLES, Indices_A.size(), GL_UNSIGNED_INT, 0);

    if (outline) {
        shaderProgram->setFloat("iOutline", 1.0f);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glLineWidth(1.0f);
        glDrawElements(GL_TRIANGLES, Indices_A.size(), GL_UNSIGNED_INT, 0);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    vao->Unbind();

    if (depthTest) glEnable(GL_DEPTH_TEST);
    if (cullFace) glEnable(GL_CULL_FACE);
    if (!blend) glDisable(GL_BLEND);
}

void Layer2D::destroy() {
    if (vao) vao->Delete();
    if (vbo) vbo->Delete();
    if (ebo) ebo->Delete();
    if (shaderProgram && shaderProgram->getStatus() == 0) shaderProgram->Delete();
    font.Delete();
    vao = nullptr;
    vbo = nullptr;
    ebo = nullptr;
    shaderProgram = nullptr;
}
