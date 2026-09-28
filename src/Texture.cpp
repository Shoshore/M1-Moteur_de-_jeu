#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include <lib/stb_image.h>

#include "Texture.h"

Texture::TextureData::TextureData() : texId(0) {}

Texture::TextureData::TextureData(const char *path) : texId(0) {
    int width, height, nrChannels;
    unsigned char *data = stbi_load(path, &width, &height, &nrChannels, 0);
    if (data != nullptr) {
        glGenTextures(1, &this->texId);
        glBindTexture(GL_TEXTURE_2D, this->texId);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                        GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        GLenum format;
        if (nrChannels == 1)
            format = GL_RED;
        else if (nrChannels == 3)
            format = GL_RGB;
        else
            format = GL_RGBA;

        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format,
                     GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(data);
    } else {
        std::cout << "Failed to load texture" << std::endl;
    }
}

Texture::TextureData::~TextureData() {
    if (this->texId != 0)
        glDeleteTextures(1, &this->texId);
}

Texture::Texture() : data(std::make_shared<TextureData>()) {}

Texture::Texture(const char *path)
    : data(std::make_shared<TextureData>(path)) {}

uint Texture::getId() const { return this->data->texId; }

void Texture::bind(GLuint programId, const char *name, int pos) const {
    glUseProgram(programId);
    glActiveTexture(GL_TEXTURE0 + pos);
    glBindTexture(GL_TEXTURE_2D, this->data->texId);
    glUniform1i(glGetUniformLocation(programId, name), pos);
}

void Texture::cleanUp() { this->data.reset(); }