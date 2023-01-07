#pragma once

#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <variant>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

class Shader {
public:
    Shader() { }
    Shader(
        const std::string &vertex_shader_filepath,
        const std::string &fragment_shader_filepath
    ) : vertex_shader_filepath(vertex_shader_filepath),
        fragment_shader_filepath(fragment_shader_filepath) {
        compile_shader();
        link_shader();
    }

    void destroy() const {
        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);
        glDeleteProgram(program);
    }

    void activate() const {
        glUseProgram(program);
    }

    void deactivate() const {
        glUseProgram(0);
    }

    void set_uniform(
        const std::string &uniform_name,
        const std::variant<bool, GLint, GLuint, GLfloat, glm::vec2, glm::vec3, glm::mat4> &value
    ) const {
        activate();

        // get location of uniform variable
        const GLint location = glGetUniformLocation(program, uniform_name.c_str());

        // set value
        struct Visitor {
            GLint location;
            Visitor(GLint location) : location(location) { }

            void operator()(bool value) { 
                glUniform1i(location, value);
            }
            void operator()(GLint value) {
                glUniform1i(location, value);
            }
            void operator()(GLuint value) {
                glUniform1ui(location, value);
            }
            void operator()(GLfloat value) {
                glUniform1f(location, value);
            }
            void operator()(const glm::vec2 &value) {
                glUniform2fv(location, 1, glm::value_ptr(value));
            }
            void operator()(const glm::vec3 &value) {
                glUniform3fv(location, 1, glm::value_ptr(value));
            }
            void operator()(const glm::mat4 &value) {
                glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));
            }
        };
        std::visit(Visitor{location}, value);
        deactivate();
    }

    void set_uniform_texture(
        const std::string &uniform_name,
        GLuint texture,
        GLuint texture_unit_number
    ) const {
        activate();

        // bind texture to specified texture unit
        glActiveTexture(GL_TEXTURE0 + texture_unit_number);
        glBindTexture(GL_TEXTURE_2D, texture);

        // set texture unit number on uniform variable
        const GLint location = glGetUniformLocation(program, uniform_name.c_str());
        glUniform1i(location, texture_unit_number);

        deactivate();
    }

    void set_UBO(
        const std::string &block_name, 
        GLuint binding_number
    ) const {
        const GLuint block_index = glGetUniformBlockIndex(program, block_name.c_str());
        // set binding number of specified block
        glUniformBlockBinding(program, block_index, binding_number);
    }

private:
    const std::string vertex_shader_filepath;
    std::string vertex_shader_source;
    const std::string fragment_shader_filepath;
    std::string fragment_shader_source;
    GLuint vertex_shader;
    GLuint fragment_shader;
    GLuint program;

    static std::string file_to_string(const std::string& filepath) {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            std::cerr << "failed to open " << filepath << std::endl;
            return "";
        }

        std::stringstream ss;
        ss << file.rdbuf();
        file.close();
        
        return ss.str();
    }

    void compile_shader() {
        // compile vertex shader
        vertex_shader = glCreateShader(GL_VERTEX_SHADER);
        vertex_shader_source = file_to_string(vertex_shader_filepath);
        const char *vertex_shader_source_c = vertex_shader_source.c_str();
        glShaderSource(vertex_shader, 1, &vertex_shader_source_c, nullptr);
        glCompileShader(vertex_shader);
        check_compile_errors(vertex_shader, "vertex shader");

        // compile fragment shader
        fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
        fragment_shader_source = file_to_string(fragment_shader_filepath);
        const char *fragment_shader_source_c = fragment_shader_source.c_str();
        glShaderSource(fragment_shader, 1, &fragment_shader_source_c, nullptr);
        glCompileShader(fragment_shader);
        check_compile_errors(fragment_shader, "fragment shader");
    }

    void link_shader() {
        program = glCreateProgram();
        glAttachShader(program, vertex_shader);
        glAttachShader(program, fragment_shader);
        glLinkProgram(program);
        glDetachShader(program, vertex_shader);
        glDetachShader(program, fragment_shader);
        check_compile_errors(program, "program");
    }

    void check_compile_errors(GLuint shader, std::string type) {
        GLint success;
        GLint log_size = 0;
        if (type != "program") {
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if (success == GL_FALSE) {
                glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_size);
                std::vector<GLchar> error_log(log_size);
                glGetShaderInfoLog(shader, log_size, &log_size, error_log.data());
                std::string error_log_string(error_log.begin(), error_log.end());
                std::cerr << "failed to compile shader of type: " << type << std::endl;
                std::cerr << error_log_string << std::endl;
                glDeleteShader(shader);
            }
        } else {
            glGetProgramiv(shader, GL_LINK_STATUS, &success);
            if (success == GL_FALSE) {
                glGetProgramiv(shader, GL_INFO_LOG_LENGTH, &log_size);
                std::vector<GLchar> error_log(log_size);
                glGetProgramInfoLog(shader, log_size, &log_size, error_log.data());
                std::string error_log_string(error_log.begin(), error_log.end());
                std::cerr << "failed to compile program" << std::endl;
                std::cerr << error_log_string << std::endl;
                glDeleteProgram(shader);
            }
        }
    }
};
