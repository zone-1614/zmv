#include <iostream>
#include <memory>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <zmv/camera.h>
#include <zmv/model.h>
#include <zmv/renderer.h>

int width = 1600;
int height = 900;
std::unique_ptr<Renderer> renderer;

GLFWwindow *window = nullptr;

void handleInput(GLFWwindow *window, const ImGuiIO &io) {
    // close app
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }

    // camera movement
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        renderer->move_camera(CameraMovement::Forward, io.DeltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        renderer->move_camera(CameraMovement::Left, io.DeltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        renderer->move_camera(CameraMovement::Backward, io.DeltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        renderer->move_camera(CameraMovement::Right, io.DeltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS) {
        renderer->move_camera(CameraMovement::Up, io.DeltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS) {
        renderer->move_camera(CameraMovement::Down, io.DeltaTime);
    }

    // camera look around
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
        const float orbit_speed = 1.0f;
        renderer->look_around_camera(orbit_speed * io.MouseDelta.x, orbit_speed * io.MouseDelta.y);
    }
}

void framebufferSizeCallback(GLFWwindow* window, int new_width, int new_height) {
    width = new_width;
    height = new_height;
    glViewport(0, 0, width, height);
    renderer->set_resulution(width, height);
}

bool initialize() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_SAMPLES, 4);

    window = glfwCreateWindow(width, height, "zmv", nullptr, nullptr);
    if (window == nullptr) {
        std::cerr << "failed to create glfw window" << std::endl;
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        std::cerr << "failed to load glad" << std::endl;
        glfwTerminate();
        return false;
    }

    glViewport(0, 0, width, height);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);

    renderer = std::make_unique<Renderer>(width, height);

    std::cout << "GL_VERSION: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GL_VENDOR: " << glGetString(GL_VENDOR) <<  std::endl;
    std::cout << "GL_RENDERER: " << glGetString(GL_RENDERER) <<  std::endl;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    ImFontConfig font_config;
    font_config.SizePixels = 20.0f;
    io.Fonts->AddFontDefault(&font_config);
    (void) io;

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    return true;
}

void begin_frame() {
    glfwPollEvents();
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void UI() {
    ImGui::Begin("zmv");

    // default model 
    static std::string default_models[3] = {
        "model/spot.obj",
        "model/bob.obj",
        // "D:/code/cpp/cg/zmv/model/nilou.obj"
        "model/nilou.obj"
    };
    int choose = 1;
    static char model_filepath[100] = {"model/spot.obj"};
    if (ImGui::Combo("default model", &choose, "spot\0bob\0nilou\0\0")) {
        strcpy(model_filepath, default_models[choose].c_str());
    }

    // custom model 
    ImGui::InputText("custom model filepath", model_filepath, 100);
    if (ImGui::Button("load model")) {
        renderer->load_model(model_filepath);
    }

    // render mode
    static RenderMode render_mode = renderer->get_render_mode();
    if (ImGui::Combo("render mode", reinterpret_cast<int*>(&render_mode), "Position\0Normal\0TexCoords\0Diffuse\0Specular\0\0")) {
        renderer->set_render_mode(render_mode);
    }

    // fov
    static float fov = renderer->get_camera_fov();
    if (ImGui::SliderFloat("fov", &fov, 10.0f, 90.0f)) {
        renderer->set_camera_fov(fov);
    }

    // movement speed
    static float movement_speed = renderer->get_camera_movement_speed();
    if (ImGui::SliderFloat("movement speed", &movement_speed, 0.0f, 10.0f)) {
        renderer->set_camera_movement_speed(movement_speed);
    }

    // look around speed
    static float look_around_speed = renderer->get_camera_look_around_speed();
    if (ImGui::SliderFloat("look around speed", &look_around_speed, 0.0f, 2.0f)) {
        renderer->set_camera_look_around_speed(look_around_speed);
    }

    // reset camera
    if (ImGui::Button("reset camera")) {
        renderer->reset_camera();
    }

    ImGui::End();
}

void end_frame() {
    ImGuiIO &io = ImGui::GetIO();
    handleInput(window, io);
    glClearColor(0.4f, 0.4f, 0.4f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    renderer->render();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(window);
}

void finalize() {
    renderer->destroy();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
}

int main() {
    if (!initialize()) {
        return -1;
    }
    while (!glfwWindowShouldClose(window)) {
        begin_frame();
        UI();
        end_frame();
    }
    finalize();
}