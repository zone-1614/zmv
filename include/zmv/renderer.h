#pragma once
#include <zmv/camera.h>
#include <zmv/model.h>
#include <zmv/shader.h>
#include <zmv/texture.h>

enum class RenderMode {
    Position, Normal, TexCoords, Diffuse, Specular
};

struct CameraBlock {
    glm::mat4 view;
    glm::mat4 projection;
};

class Renderer {
public: 
    Renderer(int width, int height) :
        width(width), height(height),
        render_mode(RenderMode::Normal),
        position_shader{"shaders/shader.vert", "shaders/position.frag"},
        normal_shader{"shaders/shader.vert", "shaders/normal.frag"},
        texCoords_shader{"shaders/shader.vert", "shaders/texcoords.frag"},
        diffuse_shader{"shaders/shader.vert", "shaders/diffuse.frag"},
        specular_shader{"shaders/shader.vert", "shaders/specular.frag"} 
    {
        camera_block.view = camera.compute_view_matrix();
        camera_block.projection = camera.compute_projection_matrix(width, height);

        glGenBuffers(1, &camera_UBO);    
        glBindBuffer(GL_UNIFORM_BUFFER, camera_UBO);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(CameraBlock), &camera_block, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
        glBindBufferBase(GL_UNIFORM_BUFFER, 0, camera_UBO);
        position_shader.set_UBO("CameraBlock", 0);
        normal_shader.set_UBO("CameraBlock", 0);
        texCoords_shader.set_UBO("CameraBlock", 0);
        diffuse_shader.set_UBO("CameraBlock", 0);
        specular_shader.set_UBO("CameraBlock", 0);
    }

    void render() {
        // render model
        switch (render_mode) {
            case RenderMode::Position:
                model.draw(position_shader);
                break;
            case RenderMode::Normal:
                model.draw(normal_shader);
                break;
            case RenderMode::TexCoords:
                model.draw(texCoords_shader);
                break;
            case RenderMode::Diffuse:
                model.draw(diffuse_shader);
                break;
            case RenderMode::Specular:
                model.draw(specular_shader);
                break;
        }
    }

    void load_model(const std::string &filepath) {
        if (model) {
            model.destroy();
        }
        model.load_model(filepath);
    }

    void set_resulution(int width, int height) {
        this->width = width;
        this->height = height;

        camera_block.projection = camera.compute_projection_matrix(width, height);
        update_camera_UBO();
    }

    RenderMode get_render_mode() const {
        return render_mode;
    }

    void set_render_mode(const RenderMode &render_mode) {
        this->render_mode = render_mode;
    }

    float get_camera_fov() const {
        return camera.fov;
    }

    void set_camera_fov(float fov) {
        camera.fov = fov;

        camera_block.projection = camera.compute_projection_matrix(width, height);
        update_camera_UBO();
    } 

    float get_camera_movement_speed() const {
        return camera.movement_speed;
    }

    void set_camera_movement_speed(float movement_speed) {
        camera.movement_speed = movement_speed;
    }

    void reset_camera() {
        camera.reset();
    }

    void move_camera(const CameraMovement &direction, float deltaTime) {
        camera.move(direction, deltaTime);

        camera_block.view = camera.compute_view_matrix();
        update_camera_UBO();
    }

    float get_camera_look_around_speed() const {
        return camera.look_around_speed;
    }

    void set_camera_look_around_speed(float look_around_speed) {
        camera.look_around_speed = look_around_speed;
    }

    void look_around_camera(float dPhi, float dTheta) {
        camera.look_around(dPhi, dTheta);

        camera_block.view = camera.compute_view_matrix();
        update_camera_UBO();
    }

    void destroy() {
        glDeleteBuffers(1, &camera_UBO);
        model.destroy();
        position_shader.destroy();
        normal_shader.destroy();
        diffuse_shader.destroy();
        specular_shader.destroy();
    }

private:
    int width;
    int height;
    RenderMode render_mode;
    Camera camera;
    Model model;

    Shader position_shader;
    Shader normal_shader;
    Shader texCoords_shader;
    Shader diffuse_shader;
    Shader specular_shader;

    GLuint camera_UBO;
    CameraBlock camera_block;

    void update_camera_UBO() {
        glBindBuffer(GL_UNIFORM_BUFFER, camera_UBO);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(CameraBlock), &camera_block, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }
};