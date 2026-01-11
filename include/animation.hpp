/**
 * @file animation.hpp
 * @brief Defines the Animation class for skeletal animation sequences
 */

#ifndef ANIMATION_HPP
#define ANIMATION_HPP

#include "animated_model.hpp"
#include "bone.hpp"
#include <assimp/scene.h>
#include <glm/glm.hpp>
#include <map>
#include <string>
#include <vector>

/**
 * @struct AssimpNodeData
 * @brief Represents a node in the animation hierarchy
 *
 * Stores transformation data and relationships for skeletal animation nodes
 */
struct AssimpNodeData {
  glm::mat4 transformation;             ///< Local transformation matrix
  std::string name;                     ///< Node identifier
  int childrenCount;                    ///< Number of child nodes
  std::vector<AssimpNodeData> children; ///< Child nodes in hierarchy
};

/**
 * @class Animation
 * @brief Manages a single animation sequence for a skeletal model
 *
 * Loads and stores animation data including bone transformations,
 * timing information, and hierarchy structure. Provides access to
 * bone keyframes for animation playback.
 */
class Animation {
public:
  /**
   * @brief Constructor - loads animation from file
   * @param animationPath Path to the model file containing animations
   * @param model Target animated model (for bone mapping)
   * @param animationIndex Index of the animation in the file (0-based)
   */
  Animation(const std::string &animationPath, class AnimatedModel *model,
            int animationIndex);

  /**
   * @brief Find a bone by name
   * @param name Name of the bone to find
   * @return Pointer to the bone, or nullptr if not found
   */
  Bone *FindBone(const std::string &name);

  /**
   * @brief Get animation playback speed
   * @return Ticks per second for this animation
   */
  inline float GetTicksPerSecond() const { return ticksPerSecond; }

  /**
   * @brief Get animation duration
   * @return Total duration in ticks
   */
  inline float GetDuration() const { return duration; }

  /**
   * @brief Get root node of animation hierarchy
   * @return Reference to root node data
   */
  inline const AssimpNodeData &GetRootNode() const { return rootNode; }

  /**
   * @brief Get bone ID mapping
   * @return Map of bone names to BoneInfo structures
   */
  inline const std::map<std::string, BoneInfo> &GetBoneIDMap() const {
    return boneInfoMap;
  }

  /**
   * @brief Get total number of bones
   * @return Number of bones in this animation
   */
  int GetBoneCount() const { return boneCount; }

  /**
   * @brief Get animation name
   * @return Name of this animation sequence
   */
  const std::string &GetName() const { return name; }

private:
  /**
   * @brief Process bones not present in the model
   * @param animation Assimp animation data
   * @param model Target model to update with new bones
   */
  void ReadMissingBones(const aiAnimation *animation,
                        class AnimatedModel *model);

  /**
   * @brief Build hierarchy tree from Assimp data
   * @param dest Destination node to populate
   * @param src Source Assimp node
   */
  void ReadHierarchyData(AssimpNodeData &dest, const aiNode *src);

  float duration;     ///< Animation length in ticks
  int ticksPerSecond; ///< Playback speed in ticks/second
  int boneCount;      ///< Total number of bones

  std::string name; ///< Animation sequence name

  std::vector<Bone> bones;                     ///< Collection of animated bones
  AssimpNodeData rootNode;                     ///< Root of the hierarchy tree
  std::map<std::string, BoneInfo> boneInfoMap; ///< Bone name to info mapping
};

#endif
