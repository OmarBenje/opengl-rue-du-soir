// Adapted from IOGL-homework/Mesh.h. Same interleaved vertex / VAO pipeline.
#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoords;
};

class Mesh {
public:
    Mesh() = default;
    ~Mesh();
    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;
    bool loadOBJ(const std::string& filename);
    void setVertices(const std::vector<Vertex>& vertices);
    void draw() const;
    static std::vector<Vertex> cube();
    static std::vector<Vertex> sphere(int slices = 12, int stacks = 8);
private:
    GLuint vao = 0, vbo = 0;
    GLsizei count = 0;
};
