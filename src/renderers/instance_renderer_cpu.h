#ifndef DIPLOMA_INSTANCE_RENDERER_CPU_H
#define DIPLOMA_INSTANCE_RENDERER_CPU_H

#include <memory>
#include "../base_renderer.h"

class InstanceRendererCPU : public IRenderer {
private:
    constexpr static int aPosLoc = 0;
    constexpr static int aModelMatLoc = 1; // Location 1, 2, 3
    constexpr static int aUVLoc = 4; // Location 4, 5, 6, 7
    constexpr static int aColorLoc = 8;
public:
    struct Instance {
        glm::mat3 model; // 36 B
        glm::u16vec2 uv[4]; // 16 B
        glm::u8vec4 color; // 4 B
    }; // 56 total
    static_assert(sizeof(Instance) == 56);

    explicit InstanceRendererCPU(int maxInstances = 4000);
    ~InstanceRendererCPU() override;

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


#endif //DIPLOMA_INSTANCE_RENDERER_CPU_H
