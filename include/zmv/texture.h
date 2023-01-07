#pragma once

#include <string>
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include <glad/glad.h>
#include <stb_image.h>

enum class TextureType {
    DIFFUSE, SPECULAR
};

class Texture {
public:
    std::string filepath;
    GLuint id;
    TextureType texture_type;

    Texture() {
        glGenTextures(1, &id);
        glBindTexture(GL_TEXTURE_2D, id);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    Texture(const std::string &filepath, const TextureType &texture_type) : Texture() {
        this->filepath = filepath;
        this->texture_type = texture_type;
        load_image(filepath);
    }

    void destroy() {
        glDeleteTextures(1, &id);
    }

    void load_image(const std::string &filepath) const {
        int width, height, channels;
        unsigned char *image = stbi_load(filepath.c_str(), &width, &height, &channels, 3);

        if (!image) {
            std::cerr << "failed to open " << filepath << std::endl;
            return ;
        }

        // set image to texture 
        glBindTexture(GL_TEXTURE_2D, id);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
        glGenerateMipmap(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);
        stbi_image_free(image);
    }
};