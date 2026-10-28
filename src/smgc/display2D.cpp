#include <smgc/display2D.h>

#include <smpb/Core.h>

#include <cmath>

static bool IsNonNegative(float value) {
    return std::isfinite(value) && value >= 0.0f;
}

SMGC2D::SMGC2D() : BaseComponent("SMGC2D") {
}

int SMGC2D::init() {
    glfwSetErrorCallback([](int error, const char* desc){
        std::cerr << "GLFW Error " << error << ": " << desc << "\n";
    });

    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    window = glfwCreateWindow(windowWidth, windowHeight, "SMGC2D", NULL, NULL);
    if (!window) {
        std::cerr << getPrefix() << "glfwCreateWindow failed\n";
        glfwTerminate();
        return -ENODEV;
    }
    glfwMakeContextCurrent(window);
    glfwGetFramebufferSize(window, &windowWidth, &windowHeight);

    glfwSetWindowUserPointer(window, this);

    glfwSetFramebufferSizeCallback(window, [](GLFWwindow* w, int fbW, int fbH) {
        auto* self = static_cast<SMGC2D*>(glfwGetWindowUserPointer(w));
        if (!self) return;
        self->windowWidth  = fbW;
        self->windowHeight = fbH;
        glViewport(0, 0, fbW, fbH);
    });

    int errorCode = gladLoadGL();
    if (errorCode == 0) {std::cerr << getPrefix() << "Critical Error: gladLoadGL failed, return code " << std::to_string(errorCode) << std::endl; return -ENODEV;}
    std::clog << getPrefix() << "GL: " << glGetString(GL_VERSION) << "\n";

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glViewport(0, 0, windowWidth, windowHeight);

    shaderProgram = std::make_unique<Shader>("files/display2D.vert", "files/display2D.frag");
    if (!shaderProgram) {std::cerr << getPrefix() << "Critical Error: shaderProgram is nullptr" << std::endl; return -ENODEV;}
    errorCode = shaderProgram->getStatus();
    if (errorCode != 0) {std::cerr << getPrefix() << "Critical Error: shaderProgram status code is " << std::to_string(errorCode) << std::endl; return errorCode;}

    vao = std::make_unique<VAO>();
    if (!vao) {std::cerr << getPrefix() << "Critical Error: vao is nullptr" << std::endl; return -ENODEV;}
    vao->Bind();

    vbo = std::make_unique<VBO>();
    if (!vbo) {std::cerr << getPrefix() << "Critical Error: vbo is nullptr" << std::endl; return -ENODEV;}

    ebo = std::make_unique<EBO>();
    if (!ebo) {std::cerr << getPrefix() << "Critical Error: ebo is nullptr" << std::endl; return -ENODEV;}
    ebo->Bind();

    vao->LinkAttrib (*vbo, 0, 2, GL_FLOAT, sizeof(Vertex2D), (void*)offsetof(Vertex2D, pos));
    vao->LinkAttrib (*vbo, 1, 2, GL_FLOAT, sizeof(Vertex2D), (void*)offsetof(Vertex2D, local));
    vao->LinkAttrib (*vbo, 2, 4, GL_FLOAT, sizeof(Vertex2D), (void*)offsetof(Vertex2D, color));
    vao->LinkAttrib (*vbo, 3, 4, GL_FLOAT, sizeof(Vertex2D), (void*)offsetof(Vertex2D, params));
    vao->LinkAttribI(*vbo, 4, 1, GL_INT,   sizeof(Vertex2D), (void*)offsetof(Vertex2D, shape));

    vao->Unbind();
    vbo->Unbind();
    ebo->Unbind();

    clearShapes();
    swapShapes();

    return 0;
}

void SMGC2D::swapShapes() {
    std::swap(Vertices_A, Vertices_B);
    std::swap(Indices_A, Indices_B);

    vao->Unbind();
    vbo->Upload(Vertices_A.data(), sizeof(Vertex2D) * Vertices_A.size());
    ebo->Upload(Indices_A);
}
void SMGC2D::clearShapes() {
    Vertices_B.clear();
    Indices_B.clear();
}

int SMGC2D::invalidArgument(const char* function, const char* name, float value) {
    std::cerr << getPrefix() << "Error in SMGC2D::" << function << ": invalid " << name << " " << std::to_string(value) << std::endl;
    return -EINVAL;
}

int SMGC2D::drawRect(const glm::vec2& pos, const glm::vec2& size, const glm::vec4& color) {
    if (!IsNonNegative(size.x)) return invalidArgument("drawRect", "size.x", size.x);
    if (!IsNonNegative(size.y)) return invalidArgument("drawRect", "size.y", size.y);
    Rect2D(Vertices_B, Indices_B, pos, size, color);
    return 0;
}
int SMGC2D::drawTriangle(const glm::vec2& a, const glm::vec2& b, const glm::vec2& c, const glm::vec4& color) {
    Triangle2D(Vertices_B, Indices_B, a, b, c, color);
    return 0;
}
int SMGC2D::drawCircle(const glm::vec2& center, float radius, const glm::vec4& color) {
    if (!IsNonNegative(radius)) return invalidArgument("drawCircle", "radius", radius);
    Circle2D(Vertices_B, Indices_B, center, radius, color);
    return 0;
}
int SMGC2D::drawEllipse(const glm::vec2& center, const glm::vec2& radii, const glm::vec4& color) {
    if (!IsNonNegative(radii.x)) return invalidArgument("drawEllipse", "radii.x", radii.x);
    if (!IsNonNegative(radii.y)) return invalidArgument("drawEllipse", "radii.y", radii.y);
    Ellipse2D(Vertices_B, Indices_B, center, radii, color);
    return 0;
}
int SMGC2D::drawRoundedRect(const glm::vec2& pos, const glm::vec2& size, float cornerRadius, const glm::vec4& color) {
    if (!IsNonNegative(size.x)) return invalidArgument("drawRoundedRect", "size.x", size.x);
    if (!IsNonNegative(size.y)) return invalidArgument("drawRoundedRect", "size.y", size.y);
    if (!IsNonNegative(cornerRadius)) return invalidArgument("drawRoundedRect", "cornerRadius", cornerRadius);
    RoundedRect2D(Vertices_B, Indices_B, pos, size, cornerRadius, color);
    return 0;
}
int SMGC2D::drawRing(const glm::vec2& center, float radius, float thickness, const glm::vec4& color) {
    if (!IsNonNegative(radius)) return invalidArgument("drawRing", "radius", radius);
    if (!IsNonNegative(thickness)) return invalidArgument("drawRing", "thickness", thickness);
    Ring2D(Vertices_B, Indices_B, center, radius, thickness, color);
    return 0;
}
int SMGC2D::drawLine(const glm::vec2& a, const glm::vec2& b, float thickness, const glm::vec4& color) {
    if (!IsNonNegative(thickness)) return invalidArgument("drawLine", "thickness", thickness);
    Line2D(Vertices_B, Indices_B, a, b, thickness, color);
    return 0;
}

void SMGC2D::setOutline(bool enabled) {
    outlineEnabled = enabled;
}
int SMGC2D::subscribeKey(int componentIdx, int key) {
    if (key < GLFW_KEY_SPACE || key > GLFW_KEY_LAST) {
        std::cerr << getPrefix() << "Error in SMGC2D::subscribeKey: invalid key " << std::to_string(key) << std::endl;
        return -EINVAL;
    }
    for (const auto& subscription : keySubscriptions) {
        if (subscription.componentIdx == componentIdx && subscription.key == key) return 0;
    }
    keySubscriptions.push_back({componentIdx, key});
    return 0;
}
int SMGC2D::unsubscribeKey(int componentIdx, int key) {
    std::erase_if(keySubscriptions, [&](const KeySubscription& subscription) {
        return subscription.componentIdx == componentIdx && subscription.key == key;
    });
    return 0;
}
void SMGC2D::sendKeyStates() {
    const std::vector<KeySubscription> subscriptions = keySubscriptions;
    for (const auto& subscription : subscriptions) {
        bool pressed = glfwGetKey(window, subscription.key) == GLFW_PRESS;
        sendTask(std::make_unique<task_SmgcKeyState>(subscription.key, pressed), subscription.componentIdx);
    }
}

int SMGC2D::draw() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    if (!shaderProgram) {std::cerr << getPrefix() << "Critical Error: shaderProgram is nullptr" << std::endl; return -ENODEV;}
    shaderProgram->Activate();
    shaderProgram->setVec2("iResolution", float(windowWidth), float(windowHeight));

    if (!vao) {std::cerr << getPrefix() << "Critical Error: vao is nullptr" << std::endl; return -ENODEV;}
    vao->Bind();

    shaderProgram->setFloat("iOutline", 0.0f);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDrawElements(GL_TRIANGLES, Indices_A.size(), GL_UNSIGNED_INT, 0);

    if (outlineEnabled) {
        shaderProgram->setFloat("iOutline", 1.0f);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glLineWidth(1.0f);
        glDrawElements(GL_TRIANGLES, Indices_A.size(), GL_UNSIGNED_INT, 0);
    }

    vao->Unbind();

    return 0;
}

int SMGC2D::processTask(std::unique_ptr<BaseTask> task) {
    uint32_t taskIdx = task->getIdx();
    if (taskIs_SmgcSwapObjects(taskIdx)) {
        swapShapes();
    } else if (taskIs_SmgcClearObjects(taskIdx)) {
        clearShapes();
    } else if (taskIs_SmgcDrawRect(taskIdx)) {
        auto* typed = static_cast<task_SmgcDrawRect*>(task.get());
        return drawRect(typed->pos, typed->size, typed->color);
    } else if (taskIs_SmgcDrawTriangle(taskIdx)) {
        auto* typed = static_cast<task_SmgcDrawTriangle*>(task.get());
        return drawTriangle(typed->a, typed->b, typed->c, typed->color);
    } else if (taskIs_SmgcDrawCircle(taskIdx)) {
        auto* typed = static_cast<task_SmgcDrawCircle*>(task.get());
        return drawCircle(typed->center, typed->radius, typed->color);
    } else if (taskIs_SmgcDrawEllipse(taskIdx)) {
        auto* typed = static_cast<task_SmgcDrawEllipse*>(task.get());
        return drawEllipse(typed->center, typed->radii, typed->color);
    } else if (taskIs_SmgcDrawRoundedRect(taskIdx)) {
        auto* typed = static_cast<task_SmgcDrawRoundedRect*>(task.get());
        return drawRoundedRect(typed->pos, typed->size, typed->cornerRadius, typed->color);
    } else if (taskIs_SmgcDrawRing(taskIdx)) {
        auto* typed = static_cast<task_SmgcDrawRing*>(task.get());
        return drawRing(typed->center, typed->radius, typed->thickness, typed->color);
    } else if (taskIs_SmgcDrawLine(taskIdx)) {
        auto* typed = static_cast<task_SmgcDrawLine*>(task.get());
        return drawLine(typed->a, typed->b, typed->thickness, typed->color);
    } else if (taskIs_SmgcSetOutline(taskIdx)) {
        auto* typed = static_cast<task_SmgcSetOutline*>(task.get());
        setOutline(typed->enabled);
    } else if (taskIs_SmgcSubscribeKey(taskIdx)) {
        auto* typed = static_cast<task_SmgcSubscribeKey*>(task.get());
        return subscribeKey(task->getCreatorIdx(), typed->key);
    } else if (taskIs_SmgcUnsubscribeKey(taskIdx)) {
        auto* typed = static_cast<task_SmgcUnsubscribeKey*>(task.get());
        return unsubscribeKey(task->getCreatorIdx(), typed->key);
    }
    return 0;
}

int SMGC2D::tick(long long tickCounter) {
    if (glfwWindowShouldClose(window)) {
        Core_stop();
    } else {
        sendKeyStates();

        int errorCode = draw();
        if (errorCode != 0) return errorCode;

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    return 0;
}

void SMGC2D::death() {
    keySubscriptions.clear();
    if (vao) vao->Delete();
    if (vbo) vbo->Delete();
    if (ebo) ebo->Delete();
    if (shaderProgram) shaderProgram->Delete();
    if (window) glfwDestroyWindow(window);
    glfwTerminate();
    Core_stop();
}
