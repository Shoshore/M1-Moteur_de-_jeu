#include "Mesh.h"
#include "glm/detail/type_vec.hpp"
#include <cstddef>
#include <iostream>
#include <map>
#include <ostream>
#include <sys/types.h>
#include <tuple>
#include <unordered_map>
#include <vector>

Mesh::MeshData::MeshData(const std::vector<glm::vec3> &vertices,
                         const std::vector<uint> &indices,
                         const std::vector<glm::vec3> &normals,
                         const std::vector<glm::vec2> &uvs)
    : vbo(0), nbo(0), ubo(0), ebo(0), vertices(vertices), normals(normals),
      uvs(uvs), indices(indices) {

    glGenBuffers(1, &this->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
    glBufferData(GL_ARRAY_BUFFER, this->vertices.size() * sizeof(glm::vec3),
                 this->vertices.data(), GL_STATIC_DRAW);

    if (this->hasNormals()) {
        glGenBuffers(1, &this->nbo);
        glBindBuffer(GL_ARRAY_BUFFER, this->nbo);
        glBufferData(GL_ARRAY_BUFFER, this->normals.size() * sizeof(glm::vec3),
                     this->normals.data(), GL_STATIC_DRAW);
    }

    if (this->hasUVs()) {
        glGenBuffers(1, &this->ubo);
        glBindBuffer(GL_ARRAY_BUFFER, this->ubo);
        glBufferData(GL_ARRAY_BUFFER, this->uvs.size() * sizeof(glm::vec2),
                     this->uvs.data(), GL_STATIC_DRAW);
    }

    glGenBuffers(1, &this->ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->indices.size() * sizeof(uint),
                 this->indices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

Mesh::MeshData::~MeshData() {
    glDeleteBuffers(1, &this->vbo);

    if (this->hasNormals()) {
        glDeleteBuffers(1, &this->nbo);
    }

    if (this->hasUVs()) {
        glDeleteBuffers(1, &this->ubo);
    }

    glDeleteBuffers(1, &this->ebo);
}

bool Mesh::MeshData::hasNormals() const { return !this->normals.empty(); }
bool Mesh::MeshData::hasUVs() const { return !this->uvs.empty(); }

GLsizei Mesh::MeshData::indexCount() const {
    return static_cast<GLsizei>(this->indices.size());
}

void Mesh::MeshData::bindToVAO() const {
    glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    if (this->hasNormals()) {
        glBindBuffer(GL_ARRAY_BUFFER, this->nbo);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    }

    if (this->hasUVs()) {
        glBindBuffer(GL_ARRAY_BUFFER, this->ubo);
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->ebo);
}

Mesh::Mesh(std::shared_ptr<MeshData> data) : vao(0), data(std::move(data)) {
    glGenVertexArrays(1, &this->vao);
    glBindVertexArray(this->vao);
    this->data->bindToVAO();
    glBindVertexArray(0);
}

Mesh::Mesh(const std::vector<glm::vec3> &vertices,
           const std::vector<uint> &indices)
    : Mesh(std::make_shared<MeshData>(vertices, indices,
                                      std::vector<glm::vec3>{},
                                      std::vector<glm::vec2>{})) {}

Mesh::Mesh(const std::vector<glm::vec3> &vertices,
           const std::vector<uint> &indices,
           const std::vector<glm::vec3> &normals)
    : Mesh(std::make_shared<MeshData>(vertices, indices, normals,
                                      std::vector<glm::vec2>{})) {}

Mesh::Mesh(const std::vector<glm::vec3> &vertices,
           const std::vector<uint> &indices, const std::vector<glm::vec2> &uvs)
    : Mesh(std::make_shared<MeshData>(vertices, indices,
                                      std::vector<glm::vec3>{}, uvs)) {}

Mesh::Mesh(const std::vector<glm::vec3> &vertices,
           const std::vector<uint> &indices,
           const std::vector<glm::vec3> &normals,
           const std::vector<glm::vec2> &uvs)
    : Mesh(std::make_shared<MeshData>(vertices, indices, normals, uvs)) {}

bool Mesh::hasNormals() const { return this->data->hasNormals(); }
bool Mesh::hasUVs() const { return this->data->hasUVs(); }

void Mesh::draw() const {
    glBindVertexArray(this->vao);
    glDrawElements(GL_TRIANGLES, this->data->indexCount(), GL_UNSIGNED_INT,
                   nullptr);
    glBindVertexArray(0);
}

void Mesh::updateMeshData(const std::vector<glm::vec3> &vertices,
                          const std::vector<uint> &indices,
                          const std::vector<glm::vec3> &normals,
                          const std::vector<glm::vec2> &uvs) {
    this->data = std::make_shared<MeshData>(vertices, indices, normals, uvs);

    glDeleteVertexArrays(1, &this->vao);
    glGenVertexArrays(1, &this->vao);
    glBindVertexArray(this->vao);
    this->data->bindToVAO();
    glBindVertexArray(0);
}

Mesh::Mesh(const Mesh &other) : vao(0), data(other.data) {
    glGenVertexArrays(1, &this->vao);
    glBindVertexArray(this->vao);
    this->data->bindToVAO();
    glBindVertexArray(0);
}

Mesh &Mesh::operator=(const Mesh &other) {
    if (this == &other) {
        return *this;
    }

    cleanUp();
    this->data = other.data;
    glGenVertexArrays(1, &this->vao);
    glBindVertexArray(this->vao);
    this->data->bindToVAO();
    glBindVertexArray(0);

    return *this;
}

void Mesh::cleanUp() {
    glDeleteVertexArrays(1, &this->vao);
    this->vao = 0;
}

std::vector<glm::vec3> Mesh::getVertices() const {
    return this->data->vertices;
}

std::vector<uint> Mesh::getIndices() const { return this->data->indices; }

std::vector<glm::vec3> Mesh::getNormals() const { return this->data->normals; }

std::vector<glm::vec2> Mesh::getUvs() const { return this->data->uvs; }

// void Mesh::applyLOD() {
//     /*
//     Faire une map indice -> nb voisins

//     Regarder l'erreur quadratique

//     Autre piste, zone ou il n'y a pas beaucoup de détails (ex: zone plate),
//     comparaison avec les normales
//     */

//     auto cp_vertices = this->getVertices();
//     auto cp_indices = this->getIndices();
//     auto cp_normals = this->getNormals();
//     auto cp_uvs = this->getUvs();

//     std::cout << "Taille des uvs = " << cp_uvs.size() << std::endl;
//     std::cout << "Taille des normales = " << cp_normals.size() << std::endl;

//     auto indice1 = cp_indices[0];
//     auto indice2 = cp_indices[1];
//     auto lastindice = cp_vertices.size() - 1;

//     glm::vec3 u = cp_vertices[indice1];
//     glm::vec3 v = cp_vertices[indice2];
//     glm::vec3 uv = (u + v) / 2;
//     glm::vec3 last = cp_vertices[lastindice];

//     cp_vertices[indice1] = uv;
//     cp_vertices[indice2] = last;
//     cp_vertices.resize(lastindice);

//     for (size_t i = 0; i < cp_indices.size(); i++) {
//         if (cp_indices[i] == indice2) {
//             cp_indices[i] = indice1;
//         } else if (cp_indices[i] == lastindice) {
//             cp_indices[i] = indice2;
//         }
//     }

//     std::vector<uint> new_indices;
//     for (size_t j = 0; j < cp_indices.size(); j += 3) {
//         if (cp_indices[j] != cp_indices[j + 1] &&
//             cp_indices[j] != cp_indices[j + 2] &&
//             cp_indices[j + 1] != cp_indices[j + 2]) {

//             new_indices.push_back(cp_indices[j]);
//             new_indices.push_back(cp_indices[j + 1]);
//             new_indices.push_back(cp_indices[j + 2]);
//         }
//     }

//     if (this->hasNormals()) {
//         cp_normals[indice1] = (cp_normals[indice1] + cp_normals[indice2]) /
//         2; cp_normals[indice2] = cp_normals[lastindice];
//         cp_normals.resize(lastindice);
//     }

//     if (this->hasUVs()) {
//         cp_uvs[indice1] = (cp_uvs[indice1] + cp_uvs[indice2]) / 2;
//         cp_uvs[indice2] = cp_uvs[lastindice];
//         cp_uvs.resize(lastindice);
//     }

//     std::cout << "Taille des indices = " << new_indices.size() << std::endl;

//     this->updateMeshData(cp_vertices, new_indices, cp_normals, cp_uvs);
// }