#ifndef DIPLOMA_BATCH_RENDERER_H
#define DIPLOMA_BATCH_RENDERER_H

#include <memory>
#include "../base_renderer.h"

class BatchRenderer : public IRenderer {
private:
    constexpr static int aPosLoc = 0;
    constexpr static int aUVLoc = 1;
    constexpr static int aColorLoc = 2;

public:
    struct Vertex {
        glm::vec2 position; // 8 B
        glm::u16vec2 uv;    // 4 B
        glm::u8vec4 color;  // 4 B
    }; // 16 B total

    explicit BatchRenderer(int numQuads = 4000);

    BatchRenderer(const BatchRenderer &other) = delete;
    BatchRenderer(BatchRenderer &&other) noexcept;

    ~BatchRenderer() override;

    void begin(const glm::mat4 &projView) override;
    void drawSprite(const UVRegion &region, glm::vec2 pos, glm::vec2 size, glm::vec2 origin, float rotation, Color color) override;

    void end() override;

    void flush();

private:
    GLuint vao{};
    GLuint vbo{};
    GLuint ibo{};
    GLuint shader{};

    GLuint boundSampler{};

    size_t numVertices{};
    std::unique_ptr<Vertex[]> vertices{};

    int drawOffset{};
    int drawElements{};

    bool inUse{};
};
#endif //DIPLOMA_BATCH_RENDERER_H
