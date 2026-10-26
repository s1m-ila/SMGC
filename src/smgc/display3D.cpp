#include <smgc/display3D.h>

// Core class
#include <smpb/Core.h>

SMGC::SMGC() : BaseComponent("SMGC") {
    // Constructor
}

int SMGC::init() {
    glfwSetErrorCallback([](int error, const char* desc){
        std::cerr << "GLFW Error " << error << ": " << desc << "\n";
    });

    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    window = glfwCreateWindow(windowWidth, windowHeight, "SMGC", NULL, NULL);
    if (!window) {
        std::cerr << getPrefix() << "glfwCreateWindow failed\n";
        glfwTerminate();
        return -ENODEV;
    }
    glfwMakeContextCurrent(window);


    glfwSetWindowUserPointer(window, this);  // привязываем this к окну

    glfwSetFramebufferSizeCallback(window, [](GLFWwindow* w, int fbW, int fbH) {
        auto* self = static_cast<SMGC*>(glfwGetWindowUserPointer(w));
        if (!self) return;
        self->windowWidth  = fbW;
        self->windowHeight = fbH;
        self->windowResize();
        glViewport(0, 0, fbW, fbH);
    });

    int errorCode = gladLoadGL();
    if (errorCode == 0) {std::cerr << getPrefix() << "Critical Error: gladLoadGL failed, return code " << std::to_string(errorCode) << std::endl; return -ENODEV;}
    std::clog << getPrefix() << "GL: " << glGetString(GL_VERSION) << "\n";

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    glViewport(0, 0, windowWidth, windowHeight);

    shaderProgram = std::make_unique<Shader>("files/default.vert", "files/default.frag");
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

    objectsBuffer = std::make_unique<ObjectsBuffer>();
    if (!objectsBuffer) {std::cerr << getPrefix() << "Critical Error: objectsBuffer is nullptr" << std::endl; return -ENODEV;}
    objectsBuffer->Bind(0);

    vao->LinkVBO(*vbo, 0, 1, 2, 3);
    
    vao->Unbind();
    vbo->Unbind();
    ebo->Unbind();

    clearObjects();
    swapObjects();

    return 0;
}

void SMGC::windowResize() {
}

void SMGC::swapObjects() {
    std::swap(Objects_A, Objects_B);
    std::swap(Vertices_A, Vertices_B);
    std::swap(Indices_A, Indices_B);

    vao->Unbind();
    vbo->Upload(Vertices_A);
    ebo->Upload(Indices_A);
}
void SMGC::clearObjects() {
    Objects_B.clear();
    Vertices_B.clear();
    Indices_B.clear();

    //Quad(Vertices_B, Indices_B);
    AntiCube(Vertices_B, Indices_B, maxDist, 1.0f);
    SetObjectAttributes(Vertices_B, 0, -2, 0.0f);
    Plane(Vertices_B, Indices_B, camera.pos, maxDist, -1.0f, epsilon);
}
void SMGC::pushObject(std::unique_ptr<Object> obj, const VertexAttributes& attributes) {
    const glm::vec3 center = attributes.attached ? glm::vec3(0.0f) : camera.pos;
    const float bound = ObjectBoundingRadius(*obj);
    const float dist = glm::distance(glm::vec3(obj->data1.y, obj->data1.z, obj->data1.w), center);
    if (bound >= 0.0f && dist - bound > maxDist) return;

    const size_t vertexStart = Vertices_B.size();
    const size_t indexStart = Indices_B.size();
    if (GenerateObject(Vertices_B, Indices_B, *obj, attributes.attached ? 1.0f : 0.0f) && dist + bound > maxDist)
        CullFarTriangles(Vertices_B, Indices_B, vertexStart, indexStart, center, maxDist);
    SetObjectAttributes(Vertices_B, vertexStart, static_cast<GLint>(Objects_B.size()), attributes.polygonal ? 1.0f : 0.0f);

    Objects_B.push_back(ToGpuObject(*obj, attributes));
}
/*void SMGC::getCamera(glm::vec3 newPos, glm::vec3 newRot) {
    camera.pos = newPos;
    camera.rot = newRot;
}
void SMGC::setCamera(glm::vec3 newPos, glm::vec3 newRot) {
    camera.pos = newPos;
    camera.rot = newRot;
}*/

void SMGC::setOutline(bool enabled) {
    outlineEnabled = enabled;
}
void SMGC::setShadows(bool enabled) {
    shadowsEnabled = enabled;
}
int SMGC::setMaxDist(float newMaxDist) {
    if (!std::isfinite(newMaxDist) || newMaxDist <= 0.0f) {
        std::cerr << getPrefix() << "Error in SMGC::setMaxDist: invalid maxDist " << std::to_string(newMaxDist) << std::endl;
        return -EINVAL;
    }
    maxDist = newMaxDist;
    return 0;
}
void SMGC::setCamera(const Camera& newCamera) {
    camera = newCamera;
}
int SMGC::subscribeKey(int componentIdx, int key) {
    if (key < GLFW_KEY_SPACE || key > GLFW_KEY_LAST) {
        std::cerr << getPrefix() << "Error in SMGC::subscribeKey: invalid key " << std::to_string(key) << std::endl;
        return -EINVAL;
    }
    for (const auto& subscription : keySubscriptions) {
        if (subscription.componentIdx == componentIdx && subscription.key == key) return 0;
    }
    keySubscriptions.push_back({componentIdx, key});
    return 0;
}
int SMGC::unsubscribeKey(int componentIdx, int key) {
    std::erase_if(keySubscriptions, [&](const KeySubscription& subscription) {
        return subscription.componentIdx == componentIdx && subscription.key == key;
    });
    return 0;
}
void SMGC::sendKeyStates() {
    const std::vector<KeySubscription> subscriptions = keySubscriptions;
    for (const auto& subscription : subscriptions) {
        bool pressed = glfwGetKey(window, subscription.key) == GLFW_PRESS;
        sendTask(std::make_unique<task_SmgcKeyState>(subscription.key, pressed), subscription.componentIdx);
    }
}

int SMGC::draw() {
    //glClearColor(1.0f, 0.2f, 0.2f, 1.0f);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (!objectsBuffer) {std::cerr << getPrefix() << "Critical Error: objectsBuffer is nullptr" << std::endl; return -ENODEV;}
    const glm::mat3 cameraRot = CameraRotation(camera.rot);
    objectsUpload.assign(Objects_A.begin(), Objects_A.end());
    for (auto& object : objectsUpload) {
        if (object.invRot[1].w > 0.5f) object = AttachedToWorld(object, camera.pos, cameraRot);
    }
    objectsBuffer->Upload(objectsUpload);

    if (!shaderProgram) {std::cerr << getPrefix() << "Critical Error: shaderProgram is nullptr" << std::endl; return -ENODEV;}
    shaderProgram->Activate();
    shaderProgram->setFloat("iTime", float(localTickCounter) / float(Core_getTPS()));
    shaderProgram->setVec2("iResolution", float(windowWidth), float(windowHeight));
    shaderProgram->setVec3("iCameraPos", camera.pos);
    shaderProgram->setVec3("iCameraRot", camera.rot);
    shaderProgram->setFloat("iMaxDist", maxDist);
    shaderProgram->setFloat("iEpsilon", epsilon);
    shaderProgram->setFloat("iShadows", shadowsEnabled ? 1.0f : 0.0f);
    
    if (!vao) {std::cerr << getPrefix() << "Critical Error: vao is nullptr" << std::endl; return -ENODEV;}
    vao->Bind();

    // 1) заливка
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(1.0f, 1.0f);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    shaderProgram->setFloat("iOutline", 1.0f);
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    glDepthFunc(GL_LESS);
    glDepthMask(GL_TRUE);
    glDrawElements(GL_TRIANGLES, Indices_A.size(), GL_UNSIGNED_INT, 0);

    shaderProgram->setFloat("iOutline", 0.0f);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glDepthFunc(GL_EQUAL);
    glDepthMask(GL_FALSE);
    glDrawElements(GL_TRIANGLES, Indices_A.size(), GL_UNSIGNED_INT, 0);

    glDepthFunc(GL_LESS);
    glDepthMask(GL_TRUE);

    glDisable(GL_POLYGON_OFFSET_FILL);

    // 2) обводка
    if (outlineEnabled) {
        shaderProgram->setFloat("iOutline", 1.0f);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glLineWidth(1.0f);
        glDrawElements(GL_TRIANGLES, Indices_A.size(), GL_UNSIGNED_INT, 0);
    }

    vao->Unbind();

    return 0;
}

int SMGC::processTask(std::unique_ptr<BaseTask> task) {
    uint32_t taskIdx = task->getIdx();
    if (taskIs_SmgcSwapObjects(taskIdx)) {
        swapObjects();
    } else if (taskIs_SmgcClearObjects(taskIdx)) {
        clearObjects();
    } else if (taskIs_SmgcPushObject(taskIdx)) {
        auto* typed = static_cast<task_SmgcPushObject*>(task.get());
        pushObject(std::move(typed->obj), typed->attributes);
        typed->obj = nullptr;
    } else if (taskIs_SmgcSetOutline(taskIdx)) {
        auto* typed = static_cast<task_SmgcSetOutline*>(task.get());
        setOutline(typed->enabled);
    } else if (taskIs_SmgcSetShadows(taskIdx)) {
        auto* typed = static_cast<task_SmgcSetShadows*>(task.get());
        setShadows(typed->enabled);
    } else if (taskIs_SmgcSetMaxDist(taskIdx)) {
        auto* typed = static_cast<task_SmgcSetMaxDist*>(task.get());
        return setMaxDist(typed->maxDist);
    } else if (taskIs_SmgcSetCamera(taskIdx)) {
        auto* typed = static_cast<task_SmgcSetCamera*>(task.get());
        setCamera(typed->camera);
    } else if (taskIs_SmgcSubscribeKey(taskIdx)) {
        auto* typed = static_cast<task_SmgcSubscribeKey*>(task.get());
        return subscribeKey(task->getCreatorIdx(), typed->key);
    } else if (taskIs_SmgcUnsubscribeKey(taskIdx)) {
        auto* typed = static_cast<task_SmgcUnsubscribeKey*>(task.get());
        return unsubscribeKey(task->getCreatorIdx(), typed->key);
    }
    return 0;
}

int SMGC::tick(long long tickCounter) {
    localTickCounter = tickCounter;
    if (glfwWindowShouldClose(window)) {
        Core_stop();
    } else {
        //camera.pos = {0.0f, 0.0f, sin(float(localTickCounter) / float(Core_getTPS())) * 5.0f};
        sendKeyStates();

        if (Core_isDebugModeEnabled()) std::clog << getPrefix() << "camera.pos: " << std::to_string(camera.pos.x) << ";" << std::to_string(camera.pos.y) << ";" << std::to_string(camera.pos.z) << std::endl;
        if (Core_isDebugModeEnabled()) std::clog << getPrefix() << "camera.rot: " << std::to_string(camera.rot.x) << ";" << std::to_string(camera.rot.y) << ";" << std::to_string(camera.rot.z) << std::endl;
        
        int errorCode = draw();
        if (errorCode != 0) return errorCode;

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    return 0;
}

void SMGC::death() {
    keySubscriptions.clear();
    if (vao) vao->Delete();
    if (vbo) vbo->Delete();
    if (ebo) ebo->Delete();
    if (shaderProgram) shaderProgram->Delete();
    if (objectsBuffer) objectsBuffer->Delete();
    if (window) glfwDestroyWindow(window);
    glfwTerminate();
    Core_stop();
}