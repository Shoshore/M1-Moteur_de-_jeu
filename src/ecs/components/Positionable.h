#ifndef POSITIONABLE
#define POSITIONABLE

#include <glm/ext.hpp>

struct Positionable {
    glm::vec3 pos;
    bool shouldTransform;

    Positionable(glm::vec3 pos, bool shouldTransform)
        : pos(pos), shouldTransform(shouldTransform) {}
    Positionable(glm::vec3 pos) : Positionable(pos, true) {}
    Positionable(bool shouldTransform) : Positionable({}, shouldTransform) {}
    Positionable() : Positionable({}, true) {}
};

#endif // POSITIONABLE