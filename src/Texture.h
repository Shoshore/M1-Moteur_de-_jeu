#ifndef TEXTURE
#define TEXTURE

#include <memory>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

using uint = unsigned int;

class Texture {
    struct TextureData {
        GLuint texId;

        TextureData();
        TextureData(const char *path);
        ~TextureData();

        TextureData(const TextureData &) = delete;
        TextureData &operator=(const TextureData &) = delete;
        TextureData(TextureData &&) = default;
        TextureData &operator=(TextureData &&) = default;
    };

    std::shared_ptr<TextureData> data;

  public:
    Texture();
    Texture(const char *path);

    uint getId() const;
    void bind(GLuint programId, const char *name, int pos) const;
    void cleanUp();
};

#endif // TEXTURE