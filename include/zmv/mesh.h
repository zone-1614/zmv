#pragma once
#include <string>
#include <vector>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <zmv/shader.h>
#include <zmv/texture.h>

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 tex_coords;
};

struct Material {
    glm::vec3 kd; // diffuse color
    glm::vec3 ks; // specular color
    glm::vec3 ka; // ambient color
    float shininess;
};

class Mesh {
public:
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    Material material;
    std::vector<unsigned int> indices_of_textures;

    Mesh(
        const std::vector<Vertex> &vertices,
        const std::vector<unsigned int> &indices, 
        const Material &material,
        const std::vector<unsigned int> indices_of_textures
    ) : vertices(vertices), indices(indices), material(material),
        indices_of_textures(indices_of_textures) {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);

        // VBO
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

        // EBO
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

        // position
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(0));

        // normal 
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, normal)));

        // texture coords
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, tex_coords)));

        glBindVertexArray(0);
    }

    void destroy() {
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
        glDeleteVertexArrays(1, &VAO);
        vertices.clear();
        indices.clear();
        indices_of_textures.clear();
    }

    void draw(const Shader &shader, const std::vector<Texture> &textures) const {
        shader.set_uniform("kd", material.kd);
        shader.set_uniform("ks", material.ks);
        shader.set_uniform("ka", material.ka);
        shader.set_uniform("shininess", material.shininess);

        std::size_t n_diffuse = 0;
        std::size_t n_specular = 0;
        for (std::size_t i = 0; i < indices_of_textures.size(); ++i) {
            const Texture &texture = textures[indices_of_textures[i]];
            const int texture_unit_number = i;

            switch (texture.texture_type) {
                case TextureType::DIFFUSE: {
                    const std::string uniform_name = "diffuseTextures[" + std::to_string(n_diffuse) + "]";
                    shader.set_uniform_texture(uniform_name, texture.id, texture_unit_number);
                    n_diffuse++;
                    break;
                }
                case TextureType::SPECULAR: {
                    const std::string uniform_name = "specularTextures[" + std::to_string(n_specular) + "]";
                    shader.set_uniform_texture(uniform_name, texture.id, texture_unit_number);
                    n_specular++;
                    break;
                }
            }
        }

        shader.set_uniform("hasDiffuseTextures", n_diffuse > 0);
        shader.set_uniform("hasSpecularTextures", n_specular > 0);

        // draw mesh
        glBindVertexArray(VAO);
        shader.activate();
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        shader.deactivate();
        glBindVertexArray(0);
    }

private:
    GLuint VAO;
    GLuint VBO;
    GLuint EBO;
};