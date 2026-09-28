#ifndef VERLET_BODY
#define VERLET_BODY

#include <glm/ext.hpp>

struct VerletBody {
    glm::vec3 last_position;
    glm::vec3 acceleration;
    float size;
    bool unmovable;
    bool isTrigger = false;

    VerletBody() {}

    VerletBody(glm::vec3 last_position, glm::vec3 acceleration, float size,
               bool unmovable)
        : last_position(last_position), acceleration(acceleration), size(size),
          unmovable(unmovable) {}

    VerletBody(glm::vec3 last_position, glm::vec3 acceleration, float size)
        : VerletBody(last_position, acceleration, size, false) {}
};

#endif // VERLET_BODY