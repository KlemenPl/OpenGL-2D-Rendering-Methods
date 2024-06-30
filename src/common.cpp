#include "common.h"

#include <stb_image.h>

void glDebugLog(GLenum source,
                GLenum type,
                unsigned int id,
                GLenum severity,
                GLsizei length,
                const char *message,
                const void *userParam) {
    const char *typeStr = "Unknown";
    switch (type) {
        case GL_DEBUG_TYPE_ERROR:
            typeStr = "Error";
            break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
            typeStr = "Deprecated Behaviour";
            break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
            typeStr = "Undefined Behaviour";
            break;
        case GL_DEBUG_TYPE_PORTABILITY:
            typeStr = "Portability";
            break;
        case GL_DEBUG_TYPE_PERFORMANCE:
            typeStr = "Performance";
            break;
        case GL_DEBUG_TYPE_OTHER:
            typeStr = "Other";
            break;
        case GL_DEBUG_TYPE_MARKER:
            typeStr = "Marker";
            break;
        case GL_DEBUG_TYPE_PUSH_GROUP:
            typeStr = "Push Group";
            break;
        case GL_DEBUG_TYPE_POP_GROUP:
            typeStr = "Pop Group";
            break;
        default:;
    }
    const char *sourceStr = "Unknown";
    switch (source) {
        case GL_DEBUG_SOURCE_API:
            sourceStr = "API";
            break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
            sourceStr = "Window System";
            break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER:
            sourceStr = "Shader Compiler";
            break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:
            sourceStr = "Third Party";
            break;
        case GL_DEBUG_SOURCE_APPLICATION:
            sourceStr = "Application";
            break;
        case GL_DEBUG_SOURCE_OTHER:
            sourceStr = "Other";
            break;
        default:;
    }

    const char *severityStr = "Unknown";
    switch (severity) {
        case GL_DEBUG_SEVERITY_HIGH:
            severityStr = "High";
            break;
        case GL_DEBUG_SEVERITY_MEDIUM:
            severityStr = "Medium";
            break;
        case GL_DEBUG_SEVERITY_LOW:
            severityStr = "Low";
            break;
        case GL_DEBUG_SEVERITY_NOTIFICATION:
            severityStr = "Notification";
            break;
        default:;
    }

    fprintf(stderr, "OpenGL [%s - %s: %s] (%d): %s\n", severityStr, sourceStr, typeStr, id, message);
}

void enableOpenGLDebugLogging() {
    GLint flags = 0;
    glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
    {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(glDebugLog, nullptr);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
    }
}

GLuint compileShaderStage(const char *source, GLenum type) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLchar infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::fprintf(stderr, "ERROR::SHADER::COMPILATION_FAILED\n%s\n", infoLog);
        return 0; // 0 is not a valid shader
    }
    return shader;
}
GLuint compileShaderProgram(const ShaderDesc &desc) {
    GLuint program = 0;
    GLuint vert = 0;
    GLuint frag = 0;
    GLuint geom = 0;

    if (desc.vertex) {
        vert = compileShaderStage(desc.vertex, GL_VERTEX_SHADER);
        if (vert == 0)
            goto cleanup;
    }
    if (desc.fragment) {
        frag = compileShaderStage(desc.fragment, GL_FRAGMENT_SHADER);
        if (frag == 0)
            goto cleanup;
    }
    if (desc.geometry) {
        geom = compileShaderStage(desc.geometry, GL_GEOMETRY_SHADER);
        if (geom == 0)
            goto cleanup;
    }

    // Frag and vert are required
    if (vert == 0 || frag == 0) {
        std::fprintf(stderr, "ERROR::SHADER::PROGRAM::MISSING_REQUIRED_STAGES\n");
        goto cleanup;
    }

    program = glCreateProgram();
    if (program == 0) {
        std::fprintf(stderr, "ERROR::SHADER::PROGRAM::CREATION_FAILED\n");
        goto cleanup;
    }

    glAttachShader(program, vert);
    glAttachShader(program, frag);
    if (geom) glAttachShader(program, geom);

    glLinkProgram(program);
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);

    if (!success) {
        GLchar infoLog[512];
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        std::fprintf(stderr, "ERROR::SHADER::PROGRAM::LINKING_FAILED\n%s\n", infoLog);
        glDeleteProgram(program);
        program = 0;
    }

cleanup:
    if (vert) glDeleteShader(vert);
    if (frag) glDeleteShader(frag);
    if (geom) glDeleteShader(geom);

    return program;
}

Texture loadTexture(const char *path) {
    Texture texture{};
    stbi_set_flip_vertically_on_load(true);
    int desiredChannels = 4;
    stbi_uc *texData = stbi_load(path, &texture.width, &texture.height, &desiredChannels, 0);
    if (!texData) {
        std::fprintf(stderr, "ERROR::TEXTURE::LOAD_FAILED\n");
    }
    // Upload to GPU
    glGenTextures(1, &texture.id);
    if (texture.id == 0) {
        std::fprintf(stderr, "ERROR::TEXTURE::UPLOAD_FAILED\n");
    }
    glBindTexture(GL_TEXTURE_2D, texture.id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture.width, texture.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texData);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(texData);

    return texture;
}
Texture loadDummyTexture() {
    Texture texture{};
    texture.width = 1;
    texture.height = 1;
    glGenTextures(1, &texture.id);
    glBindTexture(GL_TEXTURE_2D, texture.id);
    uint8_t pixel[] = {255, 255, 255, 255};
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture.width, texture.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
    return texture;

}
UVRegion getUVRegion(const Texture &texture, int x, int y, int width, int height) {
    return {texture.id,
            uint16_t(texture.width),
            uint16_t(texture.height),
            uint16_t(x), uint16_t(y),
            uint16_t(x + width), uint16_t(y + height)};

}
