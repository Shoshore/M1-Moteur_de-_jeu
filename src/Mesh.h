#ifndef MESH
#define MESH

#include <memory>
#include <vector>

#include <GL/glew.h>
#include <glm/ext.hpp>

class Mesh {
  public:
    struct MeshData {
        GLuint vbo;
        GLuint nbo;
        GLuint ubo;
        GLuint ebo;

        std::vector<glm::vec3> vertices;
        std::vector<glm::vec3> normals;
        std::vector<glm::vec2> uvs;
        std::vector<uint> indices;

        MeshData(const std::vector<glm::vec3> &vertices,
                 const std::vector<uint> &indices,
                 const std::vector<glm::vec3> &normals,
                 const std::vector<glm::vec2> &uvs);

        ~MeshData();

        MeshData(const MeshData &) = delete;
        MeshData &operator=(const MeshData &) = delete;
        MeshData(MeshData &&) = default;
        MeshData &operator=(MeshData &&) = default;

        bool hasNormals() const;
        bool hasUVs() const;

        void bindToVAO() const;

        GLsizei indexCount() const;
    };

    Mesh(std::shared_ptr<MeshData> data);

  private:
    GLuint vao;
    std::shared_ptr<MeshData> data;

  public:
    Mesh(const std::vector<glm::vec3> &vertices,
         const std::vector<uint> &indices);
    Mesh(const std::vector<glm::vec3> &vertices,
         const std::vector<uint> &indices,
         const std::vector<glm::vec3> &normals);
    Mesh(const std::vector<glm::vec3> &vertices,
         const std::vector<uint> &indices, const std::vector<glm::vec2> &uvs);
    Mesh(const std::vector<glm::vec3> &vertices,
         const std::vector<uint> &indices,
         const std::vector<glm::vec3> &normals,
         const std::vector<glm::vec2> &uvs);

    Mesh(const Mesh &other);
    Mesh &operator=(const Mesh &other);
    Mesh(Mesh &&) = default;
    Mesh &operator=(Mesh &&) = default;

    bool hasNormals() const;
    bool hasUVs() const;

    void draw() const;
    void cleanUp();

    std::vector<glm::vec3> getVertices() const;
    std::vector<uint> getIndices() const;
    std::vector<glm::vec3> getNormals() const;
    std::vector<glm::vec2> getUvs() const;
    void updateMeshData(const std::vector<glm::vec3> &vertices,
                        const std::vector<uint> &indices,
                        const std::vector<glm::vec3> &normals,
                        const std::vector<glm::vec2> &uvs);

    void applyLOD(float resolution);
};

#endif // MESH