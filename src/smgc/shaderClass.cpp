#include <smgc/shaderClass.h>

static bool checkShader(GLuint shader, const char* name) {
    GLint ok = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[2048];
        glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
        std::cerr << "Shader compile error (" << name << "):\n" << log << "\n";
        return false;
    }
    return true;
}

static bool checkProgram(GLuint prog) {
    GLint ok = 0;
    glGetProgramiv(prog, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[2048];
        glGetProgramInfoLog(prog, sizeof(log), nullptr, log);
        std::cerr << "Program link error:\n" << log << "\n";
        return false;
    }
    return true;
}

Shader::Shader(const char* vertexFile, const char* fragmentFile) {
    ID = 0;
    status = 0;
    
    auto [vertexCode, errorCode1] = readFile_S(vertexFile);
    if (errorCode1 != 0) {
        std::cerr << "CriticalError in Shader::Shader: readFile_S(\"" << vertexFile << "\") return error code " << std::to_string(errorCode1) << std::endl;
        status = errorCode1;
        return;
    }
    const char* vertexSource = vertexCode.c_str();

    auto [fragmentCode, errorCode2] = readFile_S(fragmentFile);
    if (errorCode2 != 0) {
        std::cerr << "CriticalError in Shader::Shader: readFile_S(\"" << fragmentFile << "\") return error code " << std::to_string(errorCode2) << std::endl;
        status = errorCode2;
        return;
    }
    const char* fragmentSource = fragmentCode.c_str();

    std::clog << "Info in Shader::Shader: fragmentSource = \"\"\"" << fragmentSource << "\"\"\"" << std::endl;
    std::clog << "Info in Shader::Shader: vertexSource = \"\"\"" << vertexSource << "\"\"\"" << std::endl;

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSource, NULL);
    glCompileShader(vertexShader);
    if (!checkShader(vertexShader, vertexFile)) { status = -EINVAL; return; }

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
    glCompileShader(fragmentShader);
    if (!checkShader(fragmentShader, fragmentFile)) { status = -EINVAL; return; }

    ID = glCreateProgram();
    glAttachShader(ID, vertexShader);
    glAttachShader(ID, fragmentShader);
    glLinkProgram(ID);
    if (!checkProgram(ID)) { status = -EINVAL; return; }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

void Shader::Activate() {
    glUseProgram(ID);   
}

void Shader::Delete() {
    if (ID != 0) {
        glDeleteProgram(ID);
        ID = 0;
    }
    uniformLocations.clear();
}

int Shader::getStatus() {
    // Get status - error or normal
    return status;
}

GLint Shader::getUniformLocation(const char* name) {
    auto it = uniformLocations.find(name);
    if (it != uniformLocations.end()) {
        return it->second;
    }
    GLint loc = glGetUniformLocation(ID, name);
    uniformLocations.emplace(name, loc);
    return loc;
}

void Shader::setVec3(const char* name, float x, float y, float z) {
    GLint loc = getUniformLocation(name);
    if (loc == -1) return;
    glProgramUniform3f(ID, loc, x, y, z);
}

void Shader::setVec2(const char* name, float x, float y) {
    GLint loc = getUniformLocation(name);
    if (loc == -1) return;
    glProgramUniform2f(ID, loc, x, y);
}

void Shader::setFloat(const char* name, float x) {
    GLint loc = getUniformLocation(name);
    if (loc == -1) return;
    glProgramUniform1f(ID, loc, x);
}

void Shader::setVec3(const char* name, glm::vec3 v) {
    setVec3(name, v.x, v.y, v.z);
}

void Shader::setVec2(const char* name, glm::vec2 v) {
    setVec2(name, v.x, v.y);
}