#include "instance_renderer_cpu.h"

InstanceRendererCPU::InstanceRendererCPU(int maxInstances) {
    shader = compileShaderProgram({.vertex = R"(
        #version 330 core
        layout (location = 0) in vec2 aPos;
        layout (location = 1) in mat3 aInstModel; // Location 1, 2, 3
        layout (location = 4) in vec2 aInstUV[4]; // Location 4, 5, 6, 7
        layout (location = 8) in vec4 aInstColor;

        out vec2 UV;
        out vec4 color;
        uniform mat4 uProjView;
        uniform sampler2D uTex;
        void main() {
            vec2 texSize = textureSize(uTex, 0);
            UV = aInstUV[gl_VertexID] / texSize;
            color = aInstColor;
            gl_Position = uProjView * vec4(aInstModel * vec3(aPos, 1.0), 1.0);
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

    for (int i = 0; i < 3; i++) {
        glEnableVertexAttribArray(aModelMatLoc + i); // aInstModel
        glVertexAttribPointer(aModelMatLoc + i, 3, GL_FLOAT, GL_FALSE, sizeof(Instance), (void *) (offsetof(Instance, model) + i * sizeof(glm::vec3)));
        glVertexAttribDivisor(aModelMatLoc + i, 1); // per instance
    }
    for (int i = 0; i < 4; i++) {
        glEnableVertexAttribArray(aUVLoc + i); // aInstUV
        glVertexAttribPointer(aUVLoc + i, 2, GL_UNSIGNED_SHORT, GL_FALSE, sizeof(Instance), (void *) (offsetof(Instance, uv) + i * sizeof(glm::u16vec2)));
        glVertexAttribDivisor(aUVLoc + i, 1); // per instance
    }
    glEnableVertexAttribArray(aColorLoc); // aInstColor
    glVertexAttribPointer(aColorLoc, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Instance), (void *) offsetof(Instance, color));
    glVertexAttribDivisor(aColorLoc, 1); // per instance
}

InstanceRendererCPU::~InstanceRendererCPU() {
    glDeleteProgram(shader);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &instVBO);
    glDeleteVertexArrays(1, &vao);
}

void InstanceRendererCPU::begin(const glm::mat4 &projView) {
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

void InstanceRendererCPU::drawSprite(const UVRegion &region, glm::vec2 position, glm::vec2 size, glm::vec2 origin, float rotation,
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

    // Convert model matrix
    auto model = buildTransformationMatrix(position, size, origin, rotation);

    instanceData[instanceCount++] = Instance {
        .model = model,
        .uv = {
            {region.u0, region.v1},
            {region.u0, region.v0},
            {region.u1, region.v1},
            {region.u1, region.v0},
        },
        .color = color,
    };
}

void InstanceRendererCPU::end() {
    assert(inUse);
    flush();
    inUse = false;
}

void InstanceRendererCPU::flush() {
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
