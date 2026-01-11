/**
 * @file animated_mesh.cpp
 * @brief Implementation of the AnimatedMesh class for skeletal animation
 */

#include "animated_mesh.hpp"
#include <string>

/**
 * @brief Generate vertex array from raw float data
 *
 * Converts a flat array of floats into structured AnimatedVertex objects.
 * Initializes bone IDs to -1 and weights to 0 for all bone influences.
 *
 * @param vertices Pointer to raw vertex data (position, normal, texCoord)
 * @param nVertices Number of vertices to process
 * @return Vector of properly initialized AnimatedVertex structures
 */
std::vector<AnimatedVertex> AnimatedVertex::GenVertexArray(float *vertices,
                                                           int nVertices) {
  std::vector<AnimatedVertex> ret(nVertices);
  int stride = sizeof(AnimatedVertex) / sizeof(float);

  for (int i = 0; i < nVertices; i++) {
    // Extract position (x, y, z)
    ret[i].pos = glm::vec3(vertices[i * stride + 0], vertices[i * stride + 1],
                           vertices[i * stride + 2]);
    // Extract normal (nx, ny, nz)
    ret[i].normal =
        glm::vec3(vertices[i * stride + 3], vertices[i * stride + 4],
                  vertices[i * stride + 5]);
    // Extract texture coordinates (u, v)
    ret[i].texCoord =
        glm::vec2(vertices[i * stride + 6], vertices[i * stride + 7]);

    // Initialize bone influence data to defaults
    for (int j = 0; j < Config::Animation::MAX_BONE_INFLUENCE; j++) {
      ret[i].boneIDs[j] = -1;
      ret[i].weights[j] = 0.0f;
    }
  }

  return ret;
}

/**
 * @brief Default constructor
 * Initializes all OpenGL object IDs to 0
 */
AnimatedMesh::AnimatedMesh() : VAO(0), VBO(0), EBO(0) {}

/**
 * @brief Construct textured mesh
 * @param vertices Vertex data with bone weights
 * @param indices Triangle indices
 * @param textures Texture list for the mesh
 */
AnimatedMesh::AnimatedMesh(std::vector<AnimatedVertex> vertices,
                           std::vector<unsigned int> indices,
                           std::vector<Texture> textures) {
  this->vertices = std::move(vertices);
  this->indices = std::move(indices);
  this->textures = std::move(textures);
  this->noTex = false;
  Setup();
}

/**
 * @brief Construct untextured mesh with material colors
 * @param vertices Vertex data with bone weights
 * @param indices Triangle indices
 * @param diffuse Diffuse material color
 * @param specular Specular material color
 */
AnimatedMesh::AnimatedMesh(std::vector<AnimatedVertex> vertices,
                           std::vector<unsigned int> indices, aiColor4D diffuse,
                           aiColor4D specular) {
  this->vertices = vertices;
  this->indices = indices;
  this->diffuse = diffuse;
  this->specular = specular;
  this->noTex = true;
  Setup();
}

/**
 * @brief Render the animated mesh
 *
 * Sets up shader uniforms for materials/textures and performs the draw call.
 * For untextured meshes, uses material colors. For textured meshes, binds
 * and configures all textures (diffuse, specular, etc.).
 *
 * @param shader Shader program to use for rendering
 */
void AnimatedMesh::Render(Shader &shader) {
  if (this->noTex) {
    // Use material colors for untextured mesh
    shader.Set4Float("material.diffuse", this->diffuse);
    shader.Set4Float("material.specular", this->specular);
    shader.SetInt("noTex", 1);
  } else {
    // Use textures
    shader.SetInt("noTex", 0);
    unsigned int diffuseTextureIndex = 0;
    unsigned int specularTextureIndex = 0;

    // Bind all textures and set shader uniforms
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

  // Draw the mesh
  glBindVertexArray(this->VAO);
  glDrawElements(GL_TRIANGLES, this->indices.size(), GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);
  glActiveTexture(GL_TEXTURE0);
}

/**
 * @brief Clean up OpenGL resources
 * Deletes VAO, VBO, and EBO if they exist
 */
void AnimatedMesh::Cleanup() {
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
 * @brief Destructor
 */
AnimatedMesh::~AnimatedMesh() { Cleanup(); }


/**
 * @brief Move constructor
 * Transfers ownership of all resources, including noTex, diffuse, and specular.
 * @param other Source mesh to move from
 */
AnimatedMesh::AnimatedMesh(AnimatedMesh&& other) noexcept {
    *this = std::move(other);
}
/**
 * @brief Move assignment operator
 * Cleans up current resources and transfers ownership from other mesh
 * @param other Source mesh to move from
 * @return Reference to this mesh
 */
AnimatedMesh& AnimatedMesh::operator=(AnimatedMesh&& other) noexcept {
    if (this != &other) {
        // Clean up existing resources
        Cleanup();

        // Transfer OpenGL objects
        this->VAO = other.VAO;
        this->VBO = other.VBO;
        this->EBO = other.EBO;

        // Invalidate other's OpenGL objects
        other.VAO = 0;
        other.VBO = 0;
        other.EBO = 0;

        // Move data
        this->vertices = std::move(other.vertices);
        this->indices = std::move(other.indices);
        this->textures = std::move(other.textures);

        // Preserve state info
        this->noTex = other.noTex;
        this->diffuse = other.diffuse;
        this->specular = other.specular;
    }
    return *this;
}
/**
 * @brief Set up OpenGL vertex array and buffers
 *
 * Creates and configures VAO, VBO, and EBO for the mesh.
 * Sets up vertex attributes for:
 * - Position (location 0)
 * - Normal (location 1)
 * - Texture coordinates (location 2)
 * - Bone IDs (location 3)
 * - Bone weights (location 4)
 */
void AnimatedMesh::Setup() {
  // Generate OpenGL objects
  glGenVertexArrays(1, &this->VAO);
  glGenBuffers(1, &this->VBO);
  glGenBuffers(1, &this->EBO);

  glBindVertexArray(this->VAO);

  // Upload vertex data
  glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
  glBufferData(GL_ARRAY_BUFFER, this->vertices.size() * sizeof(AnimatedVertex),
               &this->vertices[0], GL_STATIC_DRAW);

  // Upload index data
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
               this->indices.size() * sizeof(unsigned int), &this->indices[0],
               GL_STATIC_DRAW);

  // Position attribute (location 0)
  glVertexAttribPointer(Config::Animation::ATTRIB_POSITION, 3, GL_FLOAT,
                        GL_FALSE, sizeof(AnimatedVertex),
                        (void *)(offsetof(AnimatedVertex, pos)));
  glEnableVertexAttribArray(Config::Animation::ATTRIB_POSITION);

  // Normal attribute (location 1)
  glVertexAttribPointer(Config::Animation::ATTRIB_NORMAL, 3, GL_FLOAT, GL_FALSE,
                        sizeof(AnimatedVertex),
                        (void *)(offsetof(AnimatedVertex, normal)));
  glEnableVertexAttribArray(Config::Animation::ATTRIB_NORMAL);

  // Texture coordinate attribute (location 2)
  glVertexAttribPointer(Config::Animation::ATTRIB_TEXCOORD, 2, GL_FLOAT,
                        GL_FALSE, sizeof(AnimatedVertex),
                        (void *)(offsetof(AnimatedVertex, texCoord)));
  glEnableVertexAttribArray(Config::Animation::ATTRIB_TEXCOORD);

  // Bone IDs attribute (location 3) - integer attribute
  glVertexAttribIPointer(Config::Animation::ATTRIB_BONE_IDS, 4, GL_INT,
                         sizeof(AnimatedVertex),
                         (void *)(offsetof(AnimatedVertex, boneIDs)));
  glEnableVertexAttribArray(Config::Animation::ATTRIB_BONE_IDS);

  // Bone weights attribute (location 4)
  glVertexAttribPointer(Config::Animation::ATTRIB_WEIGHTS, 4, GL_FLOAT,
                        GL_FALSE, sizeof(AnimatedVertex),
                        (void *)(offsetof(AnimatedVertex, weights)));
  glEnableVertexAttribArray(Config::Animation::ATTRIB_WEIGHTS);

  glBindVertexArray(0);
}
