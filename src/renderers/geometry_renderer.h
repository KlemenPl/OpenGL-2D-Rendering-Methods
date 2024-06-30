#ifndef DIPLOMA_GEOMETRY_RENDERER_H
#define DIPLOMA_GEOMETRY_RENDERER_H

#include "../base_renderer.h"

class GeometryRenderer : public IRenderer {
public:
    struct Vertex {
        glm::vec2 position; // 8
        glm::vec2 size;     // 8
        glm::vec2 origin;   // 8
        float rotation;     // 4
        glm::u8vec4 color;  // 4
        glm::u16vec2 uv[4]; // 16
    }; // 48 total

    static_assert(sizeof(Vertex) == 48);

    GeometryRenderer();
    ~GeometryRenderer() override;

    void begin(const glm::mat4 &projView) override;
    void drawSprite(const UVRegion &region, glm::vec2 position, glm::vec2 size, glm::vec2 origin, float rotation, Color color) override;
    void end() override;

private:
    constexpr static int aPosLoc = 0;
    constexpr static int aSizeLoc = 1;
    constexpr static int aOriginLoc = 2;
    constexpr static int aRotationLoc = 3;
    constexpr static int aColorLoc = 4;
    constexpr static int aUVLoc = 5; // array of 4
    // Total of 9 attributes

    GLuint shader = 0;
    GLuint vao = 0;
    GLuint vbo = 0;
};


#endif //DIPLOMA_GEOMETRY_RENDERER_H
