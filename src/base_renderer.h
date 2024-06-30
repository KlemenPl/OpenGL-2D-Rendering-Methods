#ifndef DIPLOMA_BASE_RENDERER_H
#define DIPLOMA_BASE_RENDERER_H

#include "common.h"

class IRenderer {
public:
    virtual ~IRenderer() = default;

    virtual void begin(const glm::mat4 &projView) = 0;
    virtual void drawSprite(const UVRegion &region, glm::vec2 pos, glm::vec2 size, glm::vec2 origin, float rotation, Color color) = 0;
    virtual void end() = 0;

};

#endif //DIPLOMA_BASE_RENDERER_H
