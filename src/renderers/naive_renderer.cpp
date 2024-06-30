#include "naive_renderer.h"

NaiveRenderer::NaiveRenderer() {
    shader = compileShaderProgram({.vertex = R"(
        #version 330 core
        layout(location = 0) in vec2 aPos;
        layout(location = 1) in vec2 aUV;

        out vec2 UV;
        out vec4 color;

        uniform mat3 uModel;
        uniform mat4 uProjView;
        uniform uint uColor;

        void main() {
            UV = aUV;
            color.x = (uColor >> 24) & uint(0xFF);
            color.y = (uColor >> 16) & uint(0xFF);
            color.z = (uColor >> 8) & uint(0xFF);
            color.w = uColor & uint(0xFF);
            color = color / 255.0f;
            gl_Position = uProjView * vec4(uModel * vec3(aPos, 1.0), 1.0);
        }
        )", .fragment = R"(
        #version 330 core
        in vec2 UV;
        in vec4 color;

        out vec4 FragColor;

        uniform sampler2D tex;

        void main() {
            FragColor = texture(tex, UV) * color;
        }
    )"});
    assert(shader);

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &posVBO);
    glGenBuffers(1, &uvVBO);

    assert(vao && posVBO && uvVBO);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, posVBO);

    glm::vec2 vertices[4] {
            {0.0f, 0.0f}, // Bottom left
            {0.0f, 1.0f}, // Top left
            {1.0f, 0.0f}, // Bottom right
            {1.0f, 1.0f}, // Top right
    };
    glEnableVertexAttribArray(aPosLoc);
    glVertexAttribPointer(aPosLoc, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void *) 0);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, uvVBO);
    glEnableVertexAttribArray(aUVLoc);
    glVertexAttribPointer(aUVLoc, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void *) 0);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2), NULL, GL_DYNAMIC_DRAW);


}

NaiveRenderer::~NaiveRenderer() {
    glDeleteProgram(shader);
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &posVBO);
    glDeleteBuffers(1, &uvVBO);
}


void NaiveRenderer::begin(const glm::mat4 &projView) {
    this->projView = projView;
    glBindVertexArray(vao);
    glUseProgram(shader);
    static GLint viewProjLoc = glGetUniformLocation(shader, "uProjView");
    glUniformMatrix4fv(viewProjLoc, 1, GL_FALSE, &projView[0][0]);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

}

void NaiveRenderer::drawSprite(const UVRegion &region, glm::vec2 pos, glm::vec2 size, glm::vec2 origin, float rotation,
                               Color color) {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, region.texture);
    static GLint texLoc = glGetUniformLocation(shader, "tex");
    glUniform1i(texLoc, 0);

#if 0
    auto model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(pos + origin, 0.0f));
    model = glm::rotate(model, rotation, glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::translate(model, glm::vec3(-origin, 0.0f));
    model = glm::scale(model, glm::vec3(size, 1.0f));
#else
    auto model = buildTransformationMatrix(pos, size, origin, rotation);
#endif

    static GLint modelLoc = glGetUniformLocation(shader, "uModel");
    glUniformMatrix3fv(modelLoc, 1, GL_FALSE, &model[0][0]);

    static GLint colorLoc = glGetUniformLocation(shader, "uColor");
    uint32_t packedColor = (color.r << 24) | (color.g << 16) | (color.b << 8) | (color.a);
    glUniform1ui(colorLoc, packedColor);

    if (region != currRegion) {
        currRegion = region;
        glm::vec2 uvs[4] {
                {region.U0(), region.V1()},
                {region.U0(), region.V0()},
                {region.U1(), region.V1()},
                {region.U1(), region.V0()},
        };
        glBufferData(GL_ARRAY_BUFFER, sizeof(uvs), uvs, GL_STATIC_DRAW);
    }

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void NaiveRenderer::end() {

}

