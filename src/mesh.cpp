/**
 * @file mesh.cpp
 * @brief Implementation of the Mesh class and Vertex utilities.
 */

#include "mesh.hpp"
#include "shader.hpp"
#include "texture.hpp"
#include <string>
#include "config.hpp"

// -------------------- Vertex --------------------

/**
 * @brief Generate a vector of Vertex objects from raw float data.
 *
 * Converts interleaved float attributes into structured Vertex data.
 * Expected layout: position (3), normal (3), texCoord (2).
 *
 * @param vertices Pointer to raw float array.
 * @param nVertices Number of vertices in the array.
 * @return Vector of Vertex objects.
 */
std::vector<Vertex> Vertex::GenVertexArray(float *vertices, int nVertices) {
  std::vector<Vertex> ret(nVertices);
  int stride = Config::StaticMesh::VERTEX_FLOATS;

  for (int i = 0; i < nVertices; i++) {
    ret[i].pos = glm::vec3(vertices[i * stride + 0], vertices[i * stride + 1],
                           vertices[i * stride + 2]);
    ret[i].normal =
        glm::vec3(vertices[i * stride + 3], vertices[i * stride + 4],
                  vertices[i * stride + 5]);
    ret[i].texCoord =
        glm::vec2(vertices[i * stride + 6], vertices[i * stride + 7]);
  }
  return ret;
}

// -------------------- Mesh --------------------

Mesh::Mesh() : VAO(0), VBO(0), EBO(0), noTex(false) {}

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices,
           std::vector<Texture> textures)
    : VAO(0), VBO(0), EBO(0), noTex(false) {
  this->vertices = std::move(vertices);
  this->indices = std::move(indices);
  this->textures = std::move(textures);
  Setup();
}

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices,
           aiColor4D diffuse, aiColor4D specular)
    : VAO(0), VBO(0), EBO(0), noTex(true) {
  this->vertices = std::move(vertices);
  this->indices = std::move(indices);
  this->diffuse = diffuse;
  this->specular = specular;
  Setup();
}

Mesh::~Mesh() { Cleanup(); }

Mesh::Mesh(Mesh &&other) noexcept
    : vertices(std::move(other.vertices)), indices(std::move(other.indices)),
      textures(std::move(other.textures)), VAO(other.VAO), VBO(other.VBO),
      EBO(other.EBO), diffuse(other.diffuse), specular(other.specular),
      noTex(other.noTex) {
  // Reset other's GL handles to prevent double delete
  other.VAO = 0;
  other.VBO = 0;
  other.EBO = 0;
}

Mesh &Mesh::operator=(Mesh &&other) noexcept {
  if (this != &other) {
    Cleanup();

    this->vertices = std::move(other.vertices);
    this->indices = std::move(other.indices);
    this->textures = std::move(other.textures);
    this->VAO = other.VAO;
    this->VBO = other.VBO;
    this->EBO = other.EBO;
    this->diffuse = other.diffuse;
    this->specular = other.specular;
    this->noTex = other.noTex;

    other.VAO = 0;
    other.VBO = 0;
    other.EBO = 0;
  }
  return *this;
}

/**
 * @brief Render the mesh with given shader.
 *
 * If no textures are used, applies diffuse/specular colors instead.
 * Otherwise binds textures and updates shader uniforms.
 *
 * @param shader Shader program reference.
 */
void Mesh::Render(Shader &shader) {
  if (this->noTex) {
    shader.Set4Float("material.diffuse", this->diffuse);
    shader.Set4Float("material.specular", this->specular);
    shader.SetInt("noTex", 1);
  } else {
    shader.SetInt("noTex", 0);
    unsigned int diffuseTextureIndex = 0;
    unsigned int specularTextureIndex = 0;

    for (unsigned int i = 0; i < this->textures.size(); i++) {
      glActiveTexture(GL_TEXTURE0 + i);

      std::string name;
      switch (this->textures[i].type) {
      case aiTextureType_DIFFUSE:
        name = "diffuse" + std::to_string(diffuseTextureIndex++);
        break;
      case aiTextureType_SPECULAR:
        name = "specular" + std::to_string(specularTextureIndex++);
        break;
      default:
        name = "default";
        break;
      }

      shader.SetInt(name, i);
      this->textures[i].Bind();
    }
  }

  glBindVertexArray(this->VAO);
  glDrawElements(GL_TRIANGLES, this->indices.size(), GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);
  glActiveTexture(GL_TEXTURE0);
}

/**
 * @brief Release OpenGL buffers for this mesh.
 */
void Mesh::Cleanup() {
  if (this->VAO != 0) {
    glDeleteVertexArrays(1, &this->VAO);
    this->VAO = 0;
  }
  if (this->VBO != 0) {
    glDeleteBuffers(1, &this->VBO);
    this->VBO = 0;
  }
  if (this->EBO != 0) {
    glDeleteBuffers(1, &this->EBO);
    this->EBO = 0;
  }
}

/**
 * @brief Initialize VAO, VBO, and EBO and configure vertex attributes.
 */
void Mesh::Setup() {
  glGenVertexArrays(1, &this->VAO);
  glGenBuffers(1, &this->VBO);
  glGenBuffers(1, &this->EBO);

  glBindVertexArray(this->VAO);

  glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
  glBufferData(GL_ARRAY_BUFFER, this->vertices.size() * sizeof(Vertex),
               &this->vertices[0], GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
               this->indices.size() * sizeof(unsigned int), &this->indices[0],
               GL_STATIC_DRAW);

  // Position attribute
  glVertexAttribPointer(Config::StaticMesh::ATTRIB_POSITION, 3, GL_FLOAT,
                        GL_FALSE, sizeof(Vertex),
                        (void *)(offsetof(Vertex, pos)));
  glEnableVertexAttribArray(Config::StaticMesh::ATTRIB_POSITION);

  // Normal attribute
  glVertexAttribPointer(Config::StaticMesh::ATTRIB_NORMAL, 3, GL_FLOAT,
                        GL_FALSE, sizeof(Vertex),
                        (void *)(offsetof(Vertex, normal)));
  glEnableVertexAttribArray(Config::StaticMesh::ATTRIB_NORMAL);

  // TexCoord attribute
  glVertexAttribPointer(Config::StaticMesh::ATTRIB_TEXCOORD, 2, GL_FLOAT,
                        GL_FALSE, sizeof(Vertex),
                        (void *)(offsetof(Vertex, texCoord)));
  glEnableVertexAttribArray(Config::StaticMesh::ATTRIB_TEXCOORD);

  glBindVertexArray(0);
}
