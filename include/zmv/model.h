#pragma once
#include <filesystem>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <zmv/mesh.h>
#include <zmv/shader.h>
#include <zmv/texture.h>

class Model {
public:
    Model() { }
    Model(const std::string& filepath) { 
        load_model(filepath);
    }

    operator bool() const {
        return meshes.size() > 0;
    }

    void load_model(const std::string &filepath) {
        Assimp::Importer importer;
        const aiScene *scene = importer.ReadFile(filepath, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            std::cerr << "[Assimp]" << importer.GetErrorString() << std::endl;
            return ;
        }

        // process scene graph 
        const std::filesystem::path ps(filepath);
        process_node(scene->mRootNode, scene, ps.parent_path().string());

        // show info
        std::cout << "[Model] " << filepath << " loaded." << std::endl;
        std::cout << "[Model] number of meshes: " << meshes.size() << std::endl;

        std::size_t nVertices = 0;
        std::size_t nFaces = 0;
        for (std::size_t i = 0; i < meshes.size(); ++i) {
            nVertices += meshes[i].vertices.size();
            nFaces += meshes[i].indices.size() / 3;
        }
        std::cout << "[Model] number of vertices: " << nVertices << std::endl;
        std::cout << "[Model] number of faces: " << nFaces << std::endl;
        std::cout << "[Model] number of textures: " << textures.size() << std::endl;
    }

    void draw(const Shader &shader) const {
        for (std::size_t i = 0; i < meshes.size(); i++) {
            meshes[i].draw(shader, textures);
        }
    }

    void destroy() {
        for (auto &mesh : meshes) {
            mesh.destroy();
        }
        meshes.clear();

        for (auto &texture : textures) {
            texture.destroy();
        }
        textures.clear();
    }

private:
    std::vector<Mesh> meshes;
    std::vector<Texture> textures;

    void process_node(
        const aiNode *node, 
        const aiScene *scene, 
        const std::string &parent_path
    ) {
        for (std::size_t i = 0; i < node->mNumMeshes; ++i) {
            const aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
            meshes.push_back(process_mesh(mesh, scene, parent_path));
        }

        for (std::size_t i = 0; i < node->mNumChildren; i++) {
            process_node(node->mChildren[i], scene, parent_path);
        }
    }

    Mesh process_mesh(
        const aiMesh *mesh,
        const aiScene *scene,
        const std::string &parent_path
    ) {
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
        Material material;
        std::vector<unsigned int> indices_of_textures;

        // vertices
        for (std::size_t i = 0; i < mesh->mNumVertices; ++i) {
            Vertex vertex;
            vertex.position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);

            if (mesh->mNormals) {
                vertex.normal = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
            } else {
                vertex.normal = glm::vec3(0.0f, 0.0f, 0.0f);
            }

            if (mesh->mTextureCoords[0]) {
                vertex.tex_coords = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
            } else {
                vertex.tex_coords = glm::vec2(0.0f, 0.0f);
            }

            vertices.push_back(vertex);
        }

        // indices
        for (std::size_t i = 0; i < mesh->mNumFaces; ++i) {
            const aiFace& face = mesh->mFaces[i];
            for (std::size_t j = 0; j < face.mNumIndices; ++j) {
                indices.push_back(face.mIndices[j]);
            }
        }

        // materials
        if (scene->mMaterials[mesh->mMaterialIndex]) {
            aiMaterial* mat = scene->mMaterials[mesh->mMaterialIndex];

            // kd
            aiColor3D color;
            mat->Get(AI_MATKEY_COLOR_DIFFUSE, color);
            material.kd = glm::vec3(color.r, color.g, color.b);

            // ks
            mat->Get(AI_MATKEY_COLOR_SPECULAR, color);
            material.ks = glm::vec3(color.r, color.g, color.b);

            // ka
            mat->Get(AI_MATKEY_COLOR_AMBIENT, color);
            material.ka = glm::vec3(color.r, color.g, color.b);

            // shininess
            mat->Get(AI_MATKEY_SHININESS, material.shininess);

            // diffuse textures
            const std::filesystem::path ps_parent(parent_path);
            for (std::size_t i = 0; i < mat->GetTextureCount(aiTextureType_DIFFUSE); ++i) {
                aiString str;
                mat->GetTexture(aiTextureType_DIFFUSE, i, &str);
                const std::filesystem::path ps(str.C_Str());
                const std::string texture_path = (ps_parent / ps).string();

                const auto index = has_texture(texture_path);
                if (index) {
                    // add texture index
                    indices_of_textures.push_back(index.value());
                } else {
                    // add texture index
                    indices_of_textures.push_back(textures.size());

                    // load texture
                    textures.emplace_back(texture_path, TextureType::DIFFUSE);
                    // textures.push_back({texture_path, TextureType::DIFFUSE});
                }
            }

            // specular 
            for (std::size_t i = 0; i < mat->GetTextureCount(aiTextureType_SPECULAR); ++i) {
                aiString str;
                mat->GetTexture(aiTextureType_SPECULAR, i, &str);
                const std::filesystem::path ps(str.C_Str());
                const std::string texture_path = (ps_parent / ps).string();

                const auto index = has_texture(texture_path);
                if (index) {
                    // add texture index
                    indices_of_textures.push_back(index.value());
                } else {
                    // add texture index
                    indices_of_textures.push_back(textures.size());

                    // load texture
                    textures.emplace_back(texture_path, TextureType::SPECULAR);
                    // textures.push_back({texture_path, TextureType::SPECULAR});
                }
            }
        }

        return Mesh(vertices, indices, material, indices_of_textures);
    }

    std::optional<std::size_t> has_texture(const std::string &filepath) const {
        for (std::size_t i = 0; i < textures.size(); ++i) {
            const Texture &texture = textures[i];
            if (texture.filepath == filepath) {
                return i;
            }
        }
        return std::nullopt;
    }
};