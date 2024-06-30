#ifndef DIPLOMA_BUNNYMARK_H
#define DIPLOMA_BUNNYMARK_H

#include <vector>
#include <cstdint>
#include <cassert>
#include <random>
#include <chrono>
#include "vec2.hpp"
#include "common.h"
#include "GLFW/glfw3.h"

struct BunnyMarkOpts {
    int numRuns;
    int numQuads;
    int windowWidth;
    int windowHeight;
    UVRegion bunnyRegion;
};

template<typename R>
class BunnyMark {

    struct Bunny {
        glm::vec2 position;
        glm::vec2 size;
        glm::vec2 velocity;
        float rotation;
        Color color;
    };

    BunnyMarkOpts opts;
    // Note: Could use virtual functions, but they are slower.
    R *renderer;
    std::vector<Bunny> bunnies{};

public:

    explicit BunnyMark(R *renderer, BunnyMarkOpts opts) : renderer(renderer), opts(opts) {
        int numResults = opts.numRuns;
        assert(numResults >= 0);
        bunnies.reserve(opts.numQuads);
        setup();
    }

    ~BunnyMark() = default;

    void Run(GLFWwindow *window, const glm::mat4 &projView) {
        using Nano = std::chrono::nanoseconds;
        using Clock = std::chrono::high_resolution_clock;
        struct FrameResult {
            uint64_t total;
            uint64_t gpu;
        };

        std::vector<FrameResult> results{};
        results.reserve(opts.numRuns);

        GLuint query;
        glGenQueries(1, &query);


        double lastFrame = 0.0f;
        glfwSetTime(0);
        for (int i = 0; i < opts.numRuns; i++) {
            double currentFrame = glfwGetTime();
            double dt = currentFrame - lastFrame;
            lastFrame = currentFrame;

            auto start = Clock::now();
            glBeginQuery(GL_TIME_ELAPSED, query);

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glClearColor(0.0f, 0.0f, 0.2f, 1.0f);

            renderer->begin(projView);
            for (auto &bunny: bunnies) {
                RunFrame(dt, bunny);
            }
            renderer->end();
            glEndQuery(GL_TIME_ELAPSED);
            glfwSwapBuffers(window);
            glfwPollEvents();

            auto end = Clock::now();
            auto elapsed = std::chrono::duration_cast<Nano>(end - start).count();

            GLint done = false;
            while (!done) {
                glGetQueryObjectiv(query, GL_QUERY_RESULT_AVAILABLE, &done);
            }
            GLuint64 elapsedGpu = 0;
            glGetQueryObjectui64v(query, GL_QUERY_RESULT, &elapsedGpu);

            results.push_back(FrameResult{
                    .total = elapsed,
                    .gpu = elapsedGpu
            });

        }
        for (auto res: results) {
            printf("frame_time=%lld gpu_time=%lld\n", res.total, res.gpu);
        }

    }

private:

    int randRange(std::mt19937 &rng, int min, int max) {
        int x = rng() % (max - min);
        return min + x;
    }

    int randVelocity(std::mt19937 &rng, int min, int max) {
        int x = randRange(rng, min, max);
        int rev = rng() % 2;
        if (rev) {
            return -x;
        }
        return x;
    }

    void setup() {
        std::random_device rd;
        std::mt19937 gen(rd());
        for (int i = 0; i < opts.numQuads; i++) {
            bunnies.push_back(Bunny{
                    .position = {
                            randRange(gen, 0, opts.windowWidth),
                            randRange(gen, 0, opts.windowHeight)
                    },
                    .size = {
                            randRange(gen, 20, 60),
                            randRange(gen, 20, 60)
                    },
                    .velocity = {
                            randVelocity(gen, 120, 140),
                            randVelocity(gen, 120, 140)
                    },
                    .rotation = randRange(gen, 0, 360) / 360.0f,
                    .color = Color{
                            randRange(gen, 10, 255),
                            randRange(gen, 10, 255),
                            randRange(gen, 10, 255),
                            randRange(gen, 80, 250)
                    }
            });
        }
    }

    inline void RunFrame(float dt, Bunny &bunny) {
        bunny.position += bunny.velocity * dt;
        auto posX = bunny.position.x;
        if (posX < 0 || posX > opts.windowWidth) {
            bunny.velocity.x *= -1;
        }
        auto posY = bunny.position.y;
        if (posY < 0 || posY > opts.windowHeight) {
            bunny.velocity.y *= -1;
        }

        renderer->drawSprite(opts.bunnyRegion, bunny.position, bunny.size,
                             bunny.size * 0.5f, bunny.rotation, bunny.color);
    }

};

#endif //DIPLOMA_BUNNYMARK_H
