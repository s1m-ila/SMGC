#include <smgc/VAO.h>

VAO::VAO() {
    glGenVertexArrays(1, &ID);
}

void VAO::LinkVBO(VBO& VBO, GLuint layoutPos, GLuint layoutAttached, GLuint layoutObjectIdx, GLuint layoutPolygonal) {
    VBO.Bind();
    glVertexAttribPointer(layoutPos, 3, GL_FLOAT, GL_FALSE,
                          sizeof(Vertex), (void*)offsetof(Vertex, pos));
    glEnableVertexAttribArray(layoutPos);

    glVertexAttribPointer(layoutAttached, 1, GL_FLOAT, GL_FALSE,
                          sizeof(Vertex), (void*)offsetof(Vertex, attached));
    glEnableVertexAttribArray(layoutAttached);

    glVertexAttribIPointer(layoutObjectIdx, 1, GL_INT,
                           sizeof(Vertex), (void*)offsetof(Vertex, objectIdx));
    glEnableVertexAttribArray(layoutObjectIdx);

    glVertexAttribPointer(layoutPolygonal, 1, GL_FLOAT, GL_FALSE,
                          sizeof(Vertex), (void*)offsetof(Vertex, polygonal));
    glEnableVertexAttribArray(layoutPolygonal);
    VBO.Unbind();
}
void VAO::Bind() {
    glBindVertexArray(ID);  
}
void VAO::Unbind() {
    glBindVertexArray(0);
}
void VAO::Delete() {
    glDeleteVertexArrays(1, &ID);
}