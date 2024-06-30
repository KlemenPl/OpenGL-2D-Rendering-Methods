#include "geometry_renderer.h"

GeometryRenderer::GeometryRenderer() {
    shader = compileShaderProgram({.vertex = R"(
        #version 330 core
        layout (location = 0) in vec2 aPos;
        layout (location = 1) in vec2 aSize;
        layout (location = 2) in vec2 aOrigin;
        layout (location = 3) in float aRotation;
        layout (location = 4) in vec4 aColor;
        layout (location = 5) in vec2 aUV[4]; // Location 5, 6, 7, 8

        uniform mat4 uProjView;

        out VertexStage {
            vec4 color;
            vec2 UV[4];
            mat4 mvp;
        } vertex;

        mat4 buildMatrix(const vec2 pos, const vec2 size,
                         const vec2 origin, const float rot) {
            float c = cos(rot);
            float s = sin(rot);

            // Note: column-major order
            mat4 mat = mat4(1.0,              0.0,              0.0, 0.0,
                            0.0,              1.0,              0.0, 0.0,
                            0.0,              0.0,              1.0, 0.0,
                            pos.x + origin.x, pos.y + origin.y, 0.0, 1.0);
            mat = mat * mat4(c,   s,   0.0, 0.0,
                             -s,  c,   0.0, 0.0,
                             0.0, 0.0, 0.0, 0.0,
                             0.0, 0.0, 0.0, 1.0);
            mat = mat * mat4(1.0,       0.0,       0.0, 0.0,
                             0.0,       1.0,       0.0, 0.0,
                             0.0,       0.0,       0.0, 0.0,
                             -origin.x, -origin.y, 0.0, 1.0);
            mat = mat * mat4(size.x, 0.0,    0.0, 0.0,
                             0.0,    size.y, 0.0, 0.0,
                             0.0,    0.0,    0.0, 0.0,
                             0.0,    0.0,    0.0, 1.0);
            return mat;
        }

        void main() {
            vertex.color = aColor;
            for (int i = 0; i < 4; i++) {
                vertex.UV[i] = aUV[i];
            }
            vertex.mvp = uProjView * buildMatrix(aPos, aSize, aOrigin, aRotation);
        }
    )", .fragment = R"(
        #version 330 core
        in vec2 UV;
        in vec4 color;

        out vec4 FragColor;

        uniform sampler2D uTex;

        void main() {
            FragColor = texture(uTex, UV) * color;
        }
    )", .geometry = R"(
        #version 330 core
        layout(points) in;
        layout(triangle_strip, max_vertices = 4) out;

        out vec2 UV;
        out vec4 color;

        uniform sampler2D uTex;

        in VertexStage {
            vec4 color;
            vec2 UV[4];
            mat4 mvp;
        } vertex[];

        void main() {
            const vec2 positions[4] = vec2[4](
                vec2(1.0, 0.0), // Bottom right
                vec2(0.0, 0.0), // Bottom left
                vec2(1.0, 1.0), // Top right
                vec2(0.0, 1.0)  // Top left
            );

            color = vertex[0].color;
            vec2 texSize = textureSize(uTex, 0);
            for (int i = 0; i < 4; i++) {
                gl_Position = vertex[0].mvp * vec4(positions[i], 0.0, 1.0);
                UV = vertex[0].UV[i] / texSize;
                EmitVertex();
            }
            EndPrimitive();
        }
    )"});

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    assert(vao && vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex), nullptr, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(aPosLoc); // aPos
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *) offsetof(Vertex, position));
    glEnableVertexAttribArray(aSizeLoc); // aSize
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *) offsetof(Vertex, size));
    glEnableVertexAttribArray(aOriginLoc); // aOrigin
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *) offsetof(Vertex, origin));
    glEnableVertexAttribArray(aRotationLoc); // aRotation
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *) offsetof(Vertex, rotation));
    glEnableVertexAttribArray(aColorLoc); // aColor
    glVertexAttribPointer(aColorLoc, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (void *) (offsetof(Vertex, color)));
    for (int i = 0; i < 4; i++) {
        glEnableVertexAttribArray(aUVLoc + i); // aUV
        glVertexAttribPointer(aUVLoc + i, 2, GL_UNSIGNED_SHORT, GL_FALSE, sizeof(Vertex), (void *) (offsetof(Vertex, uv) + i * sizeof(glm::u16vec2)));
    }
}

GeometryRenderer::~GeometryRenderer() {
    glDeleteProgram(shader);
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
}

void GeometryRenderer::begin(const glm::mat4 &projView) {
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    glUseProgram(shader);
    glBindVertexArray(vao);

    static GLint uProjViewLoc = glGetUniformLocation(shader, "uProjView");
    glUniformMatrix4fv(uProjViewLoc, 1, GL_FALSE, &projView[0][0]);
}

void GeometryRenderer::drawSprite(const UVRegion &region, glm::vec2 position, glm::vec2 size, glm::vec2 origin, float rotation,
                                  Color color) {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, region.texture);
    static GLint texLoc = glGetUniformLocation(shader, "tex");
    glUniform1i(texLoc, 0);
    Vertex vertex = {
        .position = position,
        .size = size,
        .origin = origin,
        .rotation = rotation,
        .color = color,
        .uv = {
            {region.u0, region.v1},
            {region.u1, region.v1},
            {region.u0, region.v0},
            {region.u1, region.v0},
        },
    };

    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertex), &vertex);
    glDrawArrays(GL_POINTS, 0, 1);

}

void GeometryRenderer::end() {

}
