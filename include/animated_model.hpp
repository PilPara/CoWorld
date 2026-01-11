/**
 * @file animated_model.hpp
 * @brief Defines the AnimatedModel class for skeletal animated 3D models
 */

#ifndef ANIMATED_MODEL_H
#define ANIMATED_MODEL_H

#include "model.hpp"
#include "animated_mesh.hpp"
#include <map>

/**
 * @struct BoneInfo
 * @brief Contains bone transformation data for skeletal animation
 */
struct BoneInfo {
  int id;           ///< Unique identifier for this bone
  glm::mat4 offset; ///< Offset matrix from mesh space to bone space
};

/**
 * @class AnimatedModel
 * @brief 3D model with skeletal animation support
 *
 * Extends the base Model class to support bone-based animations.
 * Manages bone hierarchy, weights, and transformations for skeletal animation.
 */
class AnimatedModel : public Model {
public:
  int boneCounter = 0; ///< Total number of bones in the model
  std::map<std::string, BoneInfo>
      boneInfoMap; ///< Maps bone names to their info

  /**
   * @brief Constructor for animated model
   * @param pos Initial position in world space
   * @param size Scale factor for the model
   * @param noTex If true, use materials instead of textures
   */
  AnimatedModel(glm::vec3 pos = glm::vec3(0.0f),
                glm::vec3 size = glm::vec3(0.01f), bool noTex = false);

  /**
   * @brief Destructor
   */
  ~AnimatedModel() override;

  /**
   * @brief Load model from file
   * @param path Path to the model file (e.g., .gltf, .fbx)
   */
  void LoadModel(std::string path);

  /**
   * @brief Render the animated model
   * @param shader Shader program with bone matrix uniforms
   */
  void Render(Shader &shader);

  /**
   * @brief Get reference to bone info map
   * @return Reference to the bone name to info mapping
   */
  auto &GetBoneInfoMap() { return this->boneInfoMap; }

  /**
   * @brief Get reference to bone count
   * @return Reference to the bone counter
   */
  int &GetBoneCount() { return this->boneCounter; }

  /**
   * @brief Find bone ID by name
   * @param boneName Name of the bone to find
   * @return Bone ID if found, -1 otherwise
   */
  int FindBoneID(const std::string &boneName);

  /**
   * @brief Calculate bounding box for collision detection
   * Override to handle animated vertex positions
   */
  void CalculateBoundingBox() override;

private:
  std::vector<AnimatedMesh> animatedMeshes; ///< Collection of animated meshes

  /**
   * @brief Process node hierarchy recursively
   * @param node Current node in the scene graph
   * @param scene Assimp scene containing the model data
   */
  void ProcessNode(aiNode *node, const aiScene *scene);

  /**
   * @brief Process individual mesh with animation data
   * @param mesh Assimp mesh to process
   * @param scene Assimp scene for material access
   * @return Processed AnimatedMesh object
   */
  AnimatedMesh ProcessMesh(aiMesh *mesh, const aiScene *scene);

  /**
   * @brief Initialize vertex bone data to defaults
   * @param vertex Vertex to initialize
   */
  void SetVertexBoneDataToDefault(AnimatedVertex &vertex);

  /**
   * @brief Set bone influence data for a vertex
   * @param vertex Vertex to modify
   * @param boneID ID of the influencing bone
   * @param weight Weight of the bone's influence
   */
  void SetVertexBoneData(AnimatedVertex &vertex, int boneID, float weight);

  /**
   * @brief Extract bone weights from mesh data
   * @param vertices Vertex array to populate with bone data
   * @param mesh Source mesh containing bone information
   * @param scene Assimp scene for additional data
   */
  void ExtractBoneWeightForVertices(std::vector<AnimatedVertex> &vertices,
                                    aiMesh *mesh, const aiScene *scene);
};

#endif // !ANIMATED_MODEL_H
