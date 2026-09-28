#include "Hud.h"

#include <algorithm>

#include "lib/shader.hpp"

static float ndcX(float xPx, int w) { return (2.0f * (xPx / (float)w)) - 1.0f; }
static float ndcY(float yPx, int h) { return 1.0f - (2.0f * (yPx / (float)h)); }

void Hud::init(const char *vsPath, const char *fsPath, const char *spritePath) {
    programId = LoadShaders(vsPath, fsPath);

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vtx),
                          (void *)offsetof(Vtx, x));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vtx),
                          (void *)offsetof(Vtx, u));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vtx),
                          (void *)offsetof(Vtx, r));

    glBindVertexArray(0);

    sprite = Texture(spritePath);
    spriteId = sprite.getId();

    glBindTexture(GL_TEXTURE_2D, spriteId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glUseProgram(programId);
    samplerLoc = glGetUniformLocation(programId, "spriteTex");
    glUseProgram(0);
}

void Hud::beginFrame(int screenWidth, int screenHeight) {
    w = std::max(1, screenWidth);
    h = std::max(1, screenHeight);
    vertices.clear();
}

void Hud::pushQuadPx(float x, float y, float size, float r, float g, float b,
                     float a) {
    float u0 = 0.f, v0 = 0.f, u1 = 1.f, v1 = 1.f;

    float x0 = ndcX(x, w);
    float y0 = ndcY(y, h);
    float x1 = ndcX(x + size, w);
    float y1 = ndcY(y + size, h);

    Vtx a0{x0, y0, u0, v0, r, g, b, a};
    Vtx a1{x0, y1, u0, v1, r, g, b, a};
    Vtx a2{x1, y0, u1, v0, r, g, b, a};
    Vtx a3{x1, y1, u1, v1, r, g, b, a};

    vertices.push_back(a0);
    vertices.push_back(a1);
    vertices.push_back(a2);
    vertices.push_back(a2);
    vertices.push_back(a1);
    vertices.push_back(a3);
}

void Hud::draw(int collected, int total, int originX, int originY,
               int iconSizePx, int spacingPx, int maxWidthPx) {
    collected = std::max(0, collected);
    total = std::max(0, total);
    if (total == 0)
        return;

    int step = iconSizePx + spacingPx;
    int iconsPerRow = std::max(1, (maxWidthPx + spacingPx) / step);

    for (int i = 0; i < total; ++i) {
        int row = i / iconsPerRow;
        int col = i % iconsPerRow;

        int x = originX + col * step;
        int y = originY + row * step;

        if (i < collected) {
            pushQuadPx((float)x, (float)y, (float)iconSizePx, 1.f, 1.f, 1.f,
                       1.f);
        } else {
            pushQuadPx((float)x, (float)y, (float)iconSizePx, 0.35f, 0.35f,
                       0.35f, 0.9f);
        }
    }
}

void Hud::endFrame() {
    if (programId == 0 || vao == 0 || vbo == 0)
        return;
    if (spriteId == 0)
        return;
    if (vertices.empty())
        return;

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glUseProgram(programId);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, spriteId);
    if (samplerLoc >= 0)
        glUniform1i(samplerLoc, 0);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)(vertices.size() * sizeof(Vtx)),
                 vertices.data(), GL_DYNAMIC_DRAW);

    glDrawArrays(GL_TRIANGLES, 0, (GLsizei)vertices.size());

    glBindVertexArray(0);
    glUseProgram(0);

    glDisable(GL_BLEND);
}

void Hud::cleanup() {
    if (vbo)
        glDeleteBuffers(1, &vbo);
    if (vao)
        glDeleteVertexArrays(1, &vao);
    if (programId)
        glDeleteProgram(programId);
    vbo = 0;
    vao = 0;
    programId = 0;

    sprite.cleanUp();
    spriteId = 0;
}