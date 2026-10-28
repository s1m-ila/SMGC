#include <smgc/VBO.h>

VBO::VBO() {
    glGenBuffers(1, &ID);
}

void VBO::Upload(const std::vector<Vertex>& vertices) {
    Upload(vertices.data(), sizeof(Vertex) * vertices.size());
}

void VBO::Upload(const void* data, size_t bytes) {
    glBindBuffer(GL_ARRAY_BUFFER, ID);
    if (bytes == capacityBytes && bytes > 0)
        glBufferSubData(GL_ARRAY_BUFFER, 0, bytes, data);
    else {
        glBufferData(GL_ARRAY_BUFFER, bytes,
                     bytes == 0 ? nullptr : data,
                     GL_DYNAMIC_DRAW);
        capacityBytes = bytes;
    }
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void VBO::Bind()   { glBindBuffer(GL_ARRAY_BUFFER, ID); }
void VBO::Unbind() { glBindBuffer(GL_ARRAY_BUFFER, 0); }
void VBO::Delete() { glDeleteBuffers(1, &ID); }