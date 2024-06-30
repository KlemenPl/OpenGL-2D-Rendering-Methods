#ifndef DIPLOMA_COMMON_H
#define DIPLOMA_COMMON_H

#include <glm.hpp>
#include <ext.hpp>

#include <cstdio>

#define GLFW_INCLUDE_NONE

#include <glad/glad.h>

typedef struct ShaderDesc {
    const char *vertex;
    const char *fragment;
    const char *geometry;
} ShaderDesc;

typedef struct UVRegion {
    GLuint texture;
    uint16_t width;
    uint16_t height;
    uint16_t u0;
    uint16_t v0;
    uint16_t u1;
    uint16_t v1;

    float U0() const {
        return float(u0) / float(width);
    }
    float V0() const {
        return float(v0) / float(height);
    }
    float U1() const {
        return float(u1) / float(width);
    };
    float V1() const {
        return float(v1) / float(height);
    }
} UVRegion;

inline bool operator==(const UVRegion &lhs, const UVRegion &rhs) {
    return lhs.texture == rhs.texture &&
        lhs.u0 == rhs.u0 &&
        lhs.v0 == rhs.v0 &&
        lhs.u1 == rhs.u1 &&
        lhs.v1 == rhs.v1;
}
inline bool operator!=(const UVRegion &lhs, const UVRegion &rhs) {
    return !(lhs == rhs);
}

typedef struct Texture {
    GLuint id;
    int width;
    int height;
} Texture;

using Color = glm::u8vec4;

namespace Colors {
    inline constexpr Color WHITE = {255, 255, 255, 255};
}

class Camera2D {
public:
    glm::vec3 position = {0.0f, 0.0f, 0.0f};
    float rotation = 0.0f;
    float zoom = 1.0f;

    Camera2D() = default;

    [[nodiscard]]
    glm::mat4 getView() const {
        // translate * rotate
        return glm::translate(glm::mat4(1.0f), position) *
               glm::rotate(glm::mat4(1.0f), rotation, glm::vec3(0.0f, 0.0f, 1.0f));
    }

    [[nodiscard]]
    glm::mat4 getProjection(glm::vec2 viewSize) const {
        return glm::ortho(0.0f, viewSize.x / zoom, viewSize.y / zoom, 0.0f, -1.0f, 1.0f);
    }

    [[nodiscard]]
    glm::mat4 getCombined(glm::vec2 viewSize) const {
        return getProjection(viewSize) * getView();
    }

};

void enableOpenGLDebugLogging();

GLuint compileShaderStage(const char *source, GLenum type);

GLuint compileShaderProgram(const ShaderDesc &desc);

Texture loadTexture(const char *path);
Texture loadDummyTexture();
UVRegion getUVRegion(const Texture &texture, int x, int y, int width, int height);

static inline glm::mat3 buildTransformationMatrix(const glm::vec2 pos, const glm::vec2 size,
                                        const glm::vec2 origin, const float rotation) {

    const float c = glm::cos(rotation);
    const float s = glm::sin(rotation);

    // Note: glm stores matrices in column-major order
    auto mat = glm::mat3(1.0, 0.0, 0.0,
                         0.0, 1.0, 0.0,
                         pos.x + origin.x, pos.y + origin.y, 1.0);
    mat = mat * glm::mat3(c, s, 0.0,
                          -s, c, 0.0,
                          0.0, 0.0, 1.0);
    mat = mat * glm::mat3(1.0, 0.0, 0.0,
                          0.0, 1.0, 0.0,
                          -origin.x, -origin.y, 1.0);
    mat = mat * glm::mat3(size.x, 0.0, 0.0,
                          0.0, size.y, 0.0,
                          0.0, 0.0, 1.0);
    return mat;
}

#endif //DIPLOMA_COMMON_H
