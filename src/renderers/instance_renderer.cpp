#include "instance_renderer.h"

InstanceRenderer::InstanceRenderer(int maxInstances) {
    shader = compileShaderProgram({.vertex = R"(
        #version 330 core
        layout (location = 0) in vec2 aPos;
        layout (location = 1) in vec2 aInstPos;
        layout (location = 2) in vec2 aInstSize;
        layout (location = 3) in vec2 aInstOrigin;
        layout (location = 4) in float aInstRotation;
        layout (location = 5) in vec4 aInstColor;
        layout (location = 6) in vec2 aInstUV[4]; // loc: 6, 7, 8, 9

        out vec2 UV;
        out vec4 color;
        uniform mat4 uProjView;
        uniform sampler2D uTex;

        mat3 buildMatrix(const vec2 pos, const vec2 size,
                         const vec2 origin, const float rot) {
            float c = cos(rot);
            float s = sin(rot);

            // Note: column-major order
            mat3 mat = mat3(1.0,              0.0,              0.0,
                            0.0,              1.0,              0.0,
                            pos.x + origin.x, pos.y + origin.y, 1.0);
            mat = mat * mat3(c,   s,   0.0,
                             -s,  c,   0.0,
                             0.0, 0.0, 1.0) ;
            mat = mat * mat3(1.0,       0.0,       0.0,
                             0.0,       1.0,       0.0,
                             -origin.x, -origin.y, 1.0);
            mat = mat * mat3(size.x, 0.0,    0.0,
                             0.0,    size.y, 0.0,
                             0.0,    0.0,    1.0);
            return mat;
        }

        void main() {
            vec2 texSize = textureSize(uTex, 0);
            UV = aInstUV[gl_VertexID] / texSize;
            color = aInstColor;

            mat3 model = buildMatrix(aInstPos, aInstSize, aInstOrigin, aInstRotation);

            gl_Position = uProjView * vec4(model * vec3(aPos, 1.0), 1.0);
        }
    )", .fragment = R"(
        #version 330 core
        out vec4 FragColor;
        in vec2 UV;
        in vec4 color;
        uniform sampler2D uTex;
        void main() {
            FragColor = texture(uTex, UV) * color;
        }
    )"});
    assert(shader);

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &instVBO);
    glGenBuffers(1, &vbo);

    assert(vao && instVBO && vbo);

    glBindVertexArray(vao);

    // Generate mesh VBO
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glm::vec2 vertices[4] = {
            {0.0f, 0.0f}, // bottom left
            {0.0f, 1.0f}, // top left
            {1.0f, 0.0f}, // bottom right
            {1.0f, 1.0f}, // top right
    };
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(aPosLoc); // aPos
    glVertexAttribPointer(aPosLoc, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void *) 0);
    glVertexAttribDivisor(aPosLoc, 0);

    // Reserve space for instance data
    glBindBuffer(GL_ARRAY_BUFFER, instVBO);
    this->maxInstances = maxInstances;
    instanceData = std::make_unique<Instance[]>(maxInstances);
    // Allocate buffer on GPU
    glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr) (maxInstances * sizeof(Instance)), nullptr, GL_DYNAMIC_DRAW);

    // Instance attributes
    glEnableVertexAttribArray(aInstPosLoc);
    glVertexAttribPointer(aInstPosLoc, 2, GL_FLOAT, GL_FALSE, sizeof(Instance), (void *) (offsetof(Instance, pos)));
    glVertexAttribDivisor(aInstPosLoc, 1);

    glEnableVertexAttribArray(aInstSizeLoc);
    glVertexAttribPointer(aInstSizeLoc, 2, GL_FLOAT, GL_FALSE, sizeof(Instance), (void *) (offsetof(Instance, size)));
    glVertexAttribDivisor(aInstSizeLoc, 1);

    glEnableVertexAttribArray(aInstOriginLoc);
    glVertexAttribPointer(aInstOriginLoc, 2, GL_FLOAT, GL_FALSE, sizeof(Instance), (void *) (offsetof(Instance, origin)));
    glVertexAttribDivisor(aInstOriginLoc, 1);

    glEnableVertexAttribArray(aInstRotationLoc);
    glVertexAttribPointer(aInstRotationLoc, 1, GL_FLOAT, GL_FALSE, sizeof(Instance), (void *) (offsetof(Instance, rotation)));
    glVertexAttribDivisor(aInstRotationLoc, 1);

    glEnableVertexAttribArray(aInstColorLoc);
    glVertexAttribPointer(aInstColorLoc, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Instance), (void *) (offsetof(Instance, color)));
    glVertexAttribDivisor(aInstColorLoc, 1);

    for (int i = 0; i < 4; i++) {
        glEnableVertexAttribArray(aInstUVLoc + i);
        glVertexAttribPointer(aInstUVLoc + i, 2, GL_UNSIGNED_SHORT, GL_FALSE, sizeof(Instance),
                              (void *) (offsetof(Instance, uv) + i * sizeof(glm::u16vec2)));
        glVertexAttribDivisor(aInstUVLoc + i, 1);
    }
}

InstanceRenderer::~InstanceRenderer() {
    glDeleteProgram(shader);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &instVBO);
    glDeleteVertexArrays(1, &vao);
}

void InstanceRenderer::begin(const glm::mat4 &projView) {
    assert(!inUse);
    inUse = true;
    instanceCount = 0;

    glBindVertexArray(vao);
    glUseProgram(shader);
    static GLint uProjViewLoc = glGetUniformLocation(shader, "uProjView");
    glUniformMatrix4fv(uProjViewLoc, 1, GL_FALSE, &projView[0][0]);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
}

void InstanceRenderer::drawSprite(const UVRegion &region, glm::vec2 position, glm::vec2 size, glm::vec2 origin,
                                  float rotation,
                                  Color color) {
    assert(inUse);
    if (region.texture != boundSampler) {
        flush();
        boundSampler = region.texture;
        static GLint uTexLoc = glGetUniformLocation(shader, "uTex");
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, region.texture);
        glUniform1i(uTexLoc, 0);

    }
    if (instanceCount >= maxInstances) {
        flush();
    }

    instanceData[instanceCount++] = Instance{
        .pos = position,
        .size = size,
        .origin = origin,
        .rotation = rotation,
        .color = color,
        .uv = {
                {region.u0, region.v1},
                {region.u0, region.v0},
                {region.u1, region.v1},
                {region.u1, region.v0},
        },
    };
}

void InstanceRenderer::end() {
    assert(inUse);
    flush();
    inUse = false;
}

void InstanceRenderer::flush() {
    assert(inUse);
    if (instanceCount == 0) {
        return;
    }

    // Send instance data to GPU
    glBindBuffer(GL_ARRAY_BUFFER, instVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, (GLsizeiptr) (instanceCount * sizeof(Instance)), instanceData.get());

    // Draw
    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, instanceCount);

    instanceCount = 0;
}
