#ifndef MAIN_H
#define MAIN_H

#define GLM_ENABLE_EXPERIMENTAL

#include <cstdio>
#include <iostream>
#include <sstream>

// OpenGL includes
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include "camera.h"
#include "lighting.h"
#include "shader.h"
#include "softbody.h"
#include "sprite.h"
#include "staticmesh.h"
#include "bonemesh.h"
#include "variantmesh.h"
#include "sm.h"
#include "ui.h"
#include "util.h"

/* Pre-defined variables (paths, etc.) */
const char* vert_circle = PROJDIR "Shaders/circle.vert.glsl";
const char* frag_circle = PROJDIR "Shaders/circle.frag.glsl";
const char* vert_smesh = PROJDIR "Shaders/staticMesh.vert";
const char* frag_smesh = PROJDIR "Shaders/staticMesh.frag";
const char* vert_sbody = PROJDIR "Shaders/softBody.vert";
const char* frag_sbody = PROJDIR "Shaders/softBody.frag";
const char* vert_blank = PROJDIR "Shaders/blank.vert";
const char* frag_blank = PROJDIR "Shaders/blank.frag";

/* Sprites */
#define SPRITE_PH "strawberry_big.png"  // Placeholder sprite
#define SPRITE_BLANK "blank.png"        // Blank sprite

/* Meshes */
#define MESH_CUBE "cube.gltf"
#define MESH_MONKEY "monkey.gltf"
#define MESH_SBUNNY "softbunny.ply"

/* Variables */
Shader* startShader, *lightShader, *sbShader;
Lighting* startLight, *sbLight;
StaticMesh *startMeshA, *startMeshB, *lightMesh;
SoftBody* sb;
float radius = 1;
vec3 lightPos = vec3(0, 20, -5);
vec3 lightCol = vec3(0.2, 1, 1);

/* Main Functions */
// Initialise function. Runs before the main loop.
void init();
// Display function. Runs inside the main loop. Use for rendering objects to the screen.
void display();
// Update function. Runs inside the main loop. Use for non-rendering tasks such as updating timers
void update();

/* Additional utility functions */
// Resize callback
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    SM::updateWindow(width, height);
    glViewport(0, 0, width, height);
}

// error/debug message callback, from https://deccer.github.io/OpenGL-Getting-Started/02-debugging/02-debug-callback/
void GLAPIENTRY
MessageCallback(GLenum source,
                GLenum type,
                GLuint id,
                GLenum severity,
                GLsizei length,
                const GLchar* message,
                const void* userParam) {
    // Ignore certain verbose info messages (particularly ones on Nvidia).
    if (id == 131169 ||
        id == 131185 ||  // NV: Buffer will use video memory
        id == 131218 ||
        id == 131204 ||  // Texture cannot be used for texture mapping
        id == 131222 ||
        id == 131154 ||  // NV: pixel transfer is synchronized with 3D rendering
        id == 0          // gl{Push, Pop}DebugGroup
    )
        return;

    std::stringstream debugMessageStream;
    debugMessageStream << message << '\n';

    switch (source) {
        case GL_DEBUG_SOURCE_API:
            debugMessageStream << "Source: API";
            break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
            debugMessageStream << "Source: Window Manager";
            break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER:
            debugMessageStream << "Source: Shader Compiler";
            break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:
            debugMessageStream << "Source: Third Party";
            break;
        case GL_DEBUG_SOURCE_APPLICATION:
            debugMessageStream << "Source: Application";
            break;
        case GL_DEBUG_SOURCE_OTHER:
            debugMessageStream << "Source: Other";
            break;
    }

    debugMessageStream << '\n';

    switch (type) {
        case GL_DEBUG_TYPE_ERROR:
            debugMessageStream << "Type: Error";
            break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
            debugMessageStream << "Type: Deprecated Behaviour";
            break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
            debugMessageStream << "Type: Undefined Behaviour";
            break;
        case GL_DEBUG_TYPE_PORTABILITY:
            debugMessageStream << "Type: Portability";
            break;
        case GL_DEBUG_TYPE_PERFORMANCE:
            debugMessageStream << "Type: Performance";
            break;
        case GL_DEBUG_TYPE_MARKER:
            debugMessageStream << "Type: Marker";
            break;
        case GL_DEBUG_TYPE_PUSH_GROUP:
            debugMessageStream << "Type: Push Group";
            break;
        case GL_DEBUG_TYPE_POP_GROUP:
            debugMessageStream << "Type: Pop Group";
            break;
        case GL_DEBUG_TYPE_OTHER:
            debugMessageStream << "Type: Other";
            break;
    }

    debugMessageStream << '\n';

    switch (severity) {
        case GL_DEBUG_SEVERITY_HIGH:
            debugMessageStream << "Severity: high";
            break;
        case GL_DEBUG_SEVERITY_MEDIUM:
            debugMessageStream << "Severity: medium";
            break;
        case GL_DEBUG_SEVERITY_LOW:
            debugMessageStream << "Severity: low";
            break;
        case GL_DEBUG_SEVERITY_NOTIFICATION:
            debugMessageStream << "Severity: notification";
            break;
    }

    fprintf(stderr, "\nOpenGL Message: {%x} {%s}", type, debugMessageStream.str().c_str());
}

#endif /* MAIN_H */
