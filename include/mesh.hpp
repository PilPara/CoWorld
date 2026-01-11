/**
 * @file mesh.hpp
 * @brief Declaration of the Mesh class and Vertex structure.
 *
 * Handles static mesh geometry, OpenGL buffers (VAO/VBO/EBO),
 * materials, and rendering with or without textures.
 */

#ifndef MESH_H
#define MESH_H

#include <glm/glm.hpp>
#include <vector>
#include <assimp/scene.h>
#include "texture.hpp"
#include "shader.hpp"

/**
 * @brief Vertex structure used in static meshes.
 * Stores position, normal, and texture coordinates.
 */
struct Vertex {
  glm::vec3 pos;      ///< Vertex position in model space
  glm::vec3 normal;   ///< Normal vector for lighting
  glm::vec2 texCoord; ///< Texture coordinates (UV)

  /**
   * @brief Generate a vector of Vertex objects from raw float data.
   * @param vertices Pointer to array of floats (interleaved attributes).
   * @param nVertices Number of vertices in the array.
   * @return Vector of Vertex objects.
   */
  static std::vector<Vertex> GenVertexArray(float *vertices, int nVertices);
};

/**
 * @brief Represents a static mesh with geometry and material data.
 * Manages GPU buffers and issues draw calls.
 */
class Mesh {
public:
  // Constructors / destructor
  Mesh();
  Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices,
       std::vector<Texture> textures);
  Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices,
       aiColor4D diffuse, aiColor4D specular);
  ~Mesh();

  // Move semantics
  Mesh(Mesh &&other) noexcept;
  Mesh &operator=(Mesh &&other) noexcept;

  /**
   * @brief Render the mesh using a shader.
   * @param shader Shader to bind uniforms and textures to.
   */
  void Render(Shader &shader);

  /**
   * @brief Release GPU resources (VAO, VBO, EBO).
   */
  void Cleanup();

  // Mesh data
  std::vector<Vertex> vertices;      ///< Vertex buffer
  std::vector<unsigned int> indices; ///< Index buffer
  std::vector<Texture> textures;     ///< Associated textures
  aiColor4D diffuse;                 ///< Diffuse color (if no texture)
  aiColor4D specular;                ///< Specular color (if no texture)
  bool noTex;                        ///< Flag for texture usage

private:
  unsigned int VAO; ///< Vertex Array Object
  unsigned int VBO; ///< Vertex Buffer Object
  unsigned int EBO; ///< Element Buffer Object

  /**
   * @brief Initialize OpenGL buffers and attribute pointers.
   */
  void Setup();
};

#endif
