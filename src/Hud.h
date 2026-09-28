#pragma once
#include <GL/glew.h>
#include <vector>

#include "src/Texture.h"

class Hud {
  public:
    void init(const char *vsPath = "shaders/hud_vs.glsl",
              const char *fsPath = "shaders/hud_fs.glsl",
              const char *spritePath = "assets/textures/star.png");

    void beginFrame(int screenWidth, int screenHeight);

    void draw(int collected, int total, int originX, int originY,
              int iconSizePx, int spacingPx, int maxWidthPx);

    void endFrame();
    void cleanup();

  private:
    struct Vtx {
        float x, y;
        float u, v;
        float r, g, b, a;
    };

    GLuint programId = 0;
    GLuint vao = 0;
    GLuint vbo = 0;
    GLint samplerLoc = -1;

    int w = 1, h = 1;

    Texture sprite;
    GLuint spriteId = 0;

    std::vector<Vtx> vertices;

    void pushQuadPx(float x, float y, float size, float r, float g, float b,
                    float a);
};