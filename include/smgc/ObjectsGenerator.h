#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <algorithm>
#include <array>
#include <cmath>
#include <vector>

#include "VBO.h"

#include "ObjectsBuffer.h"

void Quad(std::vector<Vertex>& vertices, std::vector<GLuint>& indices, float attached = 0.0f);
void AntiCube(std::vector<Vertex>& vertices, std::vector<GLuint>& indices, float radius, float attached = 0.0f);
void Plane(std::vector<Vertex>& vertices, std::vector<GLuint>& indices, const glm::vec3& center, float halfSize, float height, float halfThickness, float attached = 0.0f);

void Cube(std::vector<Vertex>& vertices, std::vector<GLuint>& indices, const Object& obj, float attached = 0.0f);
void Sphere(std::vector<Vertex>& vertices, std::vector<GLuint>& indices, const Object& obj, float attached = 0.0f);
void Capsule(std::vector<Vertex>& vertices, std::vector<GLuint>& indices, const Object& obj, float attached = 0.0f);

bool GenerateObject(std::vector<Vertex>& vertices, std::vector<GLuint>& indices, const Object& obj, float attached = 0.0f);
float ObjectBoundingRadius(const Object& obj);
GpuObject ToGpuObject(const Object& obj, const VertexAttributes& attributes);
glm::mat3 CameraRotation(const glm::vec3& deg);
GpuObject AttachedToWorld(const GpuObject& obj, const glm::vec3& cameraPos, const glm::mat3& cameraRot);
void SetObjectAttributes(std::vector<Vertex>& vertices, size_t vertexStart, GLint objectIdx, float polygonal);
void CullFarTriangles(std::vector<Vertex>& vertices, std::vector<GLuint>& indices, size_t vertexStart, size_t indexStart, const glm::vec3& center, float maxDist);
