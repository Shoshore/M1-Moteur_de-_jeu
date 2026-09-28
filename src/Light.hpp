#ifndef LIGHT
#define LIGHT

#include <glm/ext.hpp>

class Light {
  private:
    glm::vec3 lightPos;
    glm::vec3 lightColor;

  public:
    Light() : lightPos(0.0f, 0.0f, 0.0f), lightColor(1.0f, 1.0f, 1.0f) {}

    Light(const glm::vec3 &pos, const glm::vec3 &color)
        : lightPos(pos), lightColor(color) {}

    glm::vec3 getLightPos() const { return lightPos; }

    void setLightPos(const glm::vec3 &pos) { lightPos = pos; }

    glm::vec3 getLightColor() const { return lightColor; }

    void setLightColor(const glm::vec3 &color) { lightColor = color; }
};

#endif