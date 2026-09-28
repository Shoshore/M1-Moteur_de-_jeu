#ifndef COLLECTIBLE_COMPONENT
#define COLLECTIBLE_COMPONENT

struct Collectible {
    int value = 1;
    bool interacted = false;

    Collectible() = default;
    Collectible(int value) : value(value) {}
};

#endif