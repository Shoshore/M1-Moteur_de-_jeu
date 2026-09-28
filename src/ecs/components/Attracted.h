#ifndef ATTRACTED
#define ATTRACTED

#include <optional>
#include <vector>

#include <glm/ext.hpp>

#include "glm/detail/type_vec.hpp"
#include "src/ecs/utils.h"

enum AttractionMode {
    INWARD,
    OUTWARD,
    VECTOR,
};

struct Attraction {
    AttractionMode mode;
    float force;

    std::optional<EntityId> body;
    std::optional<glm::vec3> direction;

    Attraction(EntityId body, AttractionMode mode, float force)
        : mode(mode), force(force), body(body) {}

    Attraction(glm::vec3 direction, float force)
        : mode(VECTOR), force(force), direction(direction) {}
};

class Attracted {

    std::vector<Attraction> attractions;

  public:
    std::vector<Attraction> const &getAttractions() const {
        return this->attractions;
    }

    void addBodyAttraction(EntityId body, AttractionMode mode, float force) {
        this->attractions.push_back({body, mode, force});
    }

    void addDirectionAttraction(glm::vec3 direction, float force) {
        this->attractions.push_back({direction, force});
    }

    void clear() { this->attractions.clear(); }
};

#endif // ATTRACTED