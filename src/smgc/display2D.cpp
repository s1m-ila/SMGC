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
    glfwWindowHint(GLFW_SAMPLES, 4);

    window = glfwCreateWindow(windowWidth, windowHeight, "SMGC2D", NULL, NULL);
    if (!window) {
        std::cerr << getPrefix() << "glfwCreateWindow failed\n";
        glfwTerminate();
        return -ENODEV;
    }
    glfwMakeContextCurrent(window);
    glfwGetFramebufferSize(window, &windowWidth, &windowHeight);

    glfwSetWindowUserPointer(window, this);

    glfwSetMouseButtonCallback(window, [](GLFWwindow* w, int button, int action, int mods) {
        auto* self = static_cast<SMGC2D*>(glfwGetWindowUserPointer(w));
        if (!self) return;

        // порядок: 0 = ЛКМ, 1 = колесо, 2 = ПКМ
        int idx = -1;
        if      (button == GLFW_MOUSE_BUTTON_LEFT)  idx = 0;
        else if (button == GLFW_MOUSE_BUTTON_RIGHT) idx = 2;
        // GLFW_MOUSE_BUTTON_MIDDLE (колесо) можно тоже поддержать:
        else if (button == GLFW_MOUSE_BUTTON_MIDDLE) idx = 1;

        if (idx < 0) return;

        if (action == GLFW_PRESS) {
            self->mouse.state[idx]   = 1.0f;
            self->mouse.press[idx]   = 1.0f;   // нажатие в этом тике
        } else if (action == GLFW_RELEASE) {
            self->mouse.state[idx]   = 0.0f;
            self->mouse.release[idx] = 1.0f;   // отпускание в этом тике
        }
    });

    glfwSetScrollCallback(window, [](GLFWwindow* w, double /*xoffset*/, double yoffset) {
        auto* self = static_cast<SMGC2D*>(glfwGetWindowUserPointer(w));
        if (!self) return;
        self->mouse.wheelMove += static_cast<float>(yoffset);
    });

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

    glEnable(GL_MULTISAMPLE);
    glViewport(0, 0, windowWidth, windowHeight);

    errorCode = layer.init();
    if (errorCode != 0) {std::cerr << getPrefix() << "Critical Error: layer.init return error code " << std::to_string(errorCode) << std::endl; return errorCode;}

    clearShapes();
    swapShapes();

    return 0;
}

void SMGC2D::swapShapes() {
    layer.swap();
}
void SMGC2D::clearShapes() {
    layer.clear();
}

int SMGC2D::invalidArgument(const char* function, const char* name, float value) {
    std::cerr << getPrefix() << "Error in SMGC2D::" << function << ": invalid " << name << " " << std::to_string(value) << std::endl;
    return -EINVAL;
}

int SMGC2D::nullPointer(const char* function) {
    std::cerr << getPrefix() << "Error in SMGC2D::" << function << ": null pointer\n";
    return -EINVAL;
}

int SMGC2D::drawRect(const glm::vec2& pos, const glm::vec2& size, const glm::vec4& color) {
    if (!IsNonNegative(size.x)) return invalidArgument("drawRect", "size.x", size.x);
    if (!IsNonNegative(size.y)) return invalidArgument("drawRect", "size.y", size.y);
    layer.pushRect(pos, size, color);
    return 0;
}
int SMGC2D::drawTriangle(const glm::vec2& a, const glm::vec2& b, const glm::vec2& c, const glm::vec4& color) {
    layer.pushTriangle(a, b, c, color);
    return 0;
}
int SMGC2D::drawCircle(const glm::vec2& center, float radius, const glm::vec4& color) {
    if (!IsNonNegative(radius)) return invalidArgument("drawCircle", "radius", radius);
    layer.pushCircle(center, radius, color);
    return 0;
}
int SMGC2D::drawEllipse(const glm::vec2& center, const glm::vec2& radii, const glm::vec4& color) {
    if (!IsNonNegative(radii.x)) return invalidArgument("drawEllipse", "radii.x", radii.x);
    if (!IsNonNegative(radii.y)) return invalidArgument("drawEllipse", "radii.y", radii.y);
    layer.pushEllipse(center, radii, color);
    return 0;
}
int SMGC2D::drawRoundedRect(const glm::vec2& pos, const glm::vec2& size, float cornerRadius, const glm::vec4& color) {
    if (!IsNonNegative(size.x)) return invalidArgument("drawRoundedRect", "size.x", size.x);
    if (!IsNonNegative(size.y)) return invalidArgument("drawRoundedRect", "size.y", size.y);
    if (!IsNonNegative(cornerRadius)) return invalidArgument("drawRoundedRect", "cornerRadius", cornerRadius);
    layer.pushRoundedRect(pos, size, cornerRadius, color);
    return 0;
}
int SMGC2D::drawRing(const glm::vec2& center, float radius, float thickness, const glm::vec4& color) {
    if (!IsNonNegative(radius)) return invalidArgument("drawRing", "radius", radius);
    if (!IsNonNegative(thickness)) return invalidArgument("drawRing", "thickness", thickness);
    layer.pushRing(center, radius, thickness, color);
    return 0;
}
int SMGC2D::drawLine(const glm::vec2& a, const glm::vec2& b, float thickness, const glm::vec4& color) {
    if (!IsNonNegative(thickness)) return invalidArgument("drawLine", "thickness", thickness);
    layer.pushLine(a, b, thickness, color);
    return 0;
}
int SMGC2D::drawText(const std::string& text, const glm::vec2& pos, float size, const glm::vec4& color, TextAlign align) {
    if (!IsNonNegative(size)) return invalidArgument("drawText", "size", size);
    layer.pushText(text, pos, size, color, align);
    return 0;
}
int SMGC2D::drawTexture(uint32_t id, const glm::vec2& pos, const glm::vec2& size, const glm::vec4& color, const std::optional<glm::vec4>& srcRect) {
    if (!IsNonNegative(size.x)) return invalidArgument("drawTexture", "size.x", size.x);
    if (!IsNonNegative(size.y)) return invalidArgument("drawTexture", "size.y", size.y);
    if (srcRect) {
        if (!std::isfinite(srcRect->x)) return invalidArgument("drawTexture", "srcRect.x", srcRect->x);
        if (!std::isfinite(srcRect->y)) return invalidArgument("drawTexture", "srcRect.y", srcRect->y);
        if (!IsNonNegative(srcRect->z)) return invalidArgument("drawTexture", "srcRect.width", srcRect->z);
        if (!IsNonNegative(srcRect->w)) return invalidArgument("drawTexture", "srcRect.height", srcRect->w);
    }
    layer.pushTexture(id, pos, size, color, srcRect);
    return 0;
}
int SMGC2D::loadTexture(const std::string& path, uint32_t* id) {
    if (!id) return nullPointer("loadTexture");
    *id = layer.textureStore().Load(path);
    if (*id == kInvalidTexture) std::cerr << getPrefix() << "Error in SMGC2D::loadTexture: cannot load \"" << path << "\"" << std::endl;
    return 0;
}
int SMGC2D::freeTexture(uint32_t id) {
    layer.textureStore().Free(id);
    return 0;
}
int SMGC2D::getTextureSize(uint32_t id, int* width, int* height) {
    if (!width || !height) return nullPointer("getTextureSize");
    if (!layer.textureStore().GetSize(id, width, height)) {
        *width = -1;
        *height = -1;
    }
    return 0;
}
int SMGC2D::cloneTexture(uint32_t id, uint32_t* result) {
    if (!result) return nullPointer("cloneTexture");
    *result = layer.textureStore().Clone(id);
    return 0;
}
int SMGC2D::tintTexture(uint32_t id, uint32_t maskId, const glm::vec3& color, uint32_t* result) {
    if (!result) return nullPointer("tintTexture");
    *result = layer.textureStore().Tint(id, maskId, color);
    return 0;
}
int SMGC2D::mergeTextures(uint32_t baseId, uint32_t overlayId, const glm::ivec2& offset, uint32_t* result) {
    if (!result) return nullPointer("mergeTextures");
    *result = layer.textureStore().Merge(baseId, overlayId, offset);
    return 0;
}
int SMGC2D::setFont(const std::string& path) {
    int errorCode = layer.setFont(path);
    if (errorCode != 0) std::cerr << getPrefix() << "Error in SMGC2D::setFont: cannot load \"" << path << "\", the previous font is kept" << std::endl;
    return errorCode;
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

int SMGC2D::measureText(const std::string& text, float size, float* width, float* height) {
    if (!IsNonNegative(size)) return invalidArgument("measureText", "size", size);
    if (!width || !height) {
        std::cerr << getPrefix() << "Error in SMGC2D::measureText: null pointer\n";
        return -EINVAL;
    }
    const TextMetrics m = layer.measureText(text, size);
    *width  = m.width;
    *height = m.height;
    return 0;
}

int SMGC2D::draw() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    layer.draw(windowWidth, windowHeight, outlineEnabled);

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
    } else if (taskIs_SmgcDrawText(taskIdx)) {
        auto* typed = static_cast<task_SmgcDrawText*>(task.get());
        return drawText(typed->text, typed->pos, typed->size, typed->color, typed->align);
    } else if (taskIs_SmgcDrawTexture(taskIdx)) {
        auto* typed = static_cast<task_SmgcDrawTexture*>(task.get());
        return drawTexture(typed->id, typed->pos, typed->size, typed->color, typed->srcRect);
    } else if (taskIs_SmgcLoadTexture(taskIdx)) {
        auto* typed = static_cast<task_SmgcLoadTexture*>(task.get());
        return loadTexture(typed->path, typed->id);
    } else if (taskIs_SmgcFreeTexture(taskIdx)) {
        auto* typed = static_cast<task_SmgcFreeTexture*>(task.get());
        return freeTexture(typed->id);
    } else if (taskIs_SmgcGetTextureSize(taskIdx)) {
        auto* typed = static_cast<task_SmgcGetTextureSize*>(task.get());
        return getTextureSize(typed->id, typed->width, typed->height);
    } else if (taskIs_SmgcCloneTexture(taskIdx)) {
        auto* typed = static_cast<task_SmgcCloneTexture*>(task.get());
        return cloneTexture(typed->id, typed->result);
    } else if (taskIs_SmgcTintTexture(taskIdx)) {
        auto* typed = static_cast<task_SmgcTintTexture*>(task.get());
        return tintTexture(typed->id, typed->maskId, typed->color, typed->result);
    } else if (taskIs_SmgcMergeTextures(taskIdx)) {
        auto* typed = static_cast<task_SmgcMergeTextures*>(task.get());
        return mergeTextures(typed->baseId, typed->overlayId, typed->offset, typed->result);
    } else if (taskIs_SmgcSetFont(taskIdx)) {
        auto* typed = static_cast<task_SmgcSetFont*>(task.get());
        return setFont(typed->path);
    } else if (taskIs_SmgcSetOutline(taskIdx)) {
        auto* typed = static_cast<task_SmgcSetOutline*>(task.get());
        setOutline(typed->enabled);
    } else if (taskIs_SmgcSubscribeKey(taskIdx)) {
        auto* typed = static_cast<task_SmgcSubscribeKey*>(task.get());
        return subscribeKey(task->getCreatorIdx(), typed->key);
    } else if (taskIs_SmgcUnsubscribeKey(taskIdx)) {
        auto* typed = static_cast<task_SmgcUnsubscribeKey*>(task.get());
        return unsubscribeKey(task->getCreatorIdx(), typed->key);
    } else if (taskIs_SmgcGetWindowSize(taskIdx)) {
        auto* typed = static_cast<task_SmgcGetWindowSize*>(task.get());
        *typed->width = windowWidth;
        *typed->height = windowHeight;
        return 0;
    } else if (taskIs_SmgcGetMouse(taskIdx)) {
        auto* typed = static_cast<task_SmgcGetMouse*>(task.get());
        *typed->mouse = mouse;
        return 0;
    } else if (taskIs_SmgcMeasureText(taskIdx)) {
        auto* typed = static_cast<task_SmgcMeasureText*>(task.get());
        return measureText(typed->text, typed->size, typed->width, typed->height);
    }
    return 0;
}

int SMGC2D::tick(long long tickCounter) {
    // сброс однократных событий предыдущего кадра
    mouse.press   = glm::vec3(0.0f);
    mouse.release = glm::vec3(0.0f);
    mouse.wheelMove     = 0.0f;

    glfwPollEvents();   // <-- накопление Press/Release/wheel для ЭТОГО тика

    if (glfwWindowShouldClose(window)) { Core_stop(); return 0; }

    double x, y;
    glfwGetCursorPos(window, &x, &y);
    if (x < 0.0f || y < 0.0f || x > windowWidth || y > windowHeight) x = 0.0f, y = 0.0f;
    mouse.pos = glm::vec2(x, y);

    mouse.state[0] = (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT)   == GLFW_PRESS) ? 1.f : 0.f;
    mouse.state[1] = (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS) ? 1.f : 0.f;
    mouse.state[2] = (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT)  == GLFW_PRESS) ? 1.f : 0.f;

    sendKeyStates();

    int errorCode = draw();
    if (errorCode != 0) return errorCode;

    glfwSwapBuffers(window);
    return 0;
}

void SMGC2D::death() {
    keySubscriptions.clear();
    layer.destroy();
    if (window) glfwDestroyWindow(window);
    glfwTerminate();
    Core_stop();
}
