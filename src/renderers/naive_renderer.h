#ifndef DIPLOMA_NAIVE_RENDERER_H
#define DIPLOMA_NAIVE_RENDERER_H

#include "../base_renderer.h"

class NaiveRenderer : public IRenderer {
private:
    constexpr static int aPosLoc = 0;
    constexpr static int aUVLoc = 1;
public:
    NaiveRenderer();
    ~NaiveRenderer() override;

    void begin(const glm::mat4 &projView) override;
    void drawSprite(const UVRegion &region, glm::vec2 pos, glm::vec2 size, glm::vec2 origin, float rotation, Color color) override;
    void end() override;

private:
    GLuint shader = 0;
    glm::mat4 projView{};
    GLuint vao = 0;
    GLuint posVBO = 0;
    GLuint uvVBO = 0;

    UVRegion currRegion{0};
};

#endif //DIPLOMA_NAIVE_RENDERER_H
