#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "common.h"

#define TRIANGLE_STRIP


int main(int argc, const char **argv) {
    glfwInit();
    GLFWwindow *window = glfwCreateWindow(640, 640, "", nullptr, nullptr);
    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    gladLoadGL();
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    glCullFace(GL_NONE);

#if defined(TRIANGLE)
    glfwSetWindowTitle(window, "GL_TRIANGLES");
#elif defined(TRIANGLE_STRIP)
    glfwSetWindowTitle(window, "GL_TRIANGLE_STRIP");
#elif defined(TRIANGLE_FAN)
    glfwSetWindowTitle(window, "GL_TRIANGLE_FAN");
#endif


    GLuint shader = compileShaderProgram({.vertex=R"(
        #version 330 core
        layout (location = 0) in vec2 aPos;

        void main() {
            gl_Position = vec4(aPos, 0.0, 1.0);
        }
    )", .fragment = R"(
        #version 330 core
        out vec4 FragColor;
        void main() {
            FragColor = vec4(1.0, 1.0, 1.0, 1.0);
        }
    )"});

    Texture texture = loadDummyTexture();
    GLuint vao, vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void *) 0);
    glPolygonMode( GL_FRONT_AND_BACK, GL_LINE);
    glLineWidth(4.0f);
    // quad
#if defined(TRIANGLE)
    float vertices[] = {
            // bottom left quad
            -0.8f, -0.8f, // bottom left
            -0.8f, -0.1f, // top left
            -0.1f, -0.1f, // top right
            -0.1f, -0.1f, // top right
            -0.1f, -0.8f, // bottom right
            -0.8f, -0.8f, // bottom left

            // top right quad
            0.1f, 0.1f, // bottom left
            0.1f, 0.8f, // top left
            0.8f, 0.8f, // top right
            0.8f, 0.8f, // top right
            0.8f, 0.1f, // bottom right
            0.1f, 0.1f, // bottom left
    };
#elif defined(TRIANGLE_STRIP)
    float vertices[] = {
            // bottom left quad
            -0.8f, -0.8f, // bottom left
            -0.8f, -0.1f, // top left
            -0.1f, -0.8f, // bottom right
            -0.1f, -0.1f, // top right

            // Degenerate triangle
            -0.1f, -0.1f, // top right
            0.1f, 0.1f, // bottom left

            // top right quad
            0.1f, 0.1f, // bottom left
            0.1f, 0.8f, // top left
            0.8f, 0.1f, // bottom right
            0.8f, 0.8f, // top right
    };
#elif defined(TRIANGLE_FAN)
    float vertices[] = {
            // bottom left quad
            -0.8f, -0.1f, // top left
            -0.8f, -0.8f, // bottom left
            -0.1f, -0.8f, // bottom right
            -0.1f, -0.1f, // top right

            // top right quad
            0.1f, 0.8f, // top left
            0.1f, 0.1f, // bottom left
            0.8f, 0.1f, // bottom right
            0.8f, 0.8f, // top right
    };
#endif
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glUseProgram(shader);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }

        int width, height;
        glfwGetWindowSize(window, &width, &height);
        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);

#ifdef TRIANGLE
        glDrawArrays(GL_TRIANGLES, 0, 12);
#elifdef TRIANGLE_STRIP
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 10);
        //glDrawArrays(GL_TRIANGLE_STRIP, 0, 8);
#elifdef TRIANGLE_FAN
        glDrawArrays(GL_TRIANGLE_FAN, 0, 8);
#endif

        glfwSwapBuffers(window);
    }


    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
