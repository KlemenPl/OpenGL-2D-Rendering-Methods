#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm.hpp>
#include <ext.hpp>

#include <random>
#include <iostream>

#include "bunnymark.h"
#include "renderers/naive_renderer.h"
#include "renderers/batch_renderer.h"
#include "renderers/instance_renderer_cpu.h"
#include "renderers/geometry_renderer.h"
#include "renderers/geometry_batch_renderer.h"
#include "renderers/instance_renderer.h"

int parseInt(const char *str) {
    if (!str) {
        return 0;
    }

    return atoi(str);
}

template<typename R>
inline void run(R *renderer, BunnyMarkOpts opts, GLFWwindow *window, glm::mat4 projView) {
    BunnyMark<R> bunnyMark{renderer, opts};
    bunnyMark.Run(window, projView);
}

int main(int argc, const char **argv) {
    int numFrames = 0;
    int numBunnies = 0;
    int batchSize = 0; // only for batched renderers
    const char *rType = nullptr;

    for (int i = 1; i < argc; i++) {
        const char *arg = argv[i];
        const char *nextArg = nullptr;
        if (i + 1< argc) nextArg = argv[i + 1];
        if (strcmp(arg, "--num_frames") == 0) {
            numFrames = parseInt(nextArg);
        } else if (strcmp(arg, "--num_bunnies") == 0) {
            numBunnies = parseInt(nextArg);
        } else if (strcmp(arg, "--batch_size") == 0) {
            batchSize = parseInt(nextArg);
        } else if (strcmp(arg, "--renderer_type") == 0) {
            rType = nextArg;
        }
    }

    glfwInit();
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    GLFWwindow *window = glfwCreateWindow(1280, 720, "Benchmark", nullptr, nullptr);
    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    gladLoadGL();
    // Set fps to unlimited
    glfwSwapInterval(0);

    // For transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // query opengl version
    int major, minor;
    glGetIntegerv(GL_MAJOR_VERSION, &major);
    glGetIntegerv(GL_MINOR_VERSION, &minor);
    printf("OpenGL version: %d.%d\n", major, minor);

    Camera2D camera = {};

    // Load texture
    Texture texture = loadTexture("res/rabbit.png");
    assert(texture.id);
    UVRegion region = getUVRegion(texture, 0, 0, texture.width, texture.height);

    int width, height;
    glfwGetWindowSize(window, &width, &height);
    glViewport(0, 0, width, height);

    BunnyMarkOpts opts = {
            .numRuns = numFrames,
            .numQuads = numBunnies,
            .windowWidth = width,
            .windowHeight = height,
            .bunnyRegion = region,
    };

    glm::mat4 combined = camera.getCombined({width, height});
    if (strcmp(rType, "naive") == 0) {
        auto r = NaiveRenderer();
        run(&r, opts, window, combined);
    } else if (strcmp(rType, "batch") == 0) {
        auto r = BatchRenderer(batchSize);
        run(&r, opts, window, combined);
    } else if (strcmp(rType, "instance_cpu") == 0) {
        auto r = InstanceRendererCPU(batchSize);
        run(&r, opts, window, combined);
    } else if (strcmp(rType, "instance") == 0) {
        auto r = InstanceRenderer(batchSize);
        run(&r, opts, window, combined);
    }else if (strcmp(rType, "geometry") == 0) {
        auto r = GeometryRenderer();
        run(&r, opts, window, combined);
    } else if (strcmp(rType, "geometry_batch") == 0) {
        auto r = GeometryBatchRenderer(batchSize);
        run(&r, opts, window, combined);
    } else {
        assert(false && "Invalid renderer");
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
