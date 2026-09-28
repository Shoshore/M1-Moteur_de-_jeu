#include "SceneObject.h"
#include "lib/shader.hpp"

SceneObject::SceneObject(const char *vertexShaderPath,
                         const char *fragmentShaderPath, const Mesh &mesh)
    : mesh(mesh) {
    this->programId = LoadShaders(vertexShaderPath, fragmentShaderPath);
}

GLuint SceneObject::getId() const { return this->programId; }

Mesh &SceneObject::getMesh() { return this->mesh; }

const glm::vec3 &SceneObject::getAlbedo() const { return albedo; }
float SceneObject::getMetallic() const { return metallic; }
float SceneObject::getRoughness() const { return roughness; }
float SceneObject::getAo() const { return ao; }

void SceneObject::setAlbedo(const glm::vec3 &value) { albedo = value; }
void SceneObject::setMetallic(float value) { metallic = value; }
void SceneObject::setRoughness(float value) { roughness = value; }
void SceneObject::setAo(float value) { ao = value; }

bool SceneObject::addTexture(const Texture &texture, const char *varName) {
    if (!this->mesh.hasUVs()) {
        return false;
    }

    this->useTexture = true;
    int pos = this->textures.size();
    this->textures.emplace_back(varName, texture);
    texture.bind(this->programId, varName, pos);
    return true;
}

bool SceneObject::addAlbedoMap(const Texture &texture) {
    bool added = this->addTexture(texture, "albedoMap");
    this->useAlbedoMap = this->useTexture;
    return added;
}

bool SceneObject::addNormalMap(const Texture &texture) {
    bool added = this->addTexture(texture, "normalMap");
    this->useNormalMap = this->useTexture;
    return added;
}

bool SceneObject::addMetallicMap(const Texture &texture) {
    bool added = this->addTexture(texture, "metallicMap");
    this->useMetallicMap = this->useTexture;
    return added;
}

bool SceneObject::addRoughnessMap(const Texture &texture) {
    bool added = this->addTexture(texture, "roughnessMap");
    this->useRoughnessMap = this->useTexture;
    return added;
}

bool SceneObject::addAoMap(const Texture &texture) {
    bool added = this->addTexture(texture, "aoMap");
    this->useAoMap = this->useTexture;
    return added;
}

void SceneObject::draw(const Camera &camera,
                       const std::vector<Light> &lights) const {
    glUseProgram(this->programId);

    if (this->useTexture) {
        for (int pos = 0; pos < (int)this->textures.size(); ++pos) {
            const auto &[varName, texture] = this->textures[pos];
            texture.bind(this->programId, varName.c_str(), pos);
        }
    }

    glm::mat4 model = this->transform.getMatrix();
    glUniformMatrix4fv(glGetUniformLocation(this->programId, "model"), 1,
                       GL_FALSE, glm::value_ptr(model));

    glm::mat4 view = camera.getView();
    glUniformMatrix4fv(glGetUniformLocation(this->programId, "view"), 1,
                       GL_FALSE, glm::value_ptr(view));

    glm::mat4 projection = camera.getProjection();
    glUniformMatrix4fv(glGetUniformLocation(this->programId, "projection"), 1,
                       GL_FALSE, glm::value_ptr(projection));

    glm::vec3 cameraPos = camera.getPosition();
    glUniform3fv(glGetUniformLocation(this->programId, "camPos"), 1,
                 glm::value_ptr(cameraPos));

    if (!this->useAlbedoMap) {
        glUniform3fv(glGetUniformLocation(this->programId, "albedo"), 1,
                     glm::value_ptr(this->albedo));
    } else {
        glUniform1i(glGetUniformLocation(this->programId, "useAlbedoMap"), 1);
    }

    if (this->useNormalMap) {
        glUniform1i(glGetUniformLocation(this->programId, "useNormalMap"), 1);
    }

    if (!this->useMetallicMap) {
        glUniform1f(glGetUniformLocation(this->programId, "metallic"),
                    this->metallic);
    } else {
        glUniform1i(glGetUniformLocation(this->programId, "useMetallicMap"), 1);
    }

    if (!this->useRoughnessMap) {
        glUniform1f(glGetUniformLocation(this->programId, "roughness"),
                    this->roughness);
    } else {
        glUniform1i(glGetUniformLocation(this->programId, "useRoughnessMap"),
                    1);
    }

    if (!this->useAoMap) {
        glUniform1f(glGetUniformLocation(this->programId, "ao"), this->ao);
    } else {
        glUniform1i(glGetUniformLocation(this->programId, "useAoMap"), 1);
    }

    const int MAX_LIGHTS = 10;
    int count = std::min((int)lights.size(), MAX_LIGHTS);

    glUniform1i(glGetUniformLocation(programId, "lightCount"), count);

    for (int i = 0; i < count; ++i) {
        std::string base = "lights[" + std::to_string(i) + "]";
        glUniform3fv(
            glGetUniformLocation(programId, (base + ".position").c_str()), 1,
            glm::value_ptr(lights[i].getLightPos()));

        glUniform3fv(glGetUniformLocation(programId, (base + ".color").c_str()),
                     1, glm::value_ptr(lights[i].getLightColor()));
    }

    this->mesh.draw();
}

void SceneObject::cleanUp() {
    this->mesh.cleanUp();

    for (auto &[varName, texture] : this->textures) {
        texture.cleanUp();
    }

    glDeleteProgram(this->programId);
    this->programId = 0;
}

Transform SceneObject::getTransform() const { return this->transform; }

void SceneObject::setTransform(const Transform &transform) {
    this->transform = transform;
}

void SceneObject::updateMeshData(const std::vector<glm::vec3> &vertices,
                                 const std::vector<uint> &indices,
                                 const std::vector<glm::vec3> &normals,
                                 const std::vector<glm::vec2> &uvs) {
    this->mesh.updateMeshData(vertices, indices, normals, uvs);
}