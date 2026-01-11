/**
 * @file animated_mesh.hpp
 * @brief Defines the AnimatedMesh class for rendering skeletal animated 3D
 * meshes
 */

#ifndef ANIMATED_MESH_H
#define ANIMATED_MESH_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include <assimp/color4.h>
#include "shader.hpp"
#include "texture.hpp"
#include "config.hpp"

/**
 * @struct AnimatedVertex
 * @brief Vertex data structure for animated meshes with bone weights
 *
 * Contains all vertex attributes needed for skeletal animation including
 * position, normal, texture coordinates, and bone influence data.
 */
struct AnimatedVertex {
  glm::vec3 pos;       ///< Vertex position in model space
  glm::vec3 normal;    ///< Vertex normal for lighting calculations
  glm::vec2 texCoord;  ///< Texture coordinates for texture mapping
  glm::vec3 tangent;   ///< Tangent vector for normal mapping
  glm::vec3 bitangent; ///< Bitangent vector for normal mapping

  /// Bone IDs that influence this vertex (max 4 bones)
  int boneIDs[Config::Animation::MAX_BONE_INFLUENCE];

  /// Weight of each bone's influence on this vertex
  float weights[Config::Animation::MAX_BONE_INFLUENCE];

  /**
   * @brief Generate an array of AnimatedVertex from raw float data
   * @param vertices Pointer to raw vertex data
   * @param nVertices Number of vertices to process
   * @return Vector of AnimatedVertex structures
   */
  static std::vector<AnimatedVertex> GenVertexArray(float *vertices,
                                                    int nVertices);
};

/**
 * @class AnimatedMesh
 * @brief Represents a 3D mesh with skeletal animation support
 *
 * This class manages vertex data, indices, textures, and OpenGL buffers
 * for rendering animated 3D meshes. It supports both textured and
 * material-based rendering.
 */
class AnimatedMesh {
public:
  std::vector<AnimatedVertex> vertices; ///< Vertex data for the mesh
  std::vector<unsigned int> indices;    ///< Index buffer for triangle assembly
  std::vector<Texture> textures;        ///< Textures applied to this mesh
  unsigned int VAO;                     ///< OpenGL Vertex Array Object ID
  aiColor4D diffuse;                    ///< Diffuse material color
  aiColor4D specular;                   ///< Specular material color

  /**
   * @brief Default constructor
   */
  AnimatedMesh();

  /**
   * @brief Construct mesh with vertices, indices and optional textures
   * @param vertices Vertex data for the mesh
   * @param indices Index data for triangle assembly
   * @param textures Optional texture list (empty for untextured)
   */
  AnimatedMesh(std::vector<AnimatedVertex> vertices,
               std::vector<unsigned int> indices,
               std::vector<Texture> textures = {});

  /**
   * @brief Construct untextured mesh with material colors
   * @param vertices Vertex data for the mesh
   * @param indices Index data for triangle assembly
   * @param diffuse Diffuse material color
   * @param specular Specular material color
   */
  AnimatedMesh(std::vector<AnimatedVertex> vertices,
               std::vector<unsigned int> indices, aiColor4D diffuse,
               aiColor4D specular);

  /**
   * @brief Render the mesh using the provided shader
   * @param shader Shader program to use for rendering
   */
  void Render(Shader &shader);

  /**
   * @brief Clean up OpenGL resources
   */
  void Cleanup();

  // Rule of Five
  /**
   * @brief Destructor - cleans up OpenGL resources
   */
  ~AnimatedMesh();

  /**
   * @brief Copy constructor (deleted)
   */
  AnimatedMesh(const AnimatedMesh &other) = delete;

  /**
   * @brief Copy assignment operator (deleted)
   */
  AnimatedMesh &operator=(const AnimatedMesh &other) = delete;

  /**
   * @brief Move constructor
   * @param other Mesh to move from
   */
  AnimatedMesh(AnimatedMesh &&other) noexcept;

  /**
   * @brief Move assignment operator
   * @param other Mesh to move from
   * @return Reference to this mesh
   */
  AnimatedMesh &operator=(AnimatedMesh &&other) noexcept;

private:
  unsigned int VBO; ///< OpenGL Vertex Buffer Object ID
  unsigned int EBO; ///< OpenGL Element Buffer Object ID
  bool noTex;       ///< Flag indicating if mesh uses textures or materials

  /**
   * @brief Initialize OpenGL buffers and vertex attributes
   */
  void Setup();
};

#endif // !ANIMATED_MESH_H
