#pragma once

#include <glad/glad.h>
#include "VBO.h"

class VAO {
public:
    GLuint ID;
    VAO();

    void LinkVBO(VBO& VBO, GLuint layout, GLuint layoutAttached, GLuint layoutObjectIdx, GLuint layoutPolygonal);
    void LinkAttrib(VBO& VBO, GLuint layout, GLint numComponents, GLenum type, GLsizei stride, const void* offset);
    void LinkAttribI(VBO& VBO, GLuint layout, GLint numComponents, GLenum type, GLsizei stride, const void* offset);

    void Bind();
    void Unbind();
    void Delete();
};