#include "batch_renderer.h"


BatchRenderer::BatchRenderer(int numQuads) {
    shader = compileShaderProgram({.vertex = R"(
        #version 330 core
        layout (location = 0) in vec2 aPos;
        layout (location = 1) in vec2 aUV;
        layout (location = 2) in vec4 aColor;

        out vec2 UV;
        out vec4 color;
        uniform mat4 uProjView;
        uniform sampler2D uTex;
        void main() {
            vec2 texSize = textureSize(uTex, 0);
            UV = aUV / texSize;
            color = aColor;
            gl_Position = uProjView * vec4(aPos, 0.0, 1.0);
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
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ibo);

    assert(vao && vbo && ibo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

    // Populate index buffer
    size_t numIndices = numQuads * 5; // 5 indices per quad (4 + 1 for primitive restart)
    auto *indices = new GLuint[numIndices];
    for (size_t i = 0, offset = 0; i < numIndices; i += 5, offset += 4) {
        indices[i + 0] = offset + 0; // bottom left
        indices[i + 1] = offset + 3; // top left
        indices[i + 2] = offset + 1; // bottom right
        indices[i + 3] = offset + 2; // top right
        indices[i + 4] = UINT16_MAX;
    }
    // Send indices to GPU
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr) (numIndices * sizeof(*indices)), indices, GL_STATIC_DRAW);
    delete[] indices;

    // Reserve space for vertex data
    numVertices = numQuads * 4; // 4 vertices per quad
    vertices = std::make_unique<Vertex[]>(numVertices);
    // Allocate buffer on GPU
    glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr) (numVertices * sizeof(Vertex)), nullptr, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(aPosLoc); // aPos
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *) offsetof(Vertex, position));
    glEnableVertexAttribArray(aUVLoc); // aUV
    glVertexAttribPointer(1, 2, GL_UNSIGNED_SHORT, GL_FALSE, sizeof(Vertex), (void *) offsetof(Vertex, uv));
    glEnableVertexAttribArray(aColorLoc); // aColor
    // Normalize color
    glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (void *) offsetof(Vertex, color));
}

BatchRenderer::BatchRenderer(BatchRenderer &&other) noexcept {
    vao = other.vao;
    vbo = other.vbo;
    ibo = other.ibo;
    shader = other.shader;
    numVertices = other.numVertices;
    vertices = std::move(other.vertices);
    drawOffset = other.drawOffset;
    drawElements = other.drawElements;
    inUse = other.inUse;
    other.vao = 0;
    other.vbo = 0;
    other.ibo = 0;
    other.shader = 0;
    other.numVertices = 0;
    other.vertices = nullptr;
    other.drawOffset = 0;
    other.drawElements = 0;
    other.inUse = false;
}



BatchRenderer::~BatchRenderer() {
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ibo);
    glDeleteVertexArrays(1, &vao);
    glDeleteProgram(shader);
}

void BatchRenderer::begin(const glm::mat4 &projView) {
    assert(!inUse);
    inUse = true;
    drawOffset = 0;
    drawElements = 0;

    glBindVertexArray(vao);
    glUseProgram(shader);
    // Note: Uniform location is cached
    static GLint uTransformLoc = glGetUniformLocation(shader, "uProjView");
    glUniformMatrix4fv(uTransformLoc, 1, GL_FALSE, &projView[0][0]);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    glEnable(GL_PRIMITIVE_RESTART);
    glPrimitiveRestartIndex(UINT16_MAX);
}

void BatchRenderer::drawSprite(const UVRegion &region, glm::vec2 pos, glm::vec2 size, glm::vec2 origin, float rotation, Color color) {
    assert(inUse);
    if (region.texture != boundSampler) {
        flush();
        boundSampler = region.texture;
        static GLint uTexLoc = glGetUniformLocation(shader, "uTex");
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, region.texture);
        glUniform1i(uTexLoc, 0);

    }
    if (drawOffset >= numVertices) {
        flush();
    }


    // Construct model matrix
    // Origin is for rotation
#if 1
    auto model = buildTransformationMatrix(pos, size, origin, rotation);

    // Calculate vertices
    glm::vec2 bottomLeft = model * glm::vec3(0.0f, 0.0f, 1.0f);
    glm::vec2 bottomRight = model * glm::vec3(1.0f, 0.0f, 1.0f);
    glm::vec2 topRight = model * glm::vec3(1.0f, 1.0f, 1.0f);
    glm::vec2 topLeft = model * glm::vec3(0.0f, 1.0f, 1.0f);
#else
    auto model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(pos + origin, 0.0f));
    model = glm::rotate(model, rotation, glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::translate(model, glm::vec3(-origin, 0.0f));
    model = glm::scale(model, glm::vec3(size, 1.0f));

    // Calculate vertices
    glm::vec2 bottomLeft = model * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    glm::vec2 bottomRight = model * glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
    glm::vec2 topRight = model * glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);
    glm::vec2 topLeft = model * glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
#endif
    // Write vertices to buffer
    vertices[drawOffset++] = {
            bottomLeft,
            {region.u0, region.v1},
            color
    };
    vertices[drawOffset++] = {
            bottomRight,
            {region.u1, region.v1},
            color
    };
    vertices[drawOffset++] = {
            topRight,
            {region.u1, region.v0},
            color
    };
    vertices[drawOffset++] = {
            topLeft,
            {region.u0, region.v0},
            color
    };
    drawElements += 5;
}

void BatchRenderer::end() {
    assert(inUse);
    flush();
    inUse = false;
}

void BatchRenderer::flush() {
    assert(inUse);
    if (drawOffset == 0) {
        return;
    }
    // Send vertex data to GPU
    glBufferSubData(GL_ARRAY_BUFFER, 0, (GLsizeiptr) (drawOffset * sizeof(*vertices.get())), vertices.get());
    glDrawElements(GL_TRIANGLE_STRIP, drawElements, GL_UNSIGNED_INT, (void *)0);

    // Reset draw offset
    drawOffset = 0;
    drawElements = 0;
}
