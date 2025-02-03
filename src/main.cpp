#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include "main.h"

void init() {
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(MessageCallback, 0);
    srand(time(nullptr));

    startShader = new Shader("starter", vert_smesh, frag_smesh);
    lightShader = new Shader("light", vert_blank, frag_blank);
    startLight = new Lighting("start light", startShader, MATERIAL_SHINY);
    startMeshA = new StaticMesh(MESH_SBUNNY);
    startMeshB = new StaticMesh(MESH_CUBE);
    lightMesh = new StaticMesh(MESH_CUBE);
    
    sbShader = new Shader("softbody", vert_sbody, frag_sbody);
    sbLight = new Lighting("sb light", sbShader, MATERIAL_RUBBER);
    sb = new SoftBody("SoftBunny", MESH_SBUNNY);

    // startLight->addSpotLightAtt(vec3(-20, -1, -5), Util::RIGHT, vec3(0.2f), vec3(1), vec3(1));
    startLight->addPointLightAtt(lightPos, vec3(0.2f), vec3(1), vec3(1));
    sbLight->addDirLightAtt(Util::DOWN, vec3(0.2f), vec3(0.2f), vec3(1));
    sbLight->addPointLightAtt(lightPos, vec3(0.2f), vec3(1), vec3(1));
    startLight->addDirLightAtt(Util::DOWN, vec3(0.2f), vec3(0.2f), vec3(1));

    SM::startTime = timeGetTime();
}

void display() {
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);  // enable depth-testing
    glEnable(GL_BLEND);       // enable colour blending
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthFunc(GL_LESS);  // depth-testing interprets a smaller value as "closer"
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    mat4 view = SM::camera->getViewMatrix();
    mat4 projection = SM::camera->getPerspectiveMatrix();

    // startLight->use();
    // startLight->setLightAtt(view, projection, SM::camera->pos);
    // startLight->setPointLightAtt(0, lightPos);
    // startMeshB->render(translate(mat4(1), vec3(0, 1, -5)));

    sbLight->use();
    sbLight->setLightAtt(view, projection, SM::camera->pos);
    sbLight->setPointLightAtt(0, lightPos);
    sbLight->shader->setVec3("colour", vec3(1));
    sb->mesh->render(translate(mat4(1), vec3(0, 10, -5)));

    lightShader->use();
    lightShader->setVec3("viewPos", SM::camera->pos);
    lightShader->setMat4("view", view);
    lightShader->setMat4("proj", projection);
    lightShader->setVec4("colour", vec4(1));
    lightMesh->render(translate(mat4(1), lightPos) * scale(mat4(1), vec3(0.25)));
}

void update() {
    SM::updateDelta();
    if (!SM::debug) {
        SM::camera->processMovement();
    }
    sb->update();
    SM::updateTick();
}

void displayUI() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::Begin("Debug Menu", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    auto io = ImGui::GetIO();
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
    ImGui::Text("MP: %.0f, %.0f", SM::mouse.x, SM::mouse.y);
    // if (ImGui::SliderFloat3("Camera Position", &SM::camera->pos.x, -10, 10)) {
    //     SM::camera->updateViewMatrix();
    // }
    ImGui::SliderFloat3("Light Position", &lightPos.x, -10, 10);
    ImGui::SliderFloat("Gravity", &sb->gravity, -50, 50);
    ImGui::SliderFloat("Edge Compliance", &sb->edgeCompliance, 0, 10);
    // ImGui::SliderFloat("Volume Compliance", &sb->volumeCompliance, 0, 1); // should stay at 0 for stability
    ImGui::SliderFloat("Floor Y", &sb->floorY, -50, 10);
    ImGui::End();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

// key pressed
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    ImGuiIO& io = ImGui::GetIO();
    if (!io.WantCaptureKeyboard) {
        const char* key_name = glfwGetKeyName(key, 0);
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, true);
        }
        if (key == GLFW_KEY_LEFT_ALT && action == GLFW_RELEASE) {
            SM::debug = !SM::debug;
        }
        if (key == GLFW_KEY_LEFT_SHIFT) SM::camera->DOWN = action == GLFW_REPEAT || action == GLFW_PRESS;
        if (key == ' ') SM::camera->UP = action == GLFW_REPEAT || action == GLFW_PRESS;
        if (key == 'W') SM::camera->FORWARD = action == GLFW_REPEAT || action == GLFW_PRESS;
        if (key == 'S') SM::camera->BACK = action == GLFW_REPEAT || action == GLFW_PRESS;
        if (key == 'A') SM::camera->LEFT = action == GLFW_REPEAT || action == GLFW_PRESS;
        if (key == 'D') SM::camera->RIGHT = action == GLFW_REPEAT || action == GLFW_PRESS;
        if (key == 'E') SM::camera->SPRINT = action == GLFW_REPEAT || action == GLFW_PRESS;
        if (key == 'P' && action == GLFW_PRESS) SM::camera->CAN_FLY = !SM::camera->CAN_FLY;
    }
}

// mouse clicked
void click_callback(GLFWwindow* window, int button, int action, int mods) {
    ImGuiIO& io = ImGui::GetIO();
    if (!io.WantCaptureMouse) {
    }
}

// mouse moved
void mouse_pos_callback(GLFWwindow* window, double nx, double ny) {
        ImGuiIO& io = ImGui::GetIO();
        if (!SM::debug) {
            SM::camera->processView(nx - SM::mouse.x, ny - SM::mouse.y);
        }
    if (SM::tick > 100) {
    }
    SM::updateMouse(nx, ny);
}

int main(int argc, char const* argv[]) {
    // Set up OpenGL version (4.6)
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);

    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
    GLFWwindow* window = glfwCreateWindow(SM::o_width, SM::o_height, "My 3D Program", NULL, NULL);
    SM::updateWindow(SM::o_width, SM::o_height);

    if (!window) {
        printf("Failed to initialise GLFW window\n");
        glfwTerminate();
        return -1;
    }

    // Set up GLFW context
    glfwMakeContextCurrent(window);
    int version = gladLoadGL(glfwGetProcAddress);
    if (version == 0) {
        printf("Failed to initialize OpenGL context\n");
        return -1;
    }

    // Set up GLFW callbacks
    if (glfwRawMouseMotionSupported()) {
        glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
    }
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);  // resize
    glfwSetKeyCallback(window, key_callback);                           // key press
    glfwSetMouseButtonCallback(window, click_callback);                 // mouse click
    glfwSetCursorPosCallback(window, mouse_pos_callback);               // mouse pos

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    framebuffer_size_callback(window, width, height);  // manually call callback, because dimensions are fucked otherwise
    glfwSetCursorPos(window, width/2, height/2); // manually set mouse position before disabling cursor for initial camera position

    // Set up ImGui
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.FontGlobalScale = 1.5;
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init();
    // raise(SIGTRAP);
    init();
    // Main Loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        if (SM::debug) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        } else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
        update();
        display();
        displayUI();
        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
    return 0;
}
