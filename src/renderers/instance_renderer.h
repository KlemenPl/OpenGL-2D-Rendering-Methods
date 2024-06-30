#ifndef DIPLOMA_INSTANCE_RENDERER_H
#define DIPLOMA_INSTANCE_RENDERER_H

#include <memory>
#include "../base_renderer.h"

class InstanceRenderer : public IRenderer {
private:
    constexpr static int aPosLoc = 0;
    constexpr static int aInstPosLoc = 1;
    constexpr static int aInstSizeLoc = 2;
    constexpr static int aInstOriginLoc = 3;
    constexpr static int aInstRotationLoc = 4;
    constexpr static int aInstColorLoc = 5;
    constexpr static int aInstUVLoc = 6; // 6, 7, 8, 9
public:
    struct Instance {
        glm::vec2 pos;     // 8 B
        glm::vec2 size;    // 8 B
        glm::vec2 origin;  // 8 B
        float rotation;    // 4 B
        glm::u8vec4 color; // 4 B

        glm::u16vec2 uv[4]; // 16 B
    }; // 42 total
    static_assert(sizeof(Instance) == 48);

    explicit InstanceRenderer(int maxInstances = 4000);
    ~InstanceRenderer() override;

    void begin(const glm::mat4 &projView) override;
    void drawSprite(const UVRegion &region, glm::vec2 position, glm::vec2 size, glm::vec2 origin, float rotation, Color color) override;
    void end() override;
    void flush();

private:
    GLuint vao{};
    GLuint instVBO{};
    GLuint vbo{};
    GLuint shader{};

    GLuint boundSampler{};

    size_t maxInstances{};
    std::unique_ptr<Instance[]> instanceData{};

    int instanceCount{};

    bool inUse{};
};


#endif //DIPLOMA_INSTANCE_RENDERER_MATRIX_H
