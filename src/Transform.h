#ifndef TRANSFORM
#define TRANSFORM

#include "glm/detail/type_vec.hpp"
#include <iostream>

#include <glm/ext.hpp>

class Transform {
    glm::mat4 m_matrix;

    inline static glm::mat4 buildMat4(glm::mat3 mat) {
        glm::mat4 result(1.);

        for (size_t i = 0; i < 3; i++) {
            for (size_t j = 0; j < 3; j++) {
                result[i][j] = mat[i][j];
            }
        }

        return result;
    }

  public:
    Transform() : m_matrix(1.) {}

    inline glm::mat4 getMatrix() const { return this->m_matrix; }

    inline Transform &scale(float x, float y, float z) {
        glm::mat4 s(1.);

        s[0][0] = x;
        s[1][1] = y;
        s[2][2] = z;

        this->m_matrix *= s;
        return *this;
    }

    inline Transform &scale(float value) {
        this->scale(value, value, value);
        return *this;
    }
    inline Transform &scale(const glm::vec3 &s) {
        this->scale(s[0], s[1], s[2]);
        return *this;
    }

    inline Transform &skew(const glm::mat3 &skewMatrix) {
        this->m_matrix *= buildMat4(skewMatrix);
        return *this;
    }

    inline Transform &translate(float x, float y, float z) {
        glm::mat4 t(1.);

        t[3][0] = x;
        t[3][1] = y;
        t[3][2] = z;

        this->m_matrix *= t;
        return *this;
    }

    inline Transform &translate(float value) {
        this->translate(value, value, value);
        return *this;
    }

    inline Transform &translate(const glm::vec3 &s) {
        this->translate(s[0], s[1], s[2]);
        return *this;
    }

    inline Transform &rotationX(float angle) {
        glm::mat3 r(1, 0., 0., 0., cos(glm::radians(angle)),
                    sin((glm::radians(angle))), 0., -sin((glm::radians(angle))),
                    cos(glm::radians(angle)));
        this->m_matrix *= buildMat4(r);
        return *this;
    }

    inline Transform &rotationY(float angle) {
        glm::mat3 r(cos(glm::radians(angle)), 0., -sin((glm::radians(angle))),
                    0., 1., 0., sin((glm::radians(angle))), 0.,
                    cos(glm::radians(angle)));
        this->m_matrix *= buildMat4(r);
        return *this;
    }

    inline Transform &rotationZ(float angle) {
        glm::mat3 r(cos(glm::radians(angle)), sin((glm::radians(angle))), 0.,
                    -sin((glm::radians(angle))), cos(glm::radians(angle)), 0.,
                    0., 0., 1.);
        this->m_matrix *= buildMat4(r);
        return *this;
    }

    inline Transform &transform(const Transform &transform) {
        this->m_matrix *= transform.m_matrix;
        return *this;
    }

    inline glm::vec4 computeVec4(const glm::vec4 &in) const {
        return this->m_matrix * in;
    }

    inline glm::vec3 computeVec3(const glm::vec3 &in) const {
        glm::vec4 tmp = this->m_matrix * glm::vec4(in, 1.);
        return glm::vec3(tmp[0], tmp[1], tmp[2]);
    }

    inline glm::mat4 computeMat4(const glm::mat4 &in) const {
        return this->m_matrix * in;
    }

    inline glm::vec3 getPosition() const {
        return {this->m_matrix[3][0], this->m_matrix[3][1],
                this->m_matrix[3][2]};
    }

    inline void print() const {
        for (size_t i = 0; i < 4; i++) {
            for (size_t j = 0; j < 4; j++) {
                std::cout << m_matrix[j][i];
                if (j < 3)
                    std::cout << "\t";
            }
            std::cout << "\n";
        }
    }
};

inline Transform scale(float x, float y, float z) {
    Transform t;
    return t.scale(x, y, z);
}

inline Transform scale(float value) {
    Transform t;
    return t.scale(value);
}

inline Transform scale(const glm::vec3 &s) {
    Transform t;
    return t.scale(s);
}

inline Transform skew(const glm::mat3 &skewMatrix) {
    Transform t;
    return t.skew(skewMatrix);
}

inline Transform translate(float x, float y, float z) {
    Transform t;
    return t.translate(x, y, z);
}

inline Transform translate(float value) {
    Transform t;
    return t.translate(value);
}

inline Transform translate(const glm::vec3 &s) {
    Transform t;
    return t.translate(s);
}

inline Transform rotationX(float angle) {
    Transform t;
    return t.rotationX(angle);
}

inline Transform rotationY(float angle) {
    Transform t;
    return t.rotationY(angle);
}

inline Transform rotationZ(float angle) {
    Transform t;
    return t.rotationZ(angle);
}

#endif // TRANSFORM