cmake_minimum_required(VERSION 3.22)
project(Diploma CXX C)

set(CMAKE_CXX_STANDARD 23)

# GLFW Library
set(GLFW_BUILD_DOCS OFF CACHE BOOL "Build the GLFW documentation")
set(GLFW_BUILD_TESTS OFF CACHE BOOL "Build the GLFW tests")
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "Build the GLFW examples")
add_subdirectory(libs/glfw)

# stb_image Library
include_directories(libs/stb)

# GLAD Library
include_directories(libs/glad/include)
set(lib_sources libs/glad/src/glad.c libs/stb/stb_impl.c)

# GLM Library
include_directories(libs/glm)

# ImGui Library
include_directories(libs/imgui libs/imgui/backends)
set(imgui_sources
        libs/imgui/imgui.cpp
        libs/imgui/imgui_draw.cpp
        libs/imgui/imgui_widgets.cpp
        libs/imgui/imgui_tables.cpp
        libs/imgui/imgui_demo.cpp
        libs/imgui/backends/imgui_impl_glfw.cpp
        libs/imgui/backends/imgui_impl_opengl3.cpp
        src/primitive_test.cpp
)

add_library(Diploma STATIC
        src/renderers/batch_renderer.cpp
        src/renderers/batch_renderer.h

        src/renderers/geometry_batch_renderer.cpp
        src/renderers/geometry_batch_renderer.h

        src/renderers/geometry_renderer.cpp
        src/renderers/geometry_renderer.h

        src/renderers/instance_renderer.cpp
        src/renderers/instance_renderer.h

        src/renderers/instance_renderer_cpu.cpp
        src/renderers/instance_renderer_cpu.h

        src/renderers/naive_renderer.cpp
        src/renderers/naive_renderer.h

        src/base_renderer.h
        src/bunnymark.h
        src/common.h
        src/common.cpp


        ${lib_sources}
        ${imgui_sources})
target_link_libraries(Diploma PUBLIC glfw)

add_executable(Renderer src/main.cpp)
target_link_libraries(Renderer PUBLIC Diploma)

add_executable(Benchmark src/benchmark.cpp)
target_link_libraries(Benchmark PUBLIC Diploma)

add_executable(PrimitiveTest src/primitive_test.cpp)
target_link_libraries(PrimitiveTest PUBLIC Diploma)
