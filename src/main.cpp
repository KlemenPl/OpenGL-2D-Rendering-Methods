#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm.hpp>
#include <ext.hpp>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <memory>

#include "common.h"
#include "base_renderer.h"
#include "renderers/batch_renderer.h"
#include "renderers/naive_renderer.h"
#include "renderers/instance_renderer_cpu.h"
#include "renderers/instance_renderer.h"
#include "renderers/geometry_renderer.h"
#include "renderers/geometry_batch_renderer.h"

int main(int argc, const char **argv) {
    glfwInit();
    GLFWwindow *window = glfwCreateWindow(1280, 720, "Renderer", nullptr, nullptr);
    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    gladLoadGL();
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    glCullFace(GL_NONE);

    // Setup ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void) io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    Camera2D camera = {};

    // Load texture
    Texture texture = loadTexture("res/rabbit.png");
    UVRegion region = getUVRegion(texture, 0, 0, texture.width, texture.height);

    {
        std::unique_ptr<IRenderer> batch = std::make_unique<GeometryBatchRenderer>();

        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();

            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
                glfwSetWindowShouldClose(window, GLFW_TRUE);
            }

            int width, height;
            glfwGetWindowSize(window, &width, &height);
            glViewport(0, 0, width, height);
            glm::mat4 combined = camera.getCombined({width, height});

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glClearColor(0.0f, 0.0f, 0.2f, 1.0f);

            static glm::vec2 pos = {0.0f, 0.0f};
            static glm::vec2 size = {100.0f, 100.0f};
            static glm::vec2 origin = {0.0f, 0.0f};
            static float rotation = 0.0f;
            static Color color = Colors::WHITE;

            batch->begin(combined);
            batch->drawSprite(region, pos, size, origin, rotation, color);
            batch->end();



            ImGui::Begin("Debug");
            ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
            ImGui::SliderFloat2("Camera Position", &camera.position.x, -100.0f, 100.0f);
            ImGui::SliderFloat("Camera Zoom", &camera.zoom, 0.1f, 10.0f);

            ImGui::SliderFloat2("Position", &pos.x, -100.0f, 100.0f);
            ImGui::SliderFloat2("Size", &size.x, 0.0f, 200.0f);
            ImGui::SliderFloat2("Origin", &origin.x, -100.0f, 100.0f);
            ImGui::SliderFloat("Rotation", &rotation, -glm::pi<float>(), glm::pi<float>());
            float color4[4] = {color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f};
            ImGui::ColorEdit4("Color", color4);
            color = {color4[0] * 255, color4[1] * 255, color4[2] * 255, color4[3] * 255};

            int u0 = region.u0;
            int v0 = region.v0;
            int u1 = region.u1;
            int v1 = region.v1;
            ImGui::SliderInt("U0", &u0, 0, region.width);
            ImGui::SliderInt("V0", &v0, 0, region.height);
            ImGui::SliderInt("U1", &u1, 0, region.width);
            ImGui::SliderInt("V1", &v1, 0, region.height);
            region.u0 = u0;
            region.v0 = v0;
            region.u1 = u1;
            region.v1 = v1;

            ImGui::End();
            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            glfwSwapBuffers(window);
        }
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
